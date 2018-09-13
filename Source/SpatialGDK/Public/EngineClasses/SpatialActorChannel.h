// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Engine/ActorChannel.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/SpatialView.h"
#include "Interop/SpatialTypebindingManager.h"
#include "Utils/RepDataUtils.h"

#include <improbable/c_worker.h>

#include "SpatialActorChannel.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKActorChannel, Log, All);

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
		return EntityId != 0;
	}

	// Called on the client when receiving an update.
	FORCEINLINE bool IsClientAutonomousProxy()
	{
		if (NetDriver->GetNetMode() != NM_Client)
		{
			return false;
		}

		FClassInfo* Info = NetDriver->TypebindingManager->FindClassInfoByClass(Actor->GetClass());
		check(Info);

		return NetDriver->View->GetAuthority(EntityId, Info->RPCComponents[RPC_Client]) == WORKER_AUTHORITY_AUTHORITATIVE;
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
	virtual void Close() override;
	virtual bool ReplicateActor() override;
	virtual void SetChannelActor(AActor* InActor) override;

	void RegisterEntityId(const Worker_EntityId& ActorEntityId);
	bool ReplicateSubobject(UObject* Obj, const FReplicationFlags& RepFlags);
	virtual bool ReplicateSubobject(UObject* Obj, FOutBunch& Bunch, const FReplicationFlags& RepFlags) override;

	FRepChangeState CreateInitialRepChangeState(UObject* Object);
	FHandoverChangeState CreateInitialHandoverChangeState(FClassInfo* ClassInfo);

	// For an object that is replicated by this channel (i.e. this channel's actor or its component), find out whether a given handle is an array.
	bool IsDynamicArrayHandle(UObject* Object, uint16 Handle);

	FObjectReplicator& PreReceiveSpatialUpdate(UObject* TargetObject);
	void PostReceiveSpatialUpdate(UObject* TargetObject, const TArray<UProperty*>& RepNotifies);

	void OnReserveEntityIdResponse(const struct Worker_ReserveEntityIdResponseOp& Op);
	void OnCreateEntityResponse(const struct Worker_CreateEntityResponseOp& Op);

protected:
	// UChannel Interface
	virtual bool CleanUp(const bool bForDestroy) override;

private:
	void DeleteEntityIfAuthoritative();
	bool IsSingletonEntity();

	void UpdateSpatialPosition();

	FVector GetActorSpatialPosition(AActor* Actor);

	void InitializeHandoverShadowData(TArray<uint8>& ShadowData, UObject* Object);
	FHandoverChangeState GetHandoverChangeList(TArray<uint8>& ShadowData, UObject* Object);

public:
	// Distinguishes between channels created for actors that went through the "old" pipeline vs actors that are triggered through SpawnActor() calls.
	//In the future we may not use an actor channel for non-core actors.
	UPROPERTY(transient)
	bool bCoreActor;

private:
	Worker_EntityId EntityId;

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
	UPROPERTY(Transient)
	bool bCreatingNewEntity;

};
