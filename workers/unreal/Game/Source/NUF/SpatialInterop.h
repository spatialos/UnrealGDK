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

// Function storing a command request, capturing all arguments by value.
using FRPCRequestFunction = TFunction<FRPCRequestResult()>;

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

UCLASS()
class NUF_API USpatialInterop : public UObject
{
	GENERATED_BODY()
public:
	USpatialInterop();

	void Init(bool bClient, USpatialOS* Instance, USpatialNetDriver* Driver, FTimerManager* TimerManager);
	void Tick(float DeltaTime);

	USpatialActorChannel* GetClientActorChannel(const worker::EntityId& EntityId) const;
	void AddClientActorChannel(const worker::EntityId& EntityId, USpatialActorChannel* Channel);

	void RegisterInteropType(UClass* Class, USpatialTypeBinding* Binding);
	void UnregisterInteropType(UClass* Class);
	USpatialTypeBinding* GetTypeBindingByClass(UClass* Class) const;

	worker::RequestId<worker::CreateEntityRequest> SendCreateEntityRequest(USpatialActorChannel* Channel, const FString& PlayerWorkerId, const TArray<uint16>& Changed);
	void SendSpatialUpdate(USpatialActorChannel* Channel, const TArray<uint16>& Changed);
	void ReceiveSpatialUpdate(USpatialActorChannel* Channel, FNetBitWriter& IncomingPayload);
	void InvokeRPC(AActor* TargetActor, const UFunction* const Function, FFrame* const Frame);

	void SendCommandRequest(FRPCRequestFunction Function, bool bReliable);
	void HandleCommandResponse(const FString& RPCName, FUntypedRequestId RequestId, const worker::EntityId& EntityId, const worker::StatusCode& StatusCode, const FString& Message);

	// Called by USpatialPackageMapClient when a UObject is "resolved" i.e. has a unreal object ref.
	void OnResolveObject(UObject* Object, const improbable::unreal::UnrealObjectRef& ObjectRef);

	void AddPendingOutgoingObjectRefUpdate(UObject* UnresolvedObject, USpatialActorChannel* DependentChannel, uint16 Handle);
	void AddPendingOutgoingRPC(UObject* UnresolvedObject, FRPCRequestFunction CommandSender, bool bReliable);
	void AddPendingIncomingObjectRefUpdate(const improbable::unreal::UnrealObjectRef& UnresolvedObjectRef, USpatialActorChannel* DependentChannel, UObjectPropertyBase* Property, uint16 Handle);
	// Pending incoming RPC

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

	// On clients, there is a 1 to 1 mapping between an actor and an actor channel (as there's just one NetConnection).
	TMap<worker::EntityId, USpatialActorChannel*> EntityToClientActorChannel;

	// Outgoing RPCs (for retry logic).
	TMap<FUntypedRequestId, TSharedPtr<FOutgoingReliableRPC>> OutgoingReliableRPCs;

	// Pending outgoing object ref property updates.
	TMap<UObject*, TArray<USpatialActorChannel*>> ChannelsAwaitingOutgoingObjectResolve;
	TMap<USpatialActorChannel*, TArray<uint16>> PendingOutgoingObjectRefHandles;

	// Pending outgoing RPCs.
	TMap<UObject*, TArray<TPair<FRPCRequestFunction, bool>>> PendingOutgoingRPCs;

	// Pending incoming object ref property updates.
	TMap<FHashableUnrealObjectRef, TArray<USpatialActorChannel*>> ChannelsAwaitingIncomingObjectResolve;
	TMap<USpatialActorChannel*, TArray<TPair<UObjectPropertyBase*, uint16>>> PendingIncomingObjectRefProperties;

private:
	void SetComponentInterests(USpatialActorChannel* ActorChannel, const worker::EntityId& EntityId);

	void ResolvePendingOutgoingObjectRefUpdates(UObject* Object);
	void ResolvePendingOutgoingRPCs(UObject* Object);
	void ResolvePendingIncomingObjectRefUpdates(UObject* Object, const improbable::unreal::UnrealObjectRef& ObjectRef);

	friend class USpatialInteropPipelineBlock;
};
