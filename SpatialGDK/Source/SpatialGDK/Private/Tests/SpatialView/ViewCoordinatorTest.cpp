// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ComponentTestUtils.h"
#include "SpatialView/ViewCoordinator.h"
#include "SpatialView/OpList/EntityComponentOpList.h"
#include "SpatialView/OpList/ExtractedOpList.h"
#include "Tests/TestDefinitions.h"

#define VIEWCOORDINATOR_TEST(TestName) GDK_TEST(Core, ViewCoordinator, TestName)

namespace SpatialGDK
{
	// A stub for controlling the series of oplists fed into the view coordinator. The list of oplists give to
	// SetListsOfOplists will be processed one list of oplists at a time on each call to Advance.
	class ConnectionHandlerStub : public AbstractConnectionHandler
	{
	public:
		void SetListsOfOpLists(TArray<TArray<OpList>> List)
		{
			ListsOfOpLists = MoveTemp(List);
		}

		virtual void Advance() override
		{
			QueuedOpLists = MoveTemp(ListsOfOpLists[0]);
			ListsOfOpLists.RemoveAt(0);
		}

		virtual uint32 GetOpListCount() override
		{
			return QueuedOpLists.Num();
		}

		virtual OpList GetNextOpList() override
		{
			OpList Temp = MoveTemp(QueuedOpLists[0]);
			QueuedOpLists.RemoveAt(0);
			return Temp;
		}

		virtual void SendMessages(TUniquePtr<MessagesToSend> Messages) override {}

		virtual const FString& GetWorkerId() const override
		{
			return WorkerId;
		}

		virtual const TArray<FString>& GetWorkerAttributes() const override
		{
			return Attributes;
		}

	private:
		TArray<TArray<OpList>> ListsOfOpLists;
		TArray<OpList> QueuedOpLists;
		FString WorkerId = "test_worker";
		TArray<FString> Attributes{"test"};
	};

VIEWCOORDINATOR_TEST(GIVEN_view_coordinator_WHEN_create_unfiltered_sub_view_THEN_returns_sub_view_which_passes_through_tagged_entity)
{
	const Worker_EntityId EntityId = 1;
	const Worker_ComponentId TagComponentId = 1;

	TArray<TArray<OpList>> ListsOfOpLists;
	TArray<OpList> FirstOpList;
	EntityComponentOpListBuilder Builder;
	Builder.AddEntity(EntityId);
	Builder.AddComponent(EntityId, ComponentData{TagComponentId});
	FirstOpList.Add(MoveTemp(Builder).CreateOpList());
	ListsOfOpLists.Add(MoveTemp(FirstOpList));

	auto Handler = MakeUnique<ConnectionHandlerStub>();
	Handler->SetListsOfOpLists(MoveTemp(ListsOfOpLists));
	ViewCoordinator Coordinator{MoveTemp(Handler)};
	auto& SubView = Coordinator.CreateUnfilteredSubView(TagComponentId);

	Coordinator.Advance();
	SubViewDelta Delta = SubView.GetViewDelta();

	TestEqual("There is one entity delta", Delta.EntityDeltas.Num(), 1);
	TestEqual("The entity delta is for the correct entity ID", Delta.EntityDeltas[0].EntityId, 1);

	return true;
}
} // namespace SpatialGDK
