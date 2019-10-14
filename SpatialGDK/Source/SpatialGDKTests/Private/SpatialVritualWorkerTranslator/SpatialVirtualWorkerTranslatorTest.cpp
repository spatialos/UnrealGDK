// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestDefinitions.h"

#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Utils/SchemaUtils.h"
#include "UObject/UObjectGlobals.h"

#include "CoreMinimal.h"

#include <WorkerSDK/improbable/c_schema.h>

#define VIRTUALWORKERTRANSLATOR_TEST(TestName) \
	TEST(Core, UVirtualWorkerTranslator, TestName)

VIRTUALWORKERTRANSLATOR_TEST(Given_the_default_mapping_WHEN_nothing_has_changed_THEN_return_the_mapping)
{
	// Currently the class is initialized with the following dummy data.
	// 	VirtualToPhysicalWorkerMapping.Add(1, "VW_A");
	// 	VirtualToPhysicalWorkerMapping.Add(2, "VW_B");
	// 	VirtualToPhysicalWorkerMapping.Add(3, "VW_C");
	// 	VirtualToPhysicalWorkerMapping.Add(4, "VW_D");
	USpatialVirtualWorkerTranslator* translator = NewObject<USpatialVirtualWorkerTranslator>();
	translator->Init(nullptr);

	TestTrue("There is a mapping for virtual worker 1", translator->GetPhysicalWorkerForVirtualWorker(1) != nullptr);
	TestTrue("VW_A returned", translator->GetPhysicalWorkerForVirtualWorker(1)->Equals("VW_A"));
	TestTrue("Worker 5 doesn't exist", translator->GetPhysicalWorkerForVirtualWorker(5) == nullptr);
	return true;
}

VIRTUALWORKERTRANSLATOR_TEST(Given_the_default_mapping_WHEN_it_is_updated_THEN_return_the_updated_mapping)
{
	// Currently the class is initialized with the following dummy data.
	// 	VirtualToPhysicalWorkerMapping.Add(1, "VW_A");
	// 	VirtualToPhysicalWorkerMapping.Add(2, "VW_B");
	// 	VirtualToPhysicalWorkerMapping.Add(3, "VW_C");
	// 	VirtualToPhysicalWorkerMapping.Add(4, "VW_D");
	USpatialVirtualWorkerTranslator* translator = NewObject<USpatialVirtualWorkerTranslator>();
	translator->Init(nullptr);

	// Replace the mapping with a new map from "the network".
	Worker_ComponentData Data = {};
	Data.component_id = SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID;
	Data.schema_type = Schema_CreateComponentData();
	Schema_Object* DataObject = Schema_GetComponentDataFields(Data.schema_type);

	// The new mapping only has the following entry:
	// 	VirtualToPhysicalWorkerMapping.Add(2, "VW_E");
	// 	VirtualToPhysicalWorkerMapping.Add(3, "VW_F");
	Schema_Object* FirstEntryObject = Schema_AddObject(DataObject, SpatialConstants::TRANSLATION_VIRTUAL_WORKER_MAPPING_ID);
	Schema_AddUint32(FirstEntryObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID, 2);
	SpatialGDK::AddStringToSchema(FirstEntryObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME, "VW_E");

	Schema_Object* SecondEntryObject = Schema_AddObject(DataObject, SpatialConstants::TRANSLATION_VIRTUAL_WORKER_MAPPING_ID);
	Schema_AddUint32(SecondEntryObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID, 3);
	SpatialGDK::AddStringToSchema(SecondEntryObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME, "VW_F");

	// Now apply the mapping to the translator and test the result.
	translator->ApplyVirtualWorkerManagerData(Data);
	
	TestTrue("There is a mapping for virtual worker 2", translator->GetPhysicalWorkerForVirtualWorker(2) != nullptr);
	TestTrue("VW_B overwritten with VW_E", translator->GetPhysicalWorkerForVirtualWorker(2)->Equals("VW_E"));

	TestTrue("There is a mapping for virtual worker 3", translator->GetPhysicalWorkerForVirtualWorker(3) != nullptr);
	TestTrue("VW_B overwritten with VW_F", translator->GetPhysicalWorkerForVirtualWorker(3)->Equals("VW_F"));

	TestTrue("Worker for 1 no longer exists", translator->GetPhysicalWorkerForVirtualWorker(1) == nullptr);
	return true;
}
