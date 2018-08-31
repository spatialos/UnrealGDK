// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Engine/ActorChannel.h"
#include "SpatialSender.h"
#include "SpatialReceiver.h"
#include "SpatialNetDriver.h"
#include "SpatialView.h"
#include "SpatialTypebindingManager.h"
#include "Utils/RepDataUtils.h"

#include <improbable/c_worker.h><

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

	FORCEINLINE void SetEntityId(Worker_EntityId EntityId)
	{
		this->EntityId = EntityId;
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

	FORCEINLINE FPropertyChangeState GetChangeState(const TArray<uint16>& RepChanged, const TArray<uint16>& HandoverChanged) const
	{
		return {
			(uint8*)Actor,
			RepChanged,
			ActorReplicator->RepLayout->Cmds,
			ActorReplicator->RepLayout->BaseHandleToCmdIndex,
			ActorReplicator->RepLayout->Parents,
			HandoverChanged
		};
	}

	FORCEINLINE FPropertyChangeState GetChangeStateForObject(UObject* Obj, FObjectReplicator* Replicator, const TArray<uint16>& RepChanged, const TArray<uint16>& HandoverChanged)
	{
		if (!Replicator)
		{
			auto WeakObjectPtr = TWeakObjectPtr<UObject>(Obj);
			check(ObjectHasReplicator(WeakObjectPtr));
			Replicator = &FindOrCreateReplicator(WeakObjectPtr).Get();
		}

		return {
			(uint8*)Obj,
			RepChanged,
			Replicator->RepLayout->Cmds,
			Replicator->RepLayout->BaseHandleToCmdIndex,
			Replicator->RepLayout->Parents,
			HandoverChanged
		};
	}

	FORCEINLINE FRepLayout& GetObjectRepLayout(UObject* Object)
	{
		auto WeakObjectPtr = TWeakObjectPtr<UObject>(Object);
		check(ObjectHasReplicator(WeakObjectPtr));
		return *FindOrCreateReplicator(WeakObjectPtr)->RepLayout;
	}

	FORCEINLINE FRepStateStaticBuffer& GetObjectStaticBuffer(UObject* Object)
	{
		auto WeakObjectPtr = TWeakObjectPtr<UObject>(Object);
		check(ObjectHasReplicator(WeakObjectPtr));
		return FindOrCreateReplicator(WeakObjectPtr)->RepState->StaticBuffer;
	}

	// UChannel interface
	virtual void Init(UNetConnection * InConnection, int32 ChannelIndex, bool bOpenedLocally) override;
	virtual void Close() override;
	virtual bool ReplicateActor() override;
	virtual void SetChannelActor(AActor* InActor) override;

	void RegisterEntityId(const Worker_EntityId& ActorEntityId);
	bool ReplicateSubobject(UObject *Obj, const FReplicationFlags &RepFlags);
	FPropertyChangeState CreateSubobjectChangeState(UActorComponent* Component);
	TArray<uint16> GetAllPropertyHandles(FObjectReplicator& Replicator);

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
	bool IsCriticalEntity();

	void UpdateSpatialPosition();

	FVector GetActorSpatialPosition(AActor* Actor);

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
	USpatialSender* Sender;

	UPROPERTY(transient)
	USpatialReceiver* Receiver;

	FVector LastSpatialPosition;

	TArray<uint8> HandoverPropertyShadowData;

	// If this actor channel is responsible for creating a new entity, this will be set to true during initial replication.
	UPROPERTY(Transient)
	bool bCreatingNewEntity;

};
