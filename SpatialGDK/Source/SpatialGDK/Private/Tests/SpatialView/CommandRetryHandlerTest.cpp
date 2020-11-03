// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/Callbacks.h"
#include "SpatialView/CommandRetryHandlerImpl.h"
#include "SpatialView/ComponentData.h"
#include "Tests/SpatialView/ExpectedMessagesToSend.h"
#include "Tests/SpatialView/SpatialViewUtils.h"
#include "Tests/TestDefinitions.h"

using namespace SpatialGDK;

#define COMMANDRETRYHANDLER_TEST(TestName) GDK_TEST(Core, CommandRetryHandler, TestName)

namespace SpatialGDK
{
const Worker_EntityId TestEntityId = 1;
const Worker_RequestId TestRequestId = 2;
const Worker_RequestId RetryRequestId = -TestRequestId;
const Worker_ComponentId TestComponentId = 3;
const double TestComponentValue = 20;
const Worker_CommandIndex TestCommandIndex = 4;
const uint32 TestNumOfEntities = 10;
const float TimeAdvanced = 5.f;
OpList EmptyOpList = {};
} // namespace SpatialGDK

EntityQuery CreateTestEntityQuery()
{
	Worker_EntityQuery WorkerEntityQuery;
	WorkerEntityQuery.constraint.constraint_type = WORKER_CONSTRAINT_TYPE_ENTITY_ID;
	WorkerEntityQuery.constraint.constraint.entity_id_constraint = Worker_EntityIdConstraint{ TestEntityId };
	WorkerEntityQuery.result_type = WORKER_RESULT_TYPE_SNAPSHOT;
	WorkerEntityQuery.snapshot_result_type_component_id_count = 1;
	TArray<Worker_ComponentId> WorkerComponentIds = { TestComponentId };
	WorkerEntityQuery.snapshot_result_type_component_ids = WorkerComponentIds.GetData();
	return EntityQuery(WorkerEntityQuery);
}

COMMANDRETRYHANDLER_TEST(GIVEN_success_WHEN_process_ops_THEN_no_retry)
{
	WorkerView View;
	TCommandRetryHandler<FCreateEntityRetryHandlerImpl> Handler;

	EntityComponentOpListBuilder Builder;
	Builder.AddCreateEntityCommandResponse(TestEntityId, TestRequestId, WORKER_STATUS_CODE_SUCCESS, StringStorage("Success"));
	OpList FirstOpList = MoveTemp(Builder).CreateOpList();
	TArray<ComponentData> EntityComponents;
	EntityComponents.Add(CreateTestComponentData(TestComponentId, TestComponentValue));
	Handler.SendRequest(1, { MoveTemp(EntityComponents), TestEntityId }, RETRY_UNTIL_COMPLETE, View);
	View.FlushLocalChanges();

	Handler.ProcessOps(TimeAdvanced, FirstOpList, View);
	View.FlushLocalChanges();

	Handler.ProcessOps(TimeAdvanced, EmptyOpList, View);

	const TUniquePtr<MessagesToSend> ActualMessagesPtr = View.FlushLocalChanges();
	TestTrue("MessagesToSend are equal", ExpectedMessagesToSend().Compare(*ActualMessagesPtr.Get()));
	return true;
}

COMMANDRETRYHANDLER_TEST(GIVEN_time_out_WHEN_create_entity_THEN_retry)
{
	WorkerView View;
	TCommandRetryHandler<FCreateEntityRetryHandlerImpl> Handler;
	TArray<ComponentData> EntityComponents;
	EntityComponents.Add(CreateTestComponentData(TestComponentId, TestComponentValue));

	EntityComponentOpListBuilder Builder;
	Builder.AddCreateEntityCommandResponse(TestEntityId, TestRequestId, WORKER_STATUS_CODE_TIMEOUT, StringStorage("Time out"));
	OpList FirstOpList = MoveTemp(Builder).CreateOpList();

	Handler.SendRequest(TestRequestId, { MoveTemp(EntityComponents), TestEntityId }, RETRY_UNTIL_COMPLETE, View);
	View.FlushLocalChanges();

	Handler.ProcessOps(TimeAdvanced, FirstOpList, View);
	View.FlushLocalChanges();

	Handler.ProcessOps(TimeAdvanced, EmptyOpList, View);

	ExpectedMessagesToSend TestMessages;
	TArray<ComponentData> TestComponents;
	TestComponents.Add(CreateTestComponentData(TestComponentId, TestComponentValue));
	TestMessages.AddCreateEntityRequest(RetryRequestId, TestEntityId, MoveTemp(TestComponents));
	const TUniquePtr<MessagesToSend> ActualMessagesPtr = View.FlushLocalChanges();
	TestTrue("MessagesToSend are equal", TestMessages.Compare(*ActualMessagesPtr.Get()));
	return true;
}

COMMANDRETRYHANDLER_TEST(GIVEN_time_out_WHEN_reserve_entity_ids_THEN_retry)
{
	WorkerView View;
	TCommandRetryHandler<FReserveEntityIdsRetryHandlerImpl> Handler;

	EntityComponentOpListBuilder Builder;
	Builder.AddReserveEntityIdsCommandResponse(TestEntityId, TestNumOfEntities, TestRequestId, WORKER_STATUS_CODE_TIMEOUT,
											   StringStorage("Time out"));
	OpList FirstOpList = MoveTemp(Builder).CreateOpList();
	Handler.SendRequest(TestRequestId, TestNumOfEntities, RETRY_UNTIL_COMPLETE, View);
	View.FlushLocalChanges();

	Handler.ProcessOps(TimeAdvanced, FirstOpList, View);
	View.FlushLocalChanges();

	Handler.ProcessOps(TimeAdvanced, EmptyOpList, View);

	ExpectedMessagesToSend TestMessages;
	TestMessages.AddReserveEntityIdsRequest(RetryRequestId, TestNumOfEntities);
	const TUniquePtr<MessagesToSend> ActualMessagesPtr = View.FlushLocalChanges();
	TestTrue("MessagesToSend are equal", TestMessages.Compare(*ActualMessagesPtr.Get()));
	return true;
}

COMMANDRETRYHANDLER_TEST(GIVEN_application_error_WHEN_reserve_entity_ids_THEN_no_retry)
{
	WorkerView View;
	TCommandRetryHandler<FReserveEntityIdsRetryHandlerImpl> Handler;

	EntityComponentOpListBuilder Builder;
	Builder.AddReserveEntityIdsCommandResponse(TestEntityId, TestNumOfEntities, TestRequestId, WORKER_STATUS_CODE_APPLICATION_ERROR,
											   StringStorage("Application Error"));
	OpList FirstOpList = MoveTemp(Builder).CreateOpList();
	Handler.SendRequest(TestRequestId, TestNumOfEntities, RETRY_UNTIL_COMPLETE, View);
	View.FlushLocalChanges();

	Handler.ProcessOps(TimeAdvanced, FirstOpList, View);
	View.FlushLocalChanges();

	Handler.ProcessOps(TimeAdvanced, EmptyOpList, View);

	const TUniquePtr<MessagesToSend> ActualMessagesPtr = View.FlushLocalChanges();
	TestTrue("MessagesToSend are equal", ExpectedMessagesToSend().Compare(*ActualMessagesPtr.Get()));
	return true;
}

COMMANDRETRYHANDLER_TEST(GIVEN_time_out_WHEN_delete_entity_THEN_retry)
{
	WorkerView View;
	TCommandRetryHandler<FDeleteEntityRetryHandlerImpl> Handler;

	EntityComponentOpListBuilder Builder;
	Builder.AddDeleteEntityCommandResponse(TestEntityId, TestRequestId, WORKER_STATUS_CODE_TIMEOUT, StringStorage("Time out"));
	OpList FirstOpList = MoveTemp(Builder).CreateOpList();
	Handler.SendRequest(TestRequestId, { TestEntityId }, RETRY_UNTIL_COMPLETE, View);
	View.FlushLocalChanges();

	Handler.ProcessOps(TimeAdvanced, FirstOpList, View);
	View.FlushLocalChanges();

	Handler.ProcessOps(TimeAdvanced, EmptyOpList, View);

	ExpectedMessagesToSend TestMessages;
	TestMessages.AddDeleteEntityCommandRequest(RetryRequestId, TestEntityId);
	const TUniquePtr<MessagesToSend> ActualMessagesPtr = View.FlushLocalChanges();
	TestTrue("MessagesToSend are equal", TestMessages.Compare(*ActualMessagesPtr.Get()));
	return true;
}

COMMANDRETRYHANDLER_TEST(GIVEN_time_out_WHEN_query_entity_THEN_retry)
{
	WorkerView View;
	TCommandRetryHandler<FEntityQueryRetryHandlerImpl> Handler;

	EntityComponentOpListBuilder Builder;
	TArray<OpListEntity> Entities;
	OpListEntity Entity;
	Entity.EntityId = TestEntityId;
	Entity.Components.Add(CreateTestComponentData(TestComponentId, TestComponentValue));
	Entities.Add(MoveTemp(Entity));
	Builder.AddEntityQueryCommandResponse(TestRequestId, MoveTemp(Entities), WORKER_STATUS_CODE_TIMEOUT, StringStorage("Time out"));
	OpList FirstOpList = MoveTemp(Builder).CreateOpList();
	Handler.SendRequest(TestRequestId, CreateTestEntityQuery(), RETRY_UNTIL_COMPLETE, View);
	View.FlushLocalChanges();

	Handler.ProcessOps(TimeAdvanced, FirstOpList, View);
	View.FlushLocalChanges();

	Handler.ProcessOps(TimeAdvanced, EmptyOpList, View);

	ExpectedMessagesToSend TestMessages;
	TestMessages.AddEntityQueryRequest(RetryRequestId, CreateTestEntityQuery());
	const TUniquePtr<MessagesToSend> ActualMessagesPtr = View.FlushLocalChanges();
	TestTrue("MessagesToSend are equal", TestMessages.Compare(*ActualMessagesPtr.Get()));
	return true;
}

COMMANDRETRYHANDLER_TEST(GIVEN_time_out_WHEN_entity_command_request_THEN_retry)
{
	WorkerView View;
	TCommandRetryHandler<FEntityCommandRetryHandlerImpl> Handler;

	EntityComponentOpListBuilder Builder;
	Builder.AddEntityCommandResponse(TestEntityId, TestRequestId, WORKER_STATUS_CODE_TIMEOUT, StringStorage("Time out"));
	OpList FirstOpList = MoveTemp(Builder).CreateOpList();
	Handler.SendRequest(TestRequestId, { TestEntityId, CommandRequest(TestComponentId, TestCommandIndex) }, RETRY_UNTIL_COMPLETE, View);
	View.FlushLocalChanges();

	Handler.ProcessOps(TimeAdvanced, FirstOpList, View);
	View.FlushLocalChanges();

	Handler.ProcessOps(TimeAdvanced, EmptyOpList, View);

	ExpectedMessagesToSend TestMessages;
	TestMessages.AddEntityCommandRequest(RetryRequestId, TestEntityId, TestComponentId, TestCommandIndex);
	const TUniquePtr<MessagesToSend> ActualMessagesPtr = View.FlushLocalChanges();
	TestTrue("MessagesToSend are equal", TestMessages.Compare(*ActualMessagesPtr.Get()));
	return true;
}

COMMANDRETRYHANDLER_TEST(GIVEN_authority_lost_WHEN_entity_command_request_THEN_retry)
{
	WorkerView View;
	TCommandRetryHandler<FEntityCommandRetryHandlerImpl> Handler;

	EntityComponentOpListBuilder Builder;
	Builder.AddEntityCommandResponse(TestEntityId, TestRequestId, WORKER_STATUS_CODE_AUTHORITY_LOST, StringStorage("Authority Lost"));
	OpList FirstOpList = MoveTemp(Builder).CreateOpList();
	Handler.SendRequest(TestRequestId, { TestEntityId, CommandRequest(TestComponentId, TestCommandIndex) }, RETRY_UNTIL_COMPLETE, View);
	View.FlushLocalChanges();

	Handler.ProcessOps(TimeAdvanced, FirstOpList, View);

	ExpectedMessagesToSend TestMessages;
	TestMessages.AddEntityCommandRequest(RetryRequestId, TestEntityId, TestComponentId, TestCommandIndex);
	const TUniquePtr<MessagesToSend> ActualMessagesPtr = View.FlushLocalChanges();
	TestTrue("MessagesToSend are equal", TestMessages.Compare(*ActualMessagesPtr.Get()));
	return true;
}
