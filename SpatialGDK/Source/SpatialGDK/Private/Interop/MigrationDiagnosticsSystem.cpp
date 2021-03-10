// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/MigrationDiagnosticsSystem.h"

#include "Schema/MigrationDiagnostic.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/Connection/SpatialTraceEventBuilder.h"

DEFINE_LOG_CATEGORY_STATIC(LogSpatialMigrationDiagnostics, Log, All);

namespace SpatialGDK
{
MigrationDiagnosticsSystem::MigrationDiagnosticsSystem(USpatialNetDriver& InNetDriver)
	: NetDriver(InNetDriver)
	, Connection(*InNetDriver.Connection)
	, PackageMap(*InNetDriver.PackageMap)
	, EventTracer(InNetDriver.Connection->GetEventTracer())
{
}

void MigrationDiagnosticsSystem::ProcessOps(const TArray<Worker_Op>& Ops) const
{
	for (const Worker_Op& Op : Ops)
	{
		if (Op.op_type == WORKER_OP_TYPE_COMMAND_REQUEST)
		{
			const Worker_CommandRequestOp& CommandRequest = Op.op.command_request;

			const Worker_RequestId RequestId = CommandRequest.request_id;
			const Worker_ComponentId ComponentId = CommandRequest.request.component_id;
			const Worker_CommandIndex CommandIndex = CommandRequest.request.command_index;
			const Worker_EntityId EntityId = CommandRequest.entity_id;

			if (ComponentId == SpatialConstants::MIGRATION_DIAGNOSTIC_COMPONENT_ID
				&& CommandIndex == SpatialConstants::MIGRATION_DIAGNOSTIC_COMMAND_ID)
			{
				AActor* BlockingActor = Cast<AActor>(PackageMap.GetObjectFromEntityId(EntityId));
				if (IsValid(BlockingActor))
				{
					Worker_CommandResponse Response =
						MigrationDiagnostic::CreateMigrationDiagnosticResponse(&NetDriver, EntityId, BlockingActor);

					FSpatialGDKSpanId CauseSpanId(Op.span_id);

					if (EventTracer != nullptr)
					{
						EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateSendCommandResponse(RequestId, true),
												/* Causes */ CauseSpanId.GetConstId(), /* NumCauses */ 1);
					}

					Connection.SendCommandResponse(RequestId, &Response, CauseSpanId);
				}
				else
				{
					UE_LOG(
						LogSpatialMigrationDiagnostics, Warning,
						TEXT("Migration diaganostic log failed because cannot retreive actor for entity (%llu) on authoritative worker %s"),
						EntityId, *Connection.GetWorkerId());
				}
			}
		}
		else if (Op.op_type == WORKER_OP_TYPE_COMMAND_RESPONSE)
		{
			const Worker_CommandResponseOp& CommandResponseOp = Op.op.command_response;

			const Worker_ComponentId ComponentId = CommandResponseOp.response.component_id;

			if (ComponentId == SpatialConstants::MIGRATION_DIAGNOSTIC_COMPONENT_ID)
			{
				if (CommandResponseOp.status_code != WORKER_STATUS_CODE_SUCCESS)
				{
					UE_LOG(LogSpatialMigrationDiagnostics, Warning, TEXT("Migration diaganostic log failed, status code %i."),
						   CommandResponseOp.status_code);
					continue;
				}

				Schema_Object* ResponseObject = Schema_GetCommandResponseObject(CommandResponseOp.response.schema_type);
				const Worker_EntityId EntityId = Schema_GetInt64(ResponseObject, SpatialConstants::MIGRATION_DIAGNOSTIC_ENTITY_ID);
				AActor* BlockingActor = Cast<AActor>(PackageMap.GetObjectFromEntityId(EntityId));
				if (IsValid(BlockingActor))
				{
					const FString MigrationDiagnosticLog =
						MigrationDiagnostic::CreateMigrationDiagnosticLog(&NetDriver, ResponseObject, BlockingActor);
					if (!MigrationDiagnosticLog.IsEmpty())
					{
						UE_LOG(LogSpatialMigrationDiagnostics, Warning, TEXT("%s"), *MigrationDiagnosticLog);
					}
				}
				else
				{
					UE_LOG(LogSpatialMigrationDiagnostics, Warning,
						   TEXT("Migration diaganostic log failed because blocking actor (%llu) is not valid."), EntityId);
				}
			}
		}
	}
}
} // namespace SpatialGDK
