// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Utils/SchemaUtils.h"
#include "UObject/UObjectGlobals.h"

#include "CoreMinimal.h"

#include <WorkerSDK/improbable/c_schema.h>

#define VIRTUALWORKERTRANSLATOR_TEST(TestName) \
	GDK_TEST(Core, SpatialVirtualWorkerTranslator, TestName)

VIRTUALWORKERTRANSLATOR_TEST(GIVEN_init_is_not_called_THEN_return_not_ready)
{
	TUniquePtr<SpatialVirtualWorkerTranslator> translator = MakeUnique<SpatialVirtualWorkerTranslator>(nullptr, SpatialConstants::TRANSLATOR_UNSET_PHYSICAL_NAME);

	TestFalse("Translator without local virtual worker ID is not ready.", translator->IsReady());

	return true;
}

VIRTUALWORKERTRANSLATOR_TEST(GIVEN_worker_name_specified_in_constructor_THEN_return_correct_local_worker_name)
{
	TUniquePtr<SpatialVirtualWorkerTranslator> translator = MakeUnique<SpatialVirtualWorkerTranslator>(nullptr, "my_worker_name");

	TestTrue("Local physical worker name returned correctly.", translator->GetLocalPhysicalWorkerName() == "my_worker_name");

	return true;
}

VIRTUALWORKERTRANSLATOR_TEST(GIVEN_no_mapping_WHEN_nothing_has_changed_THEN_return_no_mappings_and_unintialized_state)
{
	// The class is initialized with no data.
	TUniquePtr<SpatialVirtualWorkerTranslator> translator = MakeUnique<SpatialVirtualWorkerTranslator>(nullptr, SpatialConstants::TRANSLATOR_UNSET_PHYSICAL_NAME);

	TestTrue("Worker 1 doesn't exist", translator->GetPhysicalWorkerForVirtualWorker(1) == nullptr);
	TestTrue("Local virtual worker ID not known", translator->GetLocalVirtualWorkerId() == SpatialConstants::INVALID_VIRTUAL_WORKER_ID);
	TestFalse("Translator without local virtual worker ID is not ready.", translator->IsReady());

	return true;
}

VIRTUALWORKERTRANSLATOR_TEST(GIVEN_no_mapping_WHEN_receiving_empty_mapping_THEN_ignore_it)
{
	// The class is initialized with no data.
	TUniquePtr<SpatialVirtualWorkerTranslator> translator = MakeUnique<SpatialVirtualWorkerTranslator>(nullptr, SpatialConstants::TRANSLATOR_UNSET_PHYSICAL_NAME);

	// Create an empty mapping.
	Worker_ComponentData Data = {};
	Data.component_id = SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID;
	Data.schema_type = Schema_CreateComponentData();
	Schema_Object* DataObject = Schema_GetComponentDataFields(Data.schema_type);

	// Now apply the mapping to the translator and test the result. Because the mapping is empty,
	// it should ignore the mapping and continue to report an empty mapping.
	translator->ApplyVirtualWorkerManagerData(DataObject);

	TestTrue("Local virtual worker ID not known", translator->GetLocalVirtualWorkerId() == SpatialConstants::INVALID_VIRTUAL_WORKER_ID);
	TestFalse("Translator without local virtual worker ID is not ready.", translator->IsReady());

	return true;
}

VIRTUALWORKERTRANSLATOR_TEST(GIVEN_no_mapping_WHEN_receiving_incomplete_mapping_THEN_ignore_it)
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
	translator->ApplyVirtualWorkerManagerData(DataObject);

	TestTrue("There is no mapping for virtual worker 1", translator->GetPhysicalWorkerForVirtualWorker(1) == nullptr);
	TestTrue("There is no mapping for virtual worker 2", translator->GetPhysicalWorkerForVirtualWorker(2) == nullptr);
	TestTrue("Local virtual worker ID not known", translator->GetLocalVirtualWorkerId() == SpatialConstants::INVALID_VIRTUAL_WORKER_ID);
	TestFalse("Translator without local virtual worker ID is not ready.", translator->IsReady());

	return true;
}

VIRTUALWORKERTRANSLATOR_TEST(GIVEN_no_mapping_WHEN_a_valid_mapping_is_received_THEN_return_the_updated_mapping_and_become_ready)
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
	translator->ApplyVirtualWorkerManagerData(DataObject);
	
	TestTrue("There is a mapping for virtual worker 1", translator->GetPhysicalWorkerForVirtualWorker(1) != nullptr);
	TestTrue("Virtual worker 1 is UnsetWorkerName", translator->GetPhysicalWorkerForVirtualWorker(1)->Equals(SpatialConstants::TRANSLATOR_UNSET_PHYSICAL_NAME));

	TestTrue("There is a mapping for virtual worker 2", translator->GetPhysicalWorkerForVirtualWorker(2) != nullptr);
	TestTrue("VirtualWorker 2 is VW_F", translator->GetPhysicalWorkerForVirtualWorker(2)->Equals("VW_F"));

	TestTrue("There is no mapping for virtual worker 3", translator->GetPhysicalWorkerForVirtualWorker(3) == nullptr);

	TestTrue("Local virtual worker ID is known", translator->GetLocalVirtualWorkerId() == 1);
	TestTrue("Translator with local virtual worker ID is ready.", translator->IsReady());

	return true;
}

VIRTUALWORKERTRANSLATOR_TEST(GIVEN_have_a_valid_mapping_WHEN_an_invalid_mapping_is_received_THEN_ignore_it)
{
	// The class is initialized with no data.
	TUniquePtr<SpatialVirtualWorkerTranslator> translator = MakeUnique<SpatialVirtualWorkerTranslator>(nullptr, SpatialConstants::TRANSLATOR_UNSET_PHYSICAL_NAME);

	// Create a base mapping.
	Worker_ComponentData ValidData = {};
	ValidData.component_id = SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID;
	ValidData.schema_type = Schema_CreateComponentData();
	Schema_Object* ValidDataObject = Schema_GetComponentDataFields(ValidData.schema_type);

	// The mapping only has the following entries:
	// 	VirtualToPhysicalWorkerMapping.Add(1, "UnsetWorkerName");
	// 	VirtualToPhysicalWorkerMapping.Add(2, "VW_F");
	Schema_Object* FirstEntryObject = Schema_AddObject(ValidDataObject, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID);
	Schema_AddUint32(FirstEntryObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID, 1);
	SpatialGDK::AddStringToSchema(FirstEntryObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME, SpatialConstants::TRANSLATOR_UNSET_PHYSICAL_NAME);

	Schema_Object* SecondEntryObject = Schema_AddObject(ValidDataObject, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID);
	Schema_AddUint32(SecondEntryObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID, 2);
	SpatialGDK::AddStringToSchema(SecondEntryObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME, "VW_F");

	// Apply valid mapping to the translator.
	translator->ApplyVirtualWorkerManagerData(ValidDataObject);

	// Create an empty mapping.
	Worker_ComponentData EmptyData = {};
	EmptyData.component_id = SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID;
	EmptyData.schema_type = Schema_CreateComponentData();
	Schema_Object* EmptyDataObject = Schema_GetComponentDataFields(EmptyData.schema_type);

	// Now apply the mapping to the translator and test the result. Because the mapping is empty,
	// it should ignore the mapping and continue to report a valid mapping.
	translator->ApplyVirtualWorkerManagerData(EmptyDataObject);

	// Translator should return the values from the initial valid mapping
	TestTrue("There is a mapping for virtual worker 1", translator->GetPhysicalWorkerForVirtualWorker(1) != nullptr);
	TestTrue("Virtual worker 1 is UnsetWorkerName", translator->GetPhysicalWorkerForVirtualWorker(1)->Equals(SpatialConstants::TRANSLATOR_UNSET_PHYSICAL_NAME));

	TestTrue("There is a mapping for virtual worker 2", translator->GetPhysicalWorkerForVirtualWorker(2) != nullptr);
	TestTrue("VirtualWorker 2 is VW_F", translator->GetPhysicalWorkerForVirtualWorker(2)->Equals("VW_F"));

	TestTrue("There is no mapping for virtual worker 3", translator->GetPhysicalWorkerForVirtualWorker(3) == nullptr);

	TestTrue("Local virtual worker ID is known", translator->GetLocalVirtualWorkerId() == 1);
	TestTrue("Translator with local virtual worker ID is ready.", translator->IsReady());

	return true;
}

VIRTUALWORKERTRANSLATOR_TEST(GIVEN_have_a_valid_mapping_WHEN_another_valid_mapping_is_received_THEN_update_accordingly)
{
	// The class is initialized with no data.
	TUniquePtr<SpatialVirtualWorkerTranslator> translator = MakeUnique<SpatialVirtualWorkerTranslator>(nullptr, "ValidWorkerOne");

	// Create a valid initial mapping.
	Worker_ComponentData FirstValidData = {};
	FirstValidData.component_id = SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID;
	FirstValidData.schema_type = Schema_CreateComponentData();
	Schema_Object* FirstValidDataObject = Schema_GetComponentDataFields(FirstValidData.schema_type);

	// The mapping only has the following entries:
	// 	VirtualToPhysicalWorkerMapping.Add(1, "ValidWorkerOne");
	// 	VirtualToPhysicalWorkerMapping.Add(2, "ValidWorkerTwo");
	Schema_Object* FirstValidDataFirstEntryObject = Schema_AddObject(FirstValidDataObject, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID);
	Schema_AddUint32(FirstValidDataFirstEntryObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID, 1);
	SpatialGDK::AddStringToSchema(FirstValidDataFirstEntryObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME, "ValidWorkerOne");

	Schema_Object* FirstValidSecondEntryObject = Schema_AddObject(FirstValidDataObject, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID);
	Schema_AddUint32(FirstValidSecondEntryObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID, 2);
	SpatialGDK::AddStringToSchema(FirstValidSecondEntryObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME, "ValidWorkerTwo");

	// Apply valid mapping to the translator.
	translator->ApplyVirtualWorkerManagerData(FirstValidDataObject);

	// Create a second initial mapping.
	Worker_ComponentData SecondValidData = {};
	SecondValidData.component_id = SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID;
	SecondValidData.schema_type = Schema_CreateComponentData();
	Schema_Object* SecondValidDataObject = Schema_GetComponentDataFields(SecondValidData.schema_type);

	// The mapping only has the following entries:
	// 	VirtualToPhysicalWorkerMapping.Add(1, "ValidWorkerOne");
	// 	VirtualToPhysicalWorkerMapping.Add(2, "ValidWorkerThree");
	Schema_Object* SecondValidDataFirstEntryObject = Schema_AddObject(SecondValidDataObject, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID);
	Schema_AddUint32(SecondValidDataFirstEntryObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID, 1);
	SpatialGDK::AddStringToSchema(SecondValidDataFirstEntryObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME, "ValidWorkerOne");

	Schema_Object* SecondValidSecondEntryObject = Schema_AddObject(SecondValidDataObject, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID);
	Schema_AddUint32(SecondValidSecondEntryObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID, 2);
	SpatialGDK::AddStringToSchema(SecondValidSecondEntryObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME, "ValidWorkerThree");

	// Apply valid mapping to the translator.
	translator->ApplyVirtualWorkerManagerData(SecondValidDataObject);

	// Translator should return the values from the new mapping
	TestTrue("There is a mapping for virtual worker 1", translator->GetPhysicalWorkerForVirtualWorker(1) != nullptr);
	TestTrue("Virtual worker 1 is ValidWorkerOne", translator->GetPhysicalWorkerForVirtualWorker(1)->Equals("ValidWorkerOne"));

	TestTrue("There is an updated mapping for virtual worker 2", translator->GetPhysicalWorkerForVirtualWorker(2) != nullptr);
	TestTrue("VirtualWorker 2 is ValidWorkerThree", translator->GetPhysicalWorkerForVirtualWorker(2)->Equals("ValidWorkerThree"));

	TestTrue("Local virtual worker ID is still known", translator->GetLocalVirtualWorkerId() == 1);
	TestTrue("Translator with local virtual worker ID is still ready.", translator->IsReady());

	return true;
}


VIRTUALWORKERTRANSLATOR_TEST(GIVEN_have_a_valid_mapping_WHEN_try_to_change_local_virtual_worker_id_THEN_ignore_it)
{
	// The class is initialized with no data.
	TUniquePtr<SpatialVirtualWorkerTranslator> translator = MakeUnique<SpatialVirtualWorkerTranslator>(nullptr, "ValidWorkerOne");

	// Create a valid initial mapping.
	Worker_ComponentData FirstValidData = {};
	FirstValidData.component_id = SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID;
	FirstValidData.schema_type = Schema_CreateComponentData();
	Schema_Object* FirstValidDataObject = Schema_GetComponentDataFields(FirstValidData.schema_type);

	// The mapping only has the following entries:
	// 	VirtualToPhysicalWorkerMapping.Add(1, "ValidWorkerOne");
	Schema_Object* FirstValidDataFirstEntryObject = Schema_AddObject(FirstValidDataObject, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID);
	Schema_AddUint32(FirstValidDataFirstEntryObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID, 1);
	SpatialGDK::AddStringToSchema(FirstValidDataFirstEntryObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME, "ValidWorkerOne");

	// Apply valid mapping to the translator.
	translator->ApplyVirtualWorkerManagerData(FirstValidDataObject);

	// Create a second initial mapping.
	Worker_ComponentData SecondValidData = {};
	SecondValidData.component_id = SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID;
	SecondValidData.schema_type = Schema_CreateComponentData();
	Schema_Object* SecondValidDataObject = Schema_GetComponentDataFields(SecondValidData.schema_type);

	// The mapping only has the following entries:
	// 	VirtualToPhysicalWorkerMapping.Add(2, "ValidWorkerOne");
	Schema_Object* SecondValidDataFirstEntryObject = Schema_AddObject(SecondValidDataObject, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID);
	Schema_AddUint32(SecondValidDataFirstEntryObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID, 2);
	SpatialGDK::AddStringToSchema(SecondValidDataFirstEntryObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME, "ValidWorkerOne");

	// Apply valid mapping to the translator.
	AddExpectedError(TEXT("Received mapping containing a new and updated virtual worker ID, this shouldn't happen."), EAutomationExpectedErrorFlags::Contains, 1);
	translator->ApplyVirtualWorkerManagerData(SecondValidDataObject);

	// Translator should return the values from the original mapping
	TestTrue("There is a mapping for virtual worker 1", translator->GetPhysicalWorkerForVirtualWorker(1) != nullptr);
	TestTrue("Virtual worker 1 is ValidWorkerOne", translator->GetPhysicalWorkerForVirtualWorker(1)->Equals("ValidWorkerOne"));

	TestTrue("There is no mapping for virtual worker 2", translator->GetPhysicalWorkerForVirtualWorker(2) == nullptr);

	TestTrue("Local virtual worker ID is still known", translator->GetLocalVirtualWorkerId() == 1);
	TestTrue("Translator with local virtual worker ID is still ready.", translator->IsReady());

	return true;
}
