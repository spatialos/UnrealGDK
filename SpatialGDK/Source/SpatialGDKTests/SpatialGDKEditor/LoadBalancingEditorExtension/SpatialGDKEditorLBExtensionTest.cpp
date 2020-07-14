// // Copyright (c) Improbable Worlds Ltd, All Rights Reserved
//
// #include "Tests/TestDefinitions.h"
//
// #include "SpatialGDKEditorModule.h"
// #include "SpatialGDKEditorSettings.h"
//
// #include "TestLoadBalancingStrategyEditorExtension.h"
//
// #define LB_EXTENSION_TEST(TestName) \
// 	GDK_TEST(SpatialGDKEditor, LoadBalancingEditorExtension, TestName)
//
// namespace
// {
//
// struct TestFixture
// {
// 	TestFixture()
// 		: ExtensionManager(FModuleManager::GetModuleChecked<FSpatialGDKEditorModule>("SpatialGDKEditor").GetLBStrategyExtensionManager())
// 	{	}
//
// 	~TestFixture()
// 	{
// 		CleanupRuntimeStrategy();
//
// 		// Cleanup registered strategies for tests.
// 		ExtensionManager.UnregisterExtension<FTestLBStrategyEditorExtension>();
// 		ExtensionManager.UnregisterExtension<FTestDerivedLBStrategyEditorExtension>();
// 	}
//
// 	bool GetDefaultLaunchConfiguration(const UAbstractLBStrategy* Strategy, UAbstractRuntimeLoadBalancingStrategy*& OutConfiguration, FIntPoint& OutWorldDimensions)
// 	{
// 		CleanupRuntimeStrategy();
// 		const bool bResult = ExtensionManager.GetDefaultLaunchConfiguration(Strategy, OutConfiguration, OutWorldDimensions);
//
// 		if (OutConfiguration)
// 		{
// 			OutConfiguration->AddToRoot();
// 			RuntimeStrategy = OutConfiguration;
// 		}
//
// 		return bResult;
// 	}
//
// 	void CleanupRuntimeStrategy()
// 	{
// 		if (RuntimeStrategy)
// 		{
// 			RuntimeStrategy->RemoveFromRoot();
// 			RuntimeStrategy = nullptr;
// 		}
// 	}
//
// 	FLBStrategyEditorExtensionManager& ExtensionManager;
// 	UAbstractRuntimeLoadBalancingStrategy* RuntimeStrategy = nullptr;
// };
//
// }
// LB_EXTENSION_TEST(GIVEN_not_registered_strategy_WHEN_looking_for_extension_THEN_extension_is_not_found)
// {
// 	TestFixture Fixture;
// 	UAbstractLBStrategy* DummyStrategy = UDummyLoadBalancingStrategy::StaticClass()->GetDefaultObject<UAbstractLBStrategy>();
//
// 	UAbstractRuntimeLoadBalancingStrategy* RuntimeStrategy = nullptr;
// 	FIntPoint WorldSize;
//
// 	AddExpectedError(TEXT("Could not find editor extension for load balancing strategy"));
//
// 	bool bResult = Fixture.GetDefaultLaunchConfiguration(DummyStrategy, RuntimeStrategy, WorldSize);
//
// 	TestTrue("Non registered strategy is properly handled", !bResult);
//
// 	return true;
// }
//
// LB_EXTENSION_TEST(GIVEN_registered_strategy_WHEN_looking_for_extension_THEN_extension_is_found)
// {
// 	TestFixture Fixture;
// 	Fixture.ExtensionManager.RegisterExtension<FTestLBStrategyEditorExtension>();
//
// 	UAbstractLBStrategy* DummyStrategy = UDummyLoadBalancingStrategy::StaticClass()->GetDefaultObject<UAbstractLBStrategy>();
//
// 	UAbstractRuntimeLoadBalancingStrategy* RuntimeStrategy = nullptr;
// 	FIntPoint WorldSize;
// 	bool bResult = Fixture.GetDefaultLaunchConfiguration(DummyStrategy, RuntimeStrategy, WorldSize);
//
// 	TestTrue("Registered strategy is properly handled", bResult);
//
// 	return true;
// }
//
// LB_EXTENSION_TEST(GIVEN_registered_strategy_WHEN_getting_launch_settings_THEN_launch_settings_are_filled)
// {
// 	TestFixture Fixture;
// 	Fixture.ExtensionManager.RegisterExtension<FTestLBStrategyEditorExtension>();
//
// 	UDummyLoadBalancingStrategy* DummyStrategy = NewObject<UDummyLoadBalancingStrategy>();
//
// 	DummyStrategy->AddToRoot();
// 	DummyStrategy->NumberOfWorkers = 10;
//
// 	UAbstractRuntimeLoadBalancingStrategy* RuntimeStrategy = nullptr;
// 	FIntPoint WorldSize;
// 	bool bResult = Fixture.GetDefaultLaunchConfiguration(DummyStrategy, RuntimeStrategy, WorldSize);
//
// 	TestTrue("Registered strategy is properly handled", bResult);
//
// 	UEntityShardingRuntimeLoadBalancingStrategy* EntityShardingStrategy = Cast<UEntityShardingRuntimeLoadBalancingStrategy>(RuntimeStrategy);
//
// 	TestTrue("Launch settings are extracted", EntityShardingStrategy && EntityShardingStrategy->NumWorkers == 10);
//
// 	DummyStrategy->RemoveFromRoot();
//
// 	return true;
// }
//
//
// LB_EXTENSION_TEST(GIVEN_registered_derived_strategy_WHEN_looking_for_extension_THEN_most_derived_extension_is_found)
// {
// 	TestFixture Fixture;
// 	Fixture.ExtensionManager.RegisterExtension<FTestLBStrategyEditorExtension>();
//
// 	UAbstractLBStrategy* DummyStrategy = UDummyLoadBalancingStrategy::StaticClass()->GetDefaultObject<UAbstractLBStrategy>();
// 	UAbstractLBStrategy* DerivedDummyStrategy = UDerivedDummyLoadBalancingStrategy::StaticClass()->GetDefaultObject<UAbstractLBStrategy>();
//
// 	UAbstractRuntimeLoadBalancingStrategy* RuntimeStrategy = nullptr;
// 	FIntPoint WorldSize;
// 	FIntPoint WorldSizeDerived;
// 	bool bResult = Fixture.GetDefaultLaunchConfiguration(DummyStrategy, RuntimeStrategy, WorldSize);
// 	bResult &= Fixture.GetDefaultLaunchConfiguration(DerivedDummyStrategy, RuntimeStrategy, WorldSizeDerived);
//
// 	TestTrue("Registered strategies are properly handled", bResult);
// 	TestTrue("Common extension used", WorldSize == WorldSizeDerived && WorldSize.X == 0);
//
// 	Fixture.ExtensionManager.RegisterExtension<FTestDerivedLBStrategyEditorExtension>();
//
// 	bResult = Fixture.GetDefaultLaunchConfiguration(DummyStrategy, RuntimeStrategy, WorldSize);
// 	bResult &= Fixture.GetDefaultLaunchConfiguration(DerivedDummyStrategy, RuntimeStrategy, WorldSizeDerived);
//
// 	TestTrue("Registered strategies are properly handled", bResult);
// 	TestTrue("Most derived extension used", WorldSize != WorldSizeDerived && WorldSize.X == 0 && WorldSizeDerived.X == 4242);
//
// 	return true;
// }
