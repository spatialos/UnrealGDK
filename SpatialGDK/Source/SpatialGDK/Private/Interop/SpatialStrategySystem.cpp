// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialStrategySystem.h"

#include "LoadBalancing/LoadBalancingStrategy.h"
#include "Schema/AuthorityIntent.h"
#include "SpatialView/EntityComponentTypes.h"
#include "SpatialView/SpatialOSWorker.h"
#include "Utils/EntityFactory.h"
#include "Utils/InterestFactory.h"

#include "LoadBalancing/LBDataStorage.h"
#include "LoadBalancing/PartitionManager.h"

DEFINE_LOG_CATEGORY(LogSpatialStrategySystem);

namespace SpatialGDK
{
FSpatialStrategySystem::FSpatialStrategySystem(TUniquePtr<FPartitionManager> InPartitionsMgr, const FSubView& InLBView,
											   const FSubView& InServerWorkerView, TUniquePtr<FLoadBalancingStrategy> InStrategy)
	: LBView(InLBView)
	, PartitionsMgr(MoveTemp(InPartitionsMgr))
	, ActorInfo(ActorSetSystem, PCData)
	, DataStorages(InLBView)
	, UserDataStorages(InLBView)
	, ServerWorkerDataStorages(InServerWorkerView)
	, Strategy(MoveTemp(InStrategy))
{
	Strategy->Init(ActorInfo, UserDataStorages.DataStorages, ServerWorkerDataStorages.DataStorages);
	DataStorages.DataStorages.Add(&AuthACKView);
	DataStorages.DataStorages.Add(&NetOwningClientView);
	DataStorages.DataStorages.Add(&SetMemberView);
	DataStorages.DataStorages.Add(&PCData);

	UpdatesToConsider = DataStorages.GetComponentsToWatch();
	UpdatesToConsider = UpdatesToConsider.Union(UserDataStorages.GetComponentsToWatch());
	bStrategySystemInterestDirty = true;
}

FSpatialStrategySystem::~FSpatialStrategySystem() = default;

void FSpatialStrategySystem::Advance(ISpatialOSWorker& Connection)
{
	PartitionsMgr->AdvanceView(Connection);

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

	DataStorages.Advance();
	UserDataStorages.Advance();
	ServerWorkerDataStorages.Advance();

	TSet<Worker_EntityId_Key> RemovedEntities;
	for (const EntityDelta& Delta : LBView.GetViewDelta().EntityDeltas)
	{
		switch (Delta.Type)
		{
		case EntityDelta::ADD:
		{
			const SpatialGDK::EntityViewElement& Element = LBView.GetView().FindChecked(Delta.EntityId);
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
	SetMemberView.ClearModified();

	Strategy->Advance(Connection, LBView);
}

void FSpatialStrategySystem::ClearUserStorages()
{
	for (auto& Storage : UserDataStorages.DataStorages)
	{
		Storage->ClearModified();
	}
	for (auto& Storage : ServerWorkerDataStorages.DataStorages)
	{
		Storage->ClearModified();
	}
	DataStorages.EntitiesAdded.Empty();
	DataStorages.EntitiesRemoved.Empty();
	UserDataStorages.EntitiesAdded.Empty();
	UserDataStorages.EntitiesRemoved.Empty();
	ServerWorkerDataStorages.EntitiesAdded.Empty();
	ServerWorkerDataStorages.EntitiesRemoved.Empty();
}

void FSpatialStrategySystem::Flush(ISpatialOSWorker& Connection)
{
	// Update worker's interest if needed (now, only when additional data storage has been added)
	if (bStrategySystemInterestDirty)
	{
		UpdateStrategySystemInterest(Connection);
	}

	// Manage updates to partitions
	Strategy->TickPartitions(*PartitionsMgr);
	PartitionsMgr->Flush(Connection);

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

	// Clear the buffer of modified entities now that the strategy has been informed.
	ClearUserStorages();

	ActorSetSystem.ClearEntitiesToEvaluate();

	// If there were pending migrations, meld them with the migration requests
	for (auto PendingMigration : PendingMigrations)
	{
		if (!Ctx.EntitiesToMigrate.Contains(PendingMigration.Key))
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

	for (const auto& Migration : Ctx.EntitiesToMigrate)
	{
		Worker_EntityId EntityId = Migration.Key;

		FPartitionHandle Partition = Migration.Value;
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
			UE_LOG(LogSpatialStrategySystem, Log, TEXT("In-flight modification"));
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
			AuthDelegation.Delegations.Add(SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID, AuthIntent.PartitionId);
			if (!PCData.GetObjects().Contains(EntityToMigrate))
			{
				AuthDelegation.Delegations.Add(SpatialConstants::INTEREST_AUTH_COMPONENT_SET_ID, AuthIntent.PartitionId);
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
	if (!PartitionsMgr->IsReady())
	{
		return;
	}

	Interest ServerInterest;

	ComponentSetInterest& InterestSet = ServerInterest.ComponentInterestMap.Add(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID);
	{
		Query ServerQuery = {};
		ServerQuery.ResultComponentIds = { SpatialConstants::STRATEGYWORKER_TAG_COMPONENT_ID };
		for (auto& Component : UpdatesToConsider)
		{
			ServerQuery.ResultComponentIds.Add(Component);
		}
		ServerQuery.Constraint.ComponentConstraint = SpatialConstants::STRATEGYWORKER_TAG_COMPONENT_ID;

		InterestSet.Queries.Add(ServerQuery);
	}

	{
		Query ServerQuery = {};
		ServerQuery.ResultComponentIds = { SpatialConstants::SERVER_WORKER_COMPONENT_ID };
		for (auto& Component : ServerWorkerDataStorages.GetComponentsToWatch())
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

	ComponentUpdate Update(OwningComponentUpdatePtr(ServerInterest.CreateInterestUpdate().schema_type), Interest::ComponentId);
	Connection.SendComponentUpdate(SpatialConstants::INITIAL_STRATEGY_PARTITION_ENTITY_ID, MoveTemp(Update), {});

	bStrategySystemInterestDirty = false;
}

} // namespace SpatialGDK
