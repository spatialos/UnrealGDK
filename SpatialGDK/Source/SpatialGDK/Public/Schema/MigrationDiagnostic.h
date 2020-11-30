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

	static FString CreateMigrationDiagnosticLog(PhysicalWorkerName LocalWorkerName, Schema_Object* ResponseObject, AActor* LocalActor)
	{
		// This log is requested when the authoritative server for the owner of a migration hierarchy does not have authority over one of
		// the child actors
		PhysicalWorkerName RemoteWorkerName = GetStringFromSchema(ResponseObject, SpatialConstants::MIGRATION_DIAGNOSTIC_WORKER_ID);
		Worker_EntityId EntityId = Schema_GetInt32(ResponseObject, SpatialConstants::MIGRATION_DIAGNOSTIC_ENTITY_ID);
		bool bIsReplicatedRemotely = GetBoolFromSchema(ResponseObject, SpatialConstants::MIGRATION_DIAGNOSTIC_REPLICATES_ID);
		bool bHasAuthorityRemotely = GetBoolFromSchema(ResponseObject, SpatialConstants::MIGRATION_DIAGNOSTIC_AUTHORITY_ID);
		FString RemoteOwnerName = GetStringFromSchema(ResponseObject, SpatialConstants::MIGRATION_DIAGNOSTIC_OWNER_ID);

		FString Reason = FString::Printf(TEXT("Local worker %s does not have authority. "), *LocalWorkerName);

		if (bHasAuthorityRemotely)
		{
			Reason.Append(FString::Printf(TEXT("Remote worker %s has authority of actor. "), *RemoteWorkerName));
		}

		if (!IsValid(LocalActor))
		{
			Reason.Append(FString::Printf(TEXT("Actor not valid.")));
		}
		else
		{
			if (LocalActor->GetIsReplicated() && !bIsReplicatedRemotely)
			{
				Reason.Append(FString::Printf(TEXT("Actor replicates locally but not remotely. ")));
			}

			if (IsValid(LocalActor->GetOwner()) && LocalActor->GetOwner()->GetName() != RemoteOwnerName)
			{
				Reason.Append(FString::Printf(TEXT("Actor has different owner remotely %s. "), *RemoteOwnerName));
			}
		}

		return FString::Printf(TEXT("Prevented Actor %s 's hierarchy from migrating because Actor %s (%llu) %s"), *RemoteOwnerName,
							   *LocalActor->GetName(), EntityId, *Reason);
	}
};

} // namespace SpatialGDK
