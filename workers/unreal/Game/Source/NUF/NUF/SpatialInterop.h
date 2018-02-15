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

// Stores the result of an attempt to call an RPC sender function. Either we have an unresolved object, or we
// sent a command request.
struct FRPCRequestResult
{
	UObject* UnresolvedObject;
	FUntypedRequestId RequestId;

	FRPCRequestResult() = delete;
	FRPCRequestResult(UObject* UnresolvedObject) : UnresolvedObject{UnresolvedObject}, RequestId{0} {}
	FRPCRequestResult(FUntypedRequestId RequestId) : UnresolvedObject{nullptr}, RequestId{RequestId} {}
};

// Function storing a command request operation, capturing all arguments by value.
using FRPCRequestFunction = TFunction<FRPCRequestResult()>;

// Function storing a command response operation, capturing the command request op by value.
using FRPCResponderFunction = TFunction<TOptional<improbable::unreal::UnrealObjectRef>()>;

// Stores the number of attempts when retrying failed commands.
class FOutgoingReliableRPC
{
public:
	FOutgoingReliableRPC(FRPCRequestFunction SendCommandRequest) :
		SendCommandRequest{SendCommandRequest},
		NumAttempts{1}
	{
	}

	FRPCRequestFunction SendCommandRequest;
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

	void Init(bool bClient, USpatialOS* Instance, USpatialNetDriver* Driver, FTimerManager* TimerManager);

	// Type bindings.
	void RegisterInteropType(UClass* Class, USpatialTypeBinding* Binding);
	void UnregisterInteropType(UClass* Class);
	USpatialTypeBinding* GetTypeBindingByClass(UClass* Class) const;

	// Sending component updates and RPCs.
	worker::RequestId<worker::CreateEntityRequest> SendCreateEntityRequest(USpatialActorChannel* Channel, const FString& PlayerWorkerId, const TArray<uint16>& Changed);
	void SendSpatialUpdate(USpatialActorChannel* Channel, const TArray<uint16>& Changed);
	void ReceiveSpatialUpdate(USpatialActorChannel* Channel, FNetBitWriter& IncomingPayload);
	void InvokeRPC(AActor* TargetActor, const UFunction* const Function, FFrame* const Frame);

	// Called by USpatialPackageMapClient when a UObject is "resolved" i.e. has a unreal object ref.
	void ResolveObject(UObject* Object, const improbable::unreal::UnrealObjectRef& ObjectRef);

	// Called by USpatialInteropPipelineBlock when an actor channel is opened on the client.
	void AddActorChannel_Client(const worker::EntityId& EntityId, USpatialActorChannel* Channel);

	// Used by generated type bindings to map an entity ID to its actor channel.
	USpatialActorChannel* GetActorChannelByEntityId(const worker::EntityId& EntityId) const;

	// RPC handlers. Used by generated type bindings.
	void SendCommandRequest_Internal(FRPCRequestFunction Function, bool bReliable);
	void SendCommandResponse_Internal(FRPCResponderFunction Function);
	void HandleCommandResponse_Internal(const FString& RPCName, FUntypedRequestId RequestId, const worker::EntityId& EntityId, const worker::StatusCode& StatusCode, const FString& Message);

	// Used to queue incoming/outgoing object updates/RPCs. Used by generated type bindings.
	void QueueOutgoingObjectUpdate_Internal(UObject* UnresolvedObject, USpatialActorChannel* DependentChannel, uint16 Handle);
	void QueueOutgoingRPC_Internal(UObject* UnresolvedObject, FRPCRequestFunction CommandSender, bool bReliable);
	void QueueIncomingObjectUpdate_Internal(const improbable::unreal::UnrealObjectRef& UnresolvedObjectRef, USpatialActorChannel* DependentChannel, UObjectPropertyBase* Property, uint16 Handle);
	void QueueIncomingRPC_Internal(const improbable::unreal::UnrealObjectRef& UnresolvedObjectRef, FRPCResponderFunction Responder);

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
	bool bIsClient;

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
	TMap<UObject*, TArray<TPair<FRPCRequestFunction, bool>>> PendingOutgoingRPCs;

	// Pending incoming object ref property updates.
	TMap<FHashableUnrealObjectRef, TMap<USpatialActorChannel*, TArray<FPendingIncomingObjectProperty>>> PendingIncomingObjectRefProperties;
	
	// Pending incoming RPCs.
	TMap<FHashableUnrealObjectRef, TArray<FRPCResponderFunction>> PendingIncomingRPCs;

private:
	void SetComponentInterests(USpatialActorChannel* ActorChannel, const worker::EntityId& EntityId);

	void ResolvePendingOutgoingObjectUpdates(UObject* Object);
	void ResolvePendingOutgoingRPCs(UObject* Object);
	void ResolvePendingIncomingObjectUpdates(UObject* Object, const improbable::unreal::UnrealObjectRef& ObjectRef);
	void ResolvePendingIncomingRPCs(const improbable::unreal::UnrealObjectRef& ObjectRef);
};
