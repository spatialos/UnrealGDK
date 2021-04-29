// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/MigrationDiagnosticsSystem.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Schema/MigrationDiagnostic.h"

DEFINE_LOG_CATEGORY_STATIC(LogSpatialMigrationDiagnostics, Log, All);

namespace SpatialGDK
{
MigrationDiagnosticsSystem::MigrationDiagnosticsSystem(USpatialNetDriver& InNetDriver)
	: NetDriver(InNetDriver)
	, Connection(*InNetDriver.Connection)
	, PackageMap(*InNetDriver.PackageMap)
	, EventTracer(InNetDriver.Connection->GetEventTracer())
{
	RequestHandler.AddRequestHandler(
		SpatialConstants::MIGRATION_DIAGNOSTIC_COMPONENT_ID, SpatialConstants::MIGRATION_DIAGNOSTIC_COMMAND_ID,
		FOnCommandRequestWithOp::FDelegate::CreateRaw(this, &MigrationDiagnosticsSystem::OnMigrationDiagnosticRequest));
	ResponseHandler.AddResponseHandler(
		SpatialConstants::MIGRATION_DIAGNOSTIC_COMPONENT_ID, SpatialConstants::MIGRATION_DIAGNOSTIC_COMMAND_ID,
		FOnCommandResponseWithOp::FDelegate::CreateRaw(this, &MigrationDiagnosticsSystem::OnMigrationDiagnosticResponse));
}

void MigrationDiagnosticsSystem::OnMigrationDiagnosticRequest(const Worker_Op& Op, const Worker_CommandRequestOp& RequestOp) const
{
	const Worker_EntityId EntityId = RequestOp.entity_id;
	const Worker_RequestId RequestId = RequestOp.request_id;
	AActor* BlockingActor = Cast<AActor>(PackageMap.GetObjectFromEntityId(EntityId));
	if (IsValid(BlockingActor))
	{
		Worker_CommandResponse Response = MigrationDiagnostic::CreateMigrationDiagnosticResponse(&NetDriver, EntityId, BlockingActor);

		FSpatialGDKSpanId CauseSpanId(Op.span_id);

		if (EventTracer != nullptr)
		{
			EventTracer->TraceEvent(SEND_COMMAND_RESPONSE_EVENT_NAME, "", CauseSpanId.GetConstId(), /* NumCauses */ 1,
									[RequestId](FSpatialTraceEventDataBuilder& EventBuilder) {
										EventBuilder.AddRequestId(RequestId);
										EventBuilder.AddKeyValue("Success", true);
									});
		}

		Connection.SendCommandResponse(RequestId, &Response, CauseSpanId);
	}
	else
	{
		UE_LOG(LogSpatialMigrationDiagnostics, Warning,
			   TEXT("Migration diaganostic log failed because cannot retreive actor for entity (%llu) on authoritative worker %s"),
			   EntityId, *Connection.GetWorkerId());
	}
}

void MigrationDiagnosticsSystem::OnMigrationDiagnosticResponse(const Worker_Op& Op, const Worker_CommandResponseOp& CommandResponseOp)
{
	if (CommandResponseOp.status_code != WORKER_STATUS_CODE_SUCCESS)
	{
		UE_LOG(LogSpatialMigrationDiagnostics, Warning, TEXT("Migration diaganostic log failed, status code %i."),
			   CommandResponseOp.status_code);
		return;
	}

	Schema_Object* ResponseObject = Schema_GetCommandResponseObject(CommandResponseOp.response.schema_type);
	const Worker_EntityId EntityId = Schema_GetInt64(ResponseObject, SpatialConstants::MIGRATION_DIAGNOSTIC_ENTITY_ID);
	AActor* BlockingActor = Cast<AActor>(PackageMap.GetObjectFromEntityId(EntityId));
	if (IsValid(BlockingActor))
	{
		const FString MigrationDiagnosticLog = MigrationDiagnostic::CreateMigrationDiagnosticLog(&NetDriver, ResponseObject, BlockingActor);
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

void MigrationDiagnosticsSystem::ProcessOps(const TArray<Worker_Op>& Ops) const
{
	RequestHandler.ProcessOps(Ops);
	ResponseHandler.ProcessOps(Ops);
}
} // namespace SpatialGDK
