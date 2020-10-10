// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "EngineClasses/SpatialLoadBalanceEnforcer.h"
#include "EngineClasses/SpatialNetBitWriter.h"
#include "Interop/SpatialClassInfoManager.h"
#include "Interop/SpatialRPCService.h"
#include "Schema/RPCPayload.h"
#include "TimerManager.h"
#include "Utils/RPCContainer.h"
#include "Utils/RepDataUtils.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialSender.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialSender, Log, All);

class USpatialActorChannel;
class SpatialDispatcher;
class USpatialNetDriver;
class USpatialPackageMapClient;
class USpatialReceiver;
class USpatialStaticComponentView;
class USpatialClassInfoManager;
class USpatialWorkerConnection;

namespace SpatialGDK
{
class SpatialEventTracer;
}

struct FReliableRPCForRetry
{
	FReliableRPCForRetry(UObject* InTargetObject, UFunction* InFunction, Worker_ComponentId InComponentId, Schema_FieldId InRPCIndex,
						 const TArray<uint8>& InPayload, int InRetryIndex, const TOptional<Trace_SpanId>& InSpanId);

	TWeakObjectPtr<UObject> TargetObject;
	UFunction* Function;
	Worker_ComponentId ComponentId;
	Schema_FieldId RPCIndex;
	TArray<uint8> Payload;
	int Attempts; // For reliable RPCs

	int RetryIndex; // Index for ordering reliable RPCs on subsequent tries
	TOptional<Trace_SpanId> SpanId;
};

struct FPendingRPC
{
	FPendingRPC() = default;
	FPendingRPC(FPendingRPC&& Other) = default;

	uint32 Offset;
	Schema_FieldId Index;
	TArray<uint8> Data;
	Schema_EntityId Entity;
};

// TODO: Clear TMap entries when USpatialActorChannel gets deleted - UNR:100
// care for actor getting deleted before actor channel
using FChannelObjectPair = TPair<TWeakObjectPtr<USpatialActorChannel>, TWeakObjectPtr<UObject>>;
using FRPCsOnEntityCreationMap = TMap<TWeakObjectPtr<const UObject>, SpatialGDK::RPCsOnEntityCreation>;
using FUpdatesQueuedUntilAuthority = TMap<FEntityId, TArray<FWorkerComponentUpdate>>;
using FChannelsToUpdatePosition = TSet<TWeakObjectPtr<USpatialActorChannel>>;

UCLASS()
class SPATIALGDK_API USpatialSender : public UObject
{
	GENERATED_BODY()

public:
	void Init(USpatialNetDriver* InNetDriver, FTimerManager* InTimerManager, SpatialGDK::SpatialRPCService* InRPCService,
			  SpatialGDK::SpatialEventTracer* InEventTracer);

	// Actor Updates
	void SendComponentUpdates(UObject* Object, const FClassInfo& Info, USpatialActorChannel* Channel, const FRepChangeState* RepChanges,
							  const FHandoverChangeState* HandoverChanges, uint32& OutBytesWritten);
	void SendPositionUpdate(FEntityId EntityId, const FVector& Location);
	void SendAuthorityIntentUpdate(const AActor& Actor, VirtualWorkerId NewAuthoritativeVirtualWorkerId);
	FRPCErrorInfo SendRPC(const FPendingRPCParams& Params);
	void SendOnEntityCreationRPC(UObject* TargetObject, UFunction* Function, const SpatialGDK::RPCPayload& Payload,
								 USpatialActorChannel* Channel, const FUnrealObjectRef& TargetObjectRef);
	void SendCrossServerRPC(UObject* TargetObject, UFunction* Function, const SpatialGDK::RPCPayload& Payload,
							USpatialActorChannel* Channel, const FUnrealObjectRef& TargetObjectRef);
	FRPCErrorInfo SendLegacyRPC(UObject* TargetObject, UFunction* Function, const SpatialGDK::RPCPayload& Payload,
								USpatialActorChannel* Channel, const FUnrealObjectRef& TargetObjectRef);
	bool SendRingBufferedRPC(UObject* TargetObject, UFunction* Function, const SpatialGDK::RPCPayload& Payload,
							 USpatialActorChannel* Channel, const FUnrealObjectRef& TargetObjectRef);
	void SendCommandResponse(FRequestId RequestId, Worker_CommandResponse& Response, const Trace_SpanId CauseSpanId);
	void SendEmptyCommandResponse(Worker_ComponentId ComponentId, Schema_FieldId CommandIndex, FRequestId RequestId,
								  const Trace_SpanId CauseSpanId);
	void SendCommandFailure(FRequestId RequestId, const FString& Message, const Trace_SpanId CauseSpanI);
	void SendAddComponentForSubobject(USpatialActorChannel* Channel, UObject* Subobject, const FClassInfo& Info, uint32& OutBytesWritten);
	void SendAddComponents(FEntityId EntityId, TArray<FWorkerComponentData> ComponentDatas);
	void SendRemoveComponentForClassInfo(FEntityId EntityId, const FClassInfo& Info);
	void SendRemoveComponents(FEntityId EntityId, TArray<Worker_ComponentId> ComponentIds);
	void SendInterestBucketComponentChange(const FEntityId EntityId, const Worker_ComponentId OldComponent,
										   const Worker_ComponentId NewComponent);
	void SendActorTornOffUpdate(FEntityId EntityId, Worker_ComponentId ComponentId);

	void SendCreateEntityRequest(USpatialActorChannel* Channel, uint32& OutBytesWritten);
	void RetireEntity(const FEntityId EntityId, bool bIsNetStartupActor);

	// Creates an entity containing just a tombstone component and the minimal data to resolve an actor.
	void CreateTombstoneEntity(AActor* Actor);

	void SendRequestToClearRPCsOnEntityCreation(FEntityId EntityId);
	void ClearRPCsOnEntityCreation(FEntityId EntityId);

	void SendClientEndpointReadyUpdate(FEntityId EntityId);
	void SendServerEndpointReadyUpdate(FEntityId EntityId);

	void EnqueueRetryRPC(TSharedRef<FReliableRPCForRetry> RetryRPC);
	void FlushRetryRPCs();
	void RetryReliableRPC(TSharedRef<FReliableRPCForRetry> RetryRPC);

	void RegisterChannelForPositionUpdate(USpatialActorChannel* Channel);
	void ProcessPositionUpdates();

	void UpdateInterestComponent(AActor* Actor);

	void ProcessOrQueueOutgoingRPC(const FUnrealObjectRef& InTargetObjectRef, SpatialGDK::RPCPayload&& InPayload);
	void ProcessUpdatesQueuedUntilAuthority(FEntityId EntityId, Worker_ComponentId ComponentId);

	void FlushRPCService();

	SpatialGDK::RPCPayload CreateRPCPayloadFromParams(UObject* TargetObject, const FUnrealObjectRef& TargetObjectRef, UFunction* Function,
													  void* Params);
	void GainAuthorityThenAddComponent(USpatialActorChannel* Channel, UObject* Object, const FClassInfo* Info);

	// Creates an entity authoritative on this server worker, ensuring it will be able to receive updates for the GSM.
	UFUNCTION()
	void CreateServerWorkerEntity();
	void RetryServerWorkerEntityCreation(FEntityId EntityId, int AttemptCounte);
	void UpdateServerWorkerEntityInterestAndPosition();

	void ClearPendingRPCs(const FEntityId EntityId);

	bool ValidateOrExit_IsSupportedClass(const FString& PathName);

private:
	// Create a copy of an array of components. Deep copies all Schema_ComponentData.
	static TArray<FWorkerComponentData> CopyEntityComponentData(const TArray<FWorkerComponentData>& EntityComponents);
	// Create a copy of an array of components. Deep copies all Schema_ComponentData.
	static void DeleteEntityComponentData(TArray<FWorkerComponentData>& EntityComponents);

	// Create an entity given a set of components and an ID. Retries with the same component data and entity ID on timeout.
	void CreateEntityWithRetries(FEntityId EntityId, FString EntityName, TArray<FWorkerComponentData> Components);

	// Actor Lifecycle
	FRequestId CreateEntity(USpatialActorChannel* Channel, uint32& OutBytesWritten);
	Worker_ComponentData CreateLevelComponentData(AActor* Actor);

	void AddTombstoneToEntity(const FEntityId EntityId);

	void PeriodicallyProcessOutgoingRPCs();

	// RPC Construction
	FSpatialNetBitWriter PackRPCDataToSpatialNetBitWriter(UFunction* Function, void* Parameters) const;

	Worker_CommandRequest CreateRPCCommandRequest(UObject* TargetObject, const SpatialGDK::RPCPayload& Payload,
												  Worker_ComponentId ComponentId, Schema_FieldId CommandIndex, FEntityId& OutEntityId);
	Worker_CommandRequest CreateRetryRPCCommandRequest(const FReliableRPCForRetry& RPC, uint32 TargetObjectOffset);
	FWorkerComponentUpdate CreateRPCEventUpdate(UObject* TargetObject, const SpatialGDK::RPCPayload& Payload,
												Worker_ComponentId ComponentId, Schema_FieldId EventIndext);

	// RPC Tracking
#if !UE_BUILD_SHIPPING
	void TrackRPC(AActor* Actor, UFunction* Function, const SpatialGDK::RPCPayload& Payload, const ERPCType RPCType);
#endif

private:
	UPROPERTY()
	USpatialNetDriver* NetDriver;

	UPROPERTY()
	USpatialStaticComponentView* StaticComponentView;

	UPROPERTY()
	USpatialWorkerConnection* Connection;

	UPROPERTY()
	USpatialReceiver* Receiver;

	UPROPERTY()
	USpatialPackageMapClient* PackageMap;

	UPROPERTY()
	USpatialClassInfoManager* ClassInfoManager;

	FTimerManager* TimerManager;

	SpatialGDK::SpatialRPCService* RPCService;

	FRPCContainer OutgoingRPCs{ ERPCQueueType::Send };
	FRPCsOnEntityCreationMap OutgoingOnCreateEntityRPCs;

	TArray<TSharedRef<FReliableRPCForRetry>> RetryRPCs;

	FUpdatesQueuedUntilAuthority UpdatesQueuedUntilAuthorityMap;

	FChannelsToUpdatePosition ChannelsToUpdatePosition;

	SpatialGDK::SpatialEventTracer* EventTracer;
};
