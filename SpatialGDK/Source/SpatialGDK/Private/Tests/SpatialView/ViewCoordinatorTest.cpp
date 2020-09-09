// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ComponentTestUtils.h"
#include "SpatialView/OpList/EntityComponentOpList.h"
#include "SpatialView/OpList/ExtractedOpList.h"
#include "SpatialView/ViewCoordinator.h"
#include "Tests/TestDefinitions.h"

#define VIEWCOORDINATOR_TEST(TestName) GDK_TEST(Core, ViewCoordinator, TestName)

namespace SpatialGDK
{
// A stub for controlling the series of oplists fed into the view coordinator. The list of oplists give to
// SetListsOfOplists will be processed one list of oplists at a time on each call to Advance.
class ConnectionHandlerStub : public AbstractConnectionHandler
{
public:
	void SetListsOfOpLists(TArray<TArray<OpList>> List) { ListsOfOpLists = MoveTemp(List); }

	virtual void Advance() override
	{
		QueuedOpLists = MoveTemp(ListsOfOpLists[0]);
		ListsOfOpLists.RemoveAt(0);
	}

	virtual uint32 GetOpListCount() override { return QueuedOpLists.Num(); }

	virtual OpList GetNextOpList() override
	{
		OpList Temp = MoveTemp(QueuedOpLists[0]);
		QueuedOpLists.RemoveAt(0);
		return Temp;
	}

	virtual void SendMessages(TUniquePtr<MessagesToSend> Messages) override {}

	virtual const FString& GetWorkerId() const override { return WorkerId; }

	virtual const TArray<FString>& GetWorkerAttributes() const override { return Attributes; }

private:
	TArray<TArray<OpList>> ListsOfOpLists;
	TArray<OpList> QueuedOpLists;
	FString WorkerId = "test_worker";
	TArray<FString> Attributes{ "test" };
};

VIEWCOORDINATOR_TEST(GIVEN_view_coordinator_WHEN_create_unfiltered_sub_view_THEN_returns_sub_view_which_passes_through_only_tagged_entity)
{
	const Worker_EntityId TaggedEntityId = 1;
	const Worker_EntityId OtherEntityId = 2;
	const Worker_ComponentId TagComponentId = 1;
	const Worker_ComponentId OtherComponentId = 2;

	TArray<TArray<OpList>> ListsOfOpLists;
	TArray<OpList> OpLists;
	EntityComponentOpListBuilder Builder;
	Builder.AddEntity(TaggedEntityId);
	Builder.AddComponent(TaggedEntityId, ComponentData{ TagComponentId });
	Builder.AddEntity(OtherEntityId);
	Builder.AddComponent(OtherEntityId, ComponentData{ OtherComponentId });
	OpLists.Add(MoveTemp(Builder).CreateOpList());
	ListsOfOpLists.Add(MoveTemp(OpLists));

	auto Handler = MakeUnique<ConnectionHandlerStub>();
	Handler->SetListsOfOpLists(MoveTemp(ListsOfOpLists));
	ViewCoordinator Coordinator{ MoveTemp(Handler) };
	auto& SubView = Coordinator.CreateUnfilteredSubView(TagComponentId);

	Coordinator.Advance();
	SubViewDelta Delta = SubView.GetViewDelta();

	// Only the tagged entity should pass through to the sub view delta.
	TestEqual("There is one entity delta", Delta.EntityDeltas.Num(), 1);
	TestEqual("The entity delta is for the correct entity ID", Delta.EntityDeltas[0].EntityId, 1);

	return true;
}

VIEWCOORDINATOR_TEST(GIVEN_view_coordinator_WHEN_create_filtered_sub_view_THEN_returns_sub_view_which_filters_tagged_entities)
{
	const Worker_EntityId TaggedEntityId = 1;
	const Worker_EntityId OtherTaggedEntityId = 2;
	const Worker_ComponentId TagComponentId = 1;
	const Worker_ComponentId ValueComponentId = 2;
	const double CorrectValue = 1;
	const double IncorrectValue = 2;

	TArray<TArray<OpList>> ListsOfOpLists;
	TArray<OpList> OpLists;
	EntityComponentOpListBuilder Builder;
	Builder.AddEntity(TaggedEntityId);
	Builder.AddComponent(TaggedEntityId, ComponentData{ TagComponentId });
	Builder.AddComponent(TaggedEntityId, CreateTestComponentData(ValueComponentId, CorrectValue));
	Builder.AddEntity(OtherTaggedEntityId);
	Builder.AddComponent(OtherTaggedEntityId, ComponentData{ TagComponentId });
	Builder.AddComponent(OtherTaggedEntityId, CreateTestComponentData(ValueComponentId, IncorrectValue));
	OpLists.Add(MoveTemp(Builder).CreateOpList());
	ListsOfOpLists.Add(MoveTemp(OpLists));

	EntityComponentOpListBuilder SecondBuilder;
	TArray<OpList> SecondOpLists;
	SecondBuilder.UpdateComponent(OtherTaggedEntityId, CreateTestComponentUpdate(ValueComponentId, CorrectValue));
	SecondOpLists.Add(MoveTemp(SecondBuilder).CreateOpList());
	ListsOfOpLists.Add(MoveTemp(SecondOpLists));

	auto Handler = MakeUnique<ConnectionHandlerStub>();
	Handler->SetListsOfOpLists(MoveTemp(ListsOfOpLists));
	ViewCoordinator Coordinator{ MoveTemp(Handler) };

	auto& SubView = Coordinator.CreateSubView(
		TagComponentId,
		[CorrectValue, ValueComponentId](const Worker_EntityId& EntityId, const EntityViewElement& Element) {
			const ComponentData* It = Element.Components.FindByPredicate(ComponentIdEquality{ ValueComponentId });
			if (GetValueFromSchemaComponentData(It->GetUnderlying()) == CorrectValue)
			{
				return true;
			}
			return false;
		},
		TArray<FDispatcherRefreshCallback>{ Coordinator.CreateComponentChangedRefreshCallback(ValueComponentId) });

	Coordinator.Advance();
	SubViewDelta Delta = SubView.GetViewDelta();

	// Only the tagged entity with the correct value should pass through to the sub view delta.
	TestEqual("There is one entity delta", Delta.EntityDeltas.Num(), 1);
	TestEqual("The entity delta is for the correct entity ID", Delta.EntityDeltas[0].EntityId, 1);

	Coordinator.Advance();
	Delta = SubView.GetViewDelta();

	// The value on the other entity should have updated, so we should see an add for the second entity.
	TestEqual("There is one entity delta", Delta.EntityDeltas.Num(), 1);
	// TestEqual("The entity delta is for the correct entity ID", Delta.EntityDeltas[0].EntityId, 2);

	return true;
}
} // namespace SpatialGDK
