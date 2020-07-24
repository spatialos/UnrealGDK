// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialVirtualWorkerTranslationManager.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialOSDispatcherInterface.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"

DEFINE_LOG_CATEGORY(LogSpatialVirtualWorkerTranslationManager);

SpatialVirtualWorkerTranslationManager::SpatialVirtualWorkerTranslationManager(
	SpatialOSDispatcherInterface* InReceiver,
	SpatialOSWorkerInterface* InConnection,
	SpatialVirtualWorkerTranslator* InTranslator)
	: Receiver(InReceiver)
	, Connection(InConnection)
	, Translator(InTranslator)
	, bWorkerEntityQueryInFlight(false)
{}

void SpatialVirtualWorkerTranslationManager::SetLayerVirtualWorkerMapping(const TMap<FName, uint32>& LayerToVirtualWorker)
{
	// Currently, this should only be called once on startup. In the future we may allow for more
	// flexibility.
	VirtualWorkerId NextVirtualWorker = 1;
	for (const auto& Layer: LayerToVirtualWorker)
	{
		for (uint32 i = 0; i < Layer.Value; i++)
		{
			UnassignedLayerVirtualWorkers.Add(TPair<FName, VirtualWorkerId>{Layer.Key, NextVirtualWorker++});
		}
	}

	UE_LOG(LogSpatialVirtualWorkerTranslationManager, Log, TEXT("TranslationManager is configured to look for %d workers"), NextVirtualWorker - 1);
}

void SpatialVirtualWorkerTranslationManager::AuthorityChanged(const Worker_AuthorityChangeOp& AuthOp)
{
	check(AuthOp.component_id == SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID);

	const bool bAuthoritative = AuthOp.authority == WORKER_AUTHORITY_AUTHORITATIVE;

	if (!bAuthoritative)
	{
		UE_LOG(LogSpatialVirtualWorkerTranslationManager, Error, TEXT("Lost authority over the translation mapping. This is not supported."));
		return;
	}

	UE_LOG(LogSpatialVirtualWorkerTranslationManager, Log, TEXT("This worker now has authority over the VirtualWorker translation."));

	// TODO(zoning): The prototype had an unassigned workers list. Need to follow up with Tim/Chris about whether
	// that is necessary or we can continue to use the (possibly) stale list until we receive the query response.

	// Query for all connection entities, so we can detect if some worker has died and needs to be updated in
	// the mapping.
	QueryForServerWorkerEntities();
}

// For each entry in the map, write a VirtualWorkerMapping type object to the Schema object.
void SpatialVirtualWorkerTranslationManager::WriteMappingToSchema(Schema_Object* Object) const
{
	for (const auto& Entry : VirtualToPhysicalWorkerMapping)
	{
		Schema_Object* EntryObject = Schema_AddObject(Object, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID);
		Schema_AddUint32(EntryObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID, Entry.Key);
		SpatialGDK::AddStringToSchema(EntryObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME, Entry.Value.Key);
		Schema_AddEntityId(EntryObject, SpatialConstants::MAPPING_SERVER_WORKER_ENTITY_ID, Entry.Value.Value);
	}
}

// This will be called on the worker authoritative for the translation mapping to push the new version of the map
// to the SpatialOS storage.
void SpatialVirtualWorkerTranslationManager::SendVirtualWorkerMappingUpdate() const
{
	// Construct the mapping update based on the local virtual worker to physical worker mapping.
	FWorkerComponentUpdate Update = {};
	Update.component_id = SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID;
	Update.schema_type = Schema_CreateComponentUpdate();
	Schema_Object* UpdateObject = Schema_GetComponentUpdateFields(Update.schema_type);

	WriteMappingToSchema(UpdateObject);

	// The Translator on the worker which hosts the manager won't get the component update notification,
	// so send it across directly.
	check(Translator != nullptr);
	Translator->ApplyVirtualWorkerManagerData(UpdateObject);

	check(Connection != nullptr);
	Connection->SendComponentUpdate(SpatialConstants::INITIAL_VIRTUAL_WORKER_TRANSLATOR_ENTITY_ID, &Update);
}

void SpatialVirtualWorkerTranslationManager::QueryForServerWorkerEntities()
{
	UE_LOG(LogSpatialVirtualWorkerTranslationManager, Log, TEXT("Sending query for WorkerEntities"));

	if (bWorkerEntityQueryInFlight)
	{
		UE_LOG(LogSpatialVirtualWorkerTranslationManager, Warning, TEXT("Trying to query for worker entities while a previous query is still in flight!"));
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
	WorkerEntityQuery.result_type = WORKER_RESULT_TYPE_SNAPSHOT;

	// Make the query.
	check(Connection != nullptr);
	Worker_RequestId RequestID = Connection->SendEntityQueryRequest(&WorkerEntityQuery);
	bWorkerEntityQueryInFlight = true;

	// Register a method to handle the query response.
	EntityQueryDelegate ServerWorkerEntityQueryDelegate;
	ServerWorkerEntityQueryDelegate.BindRaw(this, &SpatialVirtualWorkerTranslationManager::ServerWorkerEntityQueryDelegate);
	check(Receiver != nullptr);
	Receiver->AddEntityQueryDelegate(RequestID, ServerWorkerEntityQueryDelegate);
}

// This method allows the translation manager to deal with the returned list of server worker entities when they are received.
// Note that this worker may have lost authority for the translation mapping in the meantime, so it's possible the
// returned information will be thrown away.
void SpatialVirtualWorkerTranslationManager::ServerWorkerEntityQueryDelegate(const Worker_EntityQueryResponseOp& Op)
{
	bWorkerEntityQueryInFlight = false;

	if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
	{
		UE_LOG(LogSpatialVirtualWorkerTranslationManager, Warning, TEXT("Could not find ServerWorker Entities via "
			"entity query: %s. Retrying."), UTF8_TO_TCHAR(Op.message));
		QueryForServerWorkerEntities();
		return;
	}

	TArray<ServerInfo> PendingServersToAssign = GetPendingServersToAssignFromQuery(Op);
	if (PendingServersToAssign.Num() != UnassignedLayerVirtualWorkers.Num())
	{
		UE_LOG(LogSpatialVirtualWorkerTranslationManager, Log, TEXT("Waiting for all virtual workers to be assigned "
			"before publishing translation update."));
		QueryForServerWorkerEntities();
		return;
	}

	UE_LOG(LogSpatialVirtualWorkerTranslationManager, Log, TEXT("ServerWorker Entity query successfully found %d "
		"workers."), PendingServersToAssign.Num());

	AssignServersWithLayerHints(PendingServersToAssign);
	checkf(PendingServersToAssign.Num() == UnassignedLayerVirtualWorkers.Num(), TEXT("When assigning servers with "
		"layer hints, we developed a mismatch in available server and pending virtaul worker counts, this is fatal."
		"PendingServersToAssign: %d. UnassignedLayerVirtualWorkers: %d."), PendingServersToAssign.Num(),
		UnassignedLayerVirtualWorkers.Num())

	uint32 CurrentVirtualWorkerIndex = 0;
	for (const ServerInfo& RemainingServersToAssign : PendingServersToAssign)
	{
		AssignWorker(RemainingServersToAssign, UnassignedLayerVirtualWorkers[CurrentVirtualWorkerIndex++].Value);
	}

	SendVirtualWorkerMappingUpdate();
}

TArray<SpatialVirtualWorkerTranslationManager::ServerInfo> SpatialVirtualWorkerTranslationManager::GetPendingServersToAssignFromQuery(const Worker_EntityQueryResponseOp& Op)
{
	TArray<ServerInfo> PendingServersToAssign;
	PendingServersToAssign.Reserve(Op.result_count);

	// The query response is an array of entities. Each of these represents a worker.
	for (uint32_t i = 0; i < Op.result_count; ++i)
	{
		const Worker_Entity& Entity = Op.results[i];
		for (uint32_t j = 0; j < Entity.component_count; j++)
		{
			const Worker_ComponentData& Data = Entity.components[j];
			// System entities which represent workers have a component on them which specifies the SpatialOS worker ID,
			// which is the string we use to refer to them as a physical worker ID.
			if (Data.component_id == SpatialConstants::SERVER_WORKER_COMPONENT_ID)
			{
				const Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

				// The translator should only acknowledge workers that are ready to begin play. This means we can make
				// guarantees based on where non-GSM-authoritative servers canBeginPlay=true as an AddComponent
				// or ComponentUpdate op. This affects how startup Actors are treated in a zoned environment.
				const bool bWorkerIsReadyToBeginPlay = SpatialGDK::GetBoolFromSchema(ComponentObject, SpatialConstants::SERVER_WORKER_READY_TO_BEGIN_PLAY_ID);
				if (!bWorkerIsReadyToBeginPlay)
				{
					continue;
				}

				PendingServersToAssign.Add(ServerInfo
                    {
                        SpatialGDK::GetStringFromSchema(ComponentObject, SpatialConstants::SERVER_WORKER_NAME_ID),
                        Entity.entity_id,
                        *SpatialGDK::GetStringFromSchema(ComponentObject, SpatialConstants::SERVER_WORKER_LAYER_HINT_ID)
                    });
			}
		}
	}

	return PendingServersToAssign;
}

void SpatialVirtualWorkerTranslationManager::AssignServersWithLayerHints(TArray<ServerInfo>& PendingServersToAssign)
{
	TArray<FString> AssignedServers;
	AssignedServers.Reserve(UnassignedLayerVirtualWorkers.Num());
	for (const ServerInfo& PendingServer : PendingServersToAssign)
	{
		// Does one of our connected servers express a preference for which layer it is in?
		if (!PendingServer.LayerHint.IsNone())
		{
			uint32 AssignedLayerVirtualWorkerIndex = INDEX_NONE;
			for (auto i = 0; i < UnassignedLayerVirtualWorkers.Num(); i++)
			{
				// If the server's preferred layer exists (and has unassigned virtual workers), assign that server.
				TPair<FName, VirtualWorkerId> CurrentLayerVirtualWorker = UnassignedLayerVirtualWorkers[i];
				if (PendingServer.LayerHint.IsEqual(CurrentLayerVirtualWorker.Key))
				{
					AssignWorker(PendingServer, CurrentLayerVirtualWorker.Value);
					AssignedServers.Add(PendingServer.WorkerName);
					AssignedLayerVirtualWorkerIndex = i;
				}
			}

			if (AssignedLayerVirtualWorkerIndex == INDEX_NONE)
			{
				UE_LOG(LogSpatialVirtualWorkerTranslationManager, Warning, TEXT("Server worker %s had layer hint %s, "
                    "but there were no workers for this layer in the load balancing strategy."),
                    *PendingServer.WorkerName, *PendingServer.LayerHint.ToString());
				continue;
			}

			UE_LOG(LogSpatialVirtualWorkerTranslationManager, Log, TEXT("Server worker %s was successfully assigned "
				"to be part of hinted layer: %s"), *PendingServer.WorkerName, *PendingServer.LayerHint.ToString());

			UnassignedLayerVirtualWorkers.RemoveAt(AssignedLayerVirtualWorkerIndex);
		}
	}

	PendingServersToAssign.RemoveAll([&](ServerInfo& Server)
	{
		return AssignedServers.Contains(Server.WorkerName);
	});
}

void SpatialVirtualWorkerTranslationManager::AssignWorker(const ServerInfo& ServerInfo, const VirtualWorkerId& VirtualWorkerId)
{
	if (PhysicalToVirtualWorkerMapping.Contains(ServerInfo.WorkerName))
	{
		return;
	}

	VirtualToPhysicalWorkerMapping.Add(VirtualWorkerId, MakeTuple(ServerInfo.WorkerName, ServerInfo.ServerWorkerEntityId));
	PhysicalToVirtualWorkerMapping.Add(ServerInfo.WorkerName, VirtualWorkerId);

	UE_LOG(LogSpatialVirtualWorkerTranslationManager, Log, TEXT("Assigned VirtualWorker %d to simulate on Worker %s"),
		VirtualWorkerId, *ServerInfo.WorkerName);
}
