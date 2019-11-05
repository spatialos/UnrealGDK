// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"

DEFINE_LOG_CATEGORY(LogSpatialVirtualWorkerTranslator);

SpatialVirtualWorkerTranslator::SpatialVirtualWorkerTranslator()
	: NetDriver(nullptr)
{
}

void SpatialVirtualWorkerTranslator::Init(USpatialNetDriver* InNetDriver)
{
	NetDriver = InNetDriver;
}

const FString* SpatialVirtualWorkerTranslator::GetPhysicalWorkerForVirtualWorker(VirtualWorkerId id)
{
	return VirtualToPhysicalWorkerMapping.Find(id);
}

void SpatialVirtualWorkerTranslator::ApplyVirtualWorkerManagerData(const Worker_ComponentData& Data)
{
	if (NetDriver)
	{
		UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("(%s) ApplyVirtualWorkerManagerData"), *NetDriver->Connection->GetWorkerId());
	}

	Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

	VirtualToPhysicalWorkerMapping.Empty();

	// The translation schema is a list of Mappings, where each entry has a virtual and physical worker ID. 
	ApplyMappingFromSchema(ComponentObject);
}

void SpatialVirtualWorkerTranslator::OnComponentUpdated(const Worker_ComponentUpdateOp& Op)
{
	if (Op.update.component_id == SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID)
	{
		// TODO(zoning): Check for whether the ACL should be updated.
	}
}

// The translation schema is a list of Mappings, where each entry has a virtual and physical worker ID. 
void SpatialVirtualWorkerTranslator::ApplyMappingFromSchema(Schema_Object* Object)
{
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
