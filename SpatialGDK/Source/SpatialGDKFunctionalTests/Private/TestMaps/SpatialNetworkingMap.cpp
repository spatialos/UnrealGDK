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
{
	MapCategory = CI_FAST;
	MapName = TEXT("SpatialNetworkingMap");
}

void USpatialNetworkingMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	// Add the tests
	GEditor->AddActor(CurrentLevel, ASpatialTestPossession::StaticClass(), FTransform::Identity);
	GEditor->AddActor(CurrentLevel, ASpatialTestRepossession::StaticClass(), FTransform::Identity);
	GEditor->AddActor(CurrentLevel, ASpatialTestRepNotify::StaticClass(), FTransform::Identity);
	GEditor->AddActor(CurrentLevel, AVisibilityTest::StaticClass(), FTransform::Identity);
	GEditor->AddActor(CurrentLevel, ARPCInInterfaceTest::StaticClass(), FTransform::Identity);
	GEditor->AddActor(CurrentLevel, ARegisterAutoDestroyActorsTestPart1::StaticClass(), FTransform::Identity);
	GEditor->AddActor(CurrentLevel, ARegisterAutoDestroyActorsTestPart2::StaticClass(), FTransform::Identity);
	GEditor->AddActor(CurrentLevel, AOwnerOnlyPropertyReplication::StaticClass(), FTransform::Identity);
	GEditor->AddActor(CurrentLevel, ADormancyAndTombstoneTest::StaticClass(), FTransform::Identity);
	GEditor->AddActor(CurrentLevel, ASpatialTestSingleServerDynamicComponents::StaticClass(), FTransform::Identity);

	// Add test helpers
	// Unfortunately, the nature of some tests requires them to have actors placed in the level, to trigger some Unreal behavior
	GEditor->AddActor(CurrentLevel, ADormancyTestActor::StaticClass(), FTransform::Identity);
	GEditor->AddActor(CurrentLevel, AReplicatedVisibilityTestActor::StaticClass(), FTransform::Identity);
}
