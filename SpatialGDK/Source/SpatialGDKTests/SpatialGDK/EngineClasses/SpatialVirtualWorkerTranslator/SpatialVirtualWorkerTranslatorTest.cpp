// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKTests/SpatialGDK/LoadBalancing/AbstractLBStrategy/LBStrategyStub.h"

#include "SpatialGDKTests/Public/GDKAutomationTestBase.h"

#include "Tests/TestDefinitions.h"

#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"

#include "Templates/UniquePtr.h"
#include "UObject/UObjectGlobals.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace
{
const PhysicalWorkerName ValidWorkerOne = TEXT("ValidWorkerOne");
const PhysicalWorkerName ValidWorkerTwo = TEXT("ValidWorkerTwo");
const PhysicalWorkerName ValidWorkerThree = TEXT("ValidWorkerThree");

const Worker_PartitionId WorkerOneId = 101;
const Worker_PartitionId WorkerTwoId = 102;
const Worker_PartitionId WorkerThreeId = 103;
} // namespace

#define VIRTUALWORKERTRANSLATOR_TEST(TestName) GDK_AUTOMATION_TEST(Core, SpatialVirtualWorkerTranslator, TestName)

VIRTUALWORKERTRANSLATOR_TEST(GIVEN_init_is_not_called_THEN_return_not_ready)
{
	ULBStrategyStub* LBStrategyStub = NewObject<ULBStrategyStub>();
	TUniquePtr<SpatialVirtualWorkerTranslator> Translator =
		MakeUnique<SpatialVirtualWorkerTranslator>(LBStrategyStub, nullptr, SpatialConstants::TRANSLATOR_UNSET_PHYSICAL_NAME);

	TestFalse("Translator without local virtual worker ID is not ready.", Translator->IsReady());
	TestEqual<VirtualWorkerId>("LBStrategy stub reports an invalid virtual worker ID.", LBStrategyStub->GetVirtualWorkerId(),
							   SpatialConstants::INVALID_VIRTUAL_WORKER_ID);

	return true;
}

VIRTUALWORKERTRANSLATOR_TEST(GIVEN_worker_name_specified_in_constructor_THEN_return_correct_local_worker_name)
{
	TUniquePtr<SpatialVirtualWorkerTranslator> Translator = MakeUnique<SpatialVirtualWorkerTranslator>(nullptr, nullptr, "my_worker_name");

	TestEqual<FString>("Local physical worker name returned correctly", Translator->GetLocalPhysicalWorkerName(), "my_worker_name");

	return true;
}

VIRTUALWORKERTRANSLATOR_TEST(GIVEN_no_mapping_WHEN_nothing_has_changed_THEN_return_no_mappings_and_unintialized_state)
{
	ULBStrategyStub* LBStrategyStub = NewObject<ULBStrategyStub>();
	TUniquePtr<SpatialVirtualWorkerTranslator> Translator =
		MakeUnique<SpatialVirtualWorkerTranslator>(LBStrategyStub, nullptr, SpatialConstants::TRANSLATOR_UNSET_PHYSICAL_NAME);

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
		MakeUnique<SpatialVirtualWorkerTranslator>(LBStrategyStub, nullptr, SpatialConstants::TRANSLATOR_UNSET_PHYSICAL_NAME);

	const SpatialGDK::VirtualWorkerTranslation TranslationData = SpatialGDK::VirtualWorkerTranslation();

	// Now apply the mapping to the translator and test the result. Because the mapping is empty,
	// it should ignore the mapping and continue to report an empty mapping.
	Translator->ApplyVirtualWorkerTranslation(TranslationData);

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
	TUniquePtr<SpatialVirtualWorkerTranslator> Translator =
		MakeUnique<SpatialVirtualWorkerTranslator>(LBStrategyStub, nullptr, ValidWorkerOne);

	const SpatialGDK::VirtualWorkerInfo VirtualWorkerOne =
		SpatialGDK::VirtualWorkerInfo{ 1, ValidWorkerOne, SpatialConstants::INVALID_ENTITY_ID, WorkerOneId,
									   SpatialConstants::INVALID_ENTITY_ID };
	const SpatialGDK::VirtualWorkerInfo VirtualWorkerTwo =
		SpatialGDK::VirtualWorkerInfo{ 2, ValidWorkerTwo, SpatialConstants::INVALID_ENTITY_ID, WorkerTwoId,
									   SpatialConstants::INVALID_ENTITY_ID };

	const TArray<SpatialGDK::VirtualWorkerInfo> VirtualWorkerList{ VirtualWorkerOne, VirtualWorkerTwo };
	const SpatialGDK::VirtualWorkerTranslation TranslationData(VirtualWorkerList, 0);

	// Now apply the mapping to the translator and test the result.
	Translator->ApplyVirtualWorkerTranslation(TranslationData);

	const PhysicalWorkerName* VirtualWorker1PhysicalName = Translator->GetPhysicalWorkerForVirtualWorker(1);
	TestNotNull("There is a mapping for virtual worker 1", VirtualWorker1PhysicalName);
	TestEqual<FString>("Virtual worker 1 is ValidWorkerOne", *VirtualWorker1PhysicalName, ValidWorkerOne);

	const PhysicalWorkerName* VirtualWorker2PhysicalName = Translator->GetPhysicalWorkerForVirtualWorker(2);
	TestNotNull("There is a mapping for virtual worker 2", VirtualWorker2PhysicalName);
	TestEqual<FString>("VirtualWorker 2 is ValidWorkerTwo", *VirtualWorker2PhysicalName, ValidWorkerTwo);

	TestNull("There is no mapping for virtual worker 3", Translator->GetPhysicalWorkerForVirtualWorker(3));

	TestEqual<VirtualWorkerId>("Local virtual worker ID is known.", Translator->GetLocalVirtualWorkerId(), 1);
	TestEqual<Worker_PartitionId>("Local claimed partition ID is known.", Translator->GetClaimedPartitionId(), WorkerOneId);
	TestTrue("Translator with local virtual worker ID is ready.", Translator->IsReady());
	TestEqual<VirtualWorkerId>("LBStrategy stub reports the correct virtual worker ID.", LBStrategyStub->GetVirtualWorkerId(), 1);

	return true;
}

VIRTUALWORKERTRANSLATOR_TEST(GIVEN_have_a_valid_mapping_WHEN_another_valid_mapping_is_received_THEN_update_accordingly)
{
	ULBStrategyStub* LBStrategyStub = NewObject<ULBStrategyStub>();
	TUniquePtr<SpatialVirtualWorkerTranslator> Translator =
		MakeUnique<SpatialVirtualWorkerTranslator>(LBStrategyStub, nullptr, ValidWorkerOne);

	const SpatialGDK::VirtualWorkerInfo VirtualWorkerOne =
		SpatialGDK::VirtualWorkerInfo{ 1, ValidWorkerOne, SpatialConstants::INVALID_ENTITY_ID, WorkerOneId,
									   SpatialConstants::INVALID_ENTITY_ID };
	const SpatialGDK::VirtualWorkerInfo VirtualWorkerTwo =
		SpatialGDK::VirtualWorkerInfo{ 2, ValidWorkerTwo, SpatialConstants::INVALID_ENTITY_ID, WorkerTwoId,
									   SpatialConstants::INVALID_ENTITY_ID };

	const TArray<SpatialGDK::VirtualWorkerInfo> VirtualWorkerList{ VirtualWorkerOne, VirtualWorkerTwo };
	const SpatialGDK::VirtualWorkerTranslation TranslationData(VirtualWorkerList, 0);

	// Apply valid mapping to the translator.
	Translator->ApplyVirtualWorkerTranslation(TranslationData);
	const SpatialGDK::VirtualWorkerInfo OtherVirtualWorkerOne =
		SpatialGDK::VirtualWorkerInfo{ 1, ValidWorkerOne, SpatialConstants::INVALID_ENTITY_ID, WorkerOneId,
									   SpatialConstants::INVALID_ENTITY_ID };
	const SpatialGDK::VirtualWorkerInfo OtherVirtualWorkerThree =
		SpatialGDK::VirtualWorkerInfo{ 2, ValidWorkerThree, SpatialConstants::INVALID_ENTITY_ID, WorkerThreeId,
									   SpatialConstants::INVALID_ENTITY_ID };

	const TArray<SpatialGDK::VirtualWorkerInfo> OtherVirtualWorkerList{ OtherVirtualWorkerOne, OtherVirtualWorkerThree };
	const SpatialGDK::VirtualWorkerTranslation OtherTranslationData(OtherVirtualWorkerList, 0);

	// Apply valid mapping to the translator.
	Translator->ApplyVirtualWorkerTranslation(OtherTranslationData);

	// Translator should return the values from the new mapping
	const PhysicalWorkerName* VirtualWorker1PhysicalName = Translator->GetPhysicalWorkerForVirtualWorker(1);
	TestNotNull("There is a mapping for virtual worker 1", VirtualWorker1PhysicalName);
	TestEqual<FString>("Virtual worker 1 is ValidWorkerOne", *VirtualWorker1PhysicalName, ValidWorkerOne);
	TestEqual<Worker_PartitionId>("Virtual worker 1 partition is 101", Translator->GetPartitionEntityForVirtualWorker(1), WorkerOneId);

	const PhysicalWorkerName* VirtualWorker2PhysicalName = Translator->GetPhysicalWorkerForVirtualWorker(2);
	TestNotNull("There is an updated mapping for virtual worker 2", VirtualWorker2PhysicalName);
	TestEqual<FString>("VirtualWorker 2 is ValidWorkerThree", *VirtualWorker2PhysicalName, ValidWorkerThree);
	TestEqual<Worker_PartitionId>("Virtual worker 2 partition is 103", Translator->GetPartitionEntityForVirtualWorker(2), WorkerThreeId);

	TestEqual<VirtualWorkerId>("Local virtual worker ID is still known.", Translator->GetLocalVirtualWorkerId(), 1);
	TestEqual<Worker_PartitionId>("Local claimed partition ID is known.", Translator->GetClaimedPartitionId(), WorkerOneId);
	TestTrue("Translator with local virtual worker ID is still ready.", Translator->IsReady());
	TestEqual<VirtualWorkerId>("LBStrategy stub reports the correct virtual worker ID.", LBStrategyStub->GetVirtualWorkerId(), 1);

	return true;
}
