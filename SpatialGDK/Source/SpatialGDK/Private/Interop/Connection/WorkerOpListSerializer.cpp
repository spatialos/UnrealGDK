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
			return new UE4_Op;
	}
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
