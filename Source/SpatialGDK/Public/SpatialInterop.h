// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "AddComponentOpWrapperBase.h"
#include "ComponentIdentifier.h"
#include "CoreMinimal.h"
#include "SpatialInterop.generated.h"
#include "SpatialTypeBinding.h"
#include "SpatialUnrealObjectRef.h"

class USpatialOS;
class USpatialActorChannel;
class USpatialPackageMapClient;
class USpatialNetDriver;

SPATIALGDK_API DECLARE_LOG_CATEGORY_EXTERN(LogSpatialOSInterop, Log, All);

// An general version of worker::RequestId.
using FUntypedRequestId = decltype(worker::RequestId<void>::Id);

// Stores the result of an attempt to call an RPC sender function. Either we have an unresolved object which needs
// to be resolved before we can send this RPC, or we successfully sent a command request.
struct FRPCCommandRequestResult
{
	UObject *UnresolvedObject;
	FUntypedRequestId RequestId;

	FRPCCommandRequestResult() = delete;
	FRPCCommandRequestResult(UObject *UnresolvedObject)
		: UnresolvedObject{UnresolvedObject}
		, RequestId{0}
	{
	}
	FRPCCommandRequestResult(FUntypedRequestId RequestId)
		: UnresolvedObject{nullptr}
		, RequestId{RequestId}
	{
	}
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
	FOutgoingReliableRPC(FRPCCommandRequestFunc SendCommandRequest)
		: SendCommandRequest{SendCommandRequest}
		, NumAttempts{1}
	{
	}

	FRPCCommandRequestFunc SendCommandRequest;
	uint32 NumAttempts;
};

// Helper types used by the maps below.
using FPendingOutgoingProperties = TPair<TArray<uint16>, TArray<uint16>>; // Pending incoming properties (replicated and migratable).
using FPendingIncomingProperties = TPair<TArray<const FRepHandleData *>, TArray<const FMigratableHandleData *>>;

// Map types for pending objects/RPCs. For pending updates, they store a map from an unresolved object to a map of channels to properties
// within those channels which depend on the unresolved object. For pending RPCs, they store a map from an unresolved object to a list of
// RPC functor objects which need to be re-executed when the object is resolved.
using FPendingOutgoingObjectUpdateMap = TMap<UObject *, TMap<USpatialActorChannel *, FPendingOutgoingProperties>>;
using FPendingOutgoingRPCMap = TMap<UObject *, TArray<TPair<FRPCCommandRequestFunc, bool>>>;
using FPendingIncomingObjectUpdateMap = TMap<FHashableUnrealObjectRef, TMap<USpatialActorChannel *, FPendingIncomingProperties>>;
using FPendingIncomingRPCMap = TMap<FHashableUnrealObjectRef, TArray<FRPCCommandResponseFunc>>;

// Helper function to write incoming replicated property data to an object.
FORCEINLINE void ApplyIncomingReplicatedPropertyUpdate(const FRepHandleData &RepHandleData, UObject *Object, const void *Value, TArray<UProperty *> &RepNotifies)
{
	uint8 *Dest = RepHandleData.GetPropertyData(reinterpret_cast<uint8 *>(Object));

	// If value has changed, add to rep notify list.
	if (RepHandleData.Property->HasAnyPropertyFlags(CPF_RepNotify))
	{
		if (RepHandleData.RepNotifyCondition == REPNOTIFY_Always || !RepHandleData.Property->Identical(Dest, Value))
		{
			RepNotifies.Add(RepHandleData.Property);
		}
	}

	// Write value to destination.
	UBoolProperty *BoolProperty = Cast<UBoolProperty>(RepHandleData.Property);
	if (BoolProperty)
	{
		// We use UBoolProperty::SetPropertyValue here explicitly to ensure that packed boolean properties
		// are de-serialized correctly without clobbering neighboring boolean values in memory.
		BoolProperty->SetPropertyValue(Dest, *static_cast<const bool *>(Value));
	}
	else
	{
		RepHandleData.Property->CopyCompleteValue(Dest, Value);
	}
}

// Helper function to write incoming migratable property data to an object.
FORCEINLINE void ApplyIncomingMigratablePropertyUpdate(const FMigratableHandleData &MigratableHandleData, UObject *Object, const void *Value)
{
	uint8 *Dest = MigratableHandleData.GetPropertyData(reinterpret_cast<uint8 *>(Object));

	// Write value to destination.
	UBoolProperty *BoolProperty = Cast<UBoolProperty>(MigratableHandleData.Property);
	if (BoolProperty)
	{
		// We use UBoolProperty::SetPropertyValue here explicitly to ensure that packed boolean properties
		// are de-serialized correctly without clobbering neighboring boolean values in memory.
		BoolProperty->SetPropertyValue(Dest, *static_cast<const bool *>(Value));
	}
	else
	{
		MigratableHandleData.Property->CopyCompleteValue(Dest, Value);
	}
}

// The system which is responsible for converting and sending Unreal updates to SpatialOS, and receiving updates from SpatialOS and
// applying them to Unreal objects.
UCLASS()
class SPATIALGDK_API USpatialInterop : public UObject
{
	GENERATED_BODY()
public:
	USpatialInterop();

	void Init(USpatialOS *Instance, USpatialNetDriver *Driver, FTimerManager *TimerManager);

	// Type bindings.
	USpatialTypeBinding *GetTypeBindingByClass(UClass *Class) const;

	// Sending component updates and RPCs.
	worker::RequestId<worker::CreateEntityRequest> SendCreateEntityRequest(USpatialActorChannel *Channel, const FVector &Location, const FString &PlayerWorkerId, const TArray<uint16> &RepChanged, const TArray<uint16> &MigChanged);
	void SendSpatialPositionUpdate(const FEntityId &EntityId, const FVector &Location);
	void SendSpatialUpdate(USpatialActorChannel *Channel, const TArray<uint16> &RepChanged, const TArray<uint16> &MigChanged);
	void InvokeRPC(AActor *TargetActor, const UFunction *const Function, FFrame *const Frame);
	void ReceiveAddComponent(USpatialActorChannel *Channel, UAddComponentOpWrapperBase *AddComponentOp);
	void PreReceiveSpatialUpdate(USpatialActorChannel *Channel);
	void PostReceiveSpatialUpdate(USpatialActorChannel *Channel, const TArray<UProperty *> &RepNotifies);

	// Called by USpatialPackageMapClient when a UObject is "resolved" i.e. has a unreal object ref.
	// This will dequeue pending object ref updates and RPCs which depend on this UObject existing in the package map.
	void ResolvePendingOperations(UObject *Object, const improbable::unreal::UnrealObjectRef &ObjectRef);

	// Called by USpatialInteropPipelineBlock when an actor channel is opened on the client.
	void AddActorChannel(const FEntityId &EntityId, USpatialActorChannel *Channel);
	void RemoveActorChannel(const FEntityId &EntityId);

	// Used by generated type bindings to map an entity ID to its actor channel.
	USpatialActorChannel *GetActorChannelByEntityId(const FEntityId &EntityId) const;

	// RPC handlers. Used by generated type bindings.
	void SendCommandRequest_Internal(FRPCCommandRequestFunc Function, bool bReliable);
	void SendCommandResponse_Internal(FRPCCommandResponseFunc Function);
	void HandleCommandResponse_Internal(const FString &RPCName, FUntypedRequestId RequestId, const FEntityId &EntityId, const worker::StatusCode &StatusCode, const FString &Message);

	// Used to queue incoming/outgoing object updates/RPCs. Used by generated type bindings.
	void QueueOutgoingObjectRepUpdate_Internal(UObject *UnresolvedObject, USpatialActorChannel *DependentChannel, uint16 Handle);
	void QueueOutgoingObjectMigUpdate_Internal(UObject *UnresolvedObject, USpatialActorChannel *DependentChannel, uint16 Handle);
	void QueueOutgoingRPC_Internal(UObject *UnresolvedObject, FRPCCommandRequestFunc CommandSender, bool bReliable);
	void QueueIncomingObjectRepUpdate_Internal(const improbable::unreal::UnrealObjectRef &UnresolvedObjectRef, USpatialActorChannel *DependentChannel, const FRepHandleData *RepHandleData);
	void QueueIncomingObjectMigUpdate_Internal(const improbable::unreal::UnrealObjectRef &UnresolvedObjectRef, USpatialActorChannel *DependentChannel, const FMigratableHandleData *MigHandleData);
	void QueueIncomingRPC_Internal(const improbable::unreal::UnrealObjectRef &UnresolvedObjectRef, FRPCCommandResponseFunc Responder);

	// Accessors.
	USpatialOS *GetSpatialOS() const
	{
		return SpatialOSInstance;
	}

	USpatialNetDriver *GetNetDriver() const
	{
		return NetDriver;
	}

private:
	UPROPERTY()
	USpatialOS *SpatialOSInstance;

	UPROPERTY()
	USpatialNetDriver *NetDriver;

	UPROPERTY()
	USpatialPackageMapClient *PackageMap;

	// Timer manager.
	FTimerManager *TimerManager;

	// Type interop bindings.
	UPROPERTY()
	TMap<UClass *, USpatialTypeBinding *> TypeBindings;

	// A map from Entity ID to actor channel.
	TMap<FEntityId, USpatialActorChannel *> EntityToActorChannel;

	// Outgoing RPCs (for retry logic).
	TMap<FUntypedRequestId, TSharedPtr<FOutgoingReliableRPC>> OutgoingReliableRPCs;

	// Pending outgoing object ref property updates.
	FPendingOutgoingObjectUpdateMap PendingOutgoingObjectUpdates;

	// Pending outgoing RPCs.
	FPendingOutgoingRPCMap PendingOutgoingRPCs;

	// Pending incoming object ref property updates.
	FPendingIncomingObjectUpdateMap PendingIncomingObjectUpdates;

	// Pending incoming RPCs.
	FPendingIncomingRPCMap PendingIncomingRPCs;

private:
	void RegisterInteropType(UClass *Class, USpatialTypeBinding *Binding);
	void UnregisterInteropType(UClass *Class);

	void SendComponentInterests(USpatialActorChannel *ActorChannel, const FEntityId &EntityId);

	void ResolvePendingOutgoingObjectUpdates(UObject *Object);
	void ResolvePendingOutgoingRPCs(UObject *Object);
	void ResolvePendingIncomingObjectUpdates(UObject *Object, const improbable::unreal::UnrealObjectRef &ObjectRef);
	void ResolvePendingIncomingRPCs(const improbable::unreal::UnrealObjectRef &ObjectRef);
};
