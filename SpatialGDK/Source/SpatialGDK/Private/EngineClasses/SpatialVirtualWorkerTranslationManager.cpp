// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialVirtualWorkerTranslationManager.h"

#include "EngineClasses/SpatialPackageMapClient.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialOSDispatcherInterface.h"
#include "SpatialConstants.h"
#include "Utils/EntityFactory.h"
#include "Utils/SchemaUtils.h"

DEFINE_LOG_CATEGORY(LogSpatialVirtualWorkerTranslationManager);

SpatialVirtualWorkerTranslationManager::SpatialVirtualWorkerTranslationManager(SpatialOSWorkerInterface* InConnection,
																			   SpatialVirtualWorkerTranslator* InTranslator)
	: Translator(InTranslator)
	, Connection(InConnection)
	, VirtualToPhysicalWorkerMapping({})
	, TotalServerCrashCount(0)
	, NumVirtualWorkers(0)
	, bWorkerEntityQueryInFlight(false)
	, ClaimPartitionHandler(*InConnection)
{
}

void SpatialVirtualWorkerTranslationManager::SetNumberOfVirtualWorkers(const uint32 InNumVirtualWorkers)
{
	UE_LOG(LogSpatialVirtualWorkerTranslationManager, Log, TEXT("TranslationManager is configured to look for %d workers"),
		   InNumVirtualWorkers);

	NumVirtualWorkers = InNumVirtualWorkers;

	// Currently, this should only be called once on startup. In the future we may allow for more flexibility.
	VirtualWorkersToAssign.Reserve(NumVirtualWorkers);

	for (uint32 i = 1; i <= NumVirtualWorkers; i++)
	{
		VirtualWorkersToAssign.Emplace(i);
	}
}

void SpatialVirtualWorkerTranslationManager::AuthorityChanged(const Worker_ComponentSetAuthorityChangeOp& AuthOp)
{
	check(AuthOp.component_set_id == SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID
		  || AuthOp.component_set_id == SpatialConstants::SERVER_WORKER_ENTITY_AUTH_COMPONENT_SET_ID);

	const bool bAuthoritative = AuthOp.authority == WORKER_AUTHORITY_AUTHORITATIVE;

	if (!bAuthoritative)
	{
		// A healthy server should never lose auth (we supported worker recovery, but that only happens for disconnected servers).
		UE_LOG(LogSpatialVirtualWorkerTranslationManager, Error,
			   TEXT("Lost authority over the translation mapping. This should never happen."));
		return;
	}

	const int32 ExistingTranslatorMappingCount = Translator->GetMappingCount();
	// Fresh deployment. We need to create partition entities before we start assigning virtual workers.
	if (ExistingTranslatorMappingCount == 0)
	{
		UE_LOG(LogSpatialVirtualWorkerTranslationManager, Log,
			   TEXT("Gained authority over the VirtualWorker translation, spawning partition entities."));
		SpawnPartitionEntitiesForVirtualWorkerIds();
	}
	// Previously snapshot partition auth server crashed. Need to find that server's entry in the translator mapping
	// so we're ready to reassign the virtual worker when the replacement server starts.
	else if (Translator->IsReady())
	{
		// Because this server has just gained authority, the local VTM state will be empty, so we copy over
		// from the translator.
		VirtualToPhysicalWorkerMapping = Translator->VirtualToPhysicalWorkerMapping;

		UE_LOG(LogSpatialVirtualWorkerTranslationManager, Warning,
			   TEXT("Newly elected VTM server received authority. Cleaning up translator map."));
		CleanupTranslatorMappingAfterAuthorityChange();
	}
	// Restarting from snapshot: reusing partition entities but have a whole new set of partition entities.
	else if (ExistingTranslatorMappingCount == NumVirtualWorkers)
	{
		// Partitions already exist, reclaim them with latest server worker entities
		UE_LOG(LogSpatialVirtualWorkerTranslationManager, Log,
			   TEXT("Gained authority over the VirtualWorker translation after snapshot restart. Resetting virtual worker mapping."));
		ResetVirtualWorkerMappingAfterSnapshotReset();
		QueryForServerWorkerEntities();
	}
	else
	{
		UE_LOG(LogSpatialVirtualWorkerTranslationManager, Error,
			   TEXT("Gained authority with invalid translator mapping count. Are you attempting to load a snapshot with a different load "
					"balancing strategy? Expected (%d) Present (%d)"),
			   NumVirtualWorkers, ExistingTranslatorMappingCount);
	}
}

void SpatialVirtualWorkerTranslationManager::SetKnownServerSystemEntities(const TArray<Worker_EntityId>& ServerSystemEntities)
{
	KnownServerSystemEntities = ServerSystemEntities;
}

void SpatialVirtualWorkerTranslationManager::OnSystemEntityRemoved(const Worker_EntityId DisconnectedSystemEntityId)
{
	uint32 DisconnectedServers = 0;

	// If we finish this loop without hitting anything, we assume it's a client that disconnected.
	for (auto& VirtualWorkerInfo : VirtualToPhysicalWorkerMapping)
	{
		if (VirtualWorkerInfo.Value.ServerSystemWorkerEntity == DisconnectedSystemEntityId)
		{
			UE_LOG(LogSpatialVirtualWorkerTranslationManager, Warning, TEXT("VTM identified disconnected server %s"),
				   *VirtualWorkerInfo.Value.PhysicalWorkerName);
			UE_LOG(LogSpatialVirtualWorkerTranslationManager, Warning, TEXT(" - virtual worker %ld"), VirtualWorkerInfo.Key);
			UE_LOG(LogSpatialVirtualWorkerTranslationManager, Warning, TEXT(" - partition %lld"), VirtualWorkerInfo.Value.PartitionId);
			UE_LOG(LogSpatialVirtualWorkerTranslationManager, Warning, TEXT(" - system entity %lld"),
				   VirtualWorkerInfo.Value.ServerSystemWorkerEntity);

			CleanupUnhandledVirtualWorker(VirtualWorkerInfo.Key);

			DisconnectedServers++;
		}
	}

	TotalServerCrashCount += DisconnectedServers;

	SendVirtualWorkerMappingUpdate();
}

void SpatialVirtualWorkerTranslationManager::CleanupTranslatorMappingAfterAuthorityChange()
{
	TArray<VirtualWorkerId> MissingVirtualWorkers;

	for (auto& VirtualWorkerInfo : VirtualToPhysicalWorkerMapping)
	{
		if (!KnownServerSystemEntities.Contains(VirtualWorkerInfo.Value.ServerSystemWorkerEntity))
		{
			UE_LOG(LogSpatialVirtualWorkerTranslationManager, Warning,
				   TEXT(" - detected virtual worker %ld with missing auth server system entity ID"), VirtualWorkerInfo.Key);
			MissingVirtualWorkers.Add(VirtualWorkerInfo.Key);
		}
	}

	// This is a different loop in case multiple partitions are now missing auth server system entities, and we don't
	// want to mess with deleting from collections while iterating through them.
	for (const auto& VirtualWorkerId : MissingVirtualWorkers)
	{
		CleanupUnhandledVirtualWorker(VirtualWorkerId);
	}

	TotalServerCrashCount += MissingVirtualWorkers.Num();

	SendVirtualWorkerMappingUpdate();
}

void SpatialVirtualWorkerTranslationManager::CleanupUnhandledVirtualWorker(const VirtualWorkerId VirtualWorker)
{
	// Get the current translator mapping for the relevant virtual worker.
	SpatialGDK::VirtualWorkerInfo* VirtualWorkerMappingInfo = VirtualToPhysicalWorkerMapping.Find(VirtualWorker);
	checkf(VirtualWorkerMappingInfo != nullptr,
		   TEXT("Failed to unset virtual worker %ld translator mapping after auth server system entity detected missing. Could not find "
				"mapping."),
		   VirtualWorker);

	// Delete the server worker entity for the crashed server.
	UE_LOG(LogSpatialVirtualWorkerTranslationManager, Warning, TEXT(" - deleting corresponding server worker entity: %lld"),
		   VirtualWorkerMappingInfo->ServerWorkerEntity);
	Translator->NetDriver->Connection->SendDeleteEntityRequest(VirtualWorkerMappingInfo->ServerWorkerEntity,
															   SpatialGDK::RETRY_UNTIL_COMPLETE);

	// Just tidying up internal state keeping.
	// In future, we could consider using this to broadcast to other servers that this virtual worker is now unhandled.
	VirtualWorkerMappingInfo->PhysicalWorkerName = FString();
	VirtualWorkerMappingInfo->ServerWorkerEntity = SpatialConstants::INVALID_ENTITY_ID;

	// Add the corresponding virtual worker ID for the missing server back into the unassigned list.
	VirtualWorkersToAssign.Emplace(VirtualWorker);
}

void SpatialVirtualWorkerTranslationManager::SpawnPartitionEntitiesForVirtualWorkerIds()
{
	UE_LOG(LogSpatialVirtualWorkerTranslationManager, Log, TEXT("Spawning partition entities for %d virtual workers"),
		   VirtualWorkersToAssign.Num());
	for (const VirtualWorkerId VirtualWorkerId : VirtualWorkersToAssign)
	{
		const Worker_EntityId PartitionId = Translator->NetDriver->PackageMap->AllocateNewEntityId();
		UE_LOG(LogSpatialVirtualWorkerTranslationManager, Log, TEXT("- Virtual Worker: %d. Entity: %lld. "), VirtualWorkerId, PartitionId);
		SpawnPartitionEntity(PartitionId, VirtualWorkerId);
	}
}

void SpatialVirtualWorkerTranslationManager::ResetVirtualWorkerMappingAfterSnapshotReset()
{
	VirtualToPhysicalWorkerMapping.Empty();
	for (const VirtualWorkerId VirtualWorker : VirtualWorkersToAssign)
	{
		const Worker_PartitionId PartitionId = Translator->GetPartitionEntityForVirtualWorker(VirtualWorker);
		check(PartitionId != SpatialConstants::INVALID_ENTITY_ID);
		VirtualToPhysicalWorkerMapping.Add(
			VirtualWorker, SpatialGDK::VirtualWorkerInfo{ VirtualWorker, FString(), SpatialConstants::INVALID_ENTITY_ID, PartitionId,
														  SpatialConstants::INVALID_ENTITY_ID });
	}

	TotalServerCrashCount = 0;
}

void SpatialVirtualWorkerTranslationManager::Advance(const TArray<Worker_Op>& Ops)
{
	CreateEntityHandler.ProcessOps(Ops);
	ClaimPartitionHandler.ProcessOps(Ops);
	QueryHandler.ProcessOps(Ops);
}

TArray<Worker_PartitionId> SpatialVirtualWorkerTranslationManager::GetAllPartitions() const
{
	TArray<Worker_PartitionId> Partitions;
	Partitions.SetNum(VirtualToPhysicalWorkerMapping.Num());
	for (const auto& WorkerInfo : VirtualToPhysicalWorkerMapping)
	{
		Partitions.Emplace(WorkerInfo.Value.PartitionId);
	}
	return Partitions;
}

// This method is called on the worker who is authoritative over the translation mapping. Based on the results of the
// system entity query, assign the VirtualWorkerIds to the workers represented by the system entities.
bool SpatialVirtualWorkerTranslationManager::AllServerWorkersAreReady(const Worker_EntityQueryResponseOp& Op, uint32& ServerWorkersNotReady)
{
	ServerWorkersNotReady = 0;

	// The query response is an array of entities. Each of these represents a worker.
	for (uint32_t i = 0; i < Op.result_count; ++i)
	{
		const Worker_Entity& Entity = Op.results[i];
		for (uint32_t j = 0; j < Entity.component_count; j++)
		{
			const Worker_ComponentData& Data = Entity.components[j];
			// Server worker entities which represent workers have a component on them which specifies the SpatialOS worker ID,
			// which is the string we use to refer to them as a physical worker ID.
			if (Data.component_id == SpatialConstants::SERVER_WORKER_COMPONENT_ID)
			{
				const Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

				// The translator should only acknowledge workers that are ready to begin play. This means we can make
				// guarantees based on where non-GSM-authoritative servers canBeginPlay=true as an AddComponent
				// or ComponentUpdate op. This affects how startup Actors are treated in a zoned environment.
				const bool bWorkerIsReadyToBeginPlay =
					SpatialGDK::GetBoolFromSchema(ComponentObject, SpatialConstants::SERVER_WORKER_READY_TO_BEGIN_PLAY_ID);
				if (!bWorkerIsReadyToBeginPlay)
				{
					ServerWorkersNotReady++;
				}
			}
		}
	}

	return ServerWorkersNotReady == 0;
}

// This method is called on the worker who is authoritative over the translation mapping. Based on the results of the
// system entity query, assign the VirtualWorkerIds to the workers represented by the system entities.
void SpatialVirtualWorkerTranslationManager::AssignPartitionsToEachServerWorkerFromQueryResponse(const Worker_EntityQueryResponseOp& Op)
{
	const TArray<TTuple<Worker_EntityId, SpatialGDK::ServerWorker>> ServerWorkers = ExtractServerWorkerDataFromQueryResponse(Op);

	int ServerNum = 0;
	for (auto& VirtualWorkerInfo : VirtualToPhysicalWorkerMapping)
	{
		VirtualWorkerId VirtualWorker = VirtualWorkerInfo.Key;
		Worker_PartitionId PartitionId = VirtualWorkerInfo.Value.PartitionId;

		// Get data from the server worker we're assign
		const TTuple<Worker_EntityId, SpatialGDK::ServerWorker>& ServerWorkerToAssign = ServerWorkers[ServerNum];
		const FString& WorkerName = ServerWorkerToAssign.Value.WorkerName;
		const Worker_EntityId& ServerSystemWorkerEntity = ServerWorkerToAssign.Value.SystemEntityId;

		// Locally assign the relevant server details.
		// This is used for lookup if ClaimPartition commands fail.
		VirtualWorkerInfo.Value.PhysicalWorkerName = WorkerName;
		VirtualWorkerInfo.Value.ServerSystemWorkerEntity = ServerSystemWorkerEntity;
		VirtualWorkerInfo.Value.ServerWorkerEntity = ServerWorkerToAssign.Key;

		UE_LOG(LogSpatialVirtualWorkerTranslationManager, Warning,
			   TEXT("Assigned VirtualWorker %d with partition ID %lld to simulate on server %s"), VirtualWorker, PartitionId, *WorkerName);

		ClaimPartitionHandler.ClaimPartition(ServerSystemWorkerEntity, PartitionId);

		ServerNum++;
	}
}

TArray<TTuple<Worker_EntityId, SpatialGDK::ServerWorker>> SpatialVirtualWorkerTranslationManager::ExtractServerWorkerDataFromQueryResponse(
	const Worker_EntityQueryResponseOp& Op)
{
	TArray<TTuple<Worker_EntityId, SpatialGDK::ServerWorker>> ServerWorkers;
	ServerWorkers.SetNum(Op.result_count);

	// The query response is an array of entities. Each of these represents a worker.
	for (uint32_t i = 0; i < Op.result_count; ++i)
	{
		const Worker_Entity& Entity = Op.results[i];
		for (uint32_t j = 0; j < Entity.component_count; j++)
		{
			const Worker_ComponentData& Data = Entity.components[j];

			// Server worker entities which represent workers have a component on them which specifies the SpatialOS worker ID,
			// which is the string we use to refer to them as a physical worker ID.
			if (Data.component_id == SpatialConstants::SERVER_WORKER_COMPONENT_ID)
			{
				ServerWorkers[i] = MakeTuple(Entity.entity_id, SpatialGDK::ServerWorker(Data));
			}
		}
	}

	return ServerWorkers;
}

// This will be called on the worker authoritative for the translation mapping to push the new version of the map
// to the SpatialOS storage.
void SpatialVirtualWorkerTranslationManager::SendVirtualWorkerMappingUpdate() const
{
	// Serialize data internal state to component data
	TArray<SpatialGDK::VirtualWorkerInfo> VirtualWorkerMappingList;
	VirtualToPhysicalWorkerMapping.GenerateValueArray(VirtualWorkerMappingList);

	const SpatialGDK::VirtualWorkerTranslation ComponentData(VirtualWorkerMappingList, TotalServerCrashCount);

	// Loopback the component data to the local translator.
	check(Translator != nullptr);
	Translator->ApplyVirtualWorkerTranslation(ComponentData);

	// Send to the Runtime.
	check(Connection != nullptr);
	FWorkerComponentUpdate Update = ComponentData.CreateVirtualWorkerTranslationUpdate();
	Connection->SendComponentUpdate(SpatialConstants::INITIAL_VIRTUAL_WORKER_TRANSLATOR_ENTITY_ID, &Update);
}

void SpatialVirtualWorkerTranslationManager::SpawnPartitionEntity(Worker_EntityId PartitionId, VirtualWorkerId VirtualWorkerId)
{
	TArray<FWorkerComponentData> Components = SpatialGDK::EntityFactory::CreatePartitionEntityComponents(
		PartitionId, Translator->NetDriver->InterestFactory.Get(), Translator->LoadBalanceStrategy.Get(), VirtualWorkerId,
		Translator->NetDriver->DebugCtx != nullptr);

	const Worker_RequestId RequestId =
		Connection->SendCreateEntityRequest(MoveTemp(Components), &PartitionId, SpatialGDK::RETRY_UNTIL_COMPLETE);

	CreateEntityDelegate OnCreateWorkerEntityResponse;
	OnCreateWorkerEntityResponse.BindLambda([this, VirtualWorkerId](const Worker_CreateEntityResponseOp& Op) {
		if (Op.status_code == WORKER_STATUS_CODE_SUCCESS)
		{
			UE_LOG(LogSpatialVirtualWorkerTranslationManager, Log,
				   TEXT("Successfully created partition entity. "
						"Entity: %lld. Virtual Worker: %d"),
				   Op.entity_id, VirtualWorkerId);
			OnPartitionEntityCreation(Op.entity_id, VirtualWorkerId);
			return;
		}

		UE_LOG(LogSpatialVirtualWorkerTranslationManager, Error,
			   TEXT("Partition entity creation failed: \"%s\". "
					"Entity: %lld. Virtual Worker: %d"),
			   UTF8_TO_TCHAR(Op.message), Op.entity_id, VirtualWorkerId);
	});

	CreateEntityHandler.AddRequest(RequestId, MoveTemp(OnCreateWorkerEntityResponse));
}

void SpatialVirtualWorkerTranslationManager::OnPartitionEntityCreation(Worker_EntityId PartitionId, VirtualWorkerId VirtualWorker)
{
	VirtualToPhysicalWorkerMapping.Add(
		VirtualWorker, SpatialGDK::VirtualWorkerInfo{ VirtualWorker, FString(), SpatialConstants::INVALID_ENTITY_ID, PartitionId,
													  SpatialConstants::INVALID_ENTITY_ID });

	UE_LOG(LogSpatialVirtualWorkerTranslationManager, Log,
		   TEXT("Adding translation manager mapping. Virtual worker %d -> Partition entity %lld"), VirtualWorker, PartitionId);

	if (VirtualToPhysicalWorkerMapping.Num() == VirtualWorkersToAssign.Num())
	{
		UE_LOG(LogSpatialVirtualWorkerTranslationManager, Log,
			   TEXT("Found all %d required partitions, querying for server worker entities"), VirtualWorkersToAssign.Num());
		QueryForServerWorkerEntities();
	}
	else
	{
		UE_LOG(LogSpatialVirtualWorkerTranslationManager, Verbose,
			   TEXT("Didn't find all %d required partitions, only found %d, currently have:"), VirtualWorkersToAssign.Num(),
			   VirtualToPhysicalWorkerMapping.Num());
		for (const auto& VirtualWorkerInfo : VirtualToPhysicalWorkerMapping)
		{
			UE_LOG(LogSpatialVirtualWorkerTranslationManager, Verbose, TEXT(" - virtual worker %d -> partition entity %lld"),
				   VirtualWorkerInfo.Key, VirtualWorkerInfo.Value.PartitionId);
		}
	}
}

void SpatialVirtualWorkerTranslationManager::QueryForServerWorkerEntities()
{
	UE_LOG(LogSpatialVirtualWorkerTranslationManager, Log, TEXT("Sending query for server worker entities"));

	if (bWorkerEntityQueryInFlight)
	{
		UE_LOG(LogSpatialVirtualWorkerTranslationManager, Warning,
			   TEXT("Trying to query for worker entities while a previous query is still in flight!"));
		return;
	}

	// Create a query for all the server worker entities. This will be used
	// to find physical workers which the virtual workers will map to.
	Worker_ComponentConstraint WorkerEntityComponentConstraint{};
	WorkerEntityComponentConstraint.component_id = SpatialConstants::SERVER_WORKER_COMPONENT_ID;

	Worker_Constraint WorkerEntityConstraint{};
	WorkerEntityConstraint.constraint_type = WORKER_CONSTRAINT_TYPE_COMPONENT;
	WorkerEntityConstraint.constraint.component_constraint = WorkerEntityComponentConstraint;

	Worker_EntityQuery WorkerEntityQuery{};
	WorkerEntityQuery.constraint = WorkerEntityConstraint;

	// Make the query.
	check(Connection != nullptr);
	const Worker_RequestId RequestID = Connection->SendEntityQueryRequest(&WorkerEntityQuery, SpatialGDK::RETRY_UNTIL_COMPLETE);
	bWorkerEntityQueryInFlight = true;

	// Register a method to handle the query response.
	EntityQueryDelegate ServerWorkerEntityQueryDelegate;
	ServerWorkerEntityQueryDelegate.BindRaw(this, &SpatialVirtualWorkerTranslationManager::ServerWorkerEntityQueryDelegate);
	QueryHandler.AddRequest(RequestID, ServerWorkerEntityQueryDelegate);
}

// This method allows the translation manager to deal with the returned list of server worker entities when they are received.
// Note that this worker may have lost authority for the translation mapping in the meantime, so it's possible the
// returned information will be thrown away.
void SpatialVirtualWorkerTranslationManager::ServerWorkerEntityQueryDelegate(const Worker_EntityQueryResponseOp& Op)
{
	bWorkerEntityQueryInFlight = false;

	if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
	{
		UE_LOG(LogSpatialVirtualWorkerTranslationManager, Error, TEXT("Server worker entity query failed: %s, retrying."),
			   UTF8_TO_TCHAR(Op.message));
		return;
	}

	if (Op.result_count != NumVirtualWorkers)
	{
		UE_LOG(LogSpatialVirtualWorkerTranslationManager, Log,
			   TEXT("Waiting for all virtual workers to be assigned before publishing translation update. "
					"We currently have %i workers connected out of the %i required"),
			   Op.result_count, NumVirtualWorkers);
		QueryForServerWorkerEntities();
		return;
	}

	uint32 ServerWorkersNotReady;
	if (!AllServerWorkersAreReady(Op, ServerWorkersNotReady))
	{
		UE_LOG(LogSpatialVirtualWorkerTranslationManager, Warning,
			   TEXT("Query found correct number of server workers but %d were not ready."), ServerWorkersNotReady);
		QueryForServerWorkerEntities();
		return;
	}

	UE_LOG(LogSpatialVirtualWorkerTranslationManager, Log, TEXT("Found all required server worker entities ready to play."));
	AssignPartitionsToEachServerWorkerFromQueryResponse(Op);

	SendVirtualWorkerMappingUpdate();
}

void SpatialVirtualWorkerTranslationManager::TryClaimPartitionForRecoveredWorker(const Worker_EntityId ServerWorkerEntity,
																				 Schema_ComponentData* ServerWorkerComponentData)
{
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(ServerWorkerComponentData);
	const FString WorkerName = SpatialGDK::GetStringFromSchema(ComponentObject, SpatialConstants::SERVER_WORKER_NAME_ID);
	const Worker_EntityId SystemEntityId = Schema_GetEntityId(ComponentObject, SpatialConstants::SERVER_WORKER_SYSTEM_ENTITY_ID);

	if (VirtualWorkersToAssign.Num() == 0)
	{
		UE_LOG(LogSpatialVirtualWorkerTranslationManager, Error,
			   TEXT("Server %s restarted detected but no virtual workers available to be reassigned"), *WorkerName);
		return;
	}

	const VirtualWorkerId VirtualWorkerToAssign = VirtualWorkersToAssign.Pop();

	SpatialGDK::VirtualWorkerInfo* VirtualWorkerMappingInfo = VirtualToPhysicalWorkerMapping.Find(VirtualWorkerToAssign);
	if (VirtualWorkerMappingInfo == nullptr)
	{
		UE_LOG(LogSpatialVirtualWorkerTranslationManager, Error,
			   TEXT("Failed to assign restarted worker %s to virtual worker %ld. Couldn't find any existing mapping in the translator."),
			   *WorkerName, VirtualWorkerToAssign);

		UE_LOG(LogSpatialVirtualWorkerTranslator, Warning, TEXT("\t-> Strategy: %s"));

		for (const auto& Entry : VirtualToPhysicalWorkerMapping)
		{
			UE_LOG(LogSpatialVirtualWorkerTranslator, Warning,
				   TEXT("\t-> Assignment: Virtual Worker %d to %s with server worker entity: %lld"), Entry.Key,
				   *(Entry.Value.PhysicalWorkerName), Entry.Value.ServerWorkerEntity);
		}
		return;
	}

	const Worker_PartitionId PartitionToAssign = VirtualWorkerMappingInfo->PartitionId;

	UE_LOG(LogSpatialVirtualWorkerTranslationManager, Warning, TEXT("VTM TryClaimPartitionForRecoveredWorker: %s"), *WorkerName);
	UE_LOG(LogSpatialVirtualWorkerTranslationManager, Warning, TEXT(" - virtual worker to assign: %ld"), VirtualWorkerToAssign);
	UE_LOG(LogSpatialVirtualWorkerTranslationManager, Warning, TEXT(" - partition to assign: %lld"), PartitionToAssign);
	UE_LOG(LogSpatialVirtualWorkerTranslationManager, Warning, TEXT(" - restarted server worker entity: %lld"), ServerWorkerEntity);
	UE_LOG(LogSpatialVirtualWorkerTranslationManager, Warning, TEXT(" - restarted system entity: %lld"), SystemEntityId);

	VirtualWorkerMappingInfo->ServerWorkerEntity = ServerWorkerEntity;
	VirtualWorkerMappingInfo->ServerSystemWorkerEntity = SystemEntityId;
	VirtualWorkerMappingInfo->PhysicalWorkerName = WorkerName;

	SendVirtualWorkerMappingUpdate();

	UE_LOG(LogSpatialVirtualWorkerTranslationManager, Warning,
		   TEXT("Reassigned VirtualWorker %d with partition ID %lld to simulate on worker %s"), VirtualWorkerToAssign, PartitionToAssign,
		   *WorkerName);

	ClaimPartitionHandler.ClaimPartition(SystemEntityId, PartitionToAssign);
}
