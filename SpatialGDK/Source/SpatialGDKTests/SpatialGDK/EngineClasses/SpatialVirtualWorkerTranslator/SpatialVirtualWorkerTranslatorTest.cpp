// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKTests/SpatialGDK/LoadBalancing/AbstractLBStrategy/LBStrategyStub.h"
#include "Tests/TestingSchemaHelpers.h"

#include "Tests/TestDefinitions.h"

#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"

#include "Templates/UniquePtr.h"
#include "UObject/UObjectGlobals.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#define VIRTUALWORKERTRANSLATOR_TEST(TestName) GDK_TEST(Core, SpatialVirtualWorkerTranslator, TestName)

VIRTUALWORKERTRANSLATOR_TEST(GIVEN_init_is_not_called_THEN_return_not_ready)
{
	ULBStrategyStub* LBStrategyStub = NewObject<ULBStrategyStub>();
	TUniquePtr<SpatialVirtualWorkerTranslator> Translator =
		MakeUnique<SpatialVirtualWorkerTranslator>(LBStrategyStub, SpatialConstants::TRANSLATOR_UNSET_PHYSICAL_NAME);

	TestFalse("Translator without local virtual worker ID is not ready.", Translator->IsReady());
	TestEqual<VirtualWorkerId>("LBStrategy stub reports an invalid virtual worker ID.", LBStrategyStub->GetVirtualWorkerId(),
							   SpatialConstants::INVALID_VIRTUAL_WORKER_ID);

	return true;
}

VIRTUALWORKERTRANSLATOR_TEST(GIVEN_worker_name_specified_in_constructor_THEN_return_correct_local_worker_name)
{
	TUniquePtr<SpatialVirtualWorkerTranslator> Translator = MakeUnique<SpatialVirtualWorkerTranslator>(nullptr, "my_worker_name");

	TestEqual<FString>("Local physical worker name returned correctly", Translator->GetLocalPhysicalWorkerName(), "my_worker_name");

	return true;
}

VIRTUALWORKERTRANSLATOR_TEST(GIVEN_no_mapping_WHEN_nothing_has_changed_THEN_return_no_mappings_and_unintialized_state)
{
	ULBStrategyStub* LBStrategyStub = NewObject<ULBStrategyStub>();
	TUniquePtr<SpatialVirtualWorkerTranslator> Translator =
		MakeUnique<SpatialVirtualWorkerTranslator>(LBStrategyStub, SpatialConstants::TRANSLATOR_UNSET_PHYSICAL_NAME);

	TestNull("Worker 1 doesn't exist", Translator->GetPhysicalWorkerForVirtualWorker(1));
	TestEqual<VirtualWorkerId>("Local virtual worker ID is not known.", Translator->GetLocalVirtualWorkerId(),
							   SpatialConstants::INVALID_VIRTUAL_WORKER_ID);
	TestFalse("Translator without local virtual worker ID is not ready.", Translator->IsReady());
	TestEqual<VirtualWorkerId>("LBStrategy stub reports an invalid virtual worker ID.", LBStrategyStub->GetVirtualWorkerId(),
							   SpatialConstants::INVALID_VIRTUAL_WORKER_ID);

	return true;
}

VIRTUALWORKERTRANSLATOR_TEST(GIVEN_no_mapping_WHEN_receiving_empty_mapping_THEN_ignore_it)
{
	ULBStrategyStub* LBStrategyStub = NewObject<ULBStrategyStub>();
	TUniquePtr<SpatialVirtualWorkerTranslator> Translator =
		MakeUnique<SpatialVirtualWorkerTranslator>(LBStrategyStub, SpatialConstants::TRANSLATOR_UNSET_PHYSICAL_NAME);

	// Create an empty mapping.
	Schema_Object* DataObject = TestingSchemaHelpers::CreateTranslationComponentDataFields();

	// Now apply the mapping to the translator and test the result. Because the mapping is empty,
	// it should ignore the mapping and continue to report an empty mapping.
	Translator->ApplyVirtualWorkerManagerData(DataObject);

	TestEqual<VirtualWorkerId>("Local virtual worker ID is not known.", Translator->GetLocalVirtualWorkerId(),
							   SpatialConstants::INVALID_VIRTUAL_WORKER_ID);
	TestFalse("Translator without local virtual worker ID is not ready.", Translator->IsReady());
	TestEqual<VirtualWorkerId>("LBStrategy stub reports an invalid virtual worker ID.", LBStrategyStub->GetVirtualWorkerId(),
							   SpatialConstants::INVALID_VIRTUAL_WORKER_ID);

	return true;
}

VIRTUALWORKERTRANSLATOR_TEST(GIVEN_no_mapping_WHEN_receiving_incomplete_mapping_THEN_ignore_it)
{
	ULBStrategyStub* LBStrategyStub = NewObject<ULBStrategyStub>();
	TUniquePtr<SpatialVirtualWorkerTranslator> Translator =
		MakeUnique<SpatialVirtualWorkerTranslator>(LBStrategyStub, SpatialConstants::TRANSLATOR_UNSET_PHYSICAL_NAME);

	// Create a base mapping.
	Schema_Object* DataObject = TestingSchemaHelpers::CreateTranslationComponentDataFields();

	// The mapping only has the following entries:
	TestingSchemaHelpers::AddTranslationComponentDataMapping(DataObject, 1, "ValidWorkerOne");
	TestingSchemaHelpers::AddTranslationComponentDataMapping(DataObject, 2, "ValidWorkerTwo");

	// Now apply the mapping to the translator and test the result. Because the mapping doesn't have an entry for this translator,
	// it should reject the mapping and continue to report an empty mapping.
	Translator->ApplyVirtualWorkerManagerData(DataObject);

	TestNull("There is no mapping for virtual worker 1", Translator->GetPhysicalWorkerForVirtualWorker(1));
	TestNull("There is no mapping for virtual worker 2", Translator->GetPhysicalWorkerForVirtualWorker(2));

	TestEqual<VirtualWorkerId>("Local virtual worker ID is not known.", Translator->GetLocalVirtualWorkerId(),
							   SpatialConstants::INVALID_VIRTUAL_WORKER_ID);
	TestFalse("Translator without local virtual worker ID is not ready.", Translator->IsReady());
	TestEqual<VirtualWorkerId>("LBStrategy stub reports an invalid virtual worker ID.", LBStrategyStub->GetVirtualWorkerId(),
							   SpatialConstants::INVALID_VIRTUAL_WORKER_ID);

	return true;
}

VIRTUALWORKERTRANSLATOR_TEST(GIVEN_no_mapping_WHEN_a_valid_mapping_is_received_THEN_return_the_updated_mapping_and_become_ready)
{
	ULBStrategyStub* LBStrategyStub = NewObject<ULBStrategyStub>();
	TUniquePtr<SpatialVirtualWorkerTranslator> Translator = MakeUnique<SpatialVirtualWorkerTranslator>(LBStrategyStub, "ValidWorkerOne");

	// Create a base mapping.
	Schema_Object* DataObject = TestingSchemaHelpers::CreateTranslationComponentDataFields();

	// The mapping only has the following entries:
	TestingSchemaHelpers::AddTranslationComponentDataMapping(DataObject, 1, "ValidWorkerOne");
	TestingSchemaHelpers::AddTranslationComponentDataMapping(DataObject, 2, "ValidWorkerTwo");

	// Now apply the mapping to the translator and test the result.
	Translator->ApplyVirtualWorkerManagerData(DataObject);

	const PhysicalWorkerName* VirtualWorker1PhysicalName = Translator->GetPhysicalWorkerForVirtualWorker(1);
	TestNotNull("There is a mapping for virtual worker 1", VirtualWorker1PhysicalName);
	TestEqual<FString>("Virtual worker 1 is ValidWorkerOne", *VirtualWorker1PhysicalName, "ValidWorkerOne");

	const PhysicalWorkerName* VirtualWorker2PhysicalName = Translator->GetPhysicalWorkerForVirtualWorker(2);
	TestNotNull("There is a mapping for virtual worker 2", VirtualWorker2PhysicalName);
	TestEqual<FString>("VirtualWorker 2 is ValidWorkerTwo", *VirtualWorker2PhysicalName, "ValidWorkerTwo");

	TestNull("There is no mapping for virtual worker 3", Translator->GetPhysicalWorkerForVirtualWorker(3));

	TestEqual<VirtualWorkerId>("Local virtual worker ID is known.", Translator->GetLocalVirtualWorkerId(), 1);
	TestTrue("Translator with local virtual worker ID is ready.", Translator->IsReady());
	TestEqual<VirtualWorkerId>("LBStrategy stub reports the correct virtual worker ID.", LBStrategyStub->GetVirtualWorkerId(), 1);

	return true;
}

VIRTUALWORKERTRANSLATOR_TEST(GIVEN_have_a_valid_mapping_WHEN_an_invalid_mapping_is_received_THEN_ignore_it)
{
	ULBStrategyStub* LBStrategyStub = NewObject<ULBStrategyStub>();
	TUniquePtr<SpatialVirtualWorkerTranslator> Translator = MakeUnique<SpatialVirtualWorkerTranslator>(LBStrategyStub, "ValidWorkerOne");

	// Create a base mapping.
	Schema_Object* ValidDataObject = TestingSchemaHelpers::CreateTranslationComponentDataFields();

	// The mapping only has the following entries:
	TestingSchemaHelpers::AddTranslationComponentDataMapping(ValidDataObject, 1, "ValidWorkerOne");
	TestingSchemaHelpers::AddTranslationComponentDataMapping(ValidDataObject, 2, "ValidWorkerTwo");

	// Apply valid mapping to the translator.
	Translator->ApplyVirtualWorkerManagerData(ValidDataObject);

	// Create an empty mapping.
	Worker_ComponentData EmptyData = {};
	EmptyData.component_id = SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID;
	EmptyData.schema_type = Schema_CreateComponentData();
	Schema_Object* EmptyDataObject = Schema_GetComponentDataFields(EmptyData.schema_type);

	// Now apply the mapping to the translator and test the result. Because the mapping is empty,
	// it should ignore the mapping and continue to report a valid mapping.
	Translator->ApplyVirtualWorkerManagerData(EmptyDataObject);

	// Translator should return the values from the initial valid mapping
	const PhysicalWorkerName* VirtualWorker1PhysicalName = Translator->GetPhysicalWorkerForVirtualWorker(1);
	TestNotNull("There is a mapping for virtual worker 1", VirtualWorker1PhysicalName);
	TestEqual<FString>("Virtual worker 1 is ValidWorkerOne", *VirtualWorker1PhysicalName, "ValidWorkerOne");

	const PhysicalWorkerName* VirtualWorker2PhysicalName = Translator->GetPhysicalWorkerForVirtualWorker(2);
	TestNotNull("There is a mapping for virtual worker 2", VirtualWorker2PhysicalName);
	TestEqual<FString>("VirtualWorker 2 is ValidWorkerTwo", *VirtualWorker2PhysicalName, "ValidWorkerTwo");

	TestNull("There is no mapping for virtual worker 3", Translator->GetPhysicalWorkerForVirtualWorker(3));

	TestEqual<VirtualWorkerId>("Local virtual worker ID is known.", Translator->GetLocalVirtualWorkerId(), 1);
	TestTrue("Translator with local virtual worker ID is ready.", Translator->IsReady());
	TestEqual<VirtualWorkerId>("LBStrategy stub reports the correct virtual worker ID.", LBStrategyStub->GetVirtualWorkerId(), 1);

	return true;
}

VIRTUALWORKERTRANSLATOR_TEST(GIVEN_have_a_valid_mapping_WHEN_another_valid_mapping_is_received_THEN_update_accordingly)
{
	ULBStrategyStub* LBStrategyStub = NewObject<ULBStrategyStub>();
	TUniquePtr<SpatialVirtualWorkerTranslator> Translator = MakeUnique<SpatialVirtualWorkerTranslator>(LBStrategyStub, "ValidWorkerOne");

	// Create a valid initial mapping.
	Schema_Object* FirstValidDataObject = TestingSchemaHelpers::CreateTranslationComponentDataFields();

	// The mapping only has the following entries:
	TestingSchemaHelpers::AddTranslationComponentDataMapping(FirstValidDataObject, 1, "ValidWorkerOne");
	TestingSchemaHelpers::AddTranslationComponentDataMapping(FirstValidDataObject, 2, "ValidWorkerTwo");

	// Apply valid mapping to the translator.
	Translator->ApplyVirtualWorkerManagerData(FirstValidDataObject);

	// Create a second mapping.
	Schema_Object* SecondValidDataObject = TestingSchemaHelpers::CreateTranslationComponentDataFields();

	// The mapping only has the following entries:
	TestingSchemaHelpers::AddTranslationComponentDataMapping(SecondValidDataObject, 1, "ValidWorkerOne");
	TestingSchemaHelpers::AddTranslationComponentDataMapping(SecondValidDataObject, 2, "ValidWorkerThree");

	// Apply valid mapping to the translator.
	Translator->ApplyVirtualWorkerManagerData(SecondValidDataObject);

	// Translator should return the values from the new mapping
	const PhysicalWorkerName* VirtualWorker1PhysicalName = Translator->GetPhysicalWorkerForVirtualWorker(1);
	TestNotNull("There is a mapping for virtual worker 1", VirtualWorker1PhysicalName);
	TestEqual<FString>("Virtual worker 1 is ValidWorkerOne", *VirtualWorker1PhysicalName, "ValidWorkerOne");

	const PhysicalWorkerName* VirtualWorker2PhysicalName = Translator->GetPhysicalWorkerForVirtualWorker(2);
	TestNotNull("There is an updated mapping for virtual worker 2", VirtualWorker2PhysicalName);
	TestEqual<FString>("VirtualWorker 2 is ValidWorkerThree", *VirtualWorker2PhysicalName, "ValidWorkerThree");

	TestEqual<VirtualWorkerId>("Local virtual worker ID is still known.", Translator->GetLocalVirtualWorkerId(), 1);
	TestTrue("Translator with local virtual worker ID is still ready.", Translator->IsReady());
	TestEqual<VirtualWorkerId>("LBStrategy stub reports the correct virtual worker ID.", LBStrategyStub->GetVirtualWorkerId(), 1);

	return true;
}

VIRTUALWORKERTRANSLATOR_TEST(GIVEN_have_a_valid_mapping_WHEN_try_to_change_local_virtual_worker_id_THEN_ignore_it)
{
	ULBStrategyStub* LBStrategyStub = NewObject<ULBStrategyStub>();
	TUniquePtr<SpatialVirtualWorkerTranslator> Translator = MakeUnique<SpatialVirtualWorkerTranslator>(LBStrategyStub, "ValidWorkerOne");

	// Create a valid initial mapping.
	Schema_Object* FirstValidDataObject = TestingSchemaHelpers::CreateTranslationComponentDataFields();

	// The mapping only has the following entries:
	// 	VirtualToPhysicalWorkerMapping.Add(1, "ValidWorkerOne");
	TestingSchemaHelpers::AddTranslationComponentDataMapping(FirstValidDataObject, 1, "ValidWorkerOne");

	// Apply valid mapping to the translator.
	Translator->ApplyVirtualWorkerManagerData(FirstValidDataObject);

	// Create a second initial mapping.
	Schema_Object* SecondValidDataObject = TestingSchemaHelpers::CreateTranslationComponentDataFields();

	// The mapping only has the following entries:
	TestingSchemaHelpers::AddTranslationComponentDataMapping(SecondValidDataObject, 2, "ValidWorkerOne");

	// Apply valid mapping to the translator.
	AddExpectedError(TEXT("Received mapping containing a new and updated virtual worker ID, this shouldn't happen."),
					 EAutomationExpectedErrorFlags::Contains, 1);
	Translator->ApplyVirtualWorkerManagerData(SecondValidDataObject);

	// Translator should return the values from the original mapping
	const PhysicalWorkerName* VirtualWorker1PhysicalName = Translator->GetPhysicalWorkerForVirtualWorker(1);
	TestNotNull("There is a mapping for virtual worker 1", VirtualWorker1PhysicalName);
	TestEqual<FString>("Virtual worker 1 is ValidWorkerOne", *VirtualWorker1PhysicalName, "ValidWorkerOne");

	TestNull("There is no mapping for virtual worker 2", Translator->GetPhysicalWorkerForVirtualWorker(2));

	TestEqual<VirtualWorkerId>("Local virtual worker ID is still known.", Translator->GetLocalVirtualWorkerId(), 1);
	TestTrue("Translator with local virtual worker ID is still ready.", Translator->IsReady());
	TestEqual<VirtualWorkerId>("LBStrategy stub reports the correct virtual worker ID.", LBStrategyStub->GetVirtualWorkerId(), 1);

	return true;
}
