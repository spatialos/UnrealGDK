// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialReceiver.h"
#include "Interop/SpatialStaticComponentView.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"

DEFINE_LOG_CATEGORY(LogSpatialVirtualWorkerTranslator);

SpatialVirtualWorkerTranslator::SpatialVirtualWorkerTranslator()
	: NetDriver(nullptr)
	, bWorkerEntityQueryInFlight(false)
	, bIsReady(false)
	, LocalVirtualWorkerId(SpatialConstants::INVALID_VIRTUAL_WORKER_ID)
{}

void SpatialVirtualWorkerTranslator::Init(USpatialNetDriver* InNetDriver)
{
	NetDriver = InNetDriver;
	// If this is being run from tests, NetDriver will be null.
	WorkerId = (NetDriver != nullptr) ? NetDriver->Connection->GetWorkerId() : "InvalidWorkerId";
}

void SpatialVirtualWorkerTranslator::AddVirtualWorkerIds(const TSet<VirtualWorkerId>& InVirtualWorkerIds)
{
	// Currently, this should only be called once on startup. In the future we may allow for more
	// flexibility. 
	if (bIsReady)
	{
		UE_LOG(LogSpatialVirtualWorkerTranslator, Warning, TEXT("(%s) SetDesiredVirtualWorkerCount called after the translator is ready, ignoring."), *WorkerId);
	}

	UnassignedVirtualWorkers.Empty();
	for (VirtualWorkerId VirtualWorkerId : InVirtualWorkerIds)
	{
		UnassignedVirtualWorkers.Enqueue(VirtualWorkerId);
	}
}

const FString* SpatialVirtualWorkerTranslator::GetPhysicalWorkerForVirtualWorker(VirtualWorkerId id)
{
	return VirtualToPhysicalWorkerMapping.Find(id);
}

void SpatialVirtualWorkerTranslator::ApplyVirtualWorkerManagerData(Schema_Object* ComponentObject)
{
    UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("(%s) ApplyVirtualWorkerManagerData"), *WorkerId);

	// The translation schema is a list of Mappings, where each entry has a virtual and physical worker ID. 
	ApplyMappingFromSchema(ComponentObject);

	for (const auto& Entry : VirtualToPhysicalWorkerMapping)
	{
		UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("(%s) assignment: %d - %s"), *WorkerId, Entry.Key, *(Entry.Value));
	}
}

void SpatialVirtualWorkerTranslator::OnComponentUpdated(const Worker_ComponentUpdateOp& Op)
{
	if (Op.update.component_id == SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID)
	{
		// TODO(zoning): Check for whether the ACL should be updated.
	}
}

void SpatialVirtualWorkerTranslator::AuthorityChanged(const Worker_AuthorityChangeOp& AuthOp)
{
	int i = 0;
	i++;
	const bool bAuthoritative = AuthOp.authority == WORKER_AUTHORITY_AUTHORITATIVE;

	if (AuthOp.component_id == SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID)
	{
		UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("(%s) Authority over the VirtualWorkerTranslator has changed. This worker %s authority."), *WorkerId, bAuthoritative ? TEXT("now has") : TEXT("does not have"));

		if (bAuthoritative)
		{
			// TODO(zoning): The prototype had an unassigned workers list. Need to follow up with Tim/Chris about whether
			// that is necessary or we can continue to use the (possibly) stale list until we receive the query response.

			// Query for all connection entities, so we can detect if some worker has died and needs to be updated in
			// the mapping.
			QueryForWorkerEntities();
		}
	}
	// TODO(zoning): If authority is received on the ACL component, we may need to update it.
}

// The translation schema is a list of Mappings, where each entry has a virtual and physical worker ID.
// This method should only be called on workers who are not authoritative over the mapping.
// TODO(harkness): Is this true? I think this will be called once the first time we get authority?
void SpatialVirtualWorkerTranslator::ApplyMappingFromSchema(Schema_Object* Object)
{
	if (NetDriver != nullptr &&
		NetDriver->StaticComponentView->HasAuthority(SpatialConstants::INITIAL_VIRTUAL_WORKER_TRANSLATOR_ENTITY_ID, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID))
	{
		UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("(%s) ApplyMappingFromSchema called, but this worker is authoritative, ignoring"), *WorkerId);
	}

	// Resize the map to accept the new data.
	VirtualToPhysicalWorkerMapping.Empty();
	int32 TranslationCount = (int32)Schema_GetObjectCount(Object, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID);
	VirtualToPhysicalWorkerMapping.Reserve(TranslationCount);

	for (int32 i = 0; i < TranslationCount; i++)
	{
		// Get each entry of the list and then unpack the virtual and physical IDs from the entry.
		Schema_Object* MappingObject = Schema_IndexObject(Object, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID, i);
		VirtualWorkerId VirtualWorkerId = Schema_GetUint32(MappingObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID);
		FString PhysicalWorkerName = SpatialGDK::GetStringFromSchema(MappingObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME);

		// Insert each into the provided map.
		VirtualToPhysicalWorkerMapping.Add(VirtualWorkerId, PhysicalWorkerName);
	}

	if (TranslationCount)
	{
		bIsReady = true;
	}
}

// For each entry in the map, write a VirtualWorkerMapping type object to the Schema object.
void SpatialVirtualWorkerTranslator::WriteMappingToSchema(Schema_Object* Object)
{
	for (auto& Entry : VirtualToPhysicalWorkerMapping)
	{
		Schema_Object* EntryObject = Schema_AddObject(Object, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID);
		Schema_AddUint32(EntryObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID, Entry.Key);
		SpatialGDK::AddStringToSchema(EntryObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME, Entry.Value);
	}
}

// This method is called on the worker who is authoritative over the translation mapping. Based on the results of the
// system entity query, assign the VirtualWorkerIds to the workers represented by the system entities.
void SpatialVirtualWorkerTranslator::ConstructVirtualWorkerMappingFromQueryResponse(const Worker_EntityQueryResponseOp& Op)
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
	bIsReady = true;
}

// This will be called on the worker authoritative for the translation mapping to push the new version of the map
// to the spatialOS storage.
void SpatialVirtualWorkerTranslator::SendVirtualWorkerMappingUpdate()
{
	if (NetDriver == nullptr)
	{
		return;
	}

	UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("(%s) SendVirtualWorkerMappingUpdate"), *WorkerId);

	check(NetDriver->StaticComponentView->HasAuthority(SpatialConstants::INITIAL_VIRTUAL_WORKER_TRANSLATOR_ENTITY_ID, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID));

	// Construct the mapping update based on the local virtual worker to physical worker mapping.
	Worker_ComponentUpdate Update = {};
	Update.component_id = SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID;
	Update.schema_type = Schema_CreateComponentUpdate();
	Schema_Object* UpdateObject = Schema_GetComponentUpdateFields(Update.schema_type);

	WriteMappingToSchema(UpdateObject);

	NetDriver->Connection->SendComponentUpdate(SpatialConstants::INITIAL_VIRTUAL_WORKER_TRANSLATOR_ENTITY_ID, &Update);

	// Broadcast locally since we won't receive the ComponentUpdate on this worker.
	// This is disabled until the Enforcer is available to update ACLs.
	// OnWorkerAssignmentChanged.Broadcast(VirtualWorkerAssignment);
}

void SpatialVirtualWorkerTranslator::QueryForWorkerEntities()
{
	if (NetDriver == nullptr)
	{
		return;
	}

	UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("(%s) Sending query for WorkerEntities"), *WorkerId);

	checkf(!bWorkerEntityQueryInFlight, TEXT("(%s) Trying to query for worker entities while a previous query is still in flight!"), *WorkerId);

	if (!NetDriver->StaticComponentView->HasAuthority(SpatialConstants::INITIAL_VIRTUAL_WORKER_TRANSLATOR_ENTITY_ID, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID))
	{
		UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("(%s) Trying QueryForWorkerEntities, but don't have authority over VIRTUAL_WORKER_MANAGER_COMPONENT.  Aborting processing."), *WorkerId);
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
	Worker_RequestId RequestID; 
	RequestID = NetDriver->Connection->SendEntityQueryRequest(&WorkerEntityQuery);
	bWorkerEntityQueryInFlight = true;

	// Register a method to handle the query response.
	EntityQueryDelegate WorkerEntityQueryDelegate;
	WorkerEntityQueryDelegate.BindRaw(this, &SpatialVirtualWorkerTranslator::WorkerEntityQueryDelegate);
	NetDriver->Receiver->AddEntityQueryDelegate(RequestID, WorkerEntityQueryDelegate);
}

// This method allows the translator to deal with the returned list of connection entities when they are received.
// Note that this worker may have lost authority for the translation mapping in the meantime, so it's possible the
// returned information will be thrown away.
void SpatialVirtualWorkerTranslator::WorkerEntityQueryDelegate(const Worker_EntityQueryResponseOp& Op)
{
	if (!NetDriver->StaticComponentView->HasAuthority(SpatialConstants::INITIAL_VIRTUAL_WORKER_TRANSLATOR_ENTITY_ID, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID))
	{
		UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("(%s) Received response to WorkerEntityQuery, but don't have authority over VIRTUAL_WORKER_MANAGER_COMPONENT.  Aborting processing."), *WorkerId);
	}
	else if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
	{
		UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("(%s) Could not find Worker Entities via entity query: %s"), *WorkerId, UTF8_TO_TCHAR(Op.message));
	}
	else if (Op.result_count == 0)
	{
		UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("(%s) Worker Entity query shows that Worker Entities do not yet exist in the world."), *WorkerId);
	}
	else
	{
		UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("(%s) Processing Worker Entity query response"), *WorkerId);
		ConstructVirtualWorkerMappingFromQueryResponse(Op);
		SendVirtualWorkerMappingUpdate();
	}

	bWorkerEntityQueryInFlight = false;
}

void SpatialVirtualWorkerTranslator::AssignWorker(const PhysicalWorkerName& Name)
{
	check(!UnassignedVirtualWorkers.IsEmpty());
	check(NetDriver->StaticComponentView->HasAuthority(SpatialConstants::INITIAL_VIRTUAL_WORKER_TRANSLATOR_ENTITY_ID, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID));

	// Get a VirtualWorkerId from the list of unassigned work.
	VirtualWorkerId Id;
	UnassignedVirtualWorkers.Dequeue(Id);

	VirtualToPhysicalWorkerMapping.Add(Id, Name);

	if (Name == WorkerId)
	{
		LocalVirtualWorkerId = Id;
	}

	UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("(%s) Assigned VirtualWorker %d to simulate on Worker %s"), *WorkerId, Id, *Name);
}
