// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Engine/ActorChannel.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialClassInfoManager.h"
#include "Interop/SpatialStaticComponentView.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Schema/StandardLibrary.h"
#include "SpatialCommonTypes.h"
#include "Utils/RepDataUtils.h"

#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialActorChannel.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialActorChannel, Log, All);

UCLASS(Transient)
class SPATIALGDK_API USpatialActorChannel : public UActorChannel
{
	GENERATED_BODY()

public:
	USpatialActorChannel(const FObjectInitializer & ObjectInitializer = FObjectInitializer::Get());

	// SpatialOS Entity ID.
	FORCEINLINE Worker_EntityId GetEntityId() const
	{
		return EntityId;
	}

	FORCEINLINE void SetEntityId(Worker_EntityId InEntityId)
	{
		EntityId = InEntityId;
	}

	FORCEINLINE bool IsReadyForReplication()
	{
		// Make sure we have authority
		if (Actor->Role != ROLE_Authority)
		{
			return false;
		}

		if (EntityId != SpatialConstants::INVALID_ENTITY_ID)
		{
			return true;
		}

		// This could happen if we've run out of entity ids at the time we called SetChannelActor.
		// If that is the case, keep trying to allocate an entity ID until we succeed.
		return TryResolveActor();
	}

	// Called on the client when receiving an update.
	FORCEINLINE bool IsClientAutonomousProxy()
	{
		if (NetDriver->GetNetMode() != NM_Client)
		{
			return false;
		}

		return NetDriver->StaticComponentView->HasAuthority(EntityId, SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID);
	}

	// Indicates whether this client worker has "ownership" (authority over Client endpoint) over the entity corresponding to this channel.
	FORCEINLINE bool IsOwnedByWorker() const
	{
		const TArray<FString>& WorkerAttributes = NetDriver->Connection->GetWorkerAttributes();
		if (const WorkerRequirementSet* WorkerRequirementsSet = NetDriver->StaticComponentView->GetComponentData<SpatialGDK::EntityAcl>(EntityId)->ComponentWriteAcl.Find(SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID))
		{
			for (const WorkerAttributeSet& AttributeSet : *WorkerRequirementsSet)
			{
				for (const FString& Attribute : AttributeSet)
				{
					if (WorkerAttributes.Contains(Attribute))
					{
						return true;
					}
				}
			}
		}

		return false;
	}

	FORCEINLINE bool IsAuthoritativeServer()
	{
		return NetDriver->IsServer() && NetDriver->StaticComponentView->HasAuthority(EntityId, SpatialConstants::POSITION_COMPONENT_ID);
	}

	FORCEINLINE FRepLayout& GetObjectRepLayout(UObject* Object)
	{
		check(ObjectHasReplicator(Object));
		return *FindOrCreateReplicator(Object)->RepLayout;
	}

	FORCEINLINE FRepStateStaticBuffer& GetObjectStaticBuffer(UObject* Object)
	{
		check(ObjectHasReplicator(Object));
		return FindOrCreateReplicator(Object)->RepState->StaticBuffer;
	}

	// UChannel interface
#if ENGINE_MINOR_VERSION <= 20
	virtual void Init(UNetConnection * InConnection, int32 ChannelIndex, bool bOpenedLocally) override;
	virtual int64 Close() override;
#else
	virtual void Init(UNetConnection * InConnection, int32 ChannelIndex, EChannelCreateFlags CreateFlag) override;
	virtual int64 Close(EChannelCloseReason Reason) override;
#endif
	virtual int64 ReplicateActor() override;
	virtual void SetChannelActor(AActor* InActor) override;

	bool TryResolveActor();

	bool ReplicateSubobject(UObject* Obj, const FClassInfo& Info, const FReplicationFlags& RepFlags);
	virtual bool ReplicateSubobject(UObject* Obj, FOutBunch& Bunch, const FReplicationFlags& RepFlags) override;

	TMap<UObject*, FClassInfo*> GetHandoverSubobjects();

	FRepChangeState CreateInitialRepChangeState(TWeakObjectPtr<UObject> Object);
	FHandoverChangeState CreateInitialHandoverChangeState(const FClassInfo& ClassInfo);

	// For an object that is replicated by this channel (i.e. this channel's actor or its component), find out whether a given handle is an array.
	bool IsDynamicArrayHandle(UObject* Object, uint16 Handle);

	void ProcessOwnershipChange();
	FObjectReplicator& PreReceiveSpatialUpdate(UObject* TargetObject);
	void PostReceiveSpatialUpdate(UObject* TargetObject, const TArray<UProperty*>& RepNotifies);

	void OnCreateEntityResponse(const struct Worker_CreateEntityResponseOp& Op);

	FVector GetActorSpatialPosition(AActor* Actor);

	void RemoveRepNotifiesWithUnresolvedObjs(TArray<UProperty*>& RepNotifies, const FRepLayout& RepLayout, const FObjectReferencesMap& RefMap, UObject* Object);
	
	void UpdateShadowData();
	void UpdateSpatialPositionWithFrequencyCheck();
	void UpdateSpatialPosition();

	FORCEINLINE void MarkInterestDirty() { bInterestDirty = true; }
	FORCEINLINE bool GetInterestDirty() const { return bInterestDirty; }

	// If this actor channel is responsible for creating a new entity, this will be set to true once the entity is created.
	bool bCreatedEntity;

	// If this actor channel is responsible for creating a new entity, this will be set to true during initial replication.
	bool bCreatingNewEntity;

protected:
	// UChannel Interface
#if ENGINE_MINOR_VERSION <= 20
	virtual bool CleanUp(const bool bForDestroy) override;
#else
	virtual bool CleanUp(const bool bForDestroy, EChannelCloseReason CloseReason) override;
#endif

private:
	void ServerProcessOwnershipChange();
	void ClientProcessOwnershipChange();

	void DeleteEntityIfAuthoritative();
	bool IsSingletonEntity();

	void SendPositionUpdate(AActor* InActor, Worker_EntityId InEntityId, const FVector& NewPosition);

	void InitializeHandoverShadowData(TArray<uint8>& ShadowData, UObject* Object);
	FHandoverChangeState GetHandoverChangeList(TArray<uint8>& ShadowData, UObject* Object);

private:
	Worker_EntityId EntityId;
	bool bFirstTick;
	bool bInterestDirty;

	// Used on the client to track gaining/losing ownership.
	bool bNetOwned;
	// Used on the server to track when the owner changes.
	FString SavedOwnerWorkerAttribute;

	UPROPERTY(transient)
	USpatialNetDriver* NetDriver;

	UPROPERTY(transient)
	class USpatialSender* Sender;

	UPROPERTY(transient)
	class USpatialReceiver* Receiver;

	FVector LastPositionSinceUpdate;
	float TimeWhenPositionLastUpdated;

	// Shadow data for Handover properties.
	// For each object with handover properties, we store a blob of memory which contains
	// the state of those properties at the last time we sent them, and is used to detect
	// when those properties change.
	TArray<uint8>* ActorHandoverShadowData;
	TMap<TWeakObjectPtr<UObject>, TSharedRef<TArray<uint8>>> HandoverShadowDataMap;
};
