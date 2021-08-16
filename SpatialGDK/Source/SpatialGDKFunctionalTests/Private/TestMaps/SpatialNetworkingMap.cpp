// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMaps/SpatialNetworkingMap.h"

#include "SpatialGDK/SpatialTestNetReceive/SpatialTestNetReceive.h"
#include "SpatialGDK/StaticSubobjectsTest/StaticSubobjectTestActor.h"
#include "SpatialGDK/StaticSubobjectsTest/StaticSubobjectsTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/DormancyTests/DormancyTestActor.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/DormancyTests/DynamicActorAwakeAfterDormantChangePropertyTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/DormancyTests/DynamicActorAwakeChangePropertyTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/DormancyTests/DynamicActorDormantAllChangePropertyTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/DormancyTests/DynamicActorSetToAwakeTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/DormancyTests/RefreshActorDormancyTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/DormancyTests/InitiallyDormantDynamicActorTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/DynamicSubobjectsTest/DynamicSubObjectTestActor.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/DynamicSubobjectsTest/DynamicSubObjectsTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/DynamicSubobjectsTest/SpatialDynamicComponentsFastReadditionTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/RegisterAutoDestroyActorsTest/RegisterAutoDestroyActorsTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestPossession/SpatialTestPossession.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestPossession/SpatialTestRepossession.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestRepNotify/SpatialTestRepNotify.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestSingleServerDynamicComponents/SpatialTestSingleServerDynamicComponents.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/UNR-3066/OwnerOnlyPropertyReplication.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/UNR-3157/RPCInInterfaceTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/UNR-3761/SpatialTestMultipleOwnership/SpatialTestMultipleOwnership.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/UnresolvedReferenceTest/UnresolvedReferenceTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/VisibilityTest/ReplicatedVisibilityTestActor.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/VisibilityTest/VisibilityTest.h"

USpatialNetworkingMap::USpatialNetworkingMap()
	: UGeneratedTestMap(EMapCategory::CI_PREMERGE, TEXT("SpatialNetworkingMap"))
{
}

void USpatialNetworkingMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	// Add the tests
	AddActorToLevel<ASpatialTestPossession>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<ASpatialTestRepossession>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<ASpatialTestRepNotify>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<AVisibilityTest>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<ARPCInInterfaceTest>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<ARegisterAutoDestroyActorsTestPart1>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<ARegisterAutoDestroyActorsTestPart2>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<AOwnerOnlyPropertyReplication>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<ASpatialTestSingleServerDynamicComponents>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<ASpatialTestMultipleOwnership>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<ADynamicSubobjectsTest>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<AStaticSubobjectsTest>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<ASpatialDynamicComponentsFastReadditionTest>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<ASpatialTestNetReceive>(CurrentLevel, FTransform::Identity);

	AddActorToLevel<AInitiallyDormantDynamicActorTest>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<ADynamicActorSetToAwakeTest>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<ADynamicActorDormantAllChangePropertyTest>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<ADynamicActorAwakeChangePropertyTest>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<ADynamicActorAwakeAfterDormantChangePropertyTest>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<ARefreshActorDormancyTest>(CurrentLevel, FTransform::Identity);

	AddActorToLevel<AUnresolvedReferenceTest>(CurrentLevel, FTransform::Identity);
	// Add test helpers
	// Unfortunately, the nature of some tests requires them to have actors placed in the level, to trigger some Unreal behavior
	AddActorToLevel<AReplicatedVisibilityTestActor>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<ADynamicSubObjectTestActor>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<AStaticSubobjectTestActor>(CurrentLevel, FTransform(FVector(-20000.0f, -20000.0f, 40.0f)));
}
