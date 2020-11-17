// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "Utils/EntityFactory.h"

#include "CoreMinimal.h"

#define ENTITYFACTORY_TEST(TestName) GDK_TEST(Core, EntityFactory, TestName)

using namespace SpatialGDK;

ENTITYFACTORY_TEST(GIVEN_an_actor_WHEN_creating_skeleton_entity_components_THEN_it_contains_required_components)
{
	AActor* Actor = NewObject<AActor>();

	SpatialGDK::EntityComponents EntityComps = EntityFactory::CreateSkeletonEntityComponents(Actor);
	EntityComps.ShiftToComponentDatas();

	bool bContainsPosition = EntityComps.ComponentDatas.ContainsByPredicate([](const FWorkerComponentData& Data) {
		return Data.component_id == SpatialConstants::POSITION_COMPONENT_ID;
	});

	TestTrue("Has position component", bContainsPosition);

	return true;
}
