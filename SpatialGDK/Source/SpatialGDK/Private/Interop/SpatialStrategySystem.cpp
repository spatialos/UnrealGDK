// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialStrategySystem.h"

#include "Interop/SpatialOSDispatcherInterface.h"
#include "LoadBalancing/LoadBalancingStrategy.h"
#include "Schema/AuthorityIntent.h"
#include "SpatialView/EntityComponentTypes.h"
#include "Utils/EntityFactory.h"
#include "Utils/InterestFactory.h"

#include "LoadBalancing/LBDataStorage.h"
#include "LoadBalancing/PartitionManager.h"

#pragma optimize("", off)

DEFINE_LOG_CATEGORY(LogSpatialStrategySystem);

namespace SpatialGDK
{
SpatialStrategySystem::SpatialStrategySystem(TUniquePtr<FPartitionManager> InPartitionsMgr, const FSubView& InLBView,
											 TUniquePtr<FLoadBalancingStrategy>&& InStrategy)
	: LBView(InLBView)
	, PartitionsMgr(MoveTemp(InPartitionsMgr))
	, Strategy(MoveTemp(InStrategy))
{
	Strategy->Init(DataStorages);

	for (auto Storage : DataStorages)
	{
		UpdatesToConsider = UpdatesToConsider.Union(Storage->GetComponentsToWatch());
	}

	bStrategySystemInterestDirty = true;
}

SpatialStrategySystem::~SpatialStrategySystem() = default;

void SpatialStrategySystem::Advance(SpatialOSWorkerInterface* Connection)
{
	PartitionsMgr->AdvanceView(Connection);

	TArray<FLBWorker> DisconnectedWorkers = PartitionsMgr->GetDisconnectedWorkers();

	if (DisconnectedWorkers.Num() > 0)
	{
		Strategy->OnWorkersDisconnected(DisconnectedWorkers);
	}

	TArray<FLBWorker> ConnectedWorkers = PartitionsMgr->GetConnectedWorkers();

	if (ConnectedWorkers.Num() > 0)
	{
		Strategy->OnWorkersConnected(ConnectedWorkers);
	}

	for (const EntityDelta& Delta : LBView.GetViewDelta().EntityDeltas)
	{
		switch (Delta.Type)
		{
		case EntityDelta::UPDATE:
		{
			for (const auto& CompleteUpdate : Delta.ComponentsRefreshed) {}
			for (const auto& Update : Delta.ComponentUpdates)
			{
				for (auto& Storage : DataStorages)
				{
					if (Storage->GetComponentsToWatch().Contains(Update.ComponentId))
					{
						Storage->OnUpdate(Delta.EntityId, Update.ComponentId, Update.Update);
					}
				}
				if (Update.ComponentId == SpatialConstants::AUTHORITY_INTENT_ACK_COMPONENT_ID)
				{
					Schema_Object* ACKObject = Schema_GetComponentUpdateFields(Update.Update);
					uint64 AuthEpoch = Schema_GetUint64(ACKObject, SpatialConstants::AUTHORITY_INTENT_ACK_ASSIGNMENT_COUNTER);
					AuthorityIntentV2& AuthIntent = AuthorityIntentView.FindChecked(Delta.EntityId);
					if (AuthEpoch == AuthIntent.AssignmentCounter)
					{
						EntitiesACKMigration.Add(Delta.EntityId);
					}
				}
				if (Update.ComponentId == SpatialConstants::NET_OWNING_CLIENT_WORKER_COMPONENT_ID)
				{
					Schema_Object* NetOwnerObject = Schema_GetComponentUpdateFields(Update.Update);
					NetOwningClientWorker& NetOwner = NetOwningClientView.FindChecked(Delta.EntityId);
					NetOwner.ApplyComponentUpdate(Update.Update);
					EntitiesClientChanged.Add(Delta.EntityId);
				}
			}
		}
		break;
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

			{
				const SpatialGDK::ComponentData* NetOwnerData = Element.Components.FindByPredicate(
					SpatialGDK::ComponentIdEquality{ SpatialConstants::NET_OWNING_CLIENT_WORKER_COMPONENT_ID });
				if (ensureAlwaysMsgf(NetOwnerData, TEXT("Entity %llu missing net owner data"), Delta.EntityId))
				{
					auto& NetOwner = NetOwningClientView.Add(Delta.EntityId, NetOwningClientWorker(NetOwnerData->GetUnderlying()));
					if (NetOwner.ClientPartitionId.IsSet())
					{
						EntitiesClientChanged.Add(Delta.EntityId);
					}
				}
			}

			for (auto& Storage : DataStorages)
			{
				Storage->OnAdded(Delta.EntityId, Element);
			}
		}
		break;
		case EntityDelta::REMOVE:
		{
			for (auto& Storage : DataStorages)
			{
				Storage->OnRemoved(Delta.EntityId);
			}
			AuthorityIntentView.Remove(Delta.EntityId);
		}
		break;
		case EntityDelta::TEMPORARILY_REMOVED:
		{
			for (auto& Storage : DataStorages)
			{
				Storage->OnRemoved(Delta.EntityId);
			}
			const SpatialGDK::EntityViewElement& Element = LBView.GetView().FindChecked(Delta.EntityId);
			for (auto& Storage : DataStorages)
			{
				Storage->OnAdded(Delta.EntityId, Element);
			}
		}
		break;
		default:
			break;
		}
	}
}

void SpatialStrategySystem::Flush(SpatialOSWorkerInterface* Connection)
{
	PartitionsMgr->Flush(Connection);

	if (bStrategySystemInterestDirty)
	{
		UpdateStrategySystemInterest(Connection);
	}

	Strategy->TickPartitions(*PartitionsMgr);

	if (!PartitionsMgr->IsReady())
	{
		return;
	}

	TSet<Worker_EntityId_Key> ModifiedEntities;
	for (auto& Storage : DataStorages)
	{
		ModifiedEntities = ModifiedEntities.Union(Storage->GetModifiedEntities());
	}

	FMigrationContext Ctx(MigratingEntities, ModifiedEntities);

	Strategy->CollectEntitiesToMigrate(Ctx);

	for (auto const& Migration : Ctx.EntitiesToMigrate)
	{
		Worker_EntityId EntityId = Migration.Key;

		if (!ensureAlways(!MigratingEntities.Contains(EntityId)))
		{
			continue;
		}

		FPartitionHandle Partition = Migration.Value;
		TOptional<Worker_PartitionId> DestPartition = PartitionsMgr->GetPartitionId(Partition);
		AuthorityIntentV2& AuthIntent = AuthorityIntentView.FindChecked(EntityId);
		if (!DestPartition)
		{
			// Have to remember the migration request !!
			// Or stop doing that altogether
			continue;
		}
		AuthIntent.PartitionId = DestPartition.GetValue();
		AuthIntent.AssignmentCounter++;

		FWorkerComponentUpdate Update;
		Update.component_id = AuthorityIntentV2::ComponentId;
		Update.schema_type = AuthIntent.CreateAuthorityIntentUpdate().schema_type;
		Connection->SendComponentUpdate(EntityId, &Update, {});
		MigratingEntities.Add(EntityId);
	}

	TSet<Worker_EntityId> EntitiesToUpdate = EntitiesClientChanged;
	EntitiesToUpdate.Append(EntitiesACKMigration);

	for (auto EntityToMigrate : EntitiesACKMigration)
	{
		AuthorityIntentV2& AuthIntent = AuthorityIntentView.FindChecked(EntityToMigrate);
		AuthorityDelegation& AuthDelegation = AuthorityDelegationView.FindChecked(EntityToMigrate);

		AuthDelegation.Delegations.Add(SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID, AuthIntent.PartitionId);
		MigratingEntities.Remove(EntityToMigrate);
	}
	EntitiesACKMigration.Empty();
	for (auto EntityToUpdateOwner : EntitiesClientChanged)
	{
		NetOwningClientWorker& NetOwnerData = NetOwningClientView.FindChecked(EntityToUpdateOwner);
		AuthorityDelegation& AuthDelegation = AuthorityDelegationView.FindChecked(EntityToUpdateOwner);
		Worker_PartitionId NewOwner = NetOwnerData.ClientPartitionId.IsSet() ? NetOwnerData.ClientPartitionId.GetValue() : 0;
		AuthDelegation.Delegations.Add(SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID, NewOwner);
	}
	EntitiesClientChanged.Empty();

	for (auto EntityToUpdate : EntitiesToUpdate)
	{
		AuthorityDelegation& AuthDelegation = AuthorityDelegationView.FindChecked(EntityToUpdate);
		FWorkerComponentUpdate Update;
		Update.component_id = SpatialConstants::AUTHORITY_DELEGATION_COMPONENT_ID;
		Update.schema_type = AuthDelegation.CreateAuthorityDelegationUpdate().schema_type;
		Connection->SendComponentUpdate(EntityToUpdate, &Update, {});
	}
}

void SpatialStrategySystem::Destroy(SpatialOSWorkerInterface* Connection) {}

void SpatialStrategySystem::UpdateStrategySystemInterest(SpatialOSWorkerInterface* Connection)
{
	if (!PartitionsMgr->IsReady())
	{
		return;
	}

	Interest ServerInterest;
	Query ServerQuery = {};
	ServerQuery.ResultComponentIds = {
		SpatialConstants::STRATEGYWORKER_TAG_COMPONENT_ID, SpatialConstants::NET_OWNING_CLIENT_WORKER_COMPONENT_ID,
		/*SpatialConstants::ACTOR_GROUP_MEMBER_COMPONENT_ID,*/ SpatialConstants::ACTOR_SET_MEMBER_COMPONENT_ID,
		SpatialConstants::AUTHORITY_INTENT_ACK_COMPONENT_ID
	};

	for (auto& Component : UpdatesToConsider)
	{
		ServerQuery.ResultComponentIds.Add(Component);
	}

	// ServerQuery.ResultComponentSetIds = { SpatialConstants::SPATIALOS_WELLKNOWN_COMPONENTSET_ID };
	ServerQuery.Constraint.ComponentConstraint = SpatialConstants::STRATEGYWORKER_TAG_COMPONENT_ID;
	ServerInterest.ComponentInterestMap.Add(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID);
	ServerInterest.ComponentInterestMap[SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID].Queries.Add(ServerQuery);

	ServerQuery = {};
	ServerQuery.ResultComponentIds = { SpatialConstants::SERVER_WORKER_COMPONENT_ID };
	ServerQuery.Constraint.ComponentConstraint = SpatialConstants::SERVER_WORKER_COMPONENT_ID;
	ServerInterest.ComponentInterestMap[SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID].Queries.Add(ServerQuery);

	ServerQuery = {};
	ServerQuery.ResultComponentIds = { SpatialConstants::WORKER_COMPONENT_ID };
	ServerQuery.Constraint.ComponentConstraint = SpatialConstants::WORKER_COMPONENT_ID;
	ServerInterest.ComponentInterestMap[SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID].Queries.Add(ServerQuery);

	ServerQuery = {};
	ServerQuery.ResultComponentIds = { SpatialConstants::PARTITION_ACK_COMPONENT_ID };
	ServerQuery.Constraint.ComponentConstraint = SpatialConstants::PARTITION_ACK_COMPONENT_ID;
	ServerInterest.ComponentInterestMap[SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID].Queries.Add(ServerQuery);

	FWorkerComponentUpdate Update;
	Update.component_id = Interest::ComponentId;
	Update.schema_type = ServerInterest.CreateInterestUpdate().schema_type;
	Connection->SendComponentUpdate(SpatialConstants::INITIAL_STRATEGY_PARTITION_ENTITY_ID, &Update, {});

	bStrategySystemInterestDirty = false;
}

} // namespace SpatialGDK
