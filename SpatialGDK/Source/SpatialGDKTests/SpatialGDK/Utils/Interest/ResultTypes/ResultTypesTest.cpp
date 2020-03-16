// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"
#include "Tests/Mocks/SpatialClassInfoManagerMock.h"

#include "Utils/Interest/ResultTypes.h"

#define RESULT_TYPES_TEST(TestName) \
	GDK_TEST(Core, ResultTypes, TestName)

// Sanity check tests for creating result types.
// Run tests inside the SpatialGDK namespace in order to test static functions.
namespace SpatialGDK
{
	TArray<Worker_ComponentId> DataComponentIds = { 1, 2 };
	TArray<Worker_ComponentId> OwnerOnlyComponentIds = { 3, 4 };
	TArray<Worker_ComponentId> HandoverComponentIds = { 5, 6 };

	USpatialClassInfoManagerMock* CreateAndPopulateMockClassInfoManager()
	{
		USpatialClassInfoManagerMock* ClassInfoManager = NewObject<USpatialClassInfoManagerMock>();

		// Ensure the mock won't be garbage collected
		ClassInfoManager->AddToRoot();

		// Initialise with fake generated components
		ClassInfoManager->SetComponentIdsForComponentType(ESchemaComponentType::SCHEMA_Data, DataComponentIds);
		ClassInfoManager->SetComponentIdsForComponentType(ESchemaComponentType::SCHEMA_OwnerOnly, OwnerOnlyComponentIds);
		ClassInfoManager->SetComponentIdsForComponentType(ESchemaComponentType::SCHEMA_Handover, HandoverComponentIds);

		return ClassInfoManager;
	}

	RESULT_TYPES_TEST(GIVEN_class_info_manager_WHEN_build_client_auth_result_type_THEN_gets_correct_components)
	{
		USpatialClassInfoManagerMock* ClassInfoManager = CreateAndPopulateMockClassInfoManager();

		TArray<Worker_ComponentId> ExpectedResultType;
		ExpectedResultType.Append(SpatialConstants::REQUIRED_COMPONENTS_FOR_AUTH_CLIENT_INTEREST);
		ExpectedResultType.Append(SpatialConstants::REQUIRED_COMPONENTS_FOR_NON_AUTH_CLIENT_INTEREST);
		ExpectedResultType.Append(DataComponentIds);
		ExpectedResultType.Append(OwnerOnlyComponentIds);

		TArray<Worker_ComponentId> ClientAuthResultType = ResultTypes::CreateClientAuthInterestResultType(ClassInfoManager);

		ExpectedResultType.Sort();
		ClientAuthResultType.Sort();

		TestEqual("Expected and actual result types are equal", ClientAuthResultType, ExpectedResultType);

		return true;
	}

	RESULT_TYPES_TEST(GIVEN_class_info_manager_WHEN_build_client_non_auth_result_type_THEN_gets_correct_components)
	{
		USpatialClassInfoManagerMock* ClassInfoManager = CreateAndPopulateMockClassInfoManager();

		TArray<Worker_ComponentId> ExpectedResultType;
		ExpectedResultType.Append(SpatialConstants::REQUIRED_COMPONENTS_FOR_NON_AUTH_CLIENT_INTEREST);
		ExpectedResultType.Append(DataComponentIds);
		ExpectedResultType.Append(OwnerOnlyComponentIds);

		TArray<Worker_ComponentId> ClientNonAuthResultType = ResultTypes::CreateClientNonAuthInterestResultType(ClassInfoManager);

		ExpectedResultType.Sort();
		ClientNonAuthResultType.Sort();

		TestEqual("Expected and actual result types are equal", ClientNonAuthResultType, ExpectedResultType);

		return true;
	}

	RESULT_TYPES_TEST(GIVEN_class_info_manager_WHEN_build_server_auth_result_type_THEN_gets_correct_components)
	{
		TArray<Worker_ComponentId> ExpectedResultType;
		ExpectedResultType.Append(SpatialConstants::REQUIRED_COMPONENTS_FOR_AUTH_SERVER_INTEREST);

		TArray<Worker_ComponentId> ServerAuthResultType = ResultTypes::CreateServerAuthInterestResultType();

		ExpectedResultType.Sort();
		ServerAuthResultType.Sort();

		TestEqual("Expected and actual result types are equal", ServerAuthResultType, ExpectedResultType);

		return true;
	}

	RESULT_TYPES_TEST(GIVEN_class_info_manager_WHEN_build_server_non_auth_result_type_THEN_gets_correct_components)
	{
		USpatialClassInfoManagerMock* ClassInfoManager = CreateAndPopulateMockClassInfoManager();

		TArray<Worker_ComponentId> ExpectedResultType;
		ExpectedResultType.Append(SpatialConstants::REQUIRED_COMPONENTS_FOR_NON_AUTH_SERVER_INTEREST);
		ExpectedResultType.Append(DataComponentIds);
		ExpectedResultType.Append(OwnerOnlyComponentIds);
		ExpectedResultType.Append(HandoverComponentIds);

		TArray<Worker_ComponentId> ServerNonAuthResultType = ResultTypes::CreateServerNonAuthInterestResultType(ClassInfoManager);

		ExpectedResultType.Sort();
		ServerNonAuthResultType.Sort();

		TestEqual("Expected and actual result types are equal", ServerNonAuthResultType, ExpectedResultType);

		return true;
	}
}

