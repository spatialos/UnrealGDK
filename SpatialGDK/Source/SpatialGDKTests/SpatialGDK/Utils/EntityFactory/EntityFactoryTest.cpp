// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "Utils/EntityFactory.h"

#include "CoreMinimal.h"

#define ENTITYFACTORY_TEST(TestName) GDK_TEST(Core, EntityFactory, TestName)

using namespace SpatialGDK;

namespace
{
struct ContainsComponent
{
	Worker_ComponentId ComponentId;
	bool operator()(const FWorkerComponentData& Data) { return Data.component_id == ComponentId; }
};
} // anonymous namespace

ENTITYFACTORY_TEST(GIVEN_an_actor_WHEN_creating_skeleton_entity_components_THEN_it_contains_required_components)
{
	AActor* Actor = NewObject<AActor>();

	SpatialGDK::EntityComponents EntityComps = EntityFactory::CreateSkeletonEntityComponents(Actor);
	EntityComps.ShiftToComponentDatas();

	TestTrue("Has position component",
			 EntityComps.ComponentDatas.ContainsByPredicate(ContainsComponent{ SpatialConstants::POSITION_COMPONENT_ID }));

	// Actor tags
	TestTrue("Has actor auth tag component",
			 EntityComps.ComponentDatas.ContainsByPredicate(ContainsComponent{ SpatialConstants::ACTOR_AUTH_TAG_COMPONENT_ID }));
	TestTrue("Has actor non-auth tag component",
			 EntityComps.ComponentDatas.ContainsByPredicate(ContainsComponent{ SpatialConstants::ACTOR_NON_AUTH_TAG_COMPONENT_ID }));
	TestTrue("Has LB tag component",
			 EntityComps.ComponentDatas.ContainsByPredicate(ContainsComponent{ SpatialConstants::LB_TAG_COMPONENT_ID }));

	return true;
}
