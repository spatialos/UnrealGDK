// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/WellKnownEntitySystem.h"

#include "Interop/SpatialReceiver.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Schema/ServerWorker.h"
#include "Utils/ComponentFactory.h"

DEFINE_LOG_CATEGORY(LogWellKnownEntitySystem);

namespace SpatialGDK
{
WellKnownEntitySystem::WellKnownEntitySystem(const FSubView& SubView, USpatialNetDriver* InNetDriver,
											 USpatialWorkerConnection* InConnection, const int InNumberOfWorkers,
											 SpatialVirtualWorkerTranslator& InVirtualWorkerTranslator,
											 UGlobalStateManager& InGlobalStateManager)
	: SubView(&SubView)
	, NetDriver(InNetDriver)
	, VirtualWorkerTranslator(&InVirtualWorkerTranslator)
	, GlobalStateManager(&InGlobalStateManager)
	, Connection(InConnection)
	, NumberOfWorkers(InNumberOfWorkers)
{
}

void WellKnownEntitySystem::Advance()
{
	const FSubViewDelta& SubViewDelta = SubView->GetViewDelta();
	for (const EntityDelta& Delta : SubViewDelta.EntityDeltas)
	{
		switch (Delta.Type)
		{
		case EntityDelta::UPDATE:
		{
			for (const ComponentChange& Change : Delta.ComponentUpdates)
			{
				ProcessComponentUpdate(Change.ComponentId, Change.Update);
			}
			for (const ComponentChange& Change : Delta.ComponentsAdded)
			{
				ProcessComponentAdd(Change.ComponentId, Change.Data);
			}
			for (const AuthorityChange& Change : Delta.AuthorityGained)
			{
				ProcessAuthorityGain(Delta.EntityId, Change.ComponentSetId);
			}
			break;
		}
		case EntityDelta::ADD:
			ProcessEntityAdd(Delta.EntityId);
			break;
		default:
			break;
		}
	}
}

void WellKnownEntitySystem::ProcessComponentUpdate(const Worker_ComponentId ComponentId, Schema_ComponentUpdate* Update)
{
	switch (ComponentId)
	{
	case SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID:
		VirtualWorkerTranslator->ApplyVirtualWorkerManagerData(Schema_GetComponentUpdateFields(Update));
		break;
	case SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID:
		GlobalStateManager->ApplyDeploymentMapUpdate(Update);
		break;
	case SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID:
		GlobalStateManager->ApplyStartupActorManagerUpdate(Update);
		break;
	case SpatialConstants::GSM_SHUTDOWN_COMPONENT_ID:
#if WITH_EDITOR
		GlobalStateManager->OnShutdownComponentUpdate(Update);
#endif // WITH_EDITOR
		break;
	default:
		break;
	}
}

void WellKnownEntitySystem::ProcessComponentAdd(const Worker_ComponentId ComponentId, Schema_ComponentData* Data)
{
	switch (ComponentId)
	{
	case SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID:
		VirtualWorkerTranslator->ApplyVirtualWorkerManagerData(Schema_GetComponentDataFields(Data));
		break;
	case SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID:
		GlobalStateManager->ApplyDeploymentMapData(Data);
		break;
	case SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID:
		GlobalStateManager->ApplyStartupActorManagerData(Data);
		break;
	case SpatialConstants::SERVER_WORKER_COMPONENT_ID:
		MaybeClaimSnapshotPartition();
		break;
	default:
		break;
	}
}

void WellKnownEntitySystem::ProcessAuthorityGain(const Worker_EntityId EntityId, const Worker_ComponentSetId ComponentSetId)
{
	GlobalStateManager->AuthorityChanged({ EntityId, ComponentSetId, WORKER_AUTHORITY_AUTHORITATIVE });

	if (SubView->GetView()[EntityId].Components.ContainsByPredicate(ComponentIdEquality{ SpatialConstants::SERVER_WORKER_COMPONENT_ID }))
	{
		GlobalStateManager->TrySendWorkerReadyToBeginPlay();
	}

	if (SubView->GetView()[EntityId].Components.ContainsByPredicate(
			ComponentIdEquality{ SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID }))
	{
		InitializeVirtualWorkerTranslationManager();
		VirtualWorkerTranslationManager->AuthorityChanged({ EntityId, ComponentSetId, WORKER_AUTHORITY_AUTHORITATIVE });
	}
}

void WellKnownEntitySystem::ProcessEntityAdd(const Worker_EntityId EntityId)
{
	const EntityViewElement& Element = SubView->GetView()[EntityId];
	for (const ComponentData& ComponentData : Element.Components)
	{
		ProcessComponentAdd(ComponentData.GetComponentId(), ComponentData.GetUnderlying());
	}
	for (const Worker_ComponentSetId ComponentId : Element.Authority)
	{
		ProcessAuthorityGain(EntityId, ComponentId);
	}
}

// This is only called if this worker has been selected by SpatialOS to be authoritative
// for the TranslationManager, otherwise the manager will never be instantiated.
void WellKnownEntitySystem::InitializeVirtualWorkerTranslationManager()
{
	VirtualWorkerTranslationManager =
		MakeUnique<SpatialVirtualWorkerTranslationManager>(NetDriver->Receiver, Connection, VirtualWorkerTranslator);
	VirtualWorkerTranslationManager->SetNumberOfVirtualWorkers(NumberOfWorkers);
}

void WellKnownEntitySystem::MaybeClaimSnapshotPartition()
{
	// Perform a naive leader election where we wait for the correct number of server workers to be present in the deployment, and then
	// whichever server has the lowest server worker entity ID becomes the leader and claims the snapshot partition.
	const Worker_EntityId LocalServerWorkerEntityId = GlobalStateManager->GetLocalServerWorkerEntityId();

	if (LocalServerWorkerEntityId == SpatialConstants::INVALID_ENTITY_ID)
	{
		UE_LOG(LogWellKnownEntitySystem, Warning, TEXT("MaybeClaimSnapshotPartition aborted due to lack of local server worker entity"));
		return;
	}

	Worker_EntityId LowestEntityId = LocalServerWorkerEntityId;

	int ServerCount = 0;
	for (const auto& Iter : SubView->GetView())
	{
		const Worker_EntityId EntityId = Iter.Key;
		const SpatialGDK::EntityViewElement& Element = Iter.Value;
		if (Element.Components.ContainsByPredicate([](const SpatialGDK::ComponentData& CompData) {
				return CompData.GetComponentId() == SpatialConstants::SERVER_WORKER_COMPONENT_ID;
			}))
		{
			ServerCount++;

			if (EntityId < LowestEntityId)
			{
				LowestEntityId = EntityId;
			}
		}
	}

	if (LocalServerWorkerEntityId == LowestEntityId && ServerCount >= NumberOfWorkers)
	{
		UE_LOG(LogWellKnownEntitySystem, Log, TEXT("MaybeClaimSnapshotPartition claiming snapshot partition"));
		GlobalStateManager->ClaimSnapshotPartition();
	}

	if (ServerCount > NumberOfWorkers)
	{
		UE_LOG(LogWellKnownEntitySystem, Warning,
			   TEXT("MaybeClaimSnapshotPartition found too many server worker entities, expected %d got %d."), NumberOfWorkers,
			   ServerCount);
	}
}

void WellKnownEntitySystem::CreateServerWorkerEntity()
{
	RetryServerWorkerEntityCreation(NetDriver->PackageMap->AllocateEntityId(), 1);
}

// Creates an entity authoritative on this server worker, ensuring it will be able to receive updates for the GSM.
void WellKnownEntitySystem::RetryServerWorkerEntityCreation(Worker_EntityId EntityId, int AttemptCounter)
{
	check(NetDriver != nullptr);

	TArray<FWorkerComponentData> Components;
	Components.Add(Position().CreateComponentData());
	Components.Add(Metadata(FString::Format(TEXT("WorkerEntity:{0}"), { Connection->GetWorkerId() })).CreateComponentData());
	Components.Add(ServerWorker(Connection->GetWorkerId(), false, Connection->GetWorkerSystemEntityId()).CreateServerWorkerData());

	AuthorityDelegationMap DelegationMap;
	DelegationMap.Add(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID, EntityId);
	Components.Add(AuthorityDelegation(DelegationMap).CreateComponentData());

	check(NetDriver != nullptr);

	// The load balance strategy won't be set up at this point, but we call this function again later when it is ready in
	// order to set the interest of the server worker according to the strategy.
	Components.Add(NetDriver->InterestFactory->CreateServerWorkerInterest(NetDriver->LoadBalanceStrategy).CreateComponentData());

	// GDK known entities completeness tags.
	Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::GDK_KNOWN_ENTITY_TAG_COMPONENT_ID));

	const Worker_RequestId RequestId = Connection->SendCreateEntityRequest(MoveTemp(Components), &EntityId, RETRY_UNTIL_COMPLETE);

	CreateEntityDelegate OnCreateWorkerEntityResponse;
	OnCreateWorkerEntityResponse.BindLambda([this, EntityId, AttemptCounter](const Worker_CreateEntityResponseOp& Op) {
		if (NetDriver == nullptr)
		{
			return;
		}

		if (Op.status_code == WORKER_STATUS_CODE_SUCCESS)
		{
			NetDriver->WorkerEntityId = Op.entity_id;

			// We claim each server worker entity as a partition for server worker interest. This is necessary for getting
			// interest in the VirtualWorkerTranslator component.
			SendClaimPartitionRequest(NetDriver->Connection->GetWorkerSystemEntityId(), Op.entity_id);

			return;
		}

		// Given the nature of commands, it's possible we have multiple create commands in flight at once. If a command fails where
		// we've already set the worker entity ID locally, this means we already successfully create the entity, so nothing needs doing.
		if (Op.status_code != WORKER_STATUS_CODE_SUCCESS && NetDriver->WorkerEntityId != SpatialConstants::INVALID_ENTITY_ID)
		{
			return;
		}

		if (Op.status_code != WORKER_STATUS_CODE_TIMEOUT)
		{
			UE_LOG(LogSpatialSender, Error, TEXT("Worker entity creation request failed: \"%s\""), UTF8_TO_TCHAR(Op.message));
			return;
		}

		if (AttemptCounter == SpatialConstants::MAX_NUMBER_COMMAND_ATTEMPTS)
		{
			UE_LOG(LogSpatialSender, Error, TEXT("Worker entity creation request timed out too many times. (%u attempts)"),
				   SpatialConstants::MAX_NUMBER_COMMAND_ATTEMPTS);
			return;
		}

		UE_LOG(LogSpatialSender, Warning, TEXT("Worker entity creation request timed out and will retry."));
		RetryServerWorkerEntityCreation(EntityId, AttemptCounter + 1);
	});

	NetDriver->Receiver->AddCreateEntityDelegate(RequestId, MoveTemp(OnCreateWorkerEntityResponse));
}

void WellKnownEntitySystem::SendClaimPartitionRequest(Worker_EntityId SystemWorkerEntityId, Worker_PartitionId PartitionId) const
{
	UE_LOG(LogSpatialSender, Log,
		   TEXT("SendClaimPartitionRequest. Worker: %s, SystemWorkerEntityId: %lld. "
				"PartitionId: %lld"),
		   *Connection->GetWorkerId(), SystemWorkerEntityId, PartitionId);
	Worker_CommandRequest CommandRequest = Worker::CreateClaimPartitionRequest(PartitionId);
	const Worker_RequestId RequestId = Connection->SendCommandRequest(SystemWorkerEntityId, &CommandRequest, RETRY_UNTIL_COMPLETE, {});
	NetDriver->Receiver->PendingPartitionAssignments.Add(RequestId, PartitionId);
}

void WellKnownEntitySystem::UpdatePartitionEntityInterestAndPosition()
{
	check(Connection != nullptr);
	check(NetDriver != nullptr);
	check(NetDriver->VirtualWorkerTranslator != nullptr
		  && NetDriver->VirtualWorkerTranslator->GetClaimedPartitionId() != SpatialConstants::INVALID_ENTITY_ID);
	check(NetDriver->LoadBalanceStrategy != nullptr && NetDriver->LoadBalanceStrategy->IsReady());

	Worker_PartitionId PartitionId = NetDriver->VirtualWorkerTranslator->GetClaimedPartitionId();
	VirtualWorkerId VirtualId = NetDriver->VirtualWorkerTranslator->GetLocalVirtualWorkerId();

	// Update the interest. If it's ready and not null, also adds interest according to the load balancing strategy.
	FWorkerComponentUpdate InterestUpdate =
		NetDriver->InterestFactory
			->CreatePartitionInterest(NetDriver->LoadBalanceStrategy, VirtualId, NetDriver->DebugCtx != nullptr /*bDebug*/)
			.CreateInterestUpdate();

	Connection->SendComponentUpdate(PartitionId, &InterestUpdate);

	// Also update the position of the partition entity to the center of the load balancing region.
	FWorkerComponentUpdate Update =
		Position::CreatePositionUpdate(Coordinates::FromFVector(NetDriver->LoadBalanceStrategy->GetWorkerEntityPosition()));
	Connection->SendComponentUpdate(PartitionId, &Update);
}

} // Namespace SpatialGDK
