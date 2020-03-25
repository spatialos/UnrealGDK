// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "ViewDelta.h"

#define VIEWDELTA_TEST(TestName) \
	GDK_TEST(Core, ViewDelta, TestName)

using namespace SpatialGDK; 

#pragma optimize("", off)

namespace
{
	//bool CompareOps(Worker_Op Op1, CreateEntityResponse Op2)
	bool CompareOps(const Worker_Op& Op1, const CreateEntityResponse& Op2)
	{
		FString Str = Op2.Message;
		if (Op1.op_type == WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE &&
			Op1.op.create_entity_response.request_id == Op2.RequestId &&
			Op1.op.create_entity_response.status_code == Op2.StatusCode &&
			//FString(Op1.op.create_entity_response.message) == Op2.Message &&
			Op1.op.create_entity_response.entity_id == Op2.EntityId)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
} // anonymous namespace

VIEWDELTA_TEST(GIVEN_ViewDelta_with_multiple_CreateEntityResponse_added_WHEN_GetCreateEntityResponse_called_THEN_multiple_CreateEntityResponses_returned)
{
	// GIVEN
	ViewDelta Delta;
	CreateEntityResponse Response{};
	Delta.AddCreateEntityResponse(Response);
	Delta.AddCreateEntityResponse(Response);

	// WHEN
	auto Responses = Delta.GetCreateEntityResponses();

	// THEN
	TestTrue("Multiple Responses returned", Responses.Num() > 1);
	return true;
}

VIEWDELTA_TEST(GIVEN_ViewDelta_with_multiple_CreateEntityResponse_added_WHEN_GenerateLegacyOpList_called_THEN_multiple_ops_returned)
{
	// GIVEN
	ViewDelta Delta;
	CreateEntityResponse Response{};
	Delta.AddCreateEntityResponse(Response);
	Delta.AddCreateEntityResponse(Response);

	// WHEN
	auto Responses = Delta.GenerateLegacyOpList();

	// THEN
	TestTrue("Multiple Responses returned", Responses->GetCount() > 1);
	return true;
}

VIEWDELTA_TEST(GIVEN_non_empty_ViewDelta_WHEN_Clear_called_THEN_GetCreateEntityResponse_returns_no_items)
{
	// GIVEN
	ViewDelta Delta;
	CreateEntityResponse Response{};
	Delta.AddCreateEntityResponse(Response);

	// WHEN
	Delta.Clear();

	// THEN
	auto Responses = Delta.GetCreateEntityResponses();
	TestTrue("No Responses returned", Responses.Num() == 0);

	return true;
}

VIEWDELTA_TEST(GIVEN_non_empty_ViewDelta_WHEN_Clear_called_THEN_GenerateLegacyOpList_returns_no_items)
{
	// GIVEN
	ViewDelta Delta;
	CreateEntityResponse Response{};
	Delta.AddCreateEntityResponse(Response);

	// WHEN
	Delta.Clear();

	// THEN
	auto Responses = Delta.GenerateLegacyOpList();
	TestTrue("No Responses returned", Responses->GetCount() == 0);

	return true;
}
