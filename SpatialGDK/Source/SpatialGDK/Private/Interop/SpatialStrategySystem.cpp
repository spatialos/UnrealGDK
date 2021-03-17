#include "Interop/SpatialStrategySystem.h"
#include "Interop/Connection/SpatialOSWorkerInterface.h"
#include "Schema/ServerWorker.h"
#include "Utils/InterestFactory.h"

DEFINE_LOG_CATEGORY(LogSpatialStrategySystem);

namespace SpatialGDK
{
void SpatialStrategySystem::Advance(SpatialOSWorkerInterface* Connection)
{
	// Messages are inspected to take care of the Worker entity creation.
	const TArray<Worker_Op>& Messages = Connection->GetWorkerMessages();
	for (const auto& Message : Messages)
	{
		switch (Message.op_type)
		{
		case WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE:
		{
			const Worker_ReserveEntityIdsResponseOp& Op = Message.op.reserve_entity_ids_response;
			if (Op.request_id == StrategyWorkerRequest)
			{
				if (Op.first_entity_id == SpatialConstants::INVALID_ENTITY_ID)
				{
					UE_LOG(LogSpatialStrategySystem, Error, TEXT("Reserve entity failed : %s"), UTF8_TO_TCHAR(Op.message));
					StrategyWorkerRequest.Reset();
				}
				else
				{
					StrategyPartition = Message.op.reserve_entity_ids_response.first_entity_id;
					CreateStrategyPartition(Connection);
				}
			}
			break;
		}
		case WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE:
		{
			const Worker_CreateEntityResponseOp& Op = Message.op.create_entity_response;
			if (Op.request_id == StrategyWorkerRequest)
			{
				if (Op.entity_id == SpatialConstants::INVALID_ENTITY_ID)
				{
					UE_LOG(LogSpatialStrategySystem, Error, TEXT("Create entity failed : %s"), UTF8_TO_TCHAR(Op.message));
				}
				StrategyWorkerRequest.Reset();

				Worker_CommandRequest ClaimRequest = Worker::CreateClaimPartitionRequest(StrategyPartition);
				StrategyWorkerRequest =
					Connection->SendCommandRequest(StrategyWorkerSystemEntityId, &ClaimRequest, SpatialGDK::RETRY_UNTIL_COMPLETE, {});
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
				StrategyWorkerRequest.Reset();
			}
		}
		break;
		}
	}

	const FSubViewDelta& SubViewDelta = SubView.GetViewDelta();
	for (const EntityDelta& Delta : SubViewDelta.EntityDeltas)
	{
		switch (Delta.Type)
		{
		case EntityDelta::UPDATE:
		{
			// TODO
		}
		break;
		case EntityDelta::ADD:
		{
			// TODO
		}
		break;
		case EntityDelta::REMOVE:
		case EntityDelta::TEMPORARILY_REMOVED:
		{
			// TODO
		}
		break;
		default:
			break;
		}
	}
}

void SpatialStrategySystem::Flush(SpatialOSWorkerInterface* Connection)
{
	// TODO
}

void SpatialStrategySystem::Init(SpatialOSWorkerInterface* Connection)
{
	StrategyWorkerRequest = Connection->SendReserveEntityIdsRequest(1, SpatialGDK::RETRY_UNTIL_COMPLETE);
}

void SpatialStrategySystem::CreateStrategyPartition(SpatialOSWorkerInterface* Connection)
{
	AuthorityDelegationMap Map;
	Map.Add(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID, StrategyPartition);

	TArray<FWorkerComponentData> Components;
	Components.Add(Position().CreateComponentData());
	Components.Add(Metadata(FString(TEXT("StrategyPartition"))).CreateComponentData());
	Components.Add(AuthorityDelegation(Map).CreateComponentData());
	Components.Add(InterestFactory::CreateStrategyWorkerInterest().CreateComponentData());

	StrategyWorkerRequest = Connection->SendCreateEntityRequest(Components, &StrategyPartition, SpatialGDK::RETRY_UNTIL_COMPLETE);
}

void SpatialStrategySystem::Destroy(SpatialOSWorkerInterface* Connection)
{
	Connection->SendDeleteEntityRequest(StrategyPartition, SpatialGDK::RETRY_UNTIL_COMPLETE);
}

} // namespace SpatialGDK
