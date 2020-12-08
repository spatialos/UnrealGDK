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
#include "Utils/SpatialLoadBalancingHandler.h"

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
		// Request information from the worker that has authority over the actor that is blocking the hierarchy from migrating
		Worker_CommandRequest CommandRequest = {};
		CommandRequest.component_id = SpatialConstants::MIGRATION_DIAGNOSTIC_COMPONENT_ID;
		CommandRequest.command_index = SpatialConstants::MIGRATION_DIAGNOSTIC_COMMAND_ID;
		CommandRequest.schema_type = Schema_CreateCommandRequest();

		return CommandRequest;
	}

	// Respond with information on the worker that has authority over the actor that is blocking the hierarchy from migrating
	static Worker_CommandResponse CreateMigrationDiagnosticResponse(USpatialNetDriver* NetDriver, Worker_EntityId EntityId,
																	AActor* BlockingActor)
	{
		check(NetDriver != nullptr);
		check(NetDriver->Connection != nullptr);
		check(NetDriver->LockingPolicy != nullptr);
		check(NetDriver->VirtualWorkerTranslator != nullptr);
		check(NetDriver->PackageMap != nullptr);

		Worker_CommandResponse CommandResponse = {};

		CommandResponse.component_id = SpatialConstants::MIGRATION_DIAGNOSTIC_COMPONENT_ID;
		CommandResponse.command_index = SpatialConstants::MIGRATION_DIAGNOSTIC_COMMAND_ID;
		CommandResponse.schema_type = Schema_CreateCommandResponse();

		Schema_Object* ResponseObject = Schema_GetCommandResponseObject(CommandResponse.schema_type);
		Schema_AddEntityId(ResponseObject, SpatialConstants::MIGRATION_DIAGNOSTIC_ENTITY_ID, EntityId);
		Schema_AddBool(ResponseObject, SpatialConstants::MIGRATION_DIAGNOSTIC_REPLICATES_ID, BlockingActor->GetIsReplicated());
		Schema_AddBool(ResponseObject, SpatialConstants::MIGRATION_DIAGNOSTIC_HAS_AUTHORITY_ID, BlockingActor->HasAuthority());

		const VirtualWorkerId LocalVirtualWorkerId = NetDriver->VirtualWorkerTranslator->GetLocalVirtualWorkerId();

		Schema_AddInt32(ResponseObject, SpatialConstants::MIGRATION_DIAGNOSTIC_AUTHORITY_WORKER_ID,
						NetDriver->VirtualWorkerTranslator->GetLocalVirtualWorkerId());
		Schema_AddBool(ResponseObject, SpatialConstants::MIGRATION_DIAGNOSTIC_LOCKED_ID, NetDriver->LockingPolicy->IsLocked(BlockingActor));

		AActor* NetOwner;
		VirtualWorkerId NewAuthWorkerId;

		FSpatialLoadBalancingHandler MigrationHandler(NetDriver);
		FSpatialLoadBalancingHandler::EvaluateActorResult Result =
			MigrationHandler.EvaluateSingleActor(BlockingActor, NetOwner, NewAuthWorkerId);
		Worker_EntityId OwnerId = NetDriver->PackageMap->GetEntityIdFromObject(NetOwner);

		Schema_AddBool(ResponseObject, SpatialConstants::MIGRATION_DIAGNOSTIC_EVALUATION_ID,
					   Result == FSpatialLoadBalancingHandler::EvaluateActorResult::Migrate);
		Schema_AddInt32(ResponseObject, SpatialConstants::MIGRATION_DIAGNOSTIC_DESTINATION_WORKER_ID, NewAuthWorkerId);
		Schema_AddEntityId(ResponseObject, SpatialConstants::MIGRATION_DIAGNOSTIC_OWNER_ID, OwnerId);

		return CommandResponse;
	}

	// Compare information from the worker that is blocked from migrating a hierarchy with the information from the authoritative
	// worker over the actor that is blocking the migration and log the results.
	static FString CreateMigrationDiagnosticLog(USpatialNetDriver* NetDriver, Schema_Object* ResponseObject, AActor* BlockingActor)
	{
		check(NetDriver != nullptr);
		check(NetDriver->Connection != nullptr);
		check(NetDriver->LockingPolicy != nullptr);

		if (ResponseObject == nullptr)
		{
			return FString::Printf(TEXT("Migration diaganostic log failed as response was empty."));
		}

		VirtualWorkerId AuthoritativeWorkerId = Schema_GetInt32(ResponseObject, SpatialConstants::MIGRATION_DIAGNOSTIC_AUTHORITY_WORKER_ID);
		Worker_EntityId BlockedEntityId = Schema_GetEntityId(ResponseObject, SpatialConstants::MIGRATION_DIAGNOSTIC_ENTITY_ID);
		bool bIsReplicated = GetBoolFromSchema(ResponseObject, SpatialConstants::MIGRATION_DIAGNOSTIC_REPLICATES_ID);
		bool bHasAuthority = GetBoolFromSchema(ResponseObject, SpatialConstants::MIGRATION_DIAGNOSTIC_HAS_AUTHORITY_ID);
		bool bIsLocked = GetBoolFromSchema(ResponseObject, SpatialConstants::MIGRATION_DIAGNOSTIC_LOCKED_ID);
		bool bCanMigrate = GetBoolFromSchema(ResponseObject, SpatialConstants::MIGRATION_DIAGNOSTIC_EVALUATION_ID);
		VirtualWorkerId DestinationWorkerId = Schema_GetInt32(ResponseObject, SpatialConstants::MIGRATION_DIAGNOSTIC_DESTINATION_WORKER_ID);
		Worker_EntityId AuthoritativeNetOwnerId = Schema_GetEntityId(ResponseObject, SpatialConstants::MIGRATION_DIAGNOSTIC_OWNER_ID);

		AActor* NetOwner = SpatialGDK::GetReplicatedHierarchyRoot(BlockingActor);
		Worker_EntityId OriginalNetOwnerId = NetDriver->PackageMap->GetEntityIdFromObject(NetOwner);

		FString Reason = FString::Printf(TEXT("Originating worker (%i) does not have authority of blocking actor. "),
										 NetDriver->VirtualWorkerTranslator->GetLocalVirtualWorkerId());

		if (BlockingActor->GetIsReplicated() && !bIsReplicated)
		{
			Reason.Append(FString::Printf(TEXT("Blocking actor replicates on originating worker but not on authoritative worker (%i). "),
										  AuthoritativeWorkerId));
		}
		else if (!bHasAuthority)
		{
			Reason.Append(
				FString::Printf(TEXT("Authoritative worker (%i) does not have authority of blocking actor. "), AuthoritativeWorkerId));
		}
		else if (!IsValid(NetOwner))
		{
			Reason.Append(FString::Printf(TEXT("Blocking actor owner is not valid. ")));
		}
		else if (IsValid(NetOwner) && OriginalNetOwnerId != AuthoritativeNetOwnerId)
		{
			Reason.Append(FString::Printf(TEXT("Blocking actor has different owner (%llu) on authoritative worker (%i). "),
										  AuthoritativeNetOwnerId, AuthoritativeWorkerId));
		}
		else if (bIsLocked)
		{
			Reason.Append(FString::Printf(TEXT("Blocking actor is locked on authoritative worker (%i). "), AuthoritativeWorkerId));
		}
		else if (NetDriver->LockingPolicy->IsLocked(BlockingActor))
		{
			Reason.Append(FString::Printf(TEXT("Blocking actor is locked on originating worker. ")));
		}
		else if (bCanMigrate)
		{
			Reason.Append(FString::Printf(TEXT("Authoritative worker (%i) believes blocked actor should migrate to worker (%i). "),
										  AuthoritativeWorkerId, DestinationWorkerId));
		}

		return FString::Printf(TEXT("Prevented owning actor %s (%llu)'s hierarchy from migrating because of blocking actor %s (%llu). %s"),
							   *GetNameSafe(NetOwner), OriginalNetOwnerId, *BlockingActor->GetName(), BlockedEntityId, *Reason);
	}
};

} // namespace SpatialGDK
