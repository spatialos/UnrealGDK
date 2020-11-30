// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/CrossServerRPCHandler.h"
#include "Interop/RPCExecutor.h"
#include "Tests/TestDefinitions.h"

#include "SpatialView/OpList/EntityComponentOpList.h"
#include "Tests/SpatialView/SpatialViewUtils.h"

#define CROSSSERVERRPCHANDLER_TEST(TestName) GDK_TEST(Core, CrossServerRPCHandler, TestName)

namespace SpatialGDK
{
const FString ExecutingCommand = TEXT("Executing Command");
const FString QueueingCommand = TEXT("Queueing Command");
const FString RPCInFlight = TEXT("RPC is already in flight.");
const Worker_EntityId TestEntityId = 1;
const Worker_RequestId SuccessRequestId = 1;
const Worker_RequestId QueueingRequestId = 2;

class MockConnectionHandler : public AbstractConnectionHandler
{
public:
	virtual void Advance() override {}

	virtual uint32 GetOpListCount() override { return 0; }

	virtual OpList GetNextOpList() override { return {}; }

	virtual void SendMessages(TUniquePtr<MessagesToSend> Messages) override {}

	virtual const FString& GetWorkerId() const override { return WorkerId; }

	virtual Worker_EntityId GetWorkerSystemEntityId() const override { return WorkerSystemEntityId; }

private:
	Worker_EntityId WorkerSystemEntityId = 1;
	FString WorkerId = TEXT("test_worker");
};

class MockRPCExecutor : public RPCExecutorInterface
{
public:
	virtual FCrossServerRPCParams TryRetrieveCrossServerRPCParams(const Worker_Op& Op) override
	{
		return { FUnrealObjectRef(),
				 Op.op.command_request.request_id,
				 { 0, 0, static_cast<uint32>(Op.op.command_request.request_id), {} },
				 Op.op.command_request.timeout_millis,
				 {} };
	}

	virtual bool ExecuteCommand(const FCrossServerRPCParams& Params) override
	{
		FTimespan PassedTime(2500000);
		if (Params.RequestId == SuccessRequestId || Params.Timestamp + PassedTime < FDateTime::Now())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s"), *ExecutingCommand);
			return true;
		}

		UE_LOG(LogTemp, Warning, TEXT("%s"), *QueueingCommand);
		return false;
	}
};

CommandRequest CreateCrossServerCommandRequest()
{
	return CommandRequest(SpatialConstants::SERVER_TO_SERVER_COMMAND_ENDPOINT_COMPONENT_ID,
						  SpatialConstants::UNREAL_RPC_ENDPOINT_COMMAND_ID);
}

CROSSSERVERRPCHANDLER_TEST(GIVEN_rpc_WHEN_resolved_and_no_queue_THEN_execute)
{
	AddExpectedError(ExecutingCommand, EAutomationExpectedErrorFlags::Exact);

	ViewDelta Delta;
	EntityView View;
	ViewCoordinator Coordinator{ MakeUnique<MockConnectionHandler>(), nullptr };
	CrossServerRPCHandler Handler(Coordinator, MakeUnique<MockRPCExecutor>());
	EntityComponentOpListBuilder Builder;
	Builder.AddEntityCommandRequest(TestEntityId, SuccessRequestId, CreateCrossServerCommandRequest());
	SetFromOpList(Delta, View, MoveTemp(Builder));
	Handler.ProcessOps(Delta.GetWorkerMessages());
	const auto& QueuedRPCs = Handler.GetQueuedCrossServerRPCs();
	TestEqual("Number of queued up Cross Server RPCs", QueuedRPCs.Num(), 0);
	return true;
}

CROSSSERVERRPCHANDLER_TEST(GIVEN_rpc_WHEN_rpc_already_queued_THEN_discard)
{
	AddExpectedError(QueueingCommand, EAutomationExpectedErrorFlags::Exact, 3);
	AddExpectedError(RPCInFlight, EAutomationExpectedErrorFlags::Exact);

	ViewDelta Delta;
	EntityView View;
	ViewCoordinator Coordinator{ MakeUnique<MockConnectionHandler>(), nullptr };
	CrossServerRPCHandler Handler(Coordinator, MakeUnique<MockRPCExecutor>());
	EntityComponentOpListBuilder Builder;
	Builder.AddEntityCommandRequest(TestEntityId, QueueingRequestId, CreateCrossServerCommandRequest());
	SetFromOpList(Delta, View, MoveTemp(Builder));
	Handler.ProcessOps(Delta.GetWorkerMessages());

	Builder = EntityComponentOpListBuilder();
	Builder.AddEntityCommandRequest(TestEntityId, QueueingRequestId, CreateCrossServerCommandRequest());

	SetFromOpList(Delta, View, MoveTemp(Builder));
	Handler.ProcessOps(Delta.GetWorkerMessages());
	const auto& QueuedRPCs = Handler.GetQueuedCrossServerRPCs();
	TestEqual("Number of queued up Cross Server RPCs", QueuedRPCs.Num(), 1);
	if (!QueuedRPCs.Contains(TestEntityId))
	{
		TestTrue("TestEntityId not in queued up RPCs", false);
	}
	else
	{
		TestEqual("Number of queued up Cross Server RPCs", QueuedRPCs[TestEntityId].Num(), 1);
	}

	return true;
}

CROSSSERVERRPCHANDLER_TEST(GIVEN_rpc_WHEN_resolved_and_queue_THEN_queue)
{
	AddExpectedError(QueueingCommand, EAutomationExpectedErrorFlags::Exact, 3);

	ViewDelta Delta;
	EntityView View;
	ViewCoordinator Coordinator{ MakeUnique<MockConnectionHandler>(), nullptr };
	CrossServerRPCHandler Handler(Coordinator, MakeUnique<MockRPCExecutor>());
	EntityComponentOpListBuilder Builder;
	Builder.AddEntityCommandRequest(TestEntityId, QueueingRequestId, CreateCrossServerCommandRequest());
	SetFromOpList(Delta, View, MoveTemp(Builder));
	Handler.ProcessOps(Delta.GetWorkerMessages());

	Builder = EntityComponentOpListBuilder();
	Builder.AddEntityCommandRequest(TestEntityId, SuccessRequestId, CreateCrossServerCommandRequest());

	SetFromOpList(Delta, View, MoveTemp(Builder));
	Handler.ProcessOps(Delta.GetWorkerMessages());
	const auto& QueuedRPCs = Handler.GetQueuedCrossServerRPCs();
	TestEqual("Number of queued up Cross Server RPCs", QueuedRPCs.Num(), 1);
	if (!QueuedRPCs.Contains(TestEntityId))
	{
		TestTrue("TestEntityId not in queued up RPCs", false);
	}
	else
	{
		TestEqual("Number of queued up Cross Server RPCs", QueuedRPCs[TestEntityId].Num(), 2);
	}
	return true;
}

CROSSSERVERRPCHANDLER_TEST(GIVEN_rpc_WHEN_unresolved_THEN_queue)
{
	AddExpectedError(QueueingCommand, EAutomationExpectedErrorFlags::Exact, 2);

	ViewDelta Delta;
	EntityView View;
	ViewCoordinator Coordinator{ MakeUnique<MockConnectionHandler>(), nullptr };
	CrossServerRPCHandler Handler(Coordinator, MakeUnique<MockRPCExecutor>());
	EntityComponentOpListBuilder Builder;
	Builder.AddEntityCommandRequest(TestEntityId, QueueingRequestId, CreateCrossServerCommandRequest());

	SetFromOpList(Delta, View, MoveTemp(Builder));
	Handler.ProcessOps(Delta.GetWorkerMessages());
	const auto& QueuedRPCs = Handler.GetQueuedCrossServerRPCs();
	TestEqual("Number of queued up Cross Server RPCs", QueuedRPCs.Num(), 1);
	if (!QueuedRPCs.Contains(TestEntityId))
	{
		TestTrue("TestEntityId not in queued up RPCs", false);
	}
	else
	{
		TestEqual("Number of queued up Cross Server RPCs", QueuedRPCs[TestEntityId].Num(), 1);
	}

	return true;
}

CROSSSERVERRPCHANDLER_TEST(GIVEN_queued_rpc_WHEN_timeout_THEN_try_execute)
{
	AddExpectedError(QueueingCommand, EAutomationExpectedErrorFlags::Exact, 2);
	AddExpectedError(ExecutingCommand, EAutomationExpectedErrorFlags::Exact);

	ViewDelta Delta;
	EntityView View;
	ViewCoordinator Coordinator{ MakeUnique<MockConnectionHandler>(), nullptr };
	CrossServerRPCHandler Handler(Coordinator, MakeUnique<MockRPCExecutor>());
	EntityComponentOpListBuilder Builder;
	Builder.AddEntityCommandRequest(TestEntityId, QueueingRequestId, CreateCrossServerCommandRequest());

	SetFromOpList(Delta, View, MoveTemp(Builder));
	Handler.ProcessOps(Delta.GetWorkerMessages());
	const auto& QueuedRPCs = Handler.GetQueuedCrossServerRPCs();
	TestEqual("Number of queued up Cross Server RPCs", QueuedRPCs.Num(), 1);
	if (!QueuedRPCs.Contains(TestEntityId))
	{
		TestTrue("TestEntityId not in queued up RPCs", false);
	}
	else
	{
		TestEqual("Number of queued up Cross Server RPCs", QueuedRPCs[TestEntityId].Num(), 1);
	}

	FPlatformProcess::Sleep(.5f);
	Builder = EntityComponentOpListBuilder();
	Delta.Clear();
	SetFromOpList(Delta, View, MoveTemp(Builder));
	Handler.ProcessOps(Delta.GetWorkerMessages());

	const auto& EmptyQueuedRPCs = Handler.GetQueuedCrossServerRPCs();
	TestEqual("Number of queued up Cross Server RPCs", EmptyQueuedRPCs.Num(), 0);
	return true;
}

} // namespace SpatialGDK
