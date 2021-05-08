// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialStrategySystem.h"

#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/Connection/SpatialOSWorkerInterface.h"
#include "Interop/SpatialOSDispatcherInterface.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "LoadBalancing/LoadBalancingCalculator.h"
#include "Schema/AuthorityIntent.h"
#include "Schema/ServerWorker.h"
#include "Utils/EntityFactory.h"
#include "Utils/InterestFactory.h"

#pragma optimize("", off)

DEFINE_LOG_CATEGORY(LogSpatialStrategySystem);

namespace SpatialGDK
{
SpatialStrategySystem::SpatialStrategySystem(const FSubView& InLBView, const FSubView& InWorkerView, const FSubView& InPartitionView,
											 Worker_EntityId InStrategyWorkerEntityId, SpatialOSWorkerInterface* Connection,
											 UAbstractLBStrategy& InStrategy, SpatialVirtualWorkerTranslator& InTranslator,
											 TUniquePtr<InterestFactory>&& InInterestF)
	: LBView(InLBView)
	, WorkerView(InWorkerView)
	, PartitionView(InPartitionView)
	, StrategyWorkerEntityId(InStrategyWorkerEntityId)
	, StrategyPartitionEntityId(SpatialConstants::INITIAL_STRATEGY_PARTITION_ENTITY_ID)
	, Strategy(InStrategy)
	, Translator(InTranslator)
	, InterestF(MoveTemp(InInterestF))
{
	Worker_CommandRequest ClaimRequest = Worker::CreateClaimPartitionRequest(StrategyPartitionEntityId);
	StrategyWorkerRequest = Connection->SendCommandRequest(StrategyWorkerEntityId, &ClaimRequest, SpatialGDK::RETRY_UNTIL_COMPLETE, {});

	NumPartitions = Strategy.GetMinimumRequiredWorkers();
	PartitionCreationRequests.Add(Connection->SendReserveEntityIdsRequest(NumPartitions, RETRY_UNTIL_COMPLETE));
	StrategyCalculator = Strategy.CreateLoadBalancingCalculator();
	StrategyCalculator->CollectComponentsToInspect(UpdatesToConsider);
	TArray<TSharedPtr<FPartitionDeclaration>> PartitionsDecl;
	StrategyCalculator->CollectPartitionsToAdd(nullptr, PartitionsDecl);

	// Should rebuild a graph to match the VirtualWorkerIds, but currently declaration array order is in DFS traversal order anyway
	for (auto Decl : PartitionsDecl)
	{
		PartitionsMap.Add(Decl, PartitionsMap.Num());
	}
}

SpatialStrategySystem::~SpatialStrategySystem() = default;

void SpatialStrategySystem::Advance(SpatialOSWorkerInterface* Connection)
{
	const TArray<Worker_Op>& Messages = Connection->GetWorkerMessages();

	for (const auto& Message : Messages)
	{
		switch (Message.op_type)
		{
		case WORKER_OP_TYPE_ENTITY_QUERY_RESPONSE:
		{
			const Worker_EntityQueryResponseOp& Op = Message.op.entity_query_response;
			if (Op.request_id == WorkerTranslationRequest)
			{
				if (Op.status_code == WORKER_STATUS_CODE_SUCCESS)
				{
					for (uint32_t i = 0; i < Op.results[0].component_count; i++)
					{
						Worker_ComponentData Data = Op.results[0].components[i];
						if (Data.component_id == SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID)
						{
							Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
							Translator.ApplyVirtualWorkerManagerData(ComponentObject);
							bTranslatorIsReady = true;
							for (uint32 VirtualWorker = 0; VirtualWorker < NumPartitions; ++VirtualWorker)
							{
								if (Translator.GetServerWorkerEntityForVirtualWorker(VirtualWorker + 1)
									== SpatialConstants::INVALID_ENTITY_ID)
								{
									bTranslatorIsReady = false;
								}
							}
						}
					}
				}
			}
			bTranslationQueryInFlight = false;
		}

		case WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE:
		{
			const Worker_CreateEntityResponseOp& Op = Message.op.create_entity_response;
			if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
			{
				UE_LOG(LogSpatialStrategySystem, Error, TEXT("Partition entity creation failed: \"%s\". Entity: %lld."),
					   UTF8_TO_TCHAR(Op.message), Op.entity_id);
			}
			else
			{
				if (PartitionCreationRequests.Contains(Op.request_id))
				{
					PartitionCreationRequests.Remove(Op.request_id);
					if (PartitionCreationRequests.Num() == 0)
					{
						bStrategyPartitionsCreated = true;
					}
				}
			}

			break;
		}
		case WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE:
		{
			const Worker_ReserveEntityIdsResponseOp& Op = Message.op.reserve_entity_ids_response;

			if (PartitionCreationRequests.Contains(Op.request_id))
			{
				PartitionCreationRequests.Remove(Op.request_id);
				if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
				{
					UE_LOG(LogSpatialStrategySystem, Error, TEXT("Reserve partition Id failed : %s"), UTF8_TO_TCHAR(Op.message));
				}
				else
				{
					for (uint32 VirtualWorker = 0; VirtualWorker < NumPartitions; ++VirtualWorker)
					{
						Worker_EntityId PartitionId = Op.first_entity_id + VirtualWorker;
						StrategyPartitions.Add(StrategyPartition(PartitionId));
						TArray<FWorkerComponentData> Components = SpatialGDK::EntityFactory::CreatePartitionEntityComponents(
							TEXT("StrategyPartition"), PartitionId, InterestF.Get(), &Strategy, VirtualWorker, false);

						const Worker_RequestId RequestId =
							Connection->SendCreateEntityRequest(MoveTemp(Components), &PartitionId, SpatialGDK::RETRY_UNTIL_COMPLETE);
						PartitionCreationRequests.Add(RequestId);
					}
				}
			}
			break;
		}
		case WORKER_OP_TYPE_COMMAND_RESPONSE:
		{
			const Worker_CommandResponseOp& Op = Message.op.command_response;
			if (Op.request_id == StrategyWorkerRequest)
			{
				if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
				{
					UE_LOG(LogSpatialStrategySystem, Error, TEXT("Claim partition failed : %s"), UTF8_TO_TCHAR(Op.message));
				}
				StrategyWorkerRequest = 0;
			}
		}
		break;
		}
	}

	for (const EntityDelta& Delta : PartitionView.GetViewDelta().EntityDeltas)
	{
		switch (Delta.Type)
		{
		case EntityDelta::UPDATE:
		{
			for (const auto& CompleteUpdate : Delta.ComponentsRefreshed)
			{
				if (CompleteUpdate.ComponentId == SpatialConstants::PARTITION_ACK_COMPONENT_ID)
				{
					Schema_Object* Object = Schema_GetComponentDataFields(CompleteUpdate.CompleteUpdate.Data);
					if (Schema_GetUint64(Object, 1) != 0)
					{
						SetPartitionReady(Delta.EntityId);
					}
				}
			}
			for (const auto& Update : Delta.ComponentUpdates)
			{
				if (Update.ComponentId == SpatialConstants::PARTITION_ACK_COMPONENT_ID)
				{
					Schema_Object* Object = Schema_GetComponentUpdateFields(Update.Update);
					if (Schema_GetUint64(Object, 1) != 0)
					{
						SetPartitionReady(Delta.EntityId);
					}
				}
			}
		}
		break;
		case EntityDelta::ADD:
		{
			const EntityViewElement& WorkerDesc = WorkerView.GetView().FindChecked(Delta.EntityId);
			for (const auto& Component : WorkerDesc.Components)
			{
				if (Component.GetComponentId() == SpatialConstants::PARTITION_ACK_COMPONENT_ID)
				{
					Schema_Object* Object = Schema_GetComponentDataFields(Component.GetUnderlying());
					if (Schema_GetUint64(Object, 1) != 0)
					{
						SetPartitionReady(Delta.EntityId);
					}
				}
			}
		}
		break;
		case EntityDelta::REMOVE:
		{
		}
		case EntityDelta::TEMPORARILY_REMOVED:
		{
		}
		break;
		default:
			break;
		}
	}

	for (const EntityDelta& Delta : WorkerView.GetViewDelta().EntityDeltas)
	{
		switch (Delta.Type)
		{
		case EntityDelta::UPDATE:
		{
			ServerWorker& ServerWorkerData = Workers.FindChecked(Delta.EntityId);
			bool bWasReady = ServerWorkerData.bReadyToBeginPlay;
			for (const auto& CompleteUpdate : Delta.ComponentsRefreshed)
			{
				if (CompleteUpdate.ComponentId == SpatialConstants::SERVER_WORKER_COMPONENT_ID)
				{
					ServerWorkerData = ServerWorker(CompleteUpdate.Data);
				}
			}
			for (const auto& Update : Delta.ComponentUpdates)
			{
				if (Update.ComponentId == SpatialConstants::SERVER_WORKER_COMPONENT_ID)
				{
					ServerWorkerData.ApplyComponentUpdate(Update.Update);
				}
			}
			if (!bWasReady && ServerWorkerData.bReadyToBeginPlay)
			{
				++NumWorkerReady;
			}
		}
		break;
		case EntityDelta::ADD:
		{
			const EntityViewElement& WorkerDesc = WorkerView.GetView().FindChecked(Delta.EntityId);
			for (const auto& Component : WorkerDesc.Components)
			{
				if (Component.GetComponentId() == SpatialConstants::SERVER_WORKER_COMPONENT_ID)
				{
					ServerWorker& ServerWorkerData = Workers.Add(Delta.EntityId, ServerWorker(Component.GetUnderlying()));
					if (ServerWorkerData.bReadyToBeginPlay)
					{
						NumWorkerReady++;
					}
				}
			}
		}
		break;
		case EntityDelta::REMOVE:
		{
		}
		case EntityDelta::TEMPORARILY_REMOVED:
		{
		}
		break;
		default:
			break;
		}
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
				if (UpdatesToConsider.Contains(Update.ComponentId))
				{
					StrategyCalculator->OnUpdate(Delta.EntityId, Update.ComponentId, Update.Update);
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

			StrategyCalculator->OnAdded(Delta.EntityId, Element);
		}
		break;
		case EntityDelta::REMOVE:
		{
			StrategyCalculator->OnRemoved(Delta.EntityId);
			AuthorityIntentView.Remove(Delta.EntityId);
		}
		break;
		case EntityDelta::TEMPORARILY_REMOVED:
		{
			StrategyCalculator->OnRemoved(Delta.EntityId);
			StrategyCalculator->OnAdded(Delta.EntityId, LBView.GetView().FindChecked(Delta.EntityId));
		}
		break;
		default:
			break;
		}
	}
}

void SpatialStrategySystem::SetPartitionReady(Worker_EntityId EntityId)
{
	for (auto& PartitionState : StrategyPartitions)
	{
		if (PartitionState.Id == EntityId)
		{
			PartitionState.bAcked = true;
			return;
		}
	}
}

void SpatialStrategySystem::CheckPartitionDistributed(SpatialOSWorkerInterface* Connection)
{
	if (bPartitionsDistributed)
	{
		return;
	}

	if (!bStrategyPartitionsCreated || NumWorkerReady < NumPartitions)
	{
		return;
	}

	if (!bTranslatorIsReady)
	{
		QueryTranslation(Connection);
		return;
	}

	for (uint32 Partition = 0; Partition < NumPartitions; ++Partition)
	{
		Worker_EntityId ServerWorkerEntity = Translator.GetServerWorkerEntityForVirtualWorker(Partition + 1);

		const ServerWorker& ServerWorkerData = Workers.FindChecked(ServerWorkerEntity);

		Worker_CommandRequest ClaimRequest = Worker::CreateClaimPartitionRequest(StrategyPartitions[Partition].Id);
		Connection->SendCommandRequest(ServerWorkerData.SystemEntityId, &ClaimRequest, SpatialGDK::RETRY_UNTIL_COMPLETE, {});
	}

	bPartitionsDistributed = true;
}

void SpatialStrategySystem::Flush(SpatialOSWorkerInterface* Connection)
{
	CheckPartitionDistributed(Connection);

	if (!bPartitionsDistributed)
	{
		return;
	}

	MigrationContext Ctx;
	Ctx.MigratingEntities = MigratingEntities;

	StrategyCalculator->CollectEntitiesToMigrate(Ctx);

	for (auto const& Migration : Ctx.EntitiesToMigrate)
	{
		Worker_EntityId EntityId = Migration.Key;

		if (!ensureAlways(!MigratingEntities.Contains(EntityId)))
		{
			continue;
		}

		TSharedPtr<FPartitionDeclaration> Partition = Migration.Value;
		uint32 PartitionIdx = PartitionsMap.FindChecked(Partition);
		AuthorityIntentV2& AuthIntent = AuthorityIntentView.FindChecked(EntityId);
		const StrategyPartition& PartitionState = StrategyPartitions[PartitionIdx];
		if (!PartitionState.bAcked)
		{
			continue;
		}
		AuthIntent.PartitionId = PartitionState.Id;
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

void SpatialStrategySystem::QueryTranslation(SpatialOSWorkerInterface* Connection)
{
	if (bTranslationQueryInFlight)
	{
		return;
	}

	// Build a constraint for the Virtual Worker Translation.
	Worker_ComponentConstraint TranslationComponentConstraint;
	TranslationComponentConstraint.component_id = SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID;

	Worker_Constraint TranslationConstraint;
	TranslationConstraint.constraint_type = WORKER_CONSTRAINT_TYPE_COMPONENT;
	TranslationConstraint.constraint.component_constraint = TranslationComponentConstraint;

	Worker_EntityQuery TranslationQuery{};
	TranslationQuery.constraint = TranslationConstraint;

	WorkerTranslationRequest = Connection->SendEntityQueryRequest(&TranslationQuery, RETRY_UNTIL_COMPLETE);
	bTranslationQueryInFlight = true;
}

} // namespace SpatialGDK
