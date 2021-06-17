#include "Interop/ActorSubviews.h"
#include "Interop/OwnershipCompletenessHandler.h"
#include "Schema/ActorOwnership.h"
#include "SpatialView/ViewCoordinator.h"

#include "SpatialView/OpList/EntityComponentOpList.h"
#include "Tests/SpatialView/SpatialViewUtils.h"
#include "Tests/SpatialView/TargetView.h"
#include "Tests/SpatialView/TestWorker.h"
#include "Tests/TestDefinitions.h"

namespace SpatialGDK
{
struct FOwnershipCompletenessTestFixture
{
	FOwnershipCompletenessTestFixture()
		: Worker(FTestWorker::Create({
			SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID,
			SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID,
		}))
		, OwnershipCompletenessHandler(FOwnershipCompletenessHandler::CreateClientOwnershipHandler())
		, OwnershipSubview(ActorSubviews::CreatePlayerOwnershipSubView(Worker.GetCoordinator(), OwnershipCompletenessHandler))
		, SimulatedSubview(ActorSubviews::CreateSimulatedSubView(Worker.GetCoordinator(), OwnershipCompletenessHandler))
	{
	}

public:
	FTestWorker Worker;
	FOwnershipCompletenessHandler OwnershipCompletenessHandler;
	FSubView& OwnershipSubview;
	FSubView& SimulatedSubview;
};

GDK_TEST(Core, OwnershipCompleteness, PlayerGainsEntityOwnership)
{
	using namespace ActorSubviews;

	constexpr FSpatialEntityId ActorEntityId{ 1 };
	constexpr FSpatialEntityId LocalClientControllerEntityId{ 2 };

	FOwnershipCompletenessTestFixture Fixture;

	Fixture.OwnershipCompletenessHandler.AddPlayerEntity(LocalClientControllerEntityId);

	{
		Fixture.Worker.GetTargetView().AddEntity(ActorEntityId);
		Fixture.Worker.GetTargetView().AddOrSetComponent(ActorEntityId, ComponentData(SpatialConstants::ACTOR_TAG_COMPONENT_ID));
		Fixture.Worker.GetTargetView().AddOrSetComponent(ActorEntityId,
														 ActorOwnership(LocalClientControllerEntityId).CreateComponentData());
	}

	constexpr float ViewAdvanceDeltaTime = 1.0f;
	Fixture.Worker.AdvanceToTargetView(ViewAdvanceDeltaTime);

	TestFalse(TEXT("Entity was player ownership incomplete when considering ownership"),
			  Fixture.OwnershipSubview.IsEntityComplete(ActorEntityId));
	TestFalse(TEXT("Entity was simulated incomplete when considering ownership"), Fixture.SimulatedSubview.IsEntityComplete(ActorEntityId));

	{
		Fixture.Worker.GetTargetView().AddOrSetComponent(ActorEntityId,
														 ComponentData(SpatialConstants::ACTOR_OWNER_ONLY_DATA_TAG_COMPONENT_ID));
		Fixture.Worker.GetTargetView().AddOrSetComponent(ActorEntityId, ComponentData(SpatialConstants::ACTOR_AUTH_TAG_COMPONENT_ID));
		Fixture.Worker.GetTargetView().AddAuthority(ActorEntityId, SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID);
	}

	Fixture.Worker.AdvanceToTargetView(ViewAdvanceDeltaTime);

	TestTrue(TEXT("Entity became player ownership complete"), Fixture.OwnershipSubview.IsEntityComplete(ActorEntityId));
	TestFalse(TEXT("Entity stayed simulated incomplete"), Fixture.SimulatedSubview.IsEntityComplete(ActorEntityId));

	return true;
}

GDK_TEST(Core, OwnershipCompleteness, PlayerLosesEntityOwnership)
{
	using namespace ActorSubviews;

	constexpr FSpatialEntityId ActorEntityId{ 1 };
	constexpr FSpatialEntityId LocalClientControllerEntityId{ 2 };
	constexpr FSpatialEntityId RemoteClientControllerEntityId{ 3 };

	FOwnershipCompletenessTestFixture Fixture;

	Fixture.OwnershipCompletenessHandler.AddPlayerEntity(LocalClientControllerEntityId);

	{
		Fixture.Worker.GetTargetView().AddEntity(ActorEntityId);
		Fixture.Worker.GetTargetView().AddOrSetComponent(ActorEntityId, ComponentData(SpatialConstants::ACTOR_TAG_COMPONENT_ID));
		Fixture.Worker.GetTargetView().AddOrSetComponent(ActorEntityId,
														 ActorOwnership(LocalClientControllerEntityId).CreateComponentData());
		Fixture.Worker.GetTargetView().AddOrSetComponent(ActorEntityId,
														 ComponentData(SpatialConstants::ACTOR_OWNER_ONLY_DATA_TAG_COMPONENT_ID));
		Fixture.Worker.GetTargetView().AddOrSetComponent(ActorEntityId, ComponentData(SpatialConstants::ACTOR_AUTH_TAG_COMPONENT_ID));
		Fixture.Worker.GetTargetView().AddAuthority(ActorEntityId, SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID);
	}

	constexpr float ViewAdvanceDeltaTime = 1.0f;
	Fixture.Worker.AdvanceToTargetView(ViewAdvanceDeltaTime);

	TestTrue(TEXT("Entity was player ownership complete"), Fixture.OwnershipSubview.IsEntityComplete(ActorEntityId));
	TestFalse(TEXT("Entity was simulated incomplete"), Fixture.SimulatedSubview.IsEntityComplete(ActorEntityId));

	{
		Fixture.Worker.GetTargetView().UpdateComponent(ActorEntityId,
													   ActorOwnership(RemoteClientControllerEntityId).CreateComponentUpdate());
	}

	Fixture.Worker.AdvanceToTargetView(ViewAdvanceDeltaTime);

	TestFalse(TEXT("Entity became player ownership incomplete"), Fixture.OwnershipSubview.IsEntityComplete(ActorEntityId));
	TestFalse(TEXT("Entity stayed simulated incomplete"), Fixture.SimulatedSubview.IsEntityComplete(ActorEntityId));

	return true;
}
} // namespace SpatialGDK
