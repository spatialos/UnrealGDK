// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMaps/SpatialNetworkingMap.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/DormancyAndTombstoneTest/DormancyAndTombstoneTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/DormancyAndTombstoneTest/DormancyTestActor.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/RegisterAutoDestroyActorsTest/RegisterAutoDestroyActorsTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestPossession/SpatialTestPossession.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestPossession/SpatialTestRepossession.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestRepNotify/SpatialTestRepNotify.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestSingleServerDynamicComponents/SpatialTestSingleServerDynamicComponents.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/UNR-3066/OwnerOnlyPropertyReplication.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/UNR-3157/RPCInInterfaceTest.h"
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
	AddActorToLevel<ADormancyAndTombstoneTest>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<ASpatialTestSingleServerDynamicComponents>(CurrentLevel, FTransform::Identity);

	// Add test helpers
	// Unfortunately, the nature of some tests requires them to have actors placed in the level, to trigger some Unreal behavior
	AddActorToLevel<ADormancyTestActor>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<AReplicatedVisibilityTestActor>(CurrentLevel, FTransform::Identity);
}
