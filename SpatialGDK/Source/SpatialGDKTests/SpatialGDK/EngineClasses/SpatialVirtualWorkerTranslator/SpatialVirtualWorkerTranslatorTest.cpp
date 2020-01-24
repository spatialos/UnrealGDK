// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Utils/SchemaUtils.h"
#include "UObject/UObjectGlobals.h"

#include "CoreMinimal.h"

#include <WorkerSDK/improbable/c_schema.h>

#define VIRTUALWORKERTRANSLATOR_TEST(TestName) \
	GDK_TEST(Core, SpatialVirtualWorkerTranslator, TestName)

VIRTUALWORKERTRANSLATOR_TEST(Given_init_is_not_called_THEN_return_not_ready)
{
	TUniquePtr<SpatialVirtualWorkerTranslator> translator = MakeUnique<SpatialVirtualWorkerTranslator>(nullptr, SpatialConstants::TRANSLATOR_UNSET_PHYSICAL_NAME);

	TestFalse("Uninitialized Translator is not ready.", translator->IsReady());

	return true;
}

VIRTUALWORKERTRANSLATOR_TEST(Given_no_mapping_WHEN_nothing_has_changed_THEN_return_no_mappings)
{
	// The class is initialized with no data.
	TUniquePtr<SpatialVirtualWorkerTranslator> translator = MakeUnique<SpatialVirtualWorkerTranslator>(nullptr, SpatialConstants::TRANSLATOR_UNSET_PHYSICAL_NAME);

	TestTrue("Worker 1 doesn't exist", translator->GetPhysicalWorkerForVirtualWorker(1) == nullptr);

	return true;
}

VIRTUALWORKERTRANSLATOR_TEST(Given_no_mapping_WHEN_receiving_incomplete_mapping_THEN_ignore_it)
{
	// The class is initialized with no data.
	TUniquePtr<SpatialVirtualWorkerTranslator> translator = MakeUnique<SpatialVirtualWorkerTranslator>(nullptr, SpatialConstants::TRANSLATOR_UNSET_PHYSICAL_NAME);

	// Create a base mapping.
	Worker_ComponentData Data = {};
	Data.component_id = SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID;
	Data.schema_type = Schema_CreateComponentData();
	Schema_Object* DataObject = Schema_GetComponentDataFields(Data.schema_type);

	// The mapping only has the following entries:
	// 	VirtualToPhysicalWorkerMapping.Add(1, "VW_E");
	// 	VirtualToPhysicalWorkerMapping.Add(2, "VW_F");
	Schema_Object* FirstEntryObject = Schema_AddObject(DataObject, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID);
	Schema_AddUint32(FirstEntryObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID, 1);
	SpatialGDK::AddStringToSchema(FirstEntryObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME, "VW_E");

	Schema_Object* SecondEntryObject = Schema_AddObject(DataObject, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID);
	Schema_AddUint32(SecondEntryObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID, 1);
	SpatialGDK::AddStringToSchema(SecondEntryObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME, "VW_F");

	// Now apply the mapping to the translator and test the result. Because the mapping doesn't have an entry for this translator,
	// it should reject the mapping and continue to report an empty mapping.
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
	translator->ApplyVirtualWorkerManagerData(ComponentObject);

	TestTrue("There is no mapping for virtual worker 1", translator->GetPhysicalWorkerForVirtualWorker(1) == nullptr);
	TestTrue("There is no mapping for virtual worker 2", translator->GetPhysicalWorkerForVirtualWorker(2) == nullptr);

	return true;
}

VIRTUALWORKERTRANSLATOR_TEST(Given_no_mapping_WHEN_it_is_updated_THEN_return_the_updated_mapping)
{
	// The class is initialized with no data.
	TUniquePtr<SpatialVirtualWorkerTranslator> translator = MakeUnique<SpatialVirtualWorkerTranslator>(nullptr, SpatialConstants::TRANSLATOR_UNSET_PHYSICAL_NAME);

	// Create a base mapping.
	Worker_ComponentData Data = {};
	Data.component_id = SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID;
	Data.schema_type = Schema_CreateComponentData();
	Schema_Object* DataObject = Schema_GetComponentDataFields(Data.schema_type);

	// The mapping only has the following entries:
	// 	VirtualToPhysicalWorkerMapping.Add(1, "UnsetWorkerName");
	// 	VirtualToPhysicalWorkerMapping.Add(2, "VW_F");
	Schema_Object* FirstEntryObject = Schema_AddObject(DataObject, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID);
	Schema_AddUint32(FirstEntryObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID, 1);
	SpatialGDK::AddStringToSchema(FirstEntryObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME, SpatialConstants::TRANSLATOR_UNSET_PHYSICAL_NAME);

	Schema_Object* SecondEntryObject = Schema_AddObject(DataObject, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID);
	Schema_AddUint32(SecondEntryObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID, 2);
	SpatialGDK::AddStringToSchema(SecondEntryObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME, "VW_F");

	// Now apply the mapping to the translator and test the result.
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
	translator->ApplyVirtualWorkerManagerData(ComponentObject);
	
	TestTrue("There is a mapping for virtual worker 1", translator->GetPhysicalWorkerForVirtualWorker(1) != nullptr);
	TestTrue("Virtual worker 1 is UnsetWorkerName", translator->GetPhysicalWorkerForVirtualWorker(1)->Equals(SpatialConstants::TRANSLATOR_UNSET_PHYSICAL_NAME));

	TestTrue("There is a mapping for virtual worker 2", translator->GetPhysicalWorkerForVirtualWorker(2) != nullptr);
	TestTrue("VirtualWorker 2 is VW_F", translator->GetPhysicalWorkerForVirtualWorker(2)->Equals("VW_F"));

	TestTrue("There is no mapping for virtual worker 3", translator->GetPhysicalWorkerForVirtualWorker(3) == nullptr);

	return true;
}
