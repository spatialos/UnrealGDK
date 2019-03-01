// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Engine/ActorChannel.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialClassInfoManager.h"
#include "Interop/SpatialStaticComponentView.h"
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

	FORCEINLINE bool IsReadyForReplication() const
	{
		// Wait until we've reserved an entity ID.		
		if (EntityId == 0)
		{
			return false;
		}

		// Make sure we have authority
		return Actor->Role == ROLE_Authority;
	}

	// Called on the client when receiving an update.
	FORCEINLINE bool IsClientAutonomousProxy()
	{
		if (NetDriver->GetNetMode() != NM_Client)
		{
			return false;
		}

		const FClassInfo& Info = NetDriver->ClassInfoManager->GetOrCreateClassInfoByClass(Actor->GetClass());

		return NetDriver->StaticComponentView->HasAuthority(EntityId, Info.SchemaComponents[SCHEMA_ClientRPC]);
	}

	FORCEINLINE bool IsOwnedByWorker() const
	{
		const FClassInfo& Info = NetDriver->ClassInfoManager->GetOrCreateClassInfoByClass(Actor->GetClass());

		const TArray<FString>& WorkerAttributes = NetDriver->Connection->GetWorkerAttributes();
		if (const WorkerRequirementSet* WorkerRequirementsSet = NetDriver->StaticComponentView->GetComponentData<improbable::EntityAcl>(EntityId)->ComponentWriteAcl.Find(Info.SchemaComponents[SCHEMA_ClientRPC]))
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
	virtual void Init(UNetConnection * InConnection, int32 ChannelIndex, bool bOpenedLocally) override;
	virtual int64 Close() override;
	virtual int64 ReplicateActor() override;
	virtual void SetChannelActor(AActor* InActor) override;

	bool ReplicateSubobject(UObject* Obj, const FClassInfo& Info, const FReplicationFlags& RepFlags);
	virtual bool ReplicateSubobject(UObject* Obj, FOutBunch& Bunch, const FReplicationFlags& RepFlags) override;

	TMap<UObject*, FClassInfo*> GetHandoverSubobjects();

	FRepChangeState CreateInitialRepChangeState(TWeakObjectPtr<UObject> Object);
	FHandoverChangeState CreateInitialHandoverChangeState(const FClassInfo& ClassInfo);

	// For an object that is replicated by this channel (i.e. this channel's actor or its component), find out whether a given handle is an array.
	bool IsDynamicArrayHandle(UObject* Object, uint16 Handle);

	void SpatialViewTick();
	FObjectReplicator& PreReceiveSpatialUpdate(UObject* TargetObject);
	void PostReceiveSpatialUpdate(UObject* TargetObject, const TArray<UProperty*>& RepNotifies);

	void OnCreateEntityResponse(const struct Worker_CreateEntityResponseOp& Op);

	FVector GetActorSpatialPosition(AActor* Actor);

	void RemoveRepNotifiesWithUnresolvedObjs(TArray<UProperty*>& RepNotifies, const FRepLayout& RepLayout, const FObjectReferencesMap& RefMap, UObject* Object);
	
	void UpdateShadowData();

	// If this actor channel is responsible for creating a new entity, this will be set to true once the entity is created.
	bool bCreatedEntity;

protected:
	// UChannel Interface
	virtual bool CleanUp(const bool bForDestroy) override;

private:
	void DeleteEntityIfAuthoritative();
	bool IsSingletonEntity();

	void UpdateSpatialPosition();

	void InitializeHandoverShadowData(TArray<uint8>& ShadowData, UObject* Object);
	FHandoverChangeState GetHandoverChangeList(TArray<uint8>& ShadowData, UObject* Object);

private:
	Worker_EntityId EntityId;
	bool bFirstTick;
	bool bNetOwned;

	UPROPERTY(transient)
	USpatialNetDriver* NetDriver;

	UPROPERTY(transient)
	class USpatialSender* Sender;

	UPROPERTY(transient)
	class USpatialReceiver* Receiver;

	FVector LastSpatialPosition;

	// Shadow data for Handover properties.
	// For each object with handover properties, we store a blob of memory which contains
	// the state of those properties at the last time we sent them, and is used to detect
	// when those properties change.
	TArray<uint8>* ActorHandoverShadowData;
	TMap<TWeakObjectPtr<UObject>, TSharedRef<TArray<uint8>>> HandoverShadowDataMap;

	// If this actor channel is responsible for creating a new entity, this will be set to true during initial replication.
	bool bCreatingNewEntity;
};
