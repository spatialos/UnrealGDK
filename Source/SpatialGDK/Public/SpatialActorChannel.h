// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include <improbable/worker.h>

#include "Engine/ActorChannel.h"
#include "EntityId.h"
#include "SpatialNetDriver.h"
#include "SpatialTypeBinding.h"
#include "improbable/standard_library.h"
#include "improbable/worker.h"
#include "SpatialActorChannel.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKActorChannel, Log, All);

class USpatialNetDriver;

// A replacement actor channel that plugs into the Engine's replication system and works with SpatialOS
UCLASS(Transient)
class SPATIALGDK_API USpatialActorChannel : public UActorChannel
{
	GENERATED_BODY()

public:
	USpatialActorChannel(const FObjectInitializer & ObjectInitializer = FObjectInitializer::Get());

	// SpatialOS Entity ID.
	FORCEINLINE FEntityId GetEntityId() const
	{
		return ActorEntityId;
	}

	FORCEINLINE void SetEntityId(FEntityId ActorEntityId)
	{
		this->ActorEntityId = ActorEntityId;
	}

	FORCEINLINE bool IsReadyForReplication() const
	{
		// Wait until we've reserved an entity ID.		
		return ActorEntityId != FEntityId{};
	}

	// Called on the client when receiving an update.
	FORCEINLINE bool IsClientAutonomousProxy(worker::ComponentId ClientRPCsComponentId)
	{
		if (SpatialNetDriver->GetNetMode() != NM_Client)
		{
			return false;
		}

		TSharedPtr<worker::View> View = WorkerView.Pin();
		if (View.Get())
		{
			// This will never fail because we can't have an actor channel without having checked out the entity.
			auto& EntityAuthority = View->ComponentAuthority[ActorEntityId.ToSpatialEntityId()];
			auto ComponentIterator = EntityAuthority.find(ClientRPCsComponentId);
			if (ComponentIterator != EntityAuthority.end())
			{
				return (*ComponentIterator).second == worker::Authority::kAuthoritative;
			}
		}
		return false;
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
	//Requires source changes to be virtual in base class.
	virtual bool ReplicateActor() override;
	virtual void SetChannelActor(AActor* InActor) override;

	void SendReserveEntityIdRequest();
	void RegisterEntityId(const FEntityId& ActorEntityId);
	bool ReplicateSubobject(UObject *Obj, const FReplicationFlags &RepFlags);
	FPropertyChangeState CreateSubobjectChangeState(UActorComponent* Component);
	TArray<uint16> GetAllPropertyHandles(FObjectReplicator& Replicator);

	// For an object that is replicated by this channel (i.e. this channel's actor or its component), find out whether a given handle is an array.
	bool IsDynamicArrayHandle(UObject* Object, uint16 Handle);

	// Called by SpatialInterop when receiving an update.
	FObjectReplicator& PreReceiveSpatialUpdate(UObject* TargetObject);
	void PostReceiveSpatialUpdate(UObject* TargetObject, const TArray<UProperty*>& RepNotifies);

	// Distinguishes between channels created for actors that went through the "old" pipeline vs actors that are triggered through SpawnActor() calls.
	//In the future we may not use an actor channel for non-core actors.
	UPROPERTY(transient)
	bool bCoreActor;

	void OnReserveEntityIdResponse(const worker::ReserveEntityIdResponseOp& Op);
	void OnCreateEntityResponse(const worker::CreateEntityResponseOp& Op);

	void OnReserveEntityIdResponseCAPI(const struct Worker_ReserveEntityIdResponseOp& Op);
	void OnCreateEntityResponseCAPI(const struct Worker_CreateEntityResponseOp& Op);

protected:
	// UChannel interface
	virtual bool CleanUp(const bool bForDestroy) override;

private:

	void DeleteEntityIfAuthoritative();

	// A critical entity is any entity built into the snapshot which should not be deleted by any worker.
	bool IsCriticalEntity();

	TWeakPtr<worker::View> WorkerView;
	FEntityId ActorEntityId;

	UPROPERTY(transient)
	USpatialNetDriver* SpatialNetDriver;

	FVector LastSpatialPosition;
	TArray<uint8> HandoverPropertyShadowData;

	// If this actor channel is responsible for creating a new entity, this will be set to true during initial replication.
	UPROPERTY(Transient)
	bool bCreatingNewEntity;

private:
	void UpdateSpatialPosition();

	static FVector GetActorSpatialPosition(AActor* Actor);
};
