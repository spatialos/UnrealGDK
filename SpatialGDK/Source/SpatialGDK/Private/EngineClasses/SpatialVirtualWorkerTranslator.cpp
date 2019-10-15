// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"

DEFINE_LOG_CATEGORY(LogSpatialVirtualWorkerTranslator);

namespace
{
	// The translation schema is a list of Mappings, where each entry has a virtual and physical worker ID. 
	inline void GetMappingFromSchema(Schema_Object* Object, Schema_FieldId Id, VirtualToPhysicalWorkerMap& Mapping)
	{
		int32 TranslationCount = static_cast<int32>(Schema_GetObjectCount(Object, 1));
		Mapping.Reserve(TranslationCount);

		for (int32 i = 0; i < TranslationCount; i++)
		{
			// Get each entry of the list and then unpack the virtual and physical IDs from the entry.
			Schema_Object* MappingObject = Schema_IndexObject(Object, SpatialConstants::TRANSLATION_VIRTUAL_WORKER_MAPPING_ID, i);
			uint32 VirtualWorkerId = Schema_GetUint32(MappingObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID);
			FString PhysicalWorkerName = SpatialGDK::GetStringFromSchema(MappingObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME);

			// Insert each into the provided map.
			Mapping.Add(VirtualWorkerId, PhysicalWorkerName);
		}
	}

	// For each entry in the map, write a VirtualWorkerMapping type object to the Schema object.
	inline void WriteMappingToSchema(Schema_Object* Object, Schema_FieldId Id, VirtualToPhysicalWorkerMap& Mapping) {
		for (auto& Entry : Mapping) {
			Schema_Object* EntryObject = Schema_AddObject(Object, SpatialConstants::TRANSLATION_VIRTUAL_WORKER_MAPPING_ID);
			Schema_AddUint32(EntryObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID, Entry.Key);
			SpatialGDK::AddStringToSchema(EntryObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME, Entry.Value);
		}
	}
}  // anonymous namesapce

USpatialVirtualWorkerTranslator::USpatialVirtualWorkerTranslator(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, NetDriver(nullptr)
{
}

void USpatialVirtualWorkerTranslator::Init(USpatialNetDriver* InNetDriver)
{
	NetDriver = InNetDriver;

	// TODO(zoning): This should generate the mappings based on connected workers.
	VirtualToPhysicalWorkerMapping.Add(1, "VW_A");
	VirtualToPhysicalWorkerMapping.Add(2, "VW_B");
	VirtualToPhysicalWorkerMapping.Add(3, "VW_C");
	VirtualToPhysicalWorkerMapping.Add(4, "VW_D");
}

const FString* USpatialVirtualWorkerTranslator::GetPhysicalWorkerForVirtualWorker(VirtualWorkerId id)
{
	return VirtualToPhysicalWorkerMapping.Find(id);
}

void USpatialVirtualWorkerTranslator::ApplyVirtualWorkerManagerData(const Worker_ComponentData& Data)
{
	if (NetDriver)
	{
		UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("(%s) ApplyVirtualWorkerManagerData"), *NetDriver->Connection->GetWorkerId());
	}

	Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

	VirtualToPhysicalWorkerMapping.Empty();

	// The translation schema is a list of Mappings, where each entry has a virtual and physical worker ID. 
 	GetMappingFromSchema(ComponentObject, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID, VirtualToPhysicalWorkerMapping);
}

void USpatialVirtualWorkerTranslator::OnComponentUpdated(const Worker_ComponentUpdateOp& Op)
{
	if (Op.update.component_id == SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID)
	{
		// TODO(zoning): Check for whether the ACL should be updated.
	}
}

