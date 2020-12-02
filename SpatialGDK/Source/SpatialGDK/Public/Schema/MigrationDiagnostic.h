// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/UnrealString.h"
#include "Engine/EngineBaseTypes.h"
#include "Engine/GameInstance.h"
#include "GameFramework/OnlineReplStructs.h"
#include "Kismet/GameplayStatics.h"
#include "Schema/Component.h"
#include "SpatialConstants.h"
#include "UObject/CoreNet.h"
#include "Utils/SchemaUtils.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{
struct MigrationDiagnostic : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::MIGRATION_DIAGNOSTIC_COMPONENT_ID;

	MigrationDiagnostic() = default;

	static Worker_CommandRequest CreateMigrationDiagnosticRequest()
	{
		Worker_CommandRequest CommandRequest = {};
		CommandRequest.component_id = SpatialConstants::MIGRATION_DIAGNOSTIC_COMPONENT_ID;
		CommandRequest.command_index = SpatialConstants::MIGRATION_DIAGNOSTIC_COMMAND_ID;
		CommandRequest.schema_type = Schema_CreateCommandRequest();

		return CommandRequest;
	}

	static Worker_CommandResponse CreateMigrationDiagnosticResponse(PhysicalWorkerName RemoteWorkerName, Worker_EntityId EntityId,
																	AActor* RemoteActor)
	{
		Worker_CommandResponse CommandResponse = {};
		CommandResponse.component_id = SpatialConstants::MIGRATION_DIAGNOSTIC_COMPONENT_ID;
		CommandResponse.command_index = SpatialConstants::MIGRATION_DIAGNOSTIC_COMMAND_ID;
		CommandResponse.schema_type = Schema_CreateCommandResponse();

		Schema_Object* ResponseObject = Schema_GetCommandResponseObject(CommandResponse.schema_type);
		AddStringToSchema(ResponseObject, SpatialConstants::MIGRATION_DIAGNOSTIC_WORKER_ID, RemoteWorkerName);
		Schema_AddInt64(ResponseObject, SpatialConstants::MIGRATION_DIAGNOSTIC_ENTITY_ID, EntityId);
		Schema_AddBool(ResponseObject, SpatialConstants::MIGRATION_DIAGNOSTIC_REPLICATES_ID, RemoteActor->GetIsReplicated());
		Schema_AddBool(ResponseObject, SpatialConstants::MIGRATION_DIAGNOSTIC_AUTHORITY_ID, RemoteActor->HasAuthority());
		AddStringToSchema(ResponseObject, SpatialConstants::MIGRATION_DIAGNOSTIC_OWNER_ID, RemoteActor->GetOwner()->GetName());

		return CommandResponse;
	}

	static FString CreateMigrationDiagnosticLog(PhysicalWorkerName OriginatingWorkerName, Schema_Object* ResponseObject, AActor* OrigintatingActor)
	{
		// This log is requested when the authoritative server for the owner of a migration hierarchy does not have authority over one of
		// the child actors
		PhysicalWorkerName AuthoritativeWorkerName = GetStringFromSchema(ResponseObject, SpatialConstants::MIGRATION_DIAGNOSTIC_WORKER_ID);
		Worker_EntityId EntityId = Schema_GetInt32(ResponseObject, SpatialConstants::MIGRATION_DIAGNOSTIC_ENTITY_ID);
		bool bIsReplicated = GetBoolFromSchema(ResponseObject, SpatialConstants::MIGRATION_DIAGNOSTIC_REPLICATES_ID);
		bool bHasAuthority = GetBoolFromSchema(ResponseObject, SpatialConstants::MIGRATION_DIAGNOSTIC_AUTHORITY_ID);
		FString AuthoritativeOwnerName = GetStringFromSchema(ResponseObject, SpatialConstants::MIGRATION_DIAGNOSTIC_OWNER_ID);

		FString Reason = FString::Printf(TEXT("Originating worker %s does not have authority of Actor. "), *OriginatingWorkerName);

		if (bHasAuthority)
		{
			Reason.Append(FString::Printf(TEXT("Worker %s has authority of Actor. "), *AuthoritativeWorkerName));
		}

		if (!IsValid(OrigintatingActor))
		{
			Reason.Append(FString::Printf(TEXT("Actor not valid.")));
		}
		else
		{
			if (OrigintatingActor->GetIsReplicated() && !bIsReplicated)
			{
				Reason.Append(FString::Printf(TEXT("Actor replicates on originating worker but not on authoritative worker. ")));
			}

			if (IsValid(OrigintatingActor->GetOwner()) && OrigintatingActor->GetOwner()->GetName() != AuthoritativeOwnerName)
			{
				Reason.Append(FString::Printf(TEXT("Actor has different owner on authoritative worker %s. "), *AuthoritativeOwnerName));
			}
		}

		return FString::Printf(TEXT("Prevented Actor %s 's hierarchy from migrating because of Actor %s (%llu) %s"), *AuthoritativeOwnerName,
							   *OrigintatingActor->GetName(), EntityId, *Reason);
	}
};

} // namespace SpatialGDK
