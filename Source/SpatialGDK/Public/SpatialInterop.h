// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "AddComponentOpWrapperBase.h"
#include "ComponentIdentifier.h"
#include "CoreMinimal.h"
#include "SpatialTypeBinding.h"
#include "SpatialUnrealObjectRef.h"
#include "SpatialInterop.generated.h"

class USpatialOS;
class USpatialActorChannel;
class USpatialPackageMapClient;
class USpatialNetDriver;

SPATIALGDK_API DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKInterop, Log, All);

// An general version of worker::RequestId.
using FUntypedRequestId = decltype(worker::RequestId<void>::Id);

// Stores the result of an attempt to call an RPC sender function. Either we have an unresolved object which needs
// to be resolved before we can send this RPC, or we successfully sent a command request.
struct FRPCCommandRequestResult
{
	const UObject* UnresolvedObject;
	FUntypedRequestId RequestId;

	FRPCCommandRequestResult() : UnresolvedObject{nullptr}, RequestId{0} {}
	FRPCCommandRequestResult(const UObject* UnresolvedObject) : UnresolvedObject{UnresolvedObject}, RequestId{0} {}
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

// Helper types used by the maps below.
using FPendingOutgoingProperties = TPair<TArray<uint16>, TArray<uint16>>; // Pending incoming properties (replicated and migratable).
using FPendingIncomingProperties = TPair<TArray<const FRepHandleData*>, TArray<const FMigratableHandleData*>>;

// Map types for pending objects/RPCs. For pending updates, they store a map from an unresolved object to a map of channels to properties
// within those channels which depend on the unresolved object. For pending RPCs, they store a map from an unresolved object to a list of
// RPC functor objects which need to be re-executed when the object is resolved.
using FPendingOutgoingObjectUpdateMap = TMap<const UObject*, TMap<USpatialActorChannel*, FPendingOutgoingProperties>>;
using FPendingOutgoingRPCMap = TMap<const UObject*, TArray<TPair<FRPCCommandRequestFunc, bool>>>;
using FPendingIncomingObjectUpdateMap = TMap<FHashableUnrealObjectRef, TMap<USpatialActorChannel*, FPendingIncomingProperties>>;
using FPendingIncomingRPCMap = TMap<FHashableUnrealObjectRef, TArray<FRPCCommandResponseFunc>>;

// When sending arrays that contain UObject* (e.g. TArray<UObject*> or TArray<FMyStruct> where FMyStruct contains a UObject*),
// if there are unresolved objects, we delay sending the array until all the objects are resolved.
// FOutgoingPendingArrayRegister will keep track of unresolved objects, which are referenced by the actor channel and property handle,
// as well as individually by each unresolved UObject*.
// If new data is replicated for the same channel and property before this queued array is sent, the queued array is discarded and all the references removed.
// When an object is resolved, it will remove its reference as well as remove itself from the set. If the set is empty as a result, that means
// all the objects are resolved, so we retry to send the update.
struct FOutgoingPendingArrayRegister
{
	TSet<const UObject*> UnresolvedObjects;
};

// OPAR stands for Outgoing Pending Array Register (FOutgoingPendingArrayRegister)
// Map of maps instead of combining USpatialActorChannel* and uint16 into a joint key type because
// it makes it easier to iterate them when resolving an object ref
using FHandleToOPARMap = TMap<uint16, TSharedPtr<FOutgoingPendingArrayRegister>>;
using FChannelToHandleToOPARMap = TMap<USpatialActorChannel*, FHandleToOPARMap>;

using FOutgoingPendingArrayUpdateMap = TMap<const UObject*, FChannelToHandleToOPARMap>;

using FResolvedObjects = TArray<TPair<UObject*, const improbable::unreal::UnrealObjectRef>>;

// Helper function to write incoming replicated property data to an object.
FORCEINLINE void ApplyIncomingReplicatedPropertyUpdate(const FRepHandleData& RepHandleData, UObject* Object, const void* Value, TSet<UProperty*>& RepNotifies)
{
	uint8* Dest = RepHandleData.GetPropertyData(reinterpret_cast<uint8*>(Object));

	check(RepHandleData.PropertyChain.Num() > 0);
	check(RepHandleData.PropertyChain[0] != nullptr);
	check(RepHandleData.Property != nullptr);

	// If the root of the property chain has a RepNotify flag and the leaf value has changed, add it to the rep notify list.
	UProperty* PropertyToNotify = RepHandleData.PropertyChain[0];
	if (PropertyToNotify->HasAnyPropertyFlags(CPF_RepNotify))
	{
		if (RepHandleData.RepNotifyCondition == REPNOTIFY_Always || !RepHandleData.Property->Identical(Dest, Value))
		{
			RepNotifies.Add(PropertyToNotify);
		}
	}

	// Write value to destination.
	UBoolProperty* BoolProperty = Cast<UBoolProperty>(RepHandleData.Property);
	if (BoolProperty)
	{
		// We use UBoolProperty::SetPropertyValue here explicitly to ensure that packed boolean properties
		// are de-serialized correctly without clobbering neighboring boolean values in memory.
		BoolProperty->SetPropertyValue(Dest, *static_cast<const bool*>(Value));
	}
	else
	{
		RepHandleData.Property->CopySingleValue(Dest, Value);
	}
}

// Helper function to write incoming migratable property data to an object.
FORCEINLINE void ApplyIncomingMigratablePropertyUpdate(const FMigratableHandleData& MigratableHandleData, UObject* Object, const void* Value)
{
	uint8* Dest = MigratableHandleData.GetPropertyData(reinterpret_cast<uint8*>(Object));

	// Write value to destination.
	UBoolProperty* BoolProperty = Cast<UBoolProperty>(MigratableHandleData.Property);
	if (BoolProperty)
	{
		// We use UBoolProperty::SetPropertyValue here explicitly to ensure that packed boolean properties
		// are de-serialized correctly without clobbering neighboring boolean values in memory.
		BoolProperty->SetPropertyValue(Dest, *static_cast<const bool*>(Value));
	}
	else
	{
		MigratableHandleData.Property->CopySingleValue(Dest, Value);
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

	void Init(USpatialOS* Instance, USpatialNetDriver* Driver, FTimerManager* TimerManager);

	// Type bindings.
	USpatialTypeBinding* GetTypeBindingByClass(UClass* Class) const;

	// Sending component updates and RPCs.
	worker::RequestId<worker::CreateEntityRequest> SendCreateEntityRequest(USpatialActorChannel* Channel, const FVector& Location, const FString& PlayerWorkerId, const TArray<uint16>& RepChanged, const TArray<uint16>& MigChanged);
	worker::RequestId<worker::DeleteEntityRequest> SendDeleteEntityRequest(const FEntityId& EntityId);
	void SendSpatialPositionUpdate(const FEntityId& EntityId, const FVector& Location);
	void SendSpatialUpdate(USpatialActorChannel* Channel, const TArray<uint16>& RepChanged, const TArray<uint16>& MigChanged);
	void InvokeRPC(AActor* TargetActor, const UFunction* const Function, UObject* CallingObject, void* Parameters);
	void ReceiveAddComponent(USpatialActorChannel* Channel, UAddComponentOpWrapperBase* AddComponentOp);
	void PreReceiveSpatialUpdate(USpatialActorChannel* Channel);
	void PostReceiveSpatialUpdate(USpatialActorChannel* Channel, const TArray<UProperty*>& RepNotifies);

	// Called by USpatialPackageMapClient when a UObject is "resolved" i.e. has a unreal object ref.
	// This will dequeue pending object ref updates and RPCs which depend on this UObject existing in the package map.
	void ResolvePendingOperations(UObject* Object, const improbable::unreal::UnrealObjectRef& ObjectRef);
	void OnLeaveCriticalSection();
	void ResolvePendingOperations_Internal(UObject* Object, const improbable::unreal::UnrealObjectRef& ObjectRef);

	bool IsAuthoritativeDestructionAllowed() const { return bAuthoritativeDestruction; }
	void StartIgnoringAuthoritativeDestruction() { bAuthoritativeDestruction = false; }
	void StopIgnoringAuthoritativeDestruction() { bAuthoritativeDestruction = true; }

	// Called by USpatialInteropPipelineBlock when an actor channel is opened on the client.
	void AddActorChannel(const FEntityId& EntityId, USpatialActorChannel* Channel);
	void RemoveActorChannel(const FEntityId& EntityId);
	void DeleteEntity(const FEntityId& EntityId);

	// Modifies component interest according to the updates this actor needs from SpatialOS.
	// Called by USpatialInteropPipelineBlock after an actor has had its components initialized with values from SpatialOS.
	void SendComponentInterests(USpatialActorChannel* ActorChannel, const FEntityId& EntityId);

	// Used by generated type bindings to map an entity ID to its actor channel.
	USpatialActorChannel* GetActorChannelByEntityId(const FEntityId& EntityId) const;

	// RPC handlers. Used by generated type bindings.
	void InvokeRPCSendHandler_Internal(FRPCCommandRequestFunc Function, bool bReliable);
	void InvokeRPCReceiveHandler_Internal(FRPCCommandResponseFunc Function);
	void HandleCommandResponse_Internal(const FString& RPCName, FUntypedRequestId RequestId, const FEntityId& EntityId, const worker::StatusCode& StatusCode, const FString& Message);

	// Used to queue incoming/outgoing object updates/RPCs. Used by generated type bindings.
	void QueueOutgoingObjectRepUpdate_Internal(const UObject* UnresolvedObject, USpatialActorChannel* DependentChannel, uint16 Handle);
	void QueueOutgoingObjectMigUpdate_Internal(const UObject* UnresolvedObject, USpatialActorChannel* DependentChannel, uint16 Handle);
	void QueueOutgoingRPC_Internal(const UObject* UnresolvedObject, FRPCCommandRequestFunc CommandSender, bool bReliable);
	void QueueIncomingObjectRepUpdate_Internal(const improbable::unreal::UnrealObjectRef& UnresolvedObjectRef, USpatialActorChannel* DependentChannel, const FRepHandleData* RepHandleData);
	void QueueIncomingObjectMigUpdate_Internal(const improbable::unreal::UnrealObjectRef& UnresolvedObjectRef, USpatialActorChannel* DependentChannel, const FMigratableHandleData* MigHandleData);
	void QueueIncomingRPC_Internal(const improbable::unreal::UnrealObjectRef& UnresolvedObjectRef, FRPCCommandResponseFunc Responder);

	void ResetOutgoingArrayRepUpdate_Internal(USpatialActorChannel* DependentChannel, uint16 Handle);
	void QueueOutgoingArrayRepUpdate_Internal(const TSet<const UObject*>& UnresolvedObjects, USpatialActorChannel* DependentChannel, uint16 Handle);
	
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
	TMap<UClass*, USpatialTypeBinding*> TypeBindings;

	// A map from Entity ID to actor channel.
	TMap<FEntityId, USpatialActorChannel*> EntityToActorChannel;

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

	FChannelToHandleToOPARMap PropertyToOPAR;
	FOutgoingPendingArrayUpdateMap ObjectToOPAR;

	// Used to queue resolved objects when added during a critical section. These objects then have
	// any pending operations resolved on them once the critical section has ended.
	FResolvedObjects ResolvedObjectQueue;

	bool bAuthoritativeDestruction;


private:
	void RegisterInteropType(UClass* Class, USpatialTypeBinding* Binding);
	void UnregisterInteropType(UClass* Class);

	void ResolvePendingOutgoingObjectUpdates(UObject* Object);
	void ResolvePendingOutgoingRPCs(UObject* Object);
	void ResolvePendingIncomingObjectUpdates(UObject* Object, const improbable::unreal::UnrealObjectRef& ObjectRef);
	void ResolvePendingIncomingRPCs(const improbable::unreal::UnrealObjectRef& ObjectRef);

	void ResolvePendingOutgoingArrayUpdates(UObject* Object);
};
