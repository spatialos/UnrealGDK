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

	FORCEINLINE FPropertyChangeState GetChangeState(const TArray<uint16>& RepChanged, const TArray<uint16>& MigChanged) const
	{
		return{
			(uint8*)Actor,
			RepChanged,
			ActorReplicator->RepLayout->Cmds,
			ActorReplicator->RepLayout->BaseHandleToCmdIndex,
			MigChanged
		};
	}

	FORCEINLINE FPropertyChangeState GetChangeStateSubobject(UObject* obj, FObjectReplicator* replicator, const TArray<uint16>& RepChanged, const TArray<uint16>& MigChanged) const
	{
		return{
			(uint8*)obj,
			RepChanged,
			replicator->RepLayout->Cmds,
			replicator->RepLayout->BaseHandleToCmdIndex,
			MigChanged
		};
	}

	// UChannel interface
	virtual void Init(UNetConnection * InConnection, int32 ChannelIndex, bool bOpenedLocally) override;
	virtual void Close() override;
	//Requires source changes to be virtual in base class.
	virtual bool ReplicateActor() override;
	virtual void SetChannelActor(AActor* InActor) override;

	bool ReplicateSubobject(UObject *Obj, const FReplicationFlags &RepFlags);
	FPropertyChangeState CreateSubobjectChangeState(UActorComponent* Component);

	// Called by SpatialInterop when receiving an update.
	void PreReceiveSpatialUpdate();
	void PostReceiveSpatialUpdate(const TArray<UProperty*>& RepNotifies);

	void PreReceiveSpatialUpdateSubobject(UActorComponent* Component);
	void PostReceiveSpatialUpdateSubobject(UActorComponent* Component, const TArray<UProperty*>& RepNotifies);

	// Distinguishes between channels created for actors that went through the "old" pipeline vs actors that are triggered through SpawnActor() calls.
	//In the future we may not use an actor channel for non-core actors.
	UPROPERTY(transient)
		bool bCoreActor;

protected:
	// UChannel interface
	virtual bool CleanUp(const bool bForDestroy) override;

private:
	void BindToSpatialView();
	void UnbindFromSpatialView() const;

	// Sends a DeleteEntity request to SpatialOS for the underlying entity, if we have authority to do so.
	void DeleteEntityIfAuthoritative();

	void OnReserveEntityIdResponse(const worker::ReserveEntityIdResponseOp& Op);
	void OnCreateEntityResponse(const worker::CreateEntityResponseOp& Op);

	TWeakPtr<worker::Connection> WorkerConnection;
	TWeakPtr<worker::View> WorkerView;
	FEntityId ActorEntityId;

	worker::Dispatcher::CallbackKey ReserveEntityCallback;
	worker::Dispatcher::CallbackKey CreateEntityCallback;

	worker::RequestId<worker::ReserveEntityIdRequest> ReserveEntityIdRequestId;
	worker::RequestId<worker::CreateEntityRequest> CreateEntityRequestId;

	UPROPERTY(transient)
		USpatialNetDriver* SpatialNetDriver;

	FVector LastSpatialPosition;
	TArray<uint8> MigratablePropertyShadowData;

	// If this actor channel is responsible for creating a new entity, this will be set to true during initial replication.
	UPROPERTY(Transient)
		bool bCreatingNewEntity;

private:
	void UpdateSpatialPosition();

	static FVector GetActorSpatialPosition(AActor* Actor);
};