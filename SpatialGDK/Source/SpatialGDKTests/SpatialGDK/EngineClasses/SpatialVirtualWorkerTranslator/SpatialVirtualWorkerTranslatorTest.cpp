// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestDefinitions.h"

#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Utils/SchemaUtils.h"
#include "UObject/UObjectGlobals.h"

#include "CoreMinimal.h"

#include <WorkerSDK/improbable/c_schema.h>

#define VIRTUALWORKERTRANSLATOR_TEST(TestName) \
	GDK_TEST(Core, SpatialVirtualWorkerTranslator, TestName)

VIRTUALWORKERTRANSLATOR_TEST(Given_init_is_not_called_THEN_return_not_ready)
{
	TUniquePtr<SpatialVirtualWorkerTranslator> translator = MakeUnique<SpatialVirtualWorkerTranslator>();

	TestFalse("Uninitialized Translator is not ready.", translator->IsReady());

	return true;
}

VIRTUALWORKERTRANSLATOR_TEST(Given_init_and_set_desired_worker_count_called_THEN_return_ready)
{
	TUniquePtr<SpatialVirtualWorkerTranslator> translator = MakeUnique<SpatialVirtualWorkerTranslator>();
	translator->Init(nullptr);
	translator->SetDesiredVirtualWorkerCount(1);  // unimportant random value.

	TestTrue("Initialized Translator is ready.", translator->IsReady());

	return true;
}

VIRTUALWORKERTRANSLATOR_TEST(Given_no_mapping_WHEN_nothing_has_changed_THEN_return_no_mappings)
{
	// The class is initialized with no data.
	TUniquePtr<SpatialVirtualWorkerTranslator> translator = MakeUnique<SpatialVirtualWorkerTranslator>();

	TestTrue("Worker 1 doesn't exist", translator->GetPhysicalWorkerForVirtualWorker(1) == nullptr);

	return true;
}

VIRTUALWORKERTRANSLATOR_TEST(Given_no_mapping_WHEN_it_is_updated_THEN_return_the_updated_mapping)
{
	// The class is initialized with no data.
	TUniquePtr<SpatialVirtualWorkerTranslator> translator = MakeUnique<SpatialVirtualWorkerTranslator>();

	// Create a base mapping.
	Worker_ComponentData Data = {};
	Data.component_id = SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID;
	Data.schema_type = Schema_CreateComponentData();
	Schema_Object* DataObject = Schema_GetComponentDataFields(Data.schema_type);

	// The mapping only has the following entries:
	// 	VirtualToPhysicalWorkerMapping.Add(2, "VW_E");
	// 	VirtualToPhysicalWorkerMapping.Add(3, "VW_F");
	Schema_Object* FirstEntryObject = Schema_AddObject(DataObject, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID);
	Schema_AddUint32(FirstEntryObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID, 2);
	SpatialGDK::AddStringToSchema(FirstEntryObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME, "VW_E");

	Schema_Object* SecondEntryObject = Schema_AddObject(DataObject, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID);
	Schema_AddUint32(SecondEntryObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID, 3);
	SpatialGDK::AddStringToSchema(SecondEntryObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME, "VW_F");

	// Now apply the mapping to the translator and test the result.
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
	translator->ApplyVirtualWorkerManagerData(ComponentObject);
	
	TestTrue("There is a mapping for virtual worker 2", translator->GetPhysicalWorkerForVirtualWorker(2) != nullptr);
	TestTrue("VW_B overwritten with VW_E", translator->GetPhysicalWorkerForVirtualWorker(2)->Equals("VW_E"));

	TestTrue("There is a mapping for virtual worker 3", translator->GetPhysicalWorkerForVirtualWorker(3) != nullptr);
	TestTrue("VW_B overwritten with VW_F", translator->GetPhysicalWorkerForVirtualWorker(3)->Equals("VW_F"));

	TestTrue("There is no mapping for virtual worker 1", translator->GetPhysicalWorkerForVirtualWorker(1) == nullptr);

	return true;
}

VIRTUALWORKERTRANSLATOR_TEST(Given_no_mapping_WHEN_query_response_received_THEN_return_the_updated_mapping)
{
	// The class is initialized with no data.
	TUniquePtr<SpatialVirtualWorkerTranslator> translator = MakeUnique<SpatialVirtualWorkerTranslator>();

	

	// Create a base mapping.
	Worker_ComponentData Data = {};
	Data.component_id = SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID;
	Data.schema_type = Schema_CreateComponentData();
	Schema_Object* DataObject = Schema_GetComponentDataFields(Data.schema_type);

	// The mapping only has the following entries:
	// 	VirtualToPhysicalWorkerMapping.Add(2, "VW_E");
	// 	VirtualToPhysicalWorkerMapping.Add(3, "VW_F");
	Schema_Object* FirstEntryObject = Schema_AddObject(DataObject, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID);
	Schema_AddUint32(FirstEntryObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID, 2);
	SpatialGDK::AddStringToSchema(FirstEntryObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME, "VW_E");

	Schema_Object* SecondEntryObject = Schema_AddObject(DataObject, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID);
	Schema_AddUint32(SecondEntryObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID, 3);
	SpatialGDK::AddStringToSchema(SecondEntryObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME, "VW_F");

	// Now apply the mapping to the translator and test the result.
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
	translator->ApplyVirtualWorkerManagerData(ComponentObject);

	TestTrue("There is a mapping for virtual worker 2", translator->GetPhysicalWorkerForVirtualWorker(2) != nullptr);
	TestTrue("VW_B overwritten with VW_E", translator->GetPhysicalWorkerForVirtualWorker(2)->Equals("VW_E"));

	TestTrue("There is a mapping for virtual worker 3", translator->GetPhysicalWorkerForVirtualWorker(3) != nullptr);
	TestTrue("VW_B overwritten with VW_F", translator->GetPhysicalWorkerForVirtualWorker(3)->Equals("VW_F"));

	TestTrue("There is no mapping for virtual worker 1", translator->GetPhysicalWorkerForVirtualWorker(1) == nullptr);

	return true;
}
