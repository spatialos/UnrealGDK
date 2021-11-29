// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialStrategySystem.h"

#include "LoadBalancing/LoadBalancingStrategy.h"
#include "Schema/AuthorityIntent.h"
#include "Schema/ChangeInterest.h"
#include "Schema/SkeletonEntityManifest.h"
#include "SpatialView/EntityComponentTypes.h"
#include "SpatialView/SpatialOSWorker.h"
#include "Utils/EntityFactory.h"
#include "Utils/InterestFactory.h"

#include "LoadBalancing/LBDataStorage.h"
#include "LoadBalancing/PartitionManager.h"

DEFINE_LOG_CATEGORY(LogSpatialStrategySystem);

namespace SpatialGDK
{
FSpatialStrategySystem::FSpatialStrategySystem(FStrategySystemViews InViews, TUniquePtr<FLoadBalancingStrategy> InStrategy,
											   TUniquePtr<InterestFactory> InInterestF)
	: Views(InViews)
	, InterestF(MoveTemp(InInterestF))
	, ManifestPublisher(InViews.FilledManifestSubView)
	, DataStorages(InViews.LBView)
	, UserDataStorages(InViews.LBView)
	, ServerWorkerDataStorages(InViews.ServerWorkerView)
	, Strategy(MoveTemp(InStrategy))
{
}

void FSpatialStrategySystem::Init(ViewCoordinator& Coordinator)
{
	PartitionsMgr = MakeUnique<SpatialGDK::FPartitionManager>(Views.ServerWorkerView, Coordinator, *InterestF);
	PartitionsMgr->Init(Coordinator);

	FLoadBalancingSharedData SharedData(*PartitionsMgr, ActorSetSystem, ManifestPublisher, *InterestF);
	Strategy->Init(Coordinator, SharedData, UserDataStorages.DataStorages, ServerWorkerDataStorages.DataStorages);
	DataStorages.DataStorages.Add(&AuthACKView);
	DataStorages.DataStorages.Add(&NetOwningClientView);
	DataStorages.DataStorages.Add(&SetMemberView);

	UpdatesToConsider = DataStorages.GetComponentsToWatch();
	UpdatesToConsider = UpdatesToConsider.Union(UserDataStorages.GetComponentsToWatch());
	bStrategySystemInterestDirty = true;

	UpdateStrategySystemInterest(Coordinator);
}

FSpatialStrategySystem::~FSpatialStrategySystem() = default;

void FSpatialStrategySystem::Advance(ISpatialOSWorker& Connection)
{
	PartitionsMgr->AdvanceView(Connection);

	if (!PartitionsMgr->IsReady())
	{
		return;
	}

	TArray<FLBWorkerHandle> DisconnectedWorkers = PartitionsMgr->GetDisconnectedWorkers();

	if (DisconnectedWorkers.Num() > 0)
	{
		Strategy->OnWorkersDisconnected(DisconnectedWorkers);
	}

	TArray<FLBWorkerHandle> ConnectedWorkers = PartitionsMgr->GetConnectedWorkers();

	if (ConnectedWorkers.Num() > 0)
	{
		Strategy->OnWorkersConnected(ConnectedWorkers);
	}

	for (auto const& Delta : Views.SkeletonManifestView.GetViewDelta().EntityDeltas)
	{
		if (Delta.Type == EntityDelta::ADD)
		{
			const EntityViewElement* Element = Views.SkeletonManifestView.GetView().Find(Delta.EntityId);
			if (!ensure(Element != nullptr))
			{
				continue;
			}
			const ComponentData* ManifestData =
				Element->Components.FindByPredicate(ComponentIdEquality({ SpatialConstants::SKELETON_ENTITY_MANIFEST_COMPONENT_ID }));
			if (!ensure(ManifestData != nullptr))
			{
				continue;
			}
			Strategy->OnSkeletonManifestReceived(Delta.EntityId, FSkeletonEntityManifest(*ManifestData));
		}
	}

	Strategy->Advance(Connection, DataStorages.EntitiesRemoved);
	if (!Strategy->IsReady())
	{
		return;
	}

	ManifestPublisher.Advance(Connection);
	DataStorages.Advance();
	UserDataStorages.Advance();
	ServerWorkerDataStorages.Advance();

	TSet<Worker_EntityId_Key> RemovedEntities;
	for (const EntityDelta& Delta : Views.LBView.GetViewDelta().EntityDeltas)
	{
		switch (Delta.Type)
		{
		case EntityDelta::ADD:
		{
			const SpatialGDK::EntityViewElement& Element = Views.LBView.GetView().FindChecked(Delta.EntityId);
			{
				const SpatialGDK::ComponentData* IntentData = Element.Components.FindByPredicate(
					SpatialGDK::ComponentIdEquality{ SpatialConstants::AUTHORITY_INTENTV2_COMPONENT_ID });
				check(IntentData);
				AuthorityIntentView.Add(Delta.EntityId, AuthorityIntentV2(IntentData->GetUnderlying()));
			}

			{
				const SpatialGDK::ComponentData* DelegationData = Element.Components.FindByPredicate(
					SpatialGDK::ComponentIdEquality{ SpatialConstants::AUTHORITY_DELEGATION_COMPONENT_ID });
				check(DelegationData);
				AuthorityDelegationView.Add(Delta.EntityId, AuthorityDelegation(DelegationData->GetUnderlying()));
			}
		}
		break;
		case EntityDelta::REMOVE:
		{
			AuthorityIntentView.Remove(Delta.EntityId);
			AuthorityDelegationView.Remove(Delta.EntityId);
			PendingMigrations.Remove(Delta.EntityId);
			RemovedEntities.Add(Delta.EntityId);
		}
		break;
		default:
			break;
		}
	}

	ActorSetSystem.Update(SetMemberView, RemovedEntities);
}

void FSpatialStrategySystem::Flush(ISpatialOSWorker& Connection)
{
	// Update worker's interest if needed (now, only when additional data storage has been added)
	if (bStrategySystemInterestDirty && Strategy->IsReady())
	{
		UpdateStrategySystemInterest(Connection);
	}

	// Manage updates to partitions
	Strategy->TickPartitions();
	PartitionsMgr->Flush(Connection);

	if (!Strategy->IsReady())
	{
		return;
	}

	// Iterator over the data storage to collect the entities which have been modified this frame.
	TSet<Worker_EntityId_Key> ModifiedEntities = ActorSetSystem.GetEntitiesToEvaluate();
	for (auto& Storage : UserDataStorages.DataStorages)
	{
		ModifiedEntities = ModifiedEntities.Union(Storage->GetModifiedEntities());
	}

	for (auto Iterator = ModifiedEntities.CreateIterator(); Iterator; ++Iterator)
	{
		if (ActorSetSystem.GetSetLeader(*Iterator) != SpatialConstants::INVALID_ENTITY_ID)
		{
			Iterator.RemoveCurrent();
			continue;
		}
		if (DataStorages.EntitiesRemoved.Contains(*Iterator))
		{
			Iterator.RemoveCurrent();
			continue;
		}
	}

	// Ask the Strategy about the entities that need migration.
	FMigrationContext Ctx(MigratingEntities, ModifiedEntities, DataStorages.EntitiesRemoved);
	Strategy->CollectEntitiesToMigrate(Ctx);

	// If there were pending migrations, meld them with the migration requests
	for (auto PendingMigration : PendingMigrations)
	{
		if (!Ctx.EntitiesToMigrate.Contains(PendingMigration.Key) && !DataStorages.EntitiesRemoved.Contains(PendingMigration.Key))
		{
			Ctx.EntitiesToMigrate.Add(PendingMigration);
		}
	}
	PendingMigrations.Empty();

	// Filter the set considering set membership
	for (auto Iterator = Ctx.EntitiesToMigrate.CreateIterator(); Iterator; ++Iterator)
	{
		Worker_EntityId EntityId = Iterator->Key;
		if (ActorSetSystem.GetSetLeader(EntityId) != SpatialConstants::INVALID_ENTITY_ID)
		{
			Iterator.RemoveCurrent();
		}
	}

	{
		// Augment the migrations with the set information.
		auto MinimalMigrations = Ctx.EntitiesToMigrate;
		for (const auto& Migration : MinimalMigrations)
		{
			Worker_EntityId EntityId = Migration.Key;
			if (auto Set = ActorSetSystem.GetSet(EntityId))
			{
				for (auto ActorInSet : *Set)
				{
					Ctx.EntitiesToMigrate.Add(ActorInSet, Migration.Value);
				}
			}
		}
	}

	{
		// Attach actors that just joined a set to their leader
		for (Worker_EntityId ToAttach : ActorSetSystem.GetEntitiesToAttach())
		{
			if (Ctx.EntitiesToMigrate.Contains(ToAttach))
			{
				continue;
			}
			Worker_EntityId Leader = ActorSetSystem.GetSetLeader(ToAttach);
			if (!ensure(Leader != SpatialConstants::INVALID_ENTITY_ID))
			{
				continue;
			}

			FPartitionHandle* LeaderPartition = EntityAssignment.Find(Leader);
			if (!ensureMsgf(LeaderPartition != nullptr,
							TEXT("Leader should have been evaluated, and should be contained in EntitiesToMigrate")))
			{
				continue;
			}
			Ctx.EntitiesToMigrate.Add(ToAttach, *LeaderPartition);
		}
	}

	for (auto DeletedEntity : Ctx.DeletedEntities)
	{
		EntityAssignment.Remove(DeletedEntity);
	}

	for (const auto& Migration : Ctx.EntitiesToMigrate)
	{
		Worker_EntityId EntityId = Migration.Key;

		FPartitionHandle Partition = Migration.Value;
		EntityAssignment.Add(EntityId, Partition);
		TOptional<Worker_PartitionId> DestPartition = PartitionsMgr->GetPartitionId(Partition);
		AuthorityIntentV2& AuthIntent = AuthorityIntentView.FindChecked(EntityId);
		// The destination partition is not ready to accept actors, push the migration into the pending map.
		if (!DestPartition)
		{
			PendingMigrations.Add(EntityId, Partition);
			continue;
		}
		PendingMigrations.Remove(EntityId);
		AuthIntent.PartitionId = DestPartition.GetValue();
		AuthIntent.AssignmentCounter++;
		if (MigratingEntities.Contains(EntityId))
		{
			UE_LOG(LogSpatialStrategySystem, Verbose, TEXT("In-flight modification"));
		}

		// Send an update to the current authoritative worker, to release auth.
		ComponentUpdate Update(OwningComponentUpdatePtr(AuthIntent.CreateAuthorityIntentUpdate().schema_type),
							   AuthorityIntentV2::ComponentId);
		Connection.SendComponentUpdate(EntityId, MoveTemp(Update), {});
		MigratingEntities.Add(EntityId);
	}

	TSet<Worker_EntityId_Key> EntitiesToUpdate = NetOwningClientView.GetModifiedEntities();
	EntitiesToUpdate.Append(AuthACKView.GetModifiedEntities());

	// Process the entities for which the migration was acked by the current authoritative worker.
	for (auto EntityToMigrate : AuthACKView.GetModifiedEntities())
	{
		const AuthorityIntentACK& AuthACK = AuthACKView.GetObjects().FindChecked(EntityToMigrate);
		AuthorityIntentV2& AuthIntent = AuthorityIntentView.FindChecked(EntityToMigrate);
		if (AuthACK.AssignmentCounter == AuthIntent.AssignmentCounter)
		{
			AuthorityDelegation& AuthDelegation = AuthorityDelegationView.FindChecked(EntityToMigrate);
			if (AuthIntent.PartitionId != 0)
			{
				AuthDelegation.Delegations.Add(SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID, AuthIntent.PartitionId);
			}
			MigratingEntities.Remove(EntityToMigrate);
		}
	}
	AuthACKView.ClearModified();

	// Process the entities needing a change to client ownership
	for (auto EntityToUpdateOwner : NetOwningClientView.GetModifiedEntities())
	{
		const NetOwningClientWorker& NetOwnerData = NetOwningClientView.GetObjects().FindChecked(EntityToUpdateOwner);
		AuthorityDelegation& AuthDelegation = AuthorityDelegationView.FindChecked(EntityToUpdateOwner);
		Worker_PartitionId NewOwner =
			NetOwnerData.ClientPartitionId.IsSet() ? NetOwnerData.ClientPartitionId.GetValue() : SpatialConstants::INVALID_PARTITION_ID;
		AuthDelegation.Delegations.Add(SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID, NewOwner);
	}
	NetOwningClientView.ClearModified();

	// Send the actual authority delegation updates
	for (auto EntityToUpdate : EntitiesToUpdate)
	{
		AuthorityDelegation& AuthDelegation = AuthorityDelegationView.FindChecked(EntityToUpdate);
		ComponentUpdate Update(OwningComponentUpdatePtr(AuthDelegation.CreateAuthorityDelegationUpdate().schema_type),
							   SpatialConstants::AUTHORITY_DELEGATION_COMPONENT_ID);
		Connection.SendComponentUpdate(EntityToUpdate, MoveTemp(Update), {});
	}

	Strategy->Flush(Connection);
}

void FSpatialStrategySystem::Destroy(ISpatialOSWorker& Connection) {}

void FSpatialStrategySystem::UpdateStrategySystemInterest(ISpatialOSWorker& Connection)
{
	if (GetDefault<USpatialGDKSettings>()->bUserSpaceServerInterest)
	{
		CreateUSIQuery(Connection);
		return;
	}

	if (!PartitionsMgr->IsReady())
	{
		return;
	}

	Interest ServerInterest;

	ComponentSetInterest& InterestSet = ServerInterest.ComponentInterestMap.Add(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID);
	{
		Query ServerQuery = {};
		ServerQuery.ResultComponentIds = { SpatialConstants::STRATEGYWORKER_TAG_COMPONENT_ID };
		for (const Worker_ComponentId& Component  : UpdatesToConsider)
		{
			ServerQuery.ResultComponentIds.Add(Component);
		}
		ServerQuery.Constraint.ComponentConstraint = SpatialConstants::STRATEGYWORKER_TAG_COMPONENT_ID;

		InterestSet.Queries.Add(ServerQuery);
	}

	{
		Query ServerQuery = {};
		ServerQuery.ResultComponentIds = { SpatialConstants::SERVER_WORKER_COMPONENT_ID };
		for (const Worker_ComponentId& Component : ServerWorkerDataStorages.GetComponentsToWatch())
		{
			ServerQuery.ResultComponentIds.Add(Component);
		}
		ServerQuery.Constraint.ComponentConstraint = SpatialConstants::SERVER_WORKER_COMPONENT_ID;
		InterestSet.Queries.Add(ServerQuery);
	}

	{
		Query ServerQuery = {};
		ServerQuery.ResultComponentIds = { SpatialConstants::WORKER_COMPONENT_ID };
		ServerQuery.Constraint.ComponentConstraint = SpatialConstants::WORKER_COMPONENT_ID;
		InterestSet.Queries.Add(ServerQuery);
	}

	{
		Query ServerQuery = {};
		ServerQuery.ResultComponentIds = { SpatialConstants::PARTITION_ACK_COMPONENT_ID };
		ServerQuery.Constraint.ComponentConstraint = SpatialConstants::PARTITION_ACK_COMPONENT_ID;
		InterestSet.Queries.Add(ServerQuery);
	}

	{
		Query SkeletonEntityManifestsQuery = {};
		SkeletonEntityManifestsQuery.ResultComponentIds = { SpatialConstants::SKELETON_ENTITY_MANIFEST_COMPONENT_ID };
		SkeletonEntityManifestsQuery.Constraint.ComponentConstraint = SpatialConstants::SKELETON_ENTITY_MANIFEST_COMPONENT_ID;
		InterestSet.Queries.Add(SkeletonEntityManifestsQuery);
	}

	ComponentUpdate Update(OwningComponentUpdatePtr(ServerInterest.CreateInterestUpdate().schema_type), Interest::ComponentId);
	Connection.SendComponentUpdate(SpatialConstants::INITIAL_STRATEGY_PARTITION_ENTITY_ID, MoveTemp(Update), {});

	bStrategySystemInterestDirty = false;
}

void FSpatialStrategySystem::CreateUSIQuery(ISpatialOSWorker& Connection)
{
	ChangeInterestRequest Request;
	Request.SystemEntityId = Connection.GetWorkerSystemEntityId();
	Request.bOverwrite = true;

	{
		ChangeInterestQuery StrategyQuery;
		StrategyQuery.TrueConstraint = true;
		StrategyQuery.Components.Add(SpatialConstants::STRATEGYWORKER_TAG_COMPONENT_ID);
		for (const Worker_ComponentId& Component : UpdatesToConsider)
		{
			StrategyQuery.Components.Add(Component);
		}

		StrategyQuery.Components.Add(SpatialConstants::SERVER_WORKER_COMPONENT_ID);
		for (const Worker_ComponentId& Component : ServerWorkerDataStorages.GetComponentsToWatch())
		{
			StrategyQuery.Components.Add(Component);
		}

		StrategyQuery.Components.Add(SpatialConstants::WORKER_COMPONENT_ID);
		StrategyQuery.Components.Add(SpatialConstants::PARTITION_ACK_COMPONENT_ID);
		StrategyQuery.Components.Add(SpatialConstants::SKELETON_ENTITY_MANIFEST_COMPONENT_ID);

		Request.QueriesToAdd.Add(MoveTemp(StrategyQuery));
	}

	Request.SendRequest(Connection);

	bStrategySystemInterestDirty = false;
}

} // namespace SpatialGDK
