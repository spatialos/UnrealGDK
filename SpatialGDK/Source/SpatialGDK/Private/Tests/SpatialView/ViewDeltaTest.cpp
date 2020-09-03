// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialViewUtils.h"
#include "Tests/TestDefinitions.h"

#include "SpatialView/OpList/EntityComponentOpList.h"
#include "SpatialView/ViewDelta.h"
#include "Tests/SpatialView/ExptectedViewDelta.h"

#define VIEWDELTA_TEST(TestName) GDK_TEST(Core, ViewDelta, TestName)

using namespace SpatialGDK;

namespace
{
void SetFromOpList(ViewDelta& Delta, EntityView& View, EntityComponentOpListBuilder& OpListBuilder)
{
	OpList Ops = MoveTemp(OpListBuilder).CreateOpList();
	TArray<OpList> OpLists;
	OpLists.Push(MoveTemp(Ops));
	Delta.SetFromOpList(MoveTemp(OpLists), View);
}
} // anonymous namespace

VIEWDELTA_TEST(GIVEN_empty_view_WHEN_add_entity_THEN_get_entity_in_view_and_delta)
{
	ViewDelta InputDelta;
	EntityView InputView;

	EntityComponentOpListBuilder OpListBuilder;
	OpListBuilder.AddEntity(TestEntityId);
	SetFromOpList(InputDelta, InputView, OpListBuilder);

	EntityView ExpectedView;
	AddEntityToView(ExpectedView, TestEntityId);

	ExpectedViewDelta ExpectedDelta;
	ExpectedDelta.AddEntityDelta(TestEntityId, ExpectedViewDelta::ADD);

	TestTrue("View Deltas are equal", ExpectedDelta.Compare(InputDelta));
	TestTrue("Views are equal", AreEquivalent(InputView, ExpectedView));

	return true;
}

VIEWDELTA_TEST(GIVEN_entity_in_view_WHEN_remove_entity_THEN_empty_view)
{
	ViewDelta InputDelta;
	EntityView InputView;
	AddEntityToView(InputView, TestEntityId);

	EntityComponentOpListBuilder OpListBuilder;
	OpListBuilder.RemoveEntity(TestEntityId);
	SetFromOpList(InputDelta, InputView, OpListBuilder);

	EntityView ExpectedView;
	ExpectedViewDelta ExpectedDelta;
	ExpectedDelta.AddEntityDelta(TestEntityId, ExpectedViewDelta::REMOVE);

	TestTrue("View Deltas are equal", ExpectedDelta.Compare(InputDelta));
	TestTrue("Views are equal", AreEquivalent(InputView, ExpectedView));

	return true;
}

VIEWDELTA_TEST(GIVEN_entity_in_view_WHEN_add_component_THEN_entity_and_component_in_view)
{
	ViewDelta InputDelta;
	EntityView InputView;
	AddEntityToView(InputView, TestEntityId);

	EntityComponentOpListBuilder OpListBuilder;
	OpListBuilder.AddComponent(TestEntityId, CreateTestComponentData(TestComponentId, TestComponentValue));
	SetFromOpList(InputDelta, InputView, OpListBuilder);

	EntityView ExpectedView;
	AddEntityToView(ExpectedView, TestEntityId);
	AddComponentToView(ExpectedView, TestEntityId, TestComponentId, TestComponentValue);

	ExpectedViewDelta ExpectedDelta;
	ExpectedDelta.AddEntityDelta(TestEntityId, ExpectedViewDelta::UPDATE)
		.AddComponentAdded(TestEntityId, CreateTestComponentData(TestComponentId, TestComponentValue));

	TestTrue("View Deltas are equal", ExpectedDelta.Compare(InputDelta));
	TestTrue("Views are equal", AreEquivalent(InputView, ExpectedView));

	return true;
}

VIEWDELTA_TEST(GIVEN_entity_and_component_in_view_WHEN_update_component_THEN_component_udated_in_view)
{
	double NewComponentValue = 30;

	ViewDelta InputDelta;
	EntityView InputView;
	AddEntityToView(InputView, TestEntityId);
	AddComponentToView(InputView, TestEntityId, TestComponentId, TestComponentValue);

	EntityComponentOpListBuilder OpListBuilder;
	OpListBuilder.UpdateComponent(TestEntityId, CreateTestComponentUpdate(TestComponentId, NewComponentValue));
	SetFromOpList(InputDelta, InputView, OpListBuilder);

	EntityView ExpectedView;
	AddEntityToView(ExpectedView, TestEntityId);
	AddComponentToView(ExpectedView, TestEntityId, TestComponentId, NewComponentValue);

	ExpectedViewDelta ExpectedDelta;
	ExpectedDelta.AddEntityDelta(TestEntityId, ExpectedViewDelta::UPDATE)
		.AddComponentUpdate(TestEntityId, CreateTestComponentUpdate(TestComponentId, NewComponentValue));

	TestTrue("View Deltas are equal", ExpectedDelta.Compare(InputDelta));
	TestTrue("Views are equal", AreEquivalent(InputView, ExpectedView));

	return true;
}

VIEWDELTA_TEST(GIVEN_entity_and_component_in_view_WHEN_remove_component_THEN_component_not_in_view)
{
	ViewDelta InputDelta;
	EntityView InputView;
	AddEntityToView(InputView, TestEntityId);
	AddComponentToView(InputView, TestEntityId, TestComponentId, TestComponentValue);

	EntityComponentOpListBuilder OpListBuilder;
	OpListBuilder.RemoveComponent(TestEntityId, TestComponentId);
	SetFromOpList(InputDelta, InputView, OpListBuilder);

	EntityView ExpectedView;
	AddEntityToView(ExpectedView, TestEntityId);

	ExpectedViewDelta ExpectedDelta;
	ExpectedDelta.AddEntityDelta(TestEntityId, ExpectedViewDelta::UPDATE).AddComponentRemoved(TestEntityId, TestComponentId);

	TestTrue("View Deltas are equal", ExpectedDelta.Compare(InputDelta));
	TestTrue("Views are equal", AreEquivalent(InputView, ExpectedView));

	return true;
}

VIEWDELTA_TEST(GIVEN_entity_and_component_in_view_WHEN_authority_gained_THEN_authority_in_view)
{
	ViewDelta InputDelta;
	EntityView InputView;
	AddEntityToView(InputView, TestEntityId);
	AddComponentToView(InputView, TestEntityId, TestComponentId, TestComponentValue);

	EntityComponentOpListBuilder OpListBuilder;
	OpListBuilder.SetAuthority(TestEntityId, TestComponentId, WORKER_AUTHORITY_AUTHORITATIVE);
	SetFromOpList(InputDelta, InputView, OpListBuilder);

	EntityView ExpectedView;
	AddEntityToView(ExpectedView, TestEntityId);
	AddComponentToView(ExpectedView, TestEntityId, TestComponentId, TestComponentValue);
	AddAuthorityToView(ExpectedView, TestEntityId, TestComponentId);

	ExpectedViewDelta ExpectedDelta;
	ExpectedDelta.AddEntityDelta(TestEntityId, ExpectedViewDelta::UPDATE).AddAuthorityGained(TestEntityId, TestComponentId);

	TestTrue("View Deltas are equal", ExpectedDelta.Compare(InputDelta));
	TestTrue("Views are equal", AreEquivalent(InputView, ExpectedView));

	return true;
}

VIEWDELTA_TEST(GIVEN_entity_and_auth_component_in_view_WHEN_authority_lost_THEN_unauth_component_in_view)
{
	ViewDelta InputDelta;
	EntityView InputView;
	AddEntityToView(InputView, TestEntityId);
	AddComponentToView(InputView, TestEntityId, TestComponentId, TestComponentValue);
	AddAuthorityToView(InputView, TestEntityId, TestComponentId);

	EntityComponentOpListBuilder OpListBuilder;
	OpListBuilder.SetAuthority(TestEntityId, TestComponentId, WORKER_AUTHORITY_NOT_AUTHORITATIVE);
	SetFromOpList(InputDelta, InputView, OpListBuilder);

	EntityView ExpectedView;
	AddEntityToView(ExpectedView, TestEntityId);
	AddComponentToView(ExpectedView, TestEntityId, TestComponentId, TestComponentValue);

	ExpectedViewDelta ExpectedDelta;
	ExpectedDelta.AddEntityDelta(TestEntityId, ExpectedViewDelta::UPDATE).AddAuthorityLost(TestEntityId, TestComponentId);

	TestTrue("View Deltas are equal", ExpectedDelta.Compare(InputDelta));
	TestTrue("Views are equal", AreEquivalent(InputView, ExpectedView));

	return true;
}

VIEWDELTA_TEST(GIVEN_connected_view_WHEN_disconnect_op_THEN_disconnected_view)
{
	ViewDelta InputDelta;
	EntityView InputView;

	EntityComponentOpListBuilder OpListBuilder;
	OpListBuilder.SetDisconnect(WORKER_CONNECTION_STATUS_CODE_REJECTED, TEXT("Disconnected"));
	SetFromOpList(InputDelta, InputView, OpListBuilder);

	EntityView ExpectedView;

	ExpectedViewDelta ExpectedDelta;
	ExpectedDelta.AddDisconnect(WORKER_CONNECTION_STATUS_CODE_REJECTED, TEXT("Disconnected"));

	TestTrue("View Deltas are equal", ExpectedDelta.Compare(InputDelta));
	TestTrue("Views are equal", AreEquivalent(InputView, ExpectedView));

	return true;
}

VIEWDELTA_TEST(GIVEN_entity_and_auth_component_in_view_WHEN_authority_lost_and_gained_THEN_authority_lost_temporarily)
{
	ViewDelta InputDelta;
	EntityView InputView;
	AddEntityToView(InputView, TestEntityId);
	AddComponentToView(InputView, TestEntityId, TestComponentId, TestComponentValue);
	AddAuthorityToView(InputView, TestEntityId, TestComponentId);

	EntityComponentOpListBuilder OpListBuilder;
	OpListBuilder.SetAuthority(TestEntityId, TestComponentId, WORKER_AUTHORITY_NOT_AUTHORITATIVE)
		.SetAuthority(TestEntityId, TestComponentId, WORKER_AUTHORITY_AUTHORITATIVE);
	SetFromOpList(InputDelta, InputView, OpListBuilder);

	EntityView ExpectedView;
	AddEntityToView(ExpectedView, TestEntityId);
	AddComponentToView(ExpectedView, TestEntityId, TestComponentId, TestComponentValue);
	AddAuthorityToView(ExpectedView, TestEntityId, TestComponentId);

	ExpectedViewDelta ExpectedDelta;
	ExpectedDelta.AddEntityDelta(TestEntityId, ExpectedViewDelta::UPDATE).AddAuthorityLostTemporarily(TestEntityId, TestComponentId);

	TestTrue("View Deltas are equal", ExpectedDelta.Compare(InputDelta));
	TestTrue("Views are equal", AreEquivalent(InputView, ExpectedView));

	return true;
}

VIEWDELTA_TEST(GIVEN_empty_view_WHEN_add_remove_THEN_get_empty_view_and_delta)
{
	ViewDelta InputDelta;
	EntityView InputView;

	EntityComponentOpListBuilder OpListBuilder;
	OpListBuilder.AddEntity(TestEntityId).RemoveEntity(TestEntityId);
	SetFromOpList(InputDelta, InputView, OpListBuilder);

	EntityView ExpectedView;
	ExpectedViewDelta ExpectedDelta;

	TestTrue("View Deltas are equal", ExpectedDelta.Compare(InputDelta));
	TestTrue("Views are equal", AreEquivalent(InputView, ExpectedView));

	return true;
}

VIEWDELTA_TEST(GIVEN_entity_and_component_in_view_WHEN_update_and_add_component_THEN_component_refresh)
{
	double EventValue = 25;
	double NewComponentValue = 30;

	ViewDelta InputDelta;
	EntityView InputView;
	AddEntityToView(InputView, TestEntityId);
	AddComponentToView(InputView, TestEntityId, TestComponentId, TestComponentValue);

	EntityComponentOpListBuilder OpListBuilder;
	OpListBuilder.UpdateComponent(TestEntityId, CreateTestComponentEvent(TestComponentId, EventValue))
		.AddComponent(TestEntityId, CreateTestComponentData(TestComponentId, NewComponentValue));
	SetFromOpList(InputDelta, InputView, OpListBuilder);

	EntityView ExpectedView;
	AddEntityToView(ExpectedView, TestEntityId);
	AddComponentToView(ExpectedView, TestEntityId, TestComponentId, NewComponentValue);

	ExpectedViewDelta ExpectedDelta;
	ExpectedDelta.AddEntityDelta(TestEntityId, ExpectedViewDelta::UPDATE)
		.AddComponentRefreshed(TestEntityId, CreateTestComponentEvent(TestComponentId, EventValue),
							   CreateTestComponentData(TestComponentId, NewComponentValue));

	TestTrue("View Deltas are equal", ExpectedDelta.Compare(InputDelta));
	TestTrue("Views are equal", AreEquivalent(InputView, ExpectedView));

	return true;
}

VIEWDELTA_TEST(GIVEN_entity_and_component_in_view_WHEN_remove_and_add_component_THEN_component_refresh)
{
	double NewComponentValue = 30;

	ViewDelta InputDelta;
	EntityView InputView;
	AddEntityToView(InputView, TestEntityId);
	AddComponentToView(InputView, TestEntityId, TestComponentId, TestComponentValue);

	EntityComponentOpListBuilder OpListBuilder;
	OpListBuilder.RemoveComponent(TestEntityId, TestComponentId)
		.AddComponent(TestEntityId, CreateTestComponentData(TestComponentId, NewComponentValue));
	SetFromOpList(InputDelta, InputView, OpListBuilder);

	EntityView ExpectedView;
	AddEntityToView(ExpectedView, TestEntityId);
	AddComponentToView(ExpectedView, TestEntityId, TestComponentId, NewComponentValue);

	ExpectedViewDelta ExpectedDelta;
	ExpectedDelta.AddEntityDelta(TestEntityId, ExpectedViewDelta::UPDATE)
		.AddComponentRefreshed(TestEntityId, ComponentUpdate(TestComponentId), CreateTestComponentData(TestComponentId, NewComponentValue));

	TestTrue("View Deltas are equal", ExpectedDelta.Compare(InputDelta));
	TestTrue("Views are equal", AreEquivalent(InputView, ExpectedView));

	return true;
}

VIEWDELTA_TEST(GIVEN_entity_view_WHEN_entity_remove_and_add_THEN_no_entity_flag)
{
	ViewDelta InputDelta;
	EntityView InputView;
	AddEntityToView(InputView, TestEntityId);

	EntityComponentOpListBuilder OpListBuilder;
	OpListBuilder.RemoveEntity(TestEntityId).AddEntity(TestEntityId);
	SetFromOpList(InputDelta, InputView, OpListBuilder);

	EntityView ExpectedView;
	AddEntityToView(ExpectedView, TestEntityId);

	ExpectedViewDelta ExpectedDelta;
	ExpectedDelta.AddEntityDelta(TestEntityId, ExpectedViewDelta::UPDATE);

	TestTrue("View Deltas are equal", ExpectedDelta.Compare(InputDelta));
	TestTrue("Views are equal", AreEquivalent(InputView, ExpectedView));

	return true;
}

VIEWDELTA_TEST(GIVEN_empty_view_WHEN_add_remove_add_THEN_entity_in_view_and_delta)
{
	ViewDelta InputDelta;
	EntityView InputView;

	EntityComponentOpListBuilder OpListBuilder;
	OpListBuilder.AddEntity(TestEntityId).RemoveEntity(TestEntityId).AddEntity(TestEntityId);
	SetFromOpList(InputDelta, InputView, OpListBuilder);

	EntityView ExpectedView;
	AddEntityToView(ExpectedView, TestEntityId);

	ExpectedViewDelta ExpectedDelta;
	ExpectedDelta.AddEntityDelta(TestEntityId, ExpectedViewDelta::EntityChangeType::ADD);

	TestTrue("View Deltas are equal", ExpectedDelta.Compare(InputDelta));
	TestTrue("Views are equal", AreEquivalent(InputView, ExpectedView));

	return true;
}

VIEWDELTA_TEST(GIVEN_empty_view_WHEN_add_entity_add_component_THEN_entity_and_component_in_view_and_delta)
{
	ViewDelta InputDelta;
	EntityView InputView;

	EntityComponentOpListBuilder OpListBuilder;
	OpListBuilder.AddEntity(TestEntityId).AddComponent(TestEntityId, CreateTestComponentData(TestComponentId, TestComponentValue));
	SetFromOpList(InputDelta, InputView, OpListBuilder);

	EntityView ExpectedView;
	AddEntityToView(ExpectedView, TestEntityId);
	AddComponentToView(ExpectedView, TestEntityId, TestComponentId, TestComponentValue);

	ExpectedViewDelta ExpectedDelta;
	ExpectedDelta.AddEntityDelta(TestEntityId, ExpectedViewDelta::EntityChangeType::ADD)
		.AddComponentAdded(TestEntityId, CreateTestComponentData(TestComponentId, TestComponentValue));

	TestTrue("View Deltas are equal", ExpectedDelta.Compare(InputDelta));
	TestTrue("Views are equal", AreEquivalent(InputView, ExpectedView));
	return true;
}

VIEWDELTA_TEST(GIVEN_entity_and_component_in_view_WHEN_remove_entity_THEN_empty_view_remove_ops_in_delta)
{
	ViewDelta InputDelta;
	EntityView InputView;
	AddEntityToView(InputView, TestEntityId);
	AddComponentToView(InputView, TestEntityId, TestComponentId, TestComponentValue);

	EntityComponentOpListBuilder OpListBuilder;
	OpListBuilder.RemoveEntity(TestEntityId);
	SetFromOpList(InputDelta, InputView, OpListBuilder);

	EntityView ExpectedView;

	ExpectedViewDelta ExpectedDelta;
	ExpectedDelta.AddEntityDelta(TestEntityId, ExpectedViewDelta::EntityChangeType::REMOVE)
		.AddComponentRemoved(TestEntityId, TestComponentId);

	TestTrue("View Deltas are equal", ExpectedDelta.Compare(InputDelta));
	TestTrue("Views are equal", AreEquivalent(InputView, ExpectedView));

	return true;
}
