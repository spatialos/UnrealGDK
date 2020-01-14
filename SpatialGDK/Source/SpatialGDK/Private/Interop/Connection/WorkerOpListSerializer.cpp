// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "WorkerOpListSerializer.h"

#if WITH_EDITOR
#include "Interop/Connection/EditorWorkerController.h"
#endif

#include "Async/Async.h"
#include "Misc/Paths.h"

#include "SpatialGDKSettings.h"
#include "Utils/ErrorCodeRemapping.h"

using namespace SpatialGDK;

#pragma optimize("", off)

UE4_Op* Worker_OpToUE4_Op(const Worker_Op* WorkerOp)
{
	const Worker_Op_Union Op = WorkerOp->op;

	switch (WorkerOp->op_type)
	{
		case WORKER_OP_TYPE_DISCONNECT:
			// TODO(Alex): ptr
			return new UE4_DisconnectOp{ Op.disconnect };
		case WORKER_OP_TYPE_FLAG_UPDATE:
			return new UE4_FlagUpdateOp{ Op.flag_update };
		case WORKER_OP_TYPE_LOG_MESSAGE:
			return new UE4_LogMessageOp{ Op.log_message };
		case WORKER_OP_TYPE_METRICS:
			return new UE4_MetricsOp{ Op.metrics };
		case WORKER_OP_TYPE_CRITICAL_SECTION:
			return new UE4_CriticalSectionOp{ Op.critical_section };
		case WORKER_OP_TYPE_ADD_ENTITY:
			return new UE4_AddEntityOp{ Op.add_entity };
		case WORKER_OP_TYPE_REMOVE_ENTITY:
			return new UE4_RemoveEntityOp{ Op.remove_entity };
		case WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE:
			return new UE4_ReserveEntityIdsResponseOp{ Op.reserve_entity_ids_response };
		case WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE:
			return new UE4_CreateEntityResponseOp{ Op.create_entity_response };
		case WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE:
			return new UE4_DeleteEntityResponseOp{ Op.delete_entity_response };
		case WORKER_OP_TYPE_ENTITY_QUERY_RESPONSE:
			return new UE4_EntityQueryResponseOp{ Op.entity_query_response };
		case WORKER_OP_TYPE_ADD_COMPONENT:
			return new UE4_AddComponentOp{ Op.add_component };
		case WORKER_OP_TYPE_REMOVE_COMPONENT:
			return new UE4_RemoveComponentOp{ Op.remove_component };
		case WORKER_OP_TYPE_AUTHORITY_CHANGE:
			return new UE4_AuthorityChangeOp{ Op.authority_change };
		case WORKER_OP_TYPE_COMPONENT_UPDATE:
			return new UE4_ComponentUpdateOp{ Op.component_update };
		case WORKER_OP_TYPE_COMMAND_REQUEST:
			return new UE4_CommandRequestOp{ Op.command_request };
		case WORKER_OP_TYPE_COMMAND_RESPONSE:
			return new UE4_CommandResponseOp{ Op.command_response };
		default:
			// TODO(Alex): 
			check(false);
			return new UE4_Op;
	}
}

Worker_Op UE4_OpToWorker_Op(const UE4_OpAndType& WorkerOpAndType)
{
	Worker_Op Op;
	Op.op_type = WorkerOpAndType.op_type;

	switch (WorkerOpAndType.op_type)
	{
	case WORKER_OP_TYPE_DISCONNECT:
	{
		// TODO(Alex): ptr
		UE4_DisconnectOp* SourceOp = static_cast<UE4_DisconnectOp*>(WorkerOpAndType.Op);
		Op.op.disconnect.connection_status_code = SourceOp->connection_status_code;
		Op.op.disconnect.reason = SourceOp->reason.c_str();
		// TODO(Alex): unsafe?
		//Op.op.disconnect.reason = static_cast<const char*>(*SourceOp->reason);
		//SourceOp.op.disconnect.reason = SourceOp->reason.c 
		//SourceOp.op.disconnect.reason = SourceOp->reason.c 
		break;
	}
	case WORKER_OP_TYPE_FLAG_UPDATE:
	{
		UE4_FlagUpdateOp* SourceOp = static_cast<UE4_FlagUpdateOp*>(WorkerOpAndType.Op);
		Op.op.flag_update.name;
		Op.op.flag_update.value;
		break;
	}
	case WORKER_OP_TYPE_LOG_MESSAGE:
	{
		UE4_LogMessageOp* SourceOp = static_cast<UE4_LogMessageOp*>(WorkerOpAndType.Op);
		Op.op.log_message.level = SourceOp->level;
		Op.op.log_message.message;
		break;
	}
	case WORKER_OP_TYPE_METRICS:
	{
		UE4_MetricsOp* SourceOp = static_cast<UE4_MetricsOp*>(WorkerOpAndType.Op);
		// TODO(Alex): is it necessary?
		Op.op.metrics.metrics;
		break;
	}
	case WORKER_OP_TYPE_CRITICAL_SECTION:
	{
		UE4_CriticalSectionOp* SourceOp = static_cast<UE4_CriticalSectionOp*>(WorkerOpAndType.Op);
		Op.op.critical_section.in_critical_section = SourceOp->in_critical_section;
		break;
	}
	case WORKER_OP_TYPE_ADD_ENTITY:
	{
		UE4_AddEntityOp* SourceOp = static_cast<UE4_AddEntityOp*>(WorkerOpAndType.Op);
		Op.op.add_entity.entity_id = SourceOp->entity_id;
		break;
	}
	case WORKER_OP_TYPE_REMOVE_ENTITY:
	{
		UE4_RemoveEntityOp* SourceOp = static_cast<UE4_RemoveEntityOp*>(WorkerOpAndType.Op);
		Op.op.remove_entity.entity_id = SourceOp->entity_id;
		break;
	}
	case WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE:
	{
		UE4_ReserveEntityIdsResponseOp* SourceOp = static_cast<UE4_ReserveEntityIdsResponseOp*>(WorkerOpAndType.Op);
		Op.op.reserve_entity_ids_response.first_entity_id = SourceOp->first_entity_id;
		Op.op.reserve_entity_ids_response.message = SourceOp->message.c_str();
		Op.op.reserve_entity_ids_response.number_of_entity_ids = SourceOp->number_of_entity_ids;
		Op.op.reserve_entity_ids_response.request_id = SourceOp->request_id;
		Op.op.reserve_entity_ids_response.status_code = SourceOp->status_code;
		break;
	}
	case WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE:
	{
		UE4_CreateEntityResponseOp* SourceOp = static_cast<UE4_CreateEntityResponseOp*>(WorkerOpAndType.Op);
		Op.op.create_entity_response.entity_id = SourceOp->entity_id;
		Op.op.create_entity_response.message = SourceOp->message.c_str();
		Op.op.create_entity_response.request_id = SourceOp->request_id;
		Op.op.create_entity_response.status_code = SourceOp->status_code;
		break;
	}
	case WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE:
	{
		UE4_DeleteEntityResponseOp* SourceOp = static_cast<UE4_DeleteEntityResponseOp*>(WorkerOpAndType.Op);
		Op.op.delete_entity_response.entity_id = SourceOp->entity_id;
		Op.op.delete_entity_response.message = SourceOp->message.c_str();
		Op.op.delete_entity_response.request_id = SourceOp->request_id;
		Op.op.delete_entity_response.status_code = SourceOp->status_code;
		break;
	}
	case WORKER_OP_TYPE_ENTITY_QUERY_RESPONSE:
	{
		UE4_EntityQueryResponseOp* SourceOp = static_cast<UE4_EntityQueryResponseOp*>(WorkerOpAndType.Op);
		Op.op.entity_query_response.message = SourceOp->message.c_str();
		Op.op.entity_query_response.request_id = SourceOp->request_id;
		Op.op.entity_query_response.result_count = SourceOp->result_count;
		Op.op.entity_query_response.results = SourceOp->results.Create_Worker_Entity();
		Op.op.entity_query_response.status_code = SourceOp->status_code;
		break;
	}
	case WORKER_OP_TYPE_ADD_COMPONENT:
	{
		UE4_AddComponentOp* SourceOp = static_cast<UE4_AddComponentOp*>(WorkerOpAndType.Op);
		{
			Op.op.add_component.data.component_id = SourceOp->data.component_id;
			Op.op.add_component.data.reserved = SourceOp->data.reserved;
			Op.op.add_component.data.schema_type = SourceOp->data.schema_type;
			Op.op.add_component.data.user_handle = SourceOp->data.user_handle;
		}
		Op.op.add_component.entity_id = SourceOp->entity_id;
		break;
	}
	case WORKER_OP_TYPE_REMOVE_COMPONENT:
	{
		UE4_RemoveComponentOp* SourceOp = static_cast<UE4_RemoveComponentOp*>(WorkerOpAndType.Op);
		Op.op.remove_component.component_id = SourceOp->component_id;
		Op.op.remove_component.entity_id = SourceOp->entity_id;
		break;
	}
	case WORKER_OP_TYPE_AUTHORITY_CHANGE:
	{
		UE4_AuthorityChangeOp* SourceOp = static_cast<UE4_AuthorityChangeOp*>(WorkerOpAndType.Op);
		Op.op.authority_change.authority = SourceOp->authority;
		Op.op.authority_change.component_id = SourceOp->component_id;
		Op.op.authority_change.entity_id = SourceOp->entity_id;
		break;
	}
	case WORKER_OP_TYPE_COMPONENT_UPDATE:
	{
		UE4_ComponentUpdateOp* SourceOp = static_cast<UE4_ComponentUpdateOp*>(WorkerOpAndType.Op);
		Op.op.component_update.entity_id = SourceOp->entity_id;
		{
			Op.op.component_update.update.component_id = SourceOp->update.component_id;
			Op.op.component_update.update.reserved = SourceOp->update.reserved;
			Op.op.component_update.update.schema_type = SourceOp->update.schema_type;
			Op.op.component_update.update.user_handle = SourceOp->update.user_handle;
		}
		break;
	}
	case WORKER_OP_TYPE_COMMAND_REQUEST:
	{
		UE4_CommandRequestOp* SourceOp = static_cast<UE4_CommandRequestOp*>(WorkerOpAndType.Op);
		{
			uint32_t attributeCount = SourceOp->caller_attribute_set.attributes.Num();
			Op.op.command_request.caller_attribute_set.attribute_count = attributeCount;
			// TODO(Alex): memory leak!
			const char** AttributeArray = new const char*[SourceOp->caller_attribute_set.attributes.Num()];
			for (uint32_t i = 0; i < attributeCount; i++)
			{
				AttributeArray[i] = SourceOp->caller_attribute_set.attributes[i].c_str();
			}
			Op.op.command_request.caller_attribute_set.attributes = AttributeArray;
		}
		Op.op.command_request.caller_worker_id = SourceOp->caller_worker_id.c_str();
		Op.op.command_request.entity_id = SourceOp->entity_id;
		{
			Op.op.command_request.request.command_index = SourceOp->request.command_index;
			Op.op.command_request.request.component_id = SourceOp->request.component_id;
			Op.op.command_request.request.reserved = SourceOp->request.reserved;
			Op.op.command_request.request.schema_type = SourceOp->request.schema_type;
			Op.op.command_request.request.user_handle = SourceOp->request.user_handle;
		}
		Op.op.command_request.request_id = SourceOp->request_id;
		Op.op.command_request.timeout_millis = SourceOp->timeout_millis;
		break;
	}
	case WORKER_OP_TYPE_COMMAND_RESPONSE:
	{
		UE4_CommandResponseOp* SourceOp = static_cast<UE4_CommandResponseOp*>(WorkerOpAndType.Op);
		Op.op.command_response.entity_id = SourceOp->entity_id;
		Op.op.command_response.message = SourceOp->message.c_str();
		Op.op.command_response.request_id = SourceOp->request_id;
		{
			Op.op.command_response.response.command_index = SourceOp->response.command_index;
			Op.op.command_response.response.component_id = SourceOp->response.component_id;
			Op.op.command_response.response.reserved = SourceOp->response.reserved;
			Op.op.command_response.response.schema_type = SourceOp->response.schema_type;
			Op.op.command_response.response.user_handle = SourceOp->response.user_handle;
		}
		Op.op.command_response.status_code = SourceOp->status_code;
		break;
	}
	default:
		// TODO(Alex): 
		break;
	}

	return Op;
}

FString OpTypeToString(uint8_t OpType)
{
	switch (OpType)
	{
		case WORKER_OP_TYPE_DISCONNECT:
			return TEXT("WORKER_OP_TYPE_DISCONNECT");
		case WORKER_OP_TYPE_FLAG_UPDATE:
			return TEXT("WORKER_OP_TYPE_FLAG_UPDATE");
		case WORKER_OP_TYPE_LOG_MESSAGE:
			return TEXT("WORKER_OP_TYPE_LOG_MESSAGE");
		case WORKER_OP_TYPE_METRICS:
			return TEXT("WORKER_OP_TYPE_METRICS");
		case WORKER_OP_TYPE_CRITICAL_SECTION:
			return TEXT("WORKER_OP_TYPE_CRITICAL_SECTION");
		case WORKER_OP_TYPE_ADD_ENTITY:
			return TEXT("WORKER_OP_TYPE_ADD_ENTITY");
		case WORKER_OP_TYPE_REMOVE_ENTITY:
			return TEXT("WORKER_OP_TYPE_REMOVE_ENTITY");
		case WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE:
			return TEXT("WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE");
		case WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE:
			return TEXT("WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE");
		case WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE:
			return TEXT("WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE");
		case WORKER_OP_TYPE_ENTITY_QUERY_RESPONSE:
			return TEXT("WORKER_OP_TYPE_ENTITY_QUERY_RESPONSE");
		case WORKER_OP_TYPE_ADD_COMPONENT:
			return TEXT("WORKER_OP_TYPE_ADD_COMPONENT");
		case WORKER_OP_TYPE_REMOVE_COMPONENT:
			return TEXT("WORKER_OP_TYPE_REMOVE_COMPONENT");
		case WORKER_OP_TYPE_AUTHORITY_CHANGE:
			return TEXT("WORKER_OP_TYPE_AUTHORITY_CHANGE");
		case WORKER_OP_TYPE_COMPONENT_UPDATE:
			return TEXT("WORKER_OP_TYPE_COMPONENT_UPDATE");
		case WORKER_OP_TYPE_COMMAND_REQUEST:
			return TEXT("WORKER_OP_TYPE_COMMAND_REQUEST");
		case WORKER_OP_TYPE_COMMAND_RESPONSE:
			return TEXT("WORKER_OP_TYPE_COMMAND_RESPONSE");
		default:
			return TEXT("INVALID OP TYPE");
	}
}

TArray<Worker_OpList*> UE4_OpLists::LoadSavedOpLists()
{
	return TArray<Worker_OpList*>{};
}

void UE4_OpLists::SerializedOpList(const Worker_OpList* OpList)
{
	UE4_OpList TempOpList;

	for (uint32_t i = 0; i < OpList->op_count; i++)
	{
		TempOpList.Push({Worker_OpToUE4_Op(&OpList->ops[i]), OpList->ops[i].op_type});
	}

	OpLists.Push(MoveTemp(TempOpList));
}

void UE4_OpLists::DumpSavedOpLists()
{
	for (auto& OpList : OpLists)
	{
		for (auto& Op : OpList)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s"), *OpTypeToString(Op.op_type));
			// TODO(Alex): move elsewhere, make safe?
			delete Op.Op;
			Op.Op = nullptr;
		}
	}

	OpLists.Empty();
}
