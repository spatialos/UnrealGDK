#include "Interop/ActorSubviews.h"
#include "Interop/OwnershipCompletenessHandler.h"
#include "Schema/ActorOwnership.h"
#include "SpatialView/ViewCoordinator.h"

#include "SpatialView/OpList/EntityComponentOpList.h"
#include "Tests/SpatialView/SpatialViewUtils.h"
#include "Tests/TestDefinitions.h"

namespace SpatialGDK
{
struct FOwnershipCompletenessTestFixture
{
	static FOwnershipCompletenessTestFixture Create()
	{
		FComponentSetData ComponentSetData;
		ComponentSetData.ComponentSets.Emplace(SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID);
		ComponentSetData.ComponentSets.Emplace(SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID);

		return FOwnershipCompletenessTestFixture(MakeUnique<MockConnectionHandler>(), ComponentSetData, /*bIsServer =*/false);
	}

private:
	FOwnershipCompletenessTestFixture(TUniquePtr<MockConnectionHandler> InConnectionHandler, FComponentSetData ComponentSetData,
									  bool bIsServer)
		: ConnectionHandler(*InConnectionHandler)
		, Coordinator(MoveTemp(InConnectionHandler), /*EventTracer =*/nullptr, MoveTemp(ComponentSetData))
		, OwnershipCompletenessHandler(bIsServer)
		, OwnershipSubview(ActorSubviews::CreatePlayerOwnershipSubView(Coordinator, OwnershipCompletenessHandler))
		, SimulatedSubview(ActorSubviews::CreateSimulatedSubView(Coordinator, OwnershipCompletenessHandler))
	{
	}

public:
	MockConnectionHandler& ConnectionHandler;
	ViewCoordinator Coordinator;
	FOwnershipCompletenessHandler OwnershipCompletenessHandler;
	FSubView& OwnershipSubview;
	FSubView& SimulatedSubview;
};

GDK_TEST(Core, OwnershipCompleteness, PlayerGainsEntityOwnership)
{
	using namespace ActorSubviews;

	constexpr Worker_EntityId ActorEntityId = 1;
	constexpr Worker_EntityId LocalClientControllerEntityId = 2;

	FOwnershipCompletenessTestFixture Fixture = FOwnershipCompletenessTestFixture::Create();

	MockConnectionHandler& ConnHandlerRef = Fixture.ConnectionHandler;

	Fixture.OwnershipCompletenessHandler.AddPlayerEntity(LocalClientControllerEntityId);

	{
		EntityComponentOpListBuilder Builder;
		Builder.AddEntity(ActorEntityId)
			.AddComponent(ActorEntityId, ComponentData(SpatialConstants::ACTOR_TAG_COMPONENT_ID))
			.AddComponent(ActorEntityId, ActorOwnership(LocalClientControllerEntityId).CreateComponentData());

		TArray<OpList> OpLists;
		OpLists.Emplace(MoveTemp(Builder).CreateOpList());

		TArray<TArray<OpList>> OpLists1;
		OpLists1.Emplace(MoveTemp(OpLists));

		ConnHandlerRef.SetListsOfOpLists(MoveTemp(OpLists1));
	}

	constexpr float ViewAdvanceDeltaTime = 1.0f;
	Fixture.Coordinator.Advance(ViewAdvanceDeltaTime);

	TestFalse(TEXT("Entity was player ownership incomplete when considering ownership"),
			  Fixture.OwnershipSubview.IsEntityComplete(ActorEntityId));
	TestFalse(TEXT("Entity was simulated incomplete when considering ownership"), Fixture.SimulatedSubview.IsEntityComplete(ActorEntityId));

	{
		EntityComponentOpListBuilder Builder;
		Builder.AddComponent(ActorEntityId, ComponentData(SpatialConstants::ACTOR_OWNER_ONLY_DATA_TAG_COMPONENT_ID))
			.AddComponent(ActorEntityId, ComponentData(SpatialConstants::ACTOR_AUTH_TAG_COMPONENT_ID))
			.SetAuthority(ActorEntityId, SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID, WORKER_AUTHORITY_AUTHORITATIVE,
						  /*Components =*/{});

		TArray<OpList> OpLists;
		OpLists.Emplace(MoveTemp(Builder).CreateOpList());

		TArray<TArray<OpList>> OpLists1;
		OpLists1.Emplace(MoveTemp(OpLists));

		ConnHandlerRef.SetListsOfOpLists(MoveTemp(OpLists1));
	}

	Fixture.Coordinator.Advance(ViewAdvanceDeltaTime);

	TestTrue(TEXT("Entity became player ownership complete"), Fixture.OwnershipSubview.IsEntityComplete(ActorEntityId));
	TestFalse(TEXT("Entity stayed simulated incomplete"), Fixture.SimulatedSubview.IsEntityComplete(ActorEntityId));

	return true;
}

GDK_TEST(Core, OwnershipCompleteness, PlayerLosesEntityOwnership)
{
	using namespace ActorSubviews;

	constexpr Worker_EntityId ActorEntityId = 1;
	constexpr Worker_EntityId LocalClientControllerEntityId = 2;
	constexpr Worker_EntityId RemoteClientControllerEntityId = 3;

	FOwnershipCompletenessTestFixture Fixture = FOwnershipCompletenessTestFixture::Create();

	Fixture.OwnershipCompletenessHandler.AddPlayerEntity(LocalClientControllerEntityId);

	{
		EntityComponentOpListBuilder Builder;
		Builder.AddEntity(ActorEntityId)
			.AddComponent(ActorEntityId, ComponentData(SpatialConstants::ACTOR_TAG_COMPONENT_ID))
			.AddComponent(ActorEntityId, ActorOwnership(LocalClientControllerEntityId).CreateComponentData())
			.AddComponent(ActorEntityId, ComponentData(SpatialConstants::ACTOR_OWNER_ONLY_DATA_TAG_COMPONENT_ID))
			.AddComponent(ActorEntityId, ComponentData(SpatialConstants::ACTOR_AUTH_TAG_COMPONENT_ID))
			.SetAuthority(ActorEntityId, SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID, WORKER_AUTHORITY_AUTHORITATIVE,
						  /*Components =*/{});

		TArray<OpList> OpLists;
		OpLists.Emplace(MoveTemp(Builder).CreateOpList());

		TArray<TArray<OpList>> OpLists1;
		OpLists1.Emplace(MoveTemp(OpLists));

		Fixture.ConnectionHandler.SetListsOfOpLists(MoveTemp(OpLists1));
	}

	constexpr float ViewAdvanceDeltaTime = 1.0f;
	Fixture.Coordinator.Advance(ViewAdvanceDeltaTime);

	TestTrue(TEXT("Entity was player ownership complete"), Fixture.OwnershipSubview.IsEntityComplete(ActorEntityId));
	TestFalse(TEXT("Entity was simulated incomplete"), Fixture.SimulatedSubview.IsEntityComplete(ActorEntityId));

	{
		EntityComponentOpListBuilder Builder;
		Builder.UpdateComponent(ActorEntityId, ActorOwnership(RemoteClientControllerEntityId).CreateComponentUpdate());

		TArray<OpList> OpLists;
		OpLists.Emplace(MoveTemp(Builder).CreateOpList());

		TArray<TArray<OpList>> OpLists1;
		OpLists1.Emplace(MoveTemp(OpLists));

		Fixture.ConnectionHandler.SetListsOfOpLists(MoveTemp(OpLists1));
	}

	Fixture.Coordinator.Advance(ViewAdvanceDeltaTime);

	TestFalse(TEXT("Entity became player ownership incomplete"), Fixture.OwnershipSubview.IsEntityComplete(ActorEntityId));
	TestFalse(TEXT("Entity stayed simulated incomplete"), Fixture.SimulatedSubview.IsEntityComplete(ActorEntityId));

	return true;
}
} // namespace SpatialGDK
