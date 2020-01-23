// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialVirtualWorkerTranslationManager.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialReceiver.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"

DEFINE_LOG_CATEGORY(LogSpatialVirtualWorkerTranslationManager);

SpatialVirtualWorkerTranslationManager::SpatialVirtualWorkerTranslationManager(
	USpatialReceiver* InReceiver,
	USpatialWorkerConnection* InConnection,
	SpatialVirtualWorkerTranslator* InTranslator)
	: Receiver(InReceiver)
	, Connection(InConnection)
	, Translator(InTranslator)
	, bWorkerEntityQueryInFlight(false)
{}

void SpatialVirtualWorkerTranslationManager::AddVirtualWorkerIds(const TSet<VirtualWorkerId>& InVirtualWorkerIds)
{
	// Currently, this should only be called once on startup. In the future we may allow for more
	// flexibility. 
	check(UnassignedVirtualWorkers.IsEmpty());
	for (VirtualWorkerId VirtualWorkerId : InVirtualWorkerIds)
	{
		UnassignedVirtualWorkers.Enqueue(VirtualWorkerId);
	}
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
	QueryForWorkerEntities();
}

// For each entry in the map, write a VirtualWorkerMapping type object to the Schema object.
void SpatialVirtualWorkerTranslationManager::WriteMappingToSchema(Schema_Object* Object) const
{
	for (const auto& Entry : VirtualToPhysicalWorkerMapping)
	{
		Schema_Object* EntryObject = Schema_AddObject(Object, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID);
		Schema_AddUint32(EntryObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID, Entry.Key);
		SpatialGDK::AddStringToSchema(EntryObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME, Entry.Value);
	}
}

// This method is called on the worker who is authoritative over the translation mapping. Based on the results of the
// system entity query, assign the VirtualWorkerIds to the workers represented by the system entities.
void SpatialVirtualWorkerTranslationManager::ConstructVirtualWorkerMappingFromQueryResponse(const Worker_EntityQueryResponseOp& Op)
{
	// The query response is an array of entities. Each of these represents a worker.
	for (uint32_t i = 0; i < Op.result_count; ++i)
	{
		const Worker_Entity& Entity = Op.results[i];
		for (uint32_t j = 0; j < Entity.component_count; j++)
		{
			const Worker_ComponentData& Data = Entity.components[j];
			// System entities which represent workers have a component on them which specifies the SpatialOS worker ID,
			// which is the string we use to refer to them as a physical worker ID.
			if (Data.component_id == SpatialConstants::WORKER_COMPONENT_ID)
			{
				const Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

				const FString& WorkerType = SpatialGDK::GetStringFromSchema(ComponentObject, SpatialConstants::WORKER_TYPE_ID);

				// TODO(zoning): Currently, this only works if server workers never die. Once we want to support replacing
				// workers, this will need to process UnassignWorker before processing AssignWorker.
				if (WorkerType.Equals(SpatialConstants::DefaultServerWorkerType.ToString()) &&
					!UnassignedVirtualWorkers.IsEmpty())
				{
					AssignWorker(SpatialGDK::GetStringFromSchema(ComponentObject, SpatialConstants::WORKER_ID_ID));
				}
			}
		}
	}
}

// This will be called on the worker authoritative for the translation mapping to push the new version of the map
// to the SpatialOS storage.
void SpatialVirtualWorkerTranslationManager::SendVirtualWorkerMappingUpdate()
{
	// Construct the mapping update based on the local virtual worker to physical worker mapping.
	Worker_ComponentUpdate Update = {};
	Update.component_id = SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID;
	Update.schema_type = Schema_CreateComponentUpdate();
	Schema_Object* UpdateObject = Schema_GetComponentUpdateFields(Update.schema_type);

	WriteMappingToSchema(UpdateObject);

	check(Connection.IsValid());
	Connection->SendComponentUpdate(SpatialConstants::INITIAL_VIRTUAL_WORKER_TRANSLATOR_ENTITY_ID, &Update);

	// The Translator on the worker which hosts the manager won't get the component update notification,
	// so send it across directly.
	check(Translator != nullptr);
	Translator->ApplyVirtualWorkerManagerData(UpdateObject);
}

void SpatialVirtualWorkerTranslationManager::QueryForWorkerEntities()
{
	UE_LOG(LogSpatialVirtualWorkerTranslationManager, Log, TEXT("Sending query for WorkerEntities"));

	if (bWorkerEntityQueryInFlight)
	{
		UE_LOG(LogSpatialVirtualWorkerTranslationManager, Warning, TEXT("Trying to query for worker entities while a previous query is still in flight!"));
		return;
	}

	// Create a query for all the system entities which represent workers. This will be used
	// to find physical workers which the virtual workers will map to.
	Worker_ComponentConstraint WorkerEntityComponentConstraint{};
	WorkerEntityComponentConstraint.component_id = SpatialConstants::WORKER_COMPONENT_ID;

	Worker_Constraint WorkerEntityConstraint{};
	WorkerEntityConstraint.constraint_type = WORKER_CONSTRAINT_TYPE_COMPONENT;
	WorkerEntityConstraint.constraint.component_constraint = WorkerEntityComponentConstraint;

	Worker_EntityQuery WorkerEntityQuery{};
	WorkerEntityQuery.constraint = WorkerEntityConstraint;
	WorkerEntityQuery.result_type = WORKER_RESULT_TYPE_SNAPSHOT;

	// Make the query.
	check(Connection.IsValid());
	Worker_RequestId RequestID = Connection->SendEntityQueryRequest(&WorkerEntityQuery);
	bWorkerEntityQueryInFlight = true;

	// Register a method to handle the query response.
	EntityQueryDelegate WorkerEntityQueryDelegate;
	WorkerEntityQueryDelegate.BindRaw(this, &SpatialVirtualWorkerTranslationManager::WorkerEntityQueryDelegate);
	check(Receiver.IsValid());
	Receiver->AddEntityQueryDelegate(RequestID, WorkerEntityQueryDelegate);
}

// This method allows the translation manager to deal with the returned list of connection entities when they are received.
// Note that this worker may have lost authority for the translation mapping in the meantime, so it's possible the
// returned information will be thrown away.
void SpatialVirtualWorkerTranslationManager::WorkerEntityQueryDelegate(const Worker_EntityQueryResponseOp& Op)
{
	bWorkerEntityQueryInFlight = false;

	if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
	{
		UE_LOG(LogSpatialVirtualWorkerTranslationManager, Warning, TEXT("Could not find Worker Entities via entity query: %s, retrying."), UTF8_TO_TCHAR(Op.message));
	}
	else
	{
		UE_LOG(LogSpatialVirtualWorkerTranslationManager, Log, TEXT(" Processing Worker Entity query response"));
		ConstructVirtualWorkerMappingFromQueryResponse(Op);
	}

	// If the translation mapping is complete, publish it. Otherwise retry the worker entity query.
	if (UnassignedVirtualWorkers.IsEmpty())
	{
		SendVirtualWorkerMappingUpdate();
	}
	else
	{
		UE_LOG(LogSpatialVirtualWorkerTranslationManager, Log, TEXT("Waiting for all virtual workers to be assigned before publishing translation update."));
		QueryForWorkerEntities();
	}
}

void SpatialVirtualWorkerTranslationManager::AssignWorker(const PhysicalWorkerName& Name)
{
	// Get a VirtualWorkerId from the list of unassigned work.
	VirtualWorkerId Id;
	UnassignedVirtualWorkers.Dequeue(Id);

	VirtualToPhysicalWorkerMapping.Add(Id, Name);

	UE_LOG(LogSpatialVirtualWorkerTranslationManager, Log, TEXT("Assigned VirtualWorker %d to simulate on Worker %s"), Id, *Name);
}
