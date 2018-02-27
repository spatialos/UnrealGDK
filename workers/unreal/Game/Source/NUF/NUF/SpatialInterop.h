// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialTypeBinding.h"
#include "SpatialUnrealObjectRef.h"
#include "SpatialInterop.generated.h"

class USpatialOS;
class USpatialActorChannel;
class USpatialPackageMapClient;
class USpatialNetDriver;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialOSInterop, Log, All);

// A helper class for creating an incoming payload consumed by ReceiveSpatialUpdate.
class FBunchPayloadWriter
{
public:
	FBunchPayloadWriter(UPackageMap* PackageMap) : Writer(PackageMap, 0)
	{
		// First bit is the "enable checksum" bit, which we set to 0.
		Writer.WriteBit(0);
	}

	template <typename T>
	void SerializeProperty(uint32 Handle, UProperty* Property, T* Value)
	{
		Writer.SerializeIntPacked(Handle);
		Property->NetSerializeItem(Writer, Writer.PackageMap, Value);
	}

	FNetBitWriter& GetNetBitWriter()
	{
		return Writer;
	}

private:
	FNetBitWriter Writer;
};

// An general version of worker::RequestId.
using FUntypedRequestId = decltype(worker::RequestId<void>::Id);

// Stores the result of an attempt to call an RPC sender function. Either we have an unresolved object which needs
// to be resolved before we can send this RPC, or we successfully sent a command request.
struct FRPCCommandRequestResult
{
	UObject* UnresolvedObject;
	FUntypedRequestId RequestId;

	FRPCCommandRequestResult() = delete;
	FRPCCommandRequestResult(UObject* UnresolvedObject) : UnresolvedObject{UnresolvedObject}, RequestId{0} {}
	FRPCCommandRequestResult(FUntypedRequestId RequestId) : UnresolvedObject{nullptr}, RequestId{RequestId} {}
};

// Function storing a command request operation, capturing all arguments by value.
using FRPCCommandRequestFunc = TFunction<FRPCCommandRequestResult()>;

// Stores the result of attempting to receive an RPC command. We either return an unresolved object which needs
// to be resolved before the RPC implementation can be called successfully, or nothing, which indicates that
// the RPC implementation was called successfully.
using FRPCCommandResponseResult = TOptional<improbable::unreal::UnrealObjectRef>;

// Function storing a command response operation, capturing the command request op by value.
using FRPCCommandResponseFunc = TFunction<FRPCCommandResponseResult()>;

// Stores the number of attempts when retrying failed commands.
class FOutgoingReliableRPC
{
public:
	FOutgoingReliableRPC(FRPCCommandRequestFunc SendCommandRequest) :
		SendCommandRequest{SendCommandRequest},
		NumAttempts{1}
	{
	}

	FRPCCommandRequestFunc SendCommandRequest;
	uint32 NumAttempts;
};

struct FPendingIncomingObjectProperty
{
	UObjectPropertyBase* ObjectProperty;
	uint16 Handle;
};

UCLASS()
class NUF_API USpatialInterop : public UObject
{
	GENERATED_BODY()
public:
	USpatialInterop();

	void Init(USpatialOS* Instance, USpatialNetDriver* Driver, FTimerManager* TimerManager);

	// Type bindings.
	USpatialTypeBinding* GetTypeBindingByClass(UClass* Class) const;

	// Sending component updates and RPCs.
	worker::RequestId<worker::CreateEntityRequest> SendCreateEntityRequest(USpatialActorChannel* Channel, const FVector& Location, const FString& PlayerWorkerId, const TArray<uint16>& Changed);
	void SendSpatialPositionUpdate(const worker::EntityId& EntityId, const FVector& Location);
	void SendSpatialUpdate(USpatialActorChannel* Channel, const TArray<uint16>& Changed);
	void ReceiveSpatialUpdate(USpatialActorChannel* Channel, FNetBitWriter& IncomingPayload);
	void InvokeRPC(AActor* TargetActor, const UFunction* const Function, FFrame* const Frame);

	// Called by USpatialPackageMapClient when a UObject is "resolved" i.e. has a unreal object ref.
	// This will dequeue pending object ref updates and RPCs which depend on this UObject existing in the package map.
	void ResolvePendingOperations(UObject* Object, const improbable::unreal::UnrealObjectRef& ObjectRef);

	// Called by USpatialInteropPipelineBlock when an actor channel is opened on the client.
	void AddActorChannel(const worker::EntityId& EntityId, USpatialActorChannel* Channel);

	// Used by generated type bindings to map an entity ID to its actor channel.
	USpatialActorChannel* GetActorChannelByEntityId(const worker::EntityId& EntityId) const;

	// RPC handlers. Used by generated type bindings.
	void SendCommandRequest_Internal(FRPCCommandRequestFunc Function, bool bReliable);
	void SendCommandResponse_Internal(FRPCCommandResponseFunc Function);
	void HandleCommandResponse_Internal(const FString& RPCName, FUntypedRequestId RequestId, const worker::EntityId& EntityId, const worker::StatusCode& StatusCode, const FString& Message);

	// Used to queue incoming/outgoing object updates/RPCs. Used by generated type bindings.
	void QueueOutgoingObjectUpdate_Internal(UObject* UnresolvedObject, USpatialActorChannel* DependentChannel, uint16 Handle);
	void QueueOutgoingRPC_Internal(UObject* UnresolvedObject, FRPCCommandRequestFunc CommandSender, bool bReliable);
	void QueueIncomingObjectUpdate_Internal(const improbable::unreal::UnrealObjectRef& UnresolvedObjectRef, USpatialActorChannel* DependentChannel, UObjectPropertyBase* Property, uint16 Handle);
	void QueueIncomingRPC_Internal(const improbable::unreal::UnrealObjectRef& UnresolvedObjectRef, FRPCCommandResponseFunc Responder);

	// Accessors.
	USpatialOS* GetSpatialOS() const
	{
		return SpatialOSInstance;
	}

	USpatialNetDriver* GetNetDriver() const
	{
		return NetDriver;
	}

private:
	UPROPERTY()
	USpatialOS* SpatialOSInstance;

	UPROPERTY()
	USpatialNetDriver* NetDriver;

	UPROPERTY()
	USpatialPackageMapClient* PackageMap;

	// Timer manager.
	FTimerManager* TimerManager;

	// Type interop bindings.
	UPROPERTY()
	TMap<UClass*, USpatialTypeBinding*> TypeBinding;

	// A map from Entity ID to actor channel.
	TMap<worker::EntityId, USpatialActorChannel*> EntityToActorChannel;

	// Outgoing RPCs (for retry logic).
	TMap<FUntypedRequestId, TSharedPtr<FOutgoingReliableRPC>> OutgoingReliableRPCs;

	// Pending outgoing object ref property updates.
	TMap<UObject*, TArray<USpatialActorChannel*>> ChannelsAwaitingOutgoingObjectResolve;
	TMap<USpatialActorChannel*, TArray<uint16>> PendingOutgoingObjectRefHandles;

	// Pending outgoing RPCs.
	TMap<UObject*, TArray<TPair<FRPCCommandRequestFunc, bool>>> PendingOutgoingRPCs;

	// Pending incoming object ref property updates.
	TMap<FHashableUnrealObjectRef, TMap<USpatialActorChannel*, TArray<FPendingIncomingObjectProperty>>> PendingIncomingObjectRefProperties;
	
	// Pending incoming RPCs.
	TMap<FHashableUnrealObjectRef, TArray<FRPCCommandResponseFunc>> PendingIncomingRPCs;

private:
	void RegisterInteropType(UClass* Class, USpatialTypeBinding* Binding);
	void UnregisterInteropType(UClass* Class);

	void SetComponentInterests_Client(USpatialActorChannel* ActorChannel, const worker::EntityId& EntityId);

	void ResolvePendingOutgoingObjectUpdates(UObject* Object);
	void ResolvePendingOutgoingRPCs(UObject* Object);
	void ResolvePendingIncomingObjectUpdates(UObject* Object, const improbable::unreal::UnrealObjectRef& ObjectRef);
	void ResolvePendingIncomingRPCs(const improbable::unreal::UnrealObjectRef& ObjectRef);
};
