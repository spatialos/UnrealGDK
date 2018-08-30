
#include "SpatialView.h"

#include "SpatialEntityPipeline.h"
#include "SpatialReceiver.h"

void USpatialView::ProcessOps(Worker_OpList* OpList)
{
	for (size_t i = 0; i < OpList->op_count; ++i)
	{
		Worker_Op* Op = &OpList->ops[i];
		switch (Op->op_type)
		{
		// Critical Section
		case WORKER_OP_TYPE_CRITICAL_SECTION:
			EntityPipeline->OnCriticalSection(Op->critical_section.in_critical_section);
			break;

		// Entity Lifetime
		case WORKER_OP_TYPE_ADD_ENTITY:
			EntityPipeline->OnAddEntity(Op->add_entity);
			break;
		case WORKER_OP_TYPE_REMOVE_ENTITY:
			EntityPipeline->OnRemoveEntity(Op->remove_entity);
			break;

		// Components
		case WORKER_OP_TYPE_ADD_COMPONENT:
			EntityPipeline->OnAddComponent(Op->add_component);
			break;
		case WORKER_OP_TYPE_REMOVE_COMPONENT:
			EntityPipeline->OnRemoveComponent(Op->remove_component);
			break;


		case WORKER_OP_TYPE_COMPONENT_UPDATE:
			Receiver->OnComponentUpdate(Op->component_update);
			break;
		// Commands
		case WORKER_OP_TYPE_COMMAND_REQUEST:
			Receiver->OnCommandRequest(Op->command_request);
			break;
		case WORKER_OP_TYPE_COMMAND_RESPONSE:
			Receiver->OnCommandResponse(Op->command_response);
			break;

		// Authority Change
		case WORKER_OP_TYPE_AUTHORITY_CHANGE:
			OnAuthorityChange(Op->authority_change);
			break;

		// World Command Responses
		case WORKER_OP_TYPE_RESERVE_ENTITY_ID_RESPONSE:
			EntityPipeline->OnReserveEntityIdResponse(Op->reserve_entity_id_response);
			break;
		case WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE:
			break;
		case WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE:
			EntityPipeline->OnCreateEntityIdResponse(Op->create_entity_response);
			break;
		case WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE:
			break;
		case WORKER_OP_TYPE_ENTITY_QUERY_RESPONSE:
			break;


		case WORKER_OP_TYPE_FLAG_UPDATE:
			break;
		case WORKER_OP_TYPE_LOG_MESSAGE:
			UE_LOG(LogTemp, Log, TEXT("SpatialOS: %s"), UTF8_TO_TCHAR(Op->log_message.message));
			break;
		case WORKER_OP_TYPE_METRICS:
			break;
		case WORKER_OP_TYPE_DISCONNECT:
			UE_LOG(LogTemp, Warning, TEXT("Disconnecting from SpatialOS"), UTF8_TO_TCHAR(Op->disconnect.reason));
			break;

		default:
			break;
		}
	}
}

Worker_Authority USpatialView::GetAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	return ComponentAuthorityMap[EntityId][ComponentId];
}

void USpatialView::OnAuthority(const Worker_Authority& Op)
{
	ComponentAuthorityMap.FindOrAdd(Op.entity_id).FindOrAdd(Op.component_id) = (Worker_Authority)Op.authority;
}
