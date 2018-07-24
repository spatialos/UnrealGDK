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
	FORCEINLINE bool IsClientAutonomousProxy(worker::ComponentId ServerRPCsComponentId)
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
			auto ComponentIterator = EntityAuthority.find(ServerRPCsComponentId);
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
			HandoverChanged
		};
	}

	FORCEINLINE FPropertyChangeState GetChangeStateSubobject(UObject* Obj, FObjectReplicator* Replicator, const TArray<uint16>& RepChanged, const TArray<uint16>& HandoverChanged) const
	{
		return {
			(uint8*)Obj,
			RepChanged,
			Replicator->RepLayout->Cmds,
			Replicator->RepLayout->BaseHandleToCmdIndex,
			HandoverChanged
		};
	}

	// UChannel interface
	virtual void Init(UNetConnection * InConnection, int32 ChannelIndex, bool bOpenedLocally) override;
	virtual void Close() override;
	//Requires source changes to be virtual in base class.
	virtual bool ReplicateActor() override;
	virtual void SetChannelActor(AActor* InActor) override;

	void SendReserveEntityIdRequest();
	bool ReplicateSubobject(UObject *Obj, const FReplicationFlags &RepFlags);
	FPropertyChangeState CreateSubobjectChangeState(UActorComponent* Component);
	TArray<uint16> SkipOverChangelistArrays(FObjectReplicator& Replicator);

	// Called by SpatialInterop when receiving an update.
	void PreReceiveSpatialUpdate(UObject* TargetObject);
	void PostReceiveSpatialUpdate(UObject* TargetObject, const TArray<UProperty*>& RepNotifies);

	// Distinguishes between channels created for actors that went through the "old" pipeline vs actors that are triggered through SpawnActor() calls.
	//In the future we may not use an actor channel for non-core actors.
	UPROPERTY(transient)
	bool bCoreActor;

	void OnReserveEntityIdResponse(const worker::ReserveEntityIdResponseOp& Op);
	void OnCreateEntityResponse(const worker::CreateEntityResponseOp& Op);

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
