// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "WorkerView.h"
#include "OpList/ViewDeltaLegacyOpList.h"

#define WORKERVIEW_TEST(TestName) \
	GDK_TEST(Core, WorkerView, TestName)

using namespace SpatialGDK; 

namespace
{
	Worker_Op CreateEmptyCreateEntityResponseOp()
	{
		Worker_Op Op{};
		Op.op_type = WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE;
		Op.op.create_entity_response = Worker_CreateEntityResponseOp{};
		return Op;
	}

} // anonymous namespace

WORKERVIEW_TEST(GIVEN_WorkerView_with_one_CreateEntityRequest_WHEN_FlushLocalChanges_called_THEN_one_CreateEntityRequest_returned)
{
	// GIVEN
	WorkerView View;
	CreateEntityRequest Request;
	View.SendCreateEntityRequest(Request);

	// WHEN
	auto Messages = View.FlushLocalChanges();

	// THEN
	TestTrue("WorkerView has one CreateEntityRequest", Messages->CreateEntityRequests.Num() == 1);

	return true;
}

WORKERVIEW_TEST(GIVEN_WorkerView_with_multiple_CreateEntityRequest_WHEN_FlushLocalChanges_called_THEN_mutliple_CreateEntityRequests_returned)
{
	// GIVEN
	WorkerView View;
	CreateEntityRequest Request;
	View.SendCreateEntityRequest(Request);
	View.SendCreateEntityRequest(Request);

	auto Messages = View.FlushLocalChanges();

	// THEN
	TestTrue("WorkerView has multiple CreateEntityRequest", Messages->CreateEntityRequests.Num() > 1);

	return true;
}

WORKERVIEW_TEST(GIVEN_WorkerView_with_one_op_enqued_WHEN_GenerateViewDelta_called_THEN_ViewDelta_with_one_op_returned)
{
	// GIVEN
	WorkerView View;
	// TODO(Alex): what type should it be
	TArray<Worker_Op> Ops;
	Ops.Push(CreateEmptyCreateEntityResponseOp());
	auto OpList = MakeUnique<ViewDeltaLegacyOpList>(Ops);
	View.EnqueueOpList(MoveTemp(OpList));

	// WHEN
	auto ViewDelta = View.GenerateViewDelta();

	// THEN
	TestTrue("ViewDelta has one op", ViewDelta->GenerateLegacyOpList()->GetCount() == 1);

	return true;
}

WORKERVIEW_TEST(GIVEN_WorkerView_with_multiple_ops_engued_WHEN_GenerateViewDelta_called_THEN_ViewDelta_with_multiple_ops_returned)
{
	// GIVEN
	WorkerView View;
	// TODO(Alex): what type should it be
	TArray<Worker_Op> Ops;
	Ops.Push(CreateEmptyCreateEntityResponseOp());
	Ops.Push(CreateEmptyCreateEntityResponseOp());
	auto OpList = MakeUnique<ViewDeltaLegacyOpList>(Ops);
	View.EnqueueOpList(MoveTemp(OpList));

	// WHEN
	auto ViewDelta = View.GenerateViewDelta();

	// THEN
	TestTrue("ViewDelta has multiple ops", ViewDelta->GenerateLegacyOpList()->GetCount() > 1);

	return true;
}
