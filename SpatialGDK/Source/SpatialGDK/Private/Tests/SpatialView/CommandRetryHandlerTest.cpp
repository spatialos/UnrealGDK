// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/Callbacks.h"
#include "SpatialView/CommandRetryHandlerImpl.h"
#include "SpatialView/ComponentData.h"
#include "Tests/SpatialView/ExpectedMessagesToSend.h"
#include "Tests/SpatialView/SpatialViewUtils.h"
#include "Tests/TestDefinitions.h"

using namespace SpatialGDK;

#define COMMANDRETRYHANDLER_TEST(TestName) GDK_TEST(Core, CommandRetryHandler, TestName)

const static Worker_EntityId TestEntityId = 1;
const static Worker_RequestId TestRequestId = 2;
const static Worker_RequestId RetryRequestId = -TestRequestId;
const static Worker_ComponentId TestComponentId = 3;
const static double TestComponentValue = 20;
const static Worker_CommandIndex TestCommandIndex = 4;
const static uint32 TestNumOfEntities = 10;
const static FString TimeOutMessage = TEXT("Time out");
const static FString AuthorityLostMessage = TEXT("Authority Lost");
const static FString SuccessMessage = TEXT("Success");
const static FString ApplicationErrorMessage = TEXT("Application Error");
const static float TimeAdvanced = 5.f;
static OpList EmptyOpList = { nullptr, 0, nullptr };

COMMANDRETRYHANDLER_TEST(GIVEN_success_WHEN_process_ops_THEN_no_retry)
{
	WorkerView View;
	TCommandRetryHandler<FCreateEntityRetryHandlerImpl> Handler;

	EntityComponentOpListBuilder Builder;
	Builder.AddCreateEntityCommandResponse(TestEntityId, TestRequestId, WORKER_STATUS_CODE_SUCCESS, SuccessMessage);
	OpList FirstOpList = MoveTemp(Builder).CreateOpList();
	TArray<ComponentData> EntityComponents;
	EntityComponents.Add(CreateTestComponentData(TestComponentId, TestComponentValue));
	Handler.SendRequest(1, { MoveTemp(EntityComponents), TestEntityId }, RETRY_UNTIL_COMPLETE, View);
	View.FlushLocalChanges();

	Handler.ProcessOps(TimeAdvanced, FirstOpList, View);
	View.FlushLocalChanges();

	Handler.ProcessOps(TimeAdvanced, EmptyOpList, View);

	TestTrue("MessagesToSend are equal", ExpectedMessagesToSend().Compare(View.FlushLocalChanges()));
	return true;
}

COMMANDRETRYHANDLER_TEST(GIVEN_time_out_WHEN_create_entity_THEN_retry)
{
	WorkerView View;
	TCommandRetryHandler<FCreateEntityRetryHandlerImpl> Handler;
	TArray<ComponentData> EntityComponents;
	EntityComponents.Add(CreateTestComponentData(TestComponentId, TestComponentValue));

	EntityComponentOpListBuilder Builder;
	Builder.AddCreateEntityCommandResponse(TestEntityId, TestRequestId, WORKER_STATUS_CODE_TIMEOUT, TimeOutMessage);
	OpList FirstOpList = MoveTemp(Builder).CreateOpList();

	Handler.SendRequest(TestRequestId, { MoveTemp(EntityComponents), TestEntityId }, RETRY_UNTIL_COMPLETE, View);
	View.FlushLocalChanges();

	Handler.ProcessOps(TimeAdvanced, FirstOpList, View);
	View.FlushLocalChanges();

	Handler.ProcessOps(TimeAdvanced, EmptyOpList, View);

	ExpectedMessagesToSend MessagesToSend;
	MessagesToSend.AddCreateEntityRequest(RetryRequestId, TestEntityId, TestComponentId, TestComponentValue);
	TestTrue("MessagesToSend are equal", MessagesToSend.Compare(View.FlushLocalChanges()));
	return true;
}

COMMANDRETRYHANDLER_TEST(GIVEN_time_out_WHEN_reserve_entity_ids_THEN_retry)
{
	WorkerView View;
	TCommandRetryHandler<FReserveEntityIdsRetryHandlerImpl> Handler;

	EntityComponentOpListBuilder Builder;
	Builder.AddReserveEntityIdsCommandResponse(TestEntityId, TestNumOfEntities, TestRequestId, WORKER_STATUS_CODE_TIMEOUT, TimeOutMessage);
	OpList FirstOpList = MoveTemp(Builder).CreateOpList();
	Handler.SendRequest(TestRequestId, TestNumOfEntities, RETRY_UNTIL_COMPLETE, View);
	View.FlushLocalChanges();

	Handler.ProcessOps(TimeAdvanced, FirstOpList, View);
	View.FlushLocalChanges();

	Handler.ProcessOps(TimeAdvanced, EmptyOpList, View);

	ExpectedMessagesToSend MessagesToSend;
	MessagesToSend.AddReserveEntityIdsRequest(RetryRequestId, TestNumOfEntities);
	TestTrue("MessagesToSend are equal", MessagesToSend.Compare(View.FlushLocalChanges()));
	return true;
}

COMMANDRETRYHANDLER_TEST(GIVEN_application_error_WHEN_reserve_entity_ids_THEN_no_retry)
{
	WorkerView View;
	TCommandRetryHandler<FReserveEntityIdsRetryHandlerImpl> Handler;

	EntityComponentOpListBuilder Builder;
	Builder.AddReserveEntityIdsCommandResponse(TestEntityId, TestNumOfEntities, TestRequestId, WORKER_STATUS_CODE_APPLICATION_ERROR,
											   ApplicationErrorMessage);
	OpList FirstOpList = MoveTemp(Builder).CreateOpList();
	Handler.SendRequest(TestRequestId, TestNumOfEntities, RETRY_UNTIL_COMPLETE, View);
	View.FlushLocalChanges();

	Handler.ProcessOps(TimeAdvanced, FirstOpList, View);
	View.FlushLocalChanges();

	Handler.ProcessOps(TimeAdvanced, EmptyOpList, View);

	TestTrue("MessagesToSend are equal", ExpectedMessagesToSend().Compare(View.FlushLocalChanges()));
	return true;
}

COMMANDRETRYHANDLER_TEST(GIVEN_time_out_WHEN_delete_entity_THEN_retry)
{
	WorkerView View;
	TCommandRetryHandler<FDeleteEntityRetryHandlerImpl> Handler;

	EntityComponentOpListBuilder Builder;
	Builder.AddDeleteEntityCommandResponse(TestEntityId, TestRequestId, WORKER_STATUS_CODE_TIMEOUT, TimeOutMessage);
	OpList FirstOpList = MoveTemp(Builder).CreateOpList();
	Handler.SendRequest(TestRequestId, TestEntityId, RETRY_UNTIL_COMPLETE, View);
	View.FlushLocalChanges();

	Handler.ProcessOps(TimeAdvanced, FirstOpList, View);
	View.FlushLocalChanges();

	Handler.ProcessOps(TimeAdvanced, EmptyOpList, View);

	ExpectedMessagesToSend MessagesToSend;
	MessagesToSend.AddDeleteEntityCommandRequest(RetryRequestId, TestEntityId);
	TestTrue("MessagesToSend are equal", MessagesToSend.Compare(View.FlushLocalChanges()));
	return true;
}

COMMANDRETRYHANDLER_TEST(GIVEN_time_out_WHEN_entity_command_request_THEN_retry)
{
	WorkerView View;
	TCommandRetryHandler<FEntityCommandRetryHandlerImpl> Handler;

	EntityComponentOpListBuilder Builder;
	Builder.AddEntityCommandResponse(TestEntityId, TestRequestId, WORKER_STATUS_CODE_TIMEOUT, TimeOutMessage);
	OpList FirstOpList = MoveTemp(Builder).CreateOpList();
	Handler.SendRequest(TestRequestId, { TestEntityId, CommandRequest(TestComponentId, TestCommandIndex) }, RETRY_UNTIL_COMPLETE, View);
	View.FlushLocalChanges();

	Handler.ProcessOps(TimeAdvanced, FirstOpList, View);
	View.FlushLocalChanges();

	Handler.ProcessOps(TimeAdvanced, EmptyOpList, View);

	ExpectedMessagesToSend MessagesToSend;
	MessagesToSend.AddEntityCommandRequest(RetryRequestId, TestEntityId);
	TestTrue("MessagesToSend are equal", MessagesToSend.Compare(View.FlushLocalChanges()));
	return true;
}

COMMANDRETRYHANDLER_TEST(GIVEN_authority_lost_WHEN_entity_command_request_THEN_retry)
{
	WorkerView View;
	TCommandRetryHandler<FEntityCommandRetryHandlerImpl> Handler;

	EntityComponentOpListBuilder Builder;
	Builder.AddEntityCommandResponse(TestEntityId, TestRequestId, WORKER_STATUS_CODE_AUTHORITY_LOST, AuthorityLostMessage);
	OpList FirstOpList = MoveTemp(Builder).CreateOpList();
	Handler.SendRequest(TestRequestId, { TestEntityId, CommandRequest(TestComponentId, TestCommandIndex) }, RETRY_UNTIL_COMPLETE, View);
	View.FlushLocalChanges();

	Handler.ProcessOps(TimeAdvanced, FirstOpList, View);
	View.FlushLocalChanges();

	Handler.ProcessOps(TimeAdvanced, EmptyOpList, View);

	ExpectedMessagesToSend MessagesToSend;
	MessagesToSend.AddEntityCommandRequest(RetryRequestId, TestEntityId);
	TestTrue("MessagesToSend are equal", MessagesToSend.Compare(View.FlushLocalChanges()));
	return true;
}
