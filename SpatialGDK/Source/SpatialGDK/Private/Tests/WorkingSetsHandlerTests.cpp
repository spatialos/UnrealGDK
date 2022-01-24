#include "Interop/WorkingSetsHandler.h"

#include "Misc/AutomationTest.h"

#include "Schema/WorkingSet.h"
#include "Tests/SpatialView/TestWorker.h"

namespace SpatialGDK
{
class FWorkingSetsHandlerTestFixture
{
public:
	constexpr static Worker_ComponentSetId ActorEntityAuthComponentSet = 1;
	constexpr static Worker_ComponentId ActorAuthorityTag = 2;
	FWorkingSetsHandlerTestFixture()
		: Worker(FTestWorker::Create({ ActorEntityAuthComponentSet }))
		, MarkerEntitiesSubview(CreateWorkingSetMarkersSubview(Worker.GetCoordinator()))
		, EntityAuthoritySubview(
			  Worker.GetCoordinator().CreateSubView(ActorAuthorityTag, FSubView::NoFilter, FSubView::NoDispatcherCallbacks))
		, Handler(DataStore, EntityAuthoritySubview)
	{
	}
	FWorkingSetCompletenessHandler& GetHandler() { return Handler; }
	ViewCoordinator& GetCoordinator() { return Worker.GetCoordinator(); }
	FTargetView& GetTargetView() { return Worker.GetTargetView(); }
	void Advance()
	{
		Worker.AdvanceToTargetView(0.0f);
		DataStore.Advance(MarkerEntitiesSubview);
		Handler.Advance(GetCoordinator());
	}

private:
	FTestWorker Worker;
	FSubView& MarkerEntitiesSubview;
	FSubView& EntityAuthoritySubview;
	FWorkingSetDataStorage DataStore;
	FWorkingSetCompletenessHandler Handler;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWorkingSetsHandlerTest, "SpatialGDK.WorkingSets.Handler1",
								 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter);

bool FWorkingSetsHandlerTest::RunTest(const FString& Parameters)
{
	FWorkingSetsHandlerTestFixture Fixture;

	FWorkingSetCompletenessHandler& Handler = Fixture.GetHandler();

	constexpr Worker_EntityId SetMemberEntityId = 100;
	{
		Fixture.GetTargetView().AddEntity(SetMemberEntityId);
		Fixture.GetTargetView().AddOrSetComponent(SetMemberEntityId, FWorkingSetMember().CreateComponentData());
	}
	Fixture.Advance();

	TestNull(TEXT("Entity not in a working set in absence of working set markers"), Handler.GetOwningSet(SetMemberEntityId));

	constexpr Worker_EntityId SetMarkerEntityId = 200;
	{
		Fixture.GetTargetView().AddEntity(SetMarkerEntityId);
		FWorkingSetState Request;
		Request.Epoch = 1;
		Request.MemberEntities = { SetMemberEntityId };
		Request.LeaderEntityId = SetMemberEntityId;
		Fixture.GetTargetView().AddOrSetComponent(SetMarkerEntityId, FWorkingSetMarkerRequest(Request).CreateComponentData());
		FWorkingSetMarkerResponse Response;
		Fixture.GetTargetView().AddOrSetComponent(SetMarkerEntityId, Response.CreateComponentData());
		Fixture.GetTargetView().AddOrSetComponent(SetMarkerEntityId, ComponentData(SpatialConstants::WORKING_SET_MARKER_TAG_COMPONENT_ID));
	}
	Fixture.Advance();

	TestNull(TEXT("Entity not in a working set when requested"), Handler.GetOwningSet(SetMemberEntityId));

	{
		FWorkingSetState Response;
		Response.Epoch = 1;
		Response.MemberEntities = { SetMemberEntityId };
		Response.LeaderEntityId = SetMarkerEntityId;

		Fixture.GetTargetView().UpdateComponent(SetMarkerEntityId, FWorkingSetMarkerResponse(Response).CreateComponentUpdate());
	}

	Fixture.Advance();

	if (!TestNotNull(TEXT("Entity in a working set when confirmed"), Handler.GetOwningSet(SetMemberEntityId)))
	{
		return false;
	}

	TestFalse(TEXT("Working set isn't complete when an entity is not auth on the worker"),
			  Handler.IsWorkingSetComplete(*Handler.GetOwningSet(SetMemberEntityId)));

	{
		Fixture.GetTargetView().AddOrSetComponent(SetMemberEntityId, ComponentData(FWorkingSetsHandlerTestFixture::ActorAuthorityTag));
	}

	Fixture.Advance();

	TestTrue(TEXT("Working set becomes complete when an entity becomes auth on the worker"),
			 Handler.IsWorkingSetComplete(*Handler.GetOwningSet(SetMemberEntityId)));

	return true;
}
} // namespace SpatialGDK
