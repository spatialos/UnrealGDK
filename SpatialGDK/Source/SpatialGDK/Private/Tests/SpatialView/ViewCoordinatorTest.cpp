// // Copyright (c) Improbable Worlds Ltd, All Rights Reserved
//
// #include "SpatialView/OpList/EntityComponentOpList.h"
// #include "SpatialView/OpList/ExtractedOpList.h"
// #include "SpatialView/ViewCoordinator.h"
// #include "Tests/SpatialView/ComponentTestUtils.h"
// #include "Tests/TestDefinitions.h"
//
// #define VIEWCOORDINATOR_TEST(TestName) GDK_TEST(Core, ViewCoordinator, TestName)
//
// namespace SpatialGDK
// {
// // A stub for controlling the series of oplists fed into the view coordinator. The list of oplists given to
// // SetListsOfOplists will be processed one list of oplists at a time on each call to Advance.
// class ConnectionHandlerStub : public AbstractConnectionHandler
// {
// public:
// 	void SetListsOfOpLists(TArray<TArray<OpList>> List) { ListsOfOpLists = MoveTemp(List); }
//
// 	virtual void Advance() override
// 	{
// 		QueuedOpLists = MoveTemp(ListsOfOpLists[0]);
// 		ListsOfOpLists.RemoveAt(0);
// 	}
//
// 	virtual uint32 GetOpListCount() override { return QueuedOpLists.Num(); }
//
// 	virtual OpList GetNextOpList() override
// 	{
// 		OpList Temp = MoveTemp(QueuedOpLists[0]);
// 		QueuedOpLists.RemoveAt(0);
// 		return Temp;
// 	}
//
// 	virtual void SendMessages(TUniquePtr<MessagesToSend> Messages) override {}
//
// 	virtual const FString& GetWorkerId() const override { return WorkerId; }
//
// 	virtual Worker_EntityId GetWorkerSystemEntityId() const override { return WorkerSystemEntityId; }
//
// private:
// 	TArray<TArray<OpList>> ListsOfOpLists;
// 	TArray<OpList> QueuedOpLists;
// 	Worker_EntityId WorkerSystemEntityId = 1;
// 	FString WorkerId = TEXT("test_worker");
// 	TArray<FString> Attributes = { TEXT("test") };
// };
//
// VIEWCOORDINATOR_TEST(GIVEN_view_coordinator_WHEN_create_unfiltered_sub_view_THEN_returns_sub_view_which_passes_through_only_tagged_entity)
// {
// 	const Worker_EntityId EntityId = 1;
// 	const Worker_EntityId TaggedEntityId = 2;
// 	const Worker_ComponentId ComponentId = 1;
// 	const Worker_ComponentId TagComponentId = 2;
//
// 	TArray<TArray<OpList>> ListsOfOpLists;
// 	TArray<OpList> OpLists;
// 	EntityComponentOpListBuilder Builder;
// 	Builder.AddEntity(TaggedEntityId);
// 	Builder.AddComponent(TaggedEntityId, ComponentData{ TagComponentId });
// 	Builder.AddEntity(EntityId);
// 	Builder.AddComponent(EntityId, ComponentData{ ComponentId });
// 	OpLists.Add(MoveTemp(Builder).CreateOpList());
// 	ListsOfOpLists.Add(MoveTemp(OpLists));
//
// 	auto Handler = MakeUnique<ConnectionHandlerStub>();
// 	Handler->SetListsOfOpLists(MoveTemp(ListsOfOpLists));
// 	ViewCoordinator Coordinator{ MoveTemp(Handler), nullptr };
// 	auto& SubView = Coordinator.CreateSubView(TagComponentId, FSubView::NoFilter, FSubView::NoDispatcherCallbacks);
//
// 	Coordinator.Advance(0.0f);
// 	FSubViewDelta Delta = SubView.GetViewDelta();
//
// 	// Only the tagged entity should pass through to the sub view delta.
// 	TestEqual("There is one entity delta", Delta.EntityDeltas.Num(), 1);
// 	if (Delta.EntityDeltas.Num() != 1)
// 	{
// 		// test already failed
// 		return true;
// 	}
// 	TestEqual("The entity delta is for the correct entity ID", Delta.EntityDeltas[0].EntityId, TaggedEntityId);
//
// 	return true;
// }
//
// VIEWCOORDINATOR_TEST(GIVEN_view_coordinator_WHEN_create_filtered_sub_view_THEN_returns_sub_view_which_filters_tagged_entities)
// {
// 	const Worker_EntityId TaggedEntityId = 2;
// 	const Worker_EntityId OtherTaggedEntityId = 3;
// 	const Worker_ComponentId TagComponentId = 2;
// 	const Worker_ComponentId ValueComponentId = 3;
// 	const double CorrectValue = 1;
// 	const double IncorrectValue = 2;
//
// 	TArray<TArray<OpList>> ListsOfOpLists;
// 	TArray<OpList> OpLists;
// 	EntityComponentOpListBuilder Builder;
// 	Builder.AddEntity(TaggedEntityId);
// 	Builder.AddComponent(TaggedEntityId, ComponentData{ TagComponentId });
// 	Builder.AddComponent(TaggedEntityId, CreateTestComponentData(ValueComponentId, CorrectValue));
// 	Builder.AddEntity(OtherTaggedEntityId);
// 	Builder.AddComponent(OtherTaggedEntityId, ComponentData{ TagComponentId });
// 	Builder.AddComponent(OtherTaggedEntityId, CreateTestComponentData(ValueComponentId, IncorrectValue));
// 	OpLists.Add(MoveTemp(Builder).CreateOpList());
// 	ListsOfOpLists.Add(MoveTemp(OpLists));
//
// 	Builder = EntityComponentOpListBuilder{};
// 	OpLists = TArray<OpList>{};
// 	Builder.UpdateComponent(OtherTaggedEntityId, CreateTestComponentUpdate(ValueComponentId, CorrectValue));
// 	OpLists.Add(MoveTemp(Builder).CreateOpList());
// 	ListsOfOpLists.Add(MoveTemp(OpLists));
//
// 	auto Handler = MakeUnique<ConnectionHandlerStub>();
// 	Handler->SetListsOfOpLists(MoveTemp(ListsOfOpLists));
// 	ViewCoordinator Coordinator{ MoveTemp(Handler), nullptr };
//
// 	auto& SubView = Coordinator.CreateSubView(
// 		TagComponentId,
// 		[CorrectValue, ValueComponentId](const Worker_EntityId&, const EntityViewElement& Element) {
// 			const ComponentData* It = Element.Components.FindByPredicate(ComponentIdEquality{ ValueComponentId });
// 			if (GetValueFromTestComponentData(It->GetUnderlying()) == CorrectValue)
// 			{
// 				return true;
// 			}
// 			return false;
// 		},
// 		TArray<FDispatcherRefreshCallback>{ Coordinator.CreateComponentChangedRefreshCallback(ValueComponentId) });
//
// 	Coordinator.Advance(0.0f);
// 	FSubViewDelta Delta = SubView.GetViewDelta();
//
// 	// Only the tagged entity with the correct value should pass through to the sub view delta.
// 	TestEqual("There is one entity delta", Delta.EntityDeltas.Num(), 1);
// 	if (Delta.EntityDeltas.Num() != 1)
// 	{
// 		return true;
// 	}
// 	TestEqual("The entity delta is for the correct entity ID", Delta.EntityDeltas[0].EntityId, TaggedEntityId);
//
// 	Coordinator.Advance(0.0f);
// 	Delta = SubView.GetViewDelta();
//
// 	// The value on the other entity should have updated, so we should see an add for the second entity.
// 	TestEqual("There is one entity delta", Delta.EntityDeltas.Num(), 1);
// 	if (Delta.EntityDeltas.Num() != 1)
// 	{
// 		return true;
// 	}
// 	TestEqual("The entity delta is for the correct entity ID", Delta.EntityDeltas[0].EntityId, OtherTaggedEntityId);
//
// 	return true;
// }
//
// VIEWCOORDINATOR_TEST(GIVEN_view_coordinator_with_multiple_tracked_subviews_WHEN_refresh_THEN_all_subviews_refreshed)
// {
// 	const Worker_EntityId TaggedEntityId = 2;
// 	const Worker_ComponentId TagComponentId = 2;
//
// 	bool EntityComplete = false;
// 	const int NumberOfSubViews = 100;
//
// 	TArray<TArray<OpList>> ListsOfOpLists;
//
// 	TArray<OpList> OpLists;
// 	EntityComponentOpListBuilder Builder;
// 	Builder.AddEntity(TaggedEntityId);
// 	Builder.AddComponent(TaggedEntityId, ComponentData{ TagComponentId });
// 	OpLists.Add(MoveTemp(Builder).CreateOpList());
// 	ListsOfOpLists.Add(MoveTemp(OpLists));
//
// 	TArray<OpList> SecondOpLists;
// 	ListsOfOpLists.Add(MoveTemp(SecondOpLists));
//
// 	auto Handler = MakeUnique<ConnectionHandlerStub>();
// 	Handler->SetListsOfOpLists(MoveTemp(ListsOfOpLists));
// 	ViewCoordinator Coordinator{ MoveTemp(Handler), nullptr };
//
// 	TArray<FSubView*> SubViews;
//
// 	for (int i = 0; i < NumberOfSubViews; ++i)
// 	{
// 		SubViews.Emplace(&Coordinator.CreateSubView(
// 			TagComponentId,
// 			[&EntityComplete](const Worker_EntityId&, const EntityViewElement&) {
// 				return EntityComplete;
// 			},
// 			FSubView::NoDispatcherCallbacks));
// 	}
//
// 	Coordinator.Advance(0.0f);
// 	FSubViewDelta Delta;
//
// 	// All the subviews should have no complete entities, so their deltas should be empty.
// 	for (int i = 0; i < NumberOfSubViews; ++i)
// 	{
// 		for (FSubView* SubView : SubViews)
// 		{
// 			Delta = SubView->GetViewDelta();
// 			TestEqual("There are no entity deltas", Delta.EntityDeltas.Num(), 0);
// 		}
// 	}
//
// 	EntityComplete = true;
// 	Coordinator.RefreshEntityCompleteness(TaggedEntityId);
// 	Coordinator.Advance(0.0f);
//
// 	// All the subviews' filters will have changed their truth value due to the change in local state.
// 	for (int i = 0; i < NumberOfSubViews; ++i)
// 	{
// 		for (FSubView* SubView : SubViews)
// 		{
// 			Delta = SubView->GetViewDelta();
// 			TestEqual("There is one entity delta", Delta.EntityDeltas.Num(), 1);
// 			if (Delta.EntityDeltas.Num() != 1)
// 			{
// 				// test has already failed
// 				return true;
// 			}
// 			TestEqual("The entity delta is for the correct entity ID", Delta.EntityDeltas[0].EntityId, TaggedEntityId);
// 		}
// 	}
//
// 	return true;
// }
// } // namespace SpatialGDK
