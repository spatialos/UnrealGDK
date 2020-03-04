// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "WorkerView.h"

#define WORKERVIEW_TEST(TestName) \
	GDK_TEST(Core, WorkerView, TestName)

using namespace SpatialGDK; 

WORKERVIEW_TEST(GIVEN_empty_WorkerView_WHEN_GenerateViewDelta_called_THEN_ViewDelta_returned)
{
	WorkerView View;
	const ViewDelta* Delta = View.GenerateViewDelta();

	TestTrue("Valid ViewDelta returned", Delta != nullptr);
	return true;
}

WORKERVIEW_TEST(GIVEN_WorkerView_with_one_CreateEntityRequest_WHEN_FlushLocalChanges_called_THEN_one_MessagesToSend_returned)
{
	return true;
}

WORKERVIEW_TEST(GIVEN_WorkerView_with_multiple_CreateEntityRequest_WHEN_FlushLocalChanges_called_THEN_mutliple_MessagesToSend_returned)
{
	return true;
}

WORKERVIEW_TEST(GIVEN_WorkerView_with_one_op_enqued_WHEN_GenerateViewDelta_called_THEN_ViewDelta_with_one_op_returned)
{
	return true;
}

WORKERVIEW_TEST(GIVEN_WorkerView_with_multiple_ops_engued_WHEN_GenerateViewDelta_called_THEN_ViewDelta_with_multiple_ops_returned)
{
	return true;
}
