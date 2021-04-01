// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EntityFactoryTestStub.h"

#include "Tests/TestDefinitions.h"

#include "Utils/EntityFactory.h"

#define ENTITYFACTORY_TEST(TestName) GDK_TEST(Core, EntityFactory, TestName)

using namespace SpatialGDK;

namespace
{
struct ContainsComponent
{
	Worker_ComponentId ComponentId;
	bool operator()(const FWorkerComponentData& Data) { return Data.component_id == ComponentId; }
};

template <typename T>
bool DoesActorTypeHavePersistentComponent()
{
	T* Actor = NewObject<T>();
	const TArray<FWorkerComponentData> ComponentDatas = EntityFactory::CreateSkeletonEntityComponents(Actor);
	const bool HasPersistentComponent = ComponentDatas.ContainsByPredicate(ContainsComponent{ SpatialConstants::PERSISTENCE_COMPONENT_ID });
	return HasPersistentComponent;
}
} // anonymous namespace

ENTITYFACTORY_TEST(GIVEN_an_actor_WHEN_creating_skeleton_entity_components_THEN_it_contains_required_components)
{
	AActor* Actor = NewObject<AActor>();

	TArray<FWorkerComponentData> ComponentDatas = EntityFactory::CreateSkeletonEntityComponents(Actor);

	// This is here because as an end-user, I would have the expectation that the skeleton of an entity would AT LEAST contain the position
	// component
	TestTrue("Has position component", ComponentDatas.ContainsByPredicate(ContainsComponent{ SpatialConstants::POSITION_COMPONENT_ID }));

	// We want the UnrealMetadata component to be present, as it has been deemed essential enough to be included in the skeleton of an
	// entity (mainly due to containing the stably named path for actors placed in the level)
	TestTrue("Has Unreal Metadata component",
			 ComponentDatas.ContainsByPredicate(ContainsComponent{ SpatialConstants::UNREAL_METADATA_COMPONENT_ID }));

	// Actor tags
	// This is here so that we can verify we are adding the expected tags (debatable as this is basically an interface test)
	TestTrue("Has actor auth tag component",
			 ComponentDatas.ContainsByPredicate(ContainsComponent{ SpatialConstants::ACTOR_AUTH_TAG_COMPONENT_ID }));
	TestTrue("Has actor tag component", ComponentDatas.ContainsByPredicate(ContainsComponent{ SpatialConstants::ACTOR_TAG_COMPONENT_ID }));
	TestTrue("Has LB tag component", ComponentDatas.ContainsByPredicate(ContainsComponent{ SpatialConstants::LB_TAG_COMPONENT_ID }));

	return true;
}

ENTITYFACTORY_TEST(
	GIVEN_a_persistent_by_default_spatial_actor_WHEN_creating_skeleton_entity_components_THEN_it_contains_a_persistence_component)
{
	TestTrue("Expected a persistent component", DoesActorTypeHavePersistentComponent<APersistentByDefaultSpatialActor>());
	return true;
}

ENTITYFACTORY_TEST(
	GIVEN_an_explicit_persistent_spatial_actor_WHEN_creating_skeleton_entity_components_THEN_it_contains_a_persistence_component)
{
	TestTrue("Expected a persistent component", DoesActorTypeHavePersistentComponent<AExplicitPersistentSpatialActor>());
	return true;
}

ENTITYFACTORY_TEST(
	GIVEN_a_non_persistent_spatial_actor_WHEN_creating_skeleton_entity_components_THEN_it_does_not_contain_a_persistence_component)
{
	TestFalse("Expected no persistent component", DoesActorTypeHavePersistentComponent<ANonPersistentSpatialActor>());
	return true;
}

ENTITYFACTORY_TEST(
	GIVEN_an_actor_subclass_of_a_persistent_by_default_spatial_actor_WHEN_creating_skeleton_entity_components_THEN_it_contains_a_persistence_component)
{
	TestTrue("Expected a persistent component", DoesActorTypeHavePersistentComponent<AActorSubclassOfPersistentByDefaultSpatialActor>());
	return true;
}

ENTITYFACTORY_TEST(
	GIVEN_an_actor_subclass_of_a_explicit_persistent_spatial_actor_WHEN_creating_skeleton_entity_components_THEN_it_contains_a_persistence_component)
{
	TestTrue("Expected a persistent component", DoesActorTypeHavePersistentComponent<AActorSubclassOfExplicitPersistentSpatialActor>());
	return true;
}

ENTITYFACTORY_TEST(
	GIVEN_an_actor_subclass_of_a_non_persistent_spatial_actor_WHEN_creating_skeleton_entity_components_THEN_it_does_not_contain_a_persistence_component)
{
	TestFalse("Expected no persistent component", DoesActorTypeHavePersistentComponent<AActorSubclassOfNonPersistentSpatialActor>());
	return true;
}

ENTITYFACTORY_TEST(
	GIVEN_a_non_persistent_actor_subclass_of_a_persistent_by_default_spatial_actor_WHEN_creating_skeleton_entity_components_THEN_it_does_not_contain_a_persistence_component)
{
	TestFalse("Expected no persistent component",
			  DoesActorTypeHavePersistentComponent<ANonPersistentActorSubclassOfPersistentByDefaultSpatialActor>());
	return true;
}

ENTITYFACTORY_TEST(
	GIVEN_a_non_persistent_actor_subclass_of_an_explicit_persistent_spatial_actor_WHEN_creating_skeleton_entity_components_THEN_it_does_not_contain_a_persistence_component)
{
	TestFalse("Expected no persistent component",
			  DoesActorTypeHavePersistentComponent<ANonPersistentActorSubclassOfExpicitPersistentSpatialActor>());
	return true;
}

ENTITYFACTORY_TEST(
	GIVEN_a_persistent_actor_subclass_of_a_non_persistent_spatial_actor_WHEN_creating_skeleton_entity_components_THEN_it_contains_a_persistence_component)
{
	TestTrue("Expected a persistent component",
			 DoesActorTypeHavePersistentComponent<APersistentActorSubclassOfNonPersistentSpatialActor>());
	return true;
}

ENTITYFACTORY_TEST(
	GIVEN_a_persistent_actor_subclass_of_a_subclass_of_non_persistent_spatial_actor_WHEN_creating_skeleton_entity_components_THEN_it_contains_a_persistence_component)
{
	TestTrue("Expected a persistent component",
			 DoesActorTypeHavePersistentComponent<APersistentActorSubclassOfActorSubclassOfNonPersistentSpatialActor>());
	return true;
}

ENTITYFACTORY_TEST(
	GIVEN_a_non_persistent_actor_subclass_of_a_subclass_of_a_persistent_by_default_spatial_actor_WHEN_creating_skeleton_entity_components_THEN_it_does_not_contain_a_persistence_component)
{
	TestFalse("Expected no persistent component",
			  DoesActorTypeHavePersistentComponent<ANonPersistentActorSubclassOfActorSubclassOfPersistentByDefaultSpatialActor>());
	return true;
}

ENTITYFACTORY_TEST(
	GIVEN_a_non_persistent_actor_subclass_of_a_subclass_of_an_explicit_persistent_spatial_actor_WHEN_creating_skeleton_entity_components_THEN_it_does_not_contain_a_persistence_component)
{
	TestFalse("Expected no persistent component",
			  DoesActorTypeHavePersistentComponent<ANonPersistentActorSubclassOfActorSubclassOfExplicitPersistentSpatialActor>());
	return true;
}
