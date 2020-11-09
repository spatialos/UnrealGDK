// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/CrossServerRPCHandler.h"
#include "Interop/RPCExecutor.h"
#include "Tests/TestDefinitions.h"

#include "SpatialView/OpList/EntityComponentOpList.h"

#define CROSSSERVERRPCHANDLER_TEST(TestName) GDK_TEST(Core, CrossServerRPCHandler, TestName)

namespace SpatialGDK
{
const FString ExecutingCommand = TEXT("Executing Command");
const FString QueueingCommand = TEXT("Queueing Command");
const FString RPCInFlight = TEXT("RPC is already in flight.");
const float ShortTimeAdvanced = 0.1f;
const float LongTimeAdvanced = 1.f;
const Worker_EntityId TestEntityId = 1;
const Worker_RequestId SuccessRequestId = 1;
const Worker_RequestId QueueingRequestId = 2;

class ConnectionHandlerStub : public AbstractConnectionHandler
{
public:
	virtual void Advance() override {}

	virtual uint32 GetOpListCount() override { return 0; }

	virtual OpList GetNextOpList() override { return {}; }

	virtual void SendMessages(TUniquePtr<MessagesToSend> Messages) override {}

	virtual const FString& GetWorkerId() const override { return WorkerId; }

	virtual const TArray<FString>& GetWorkerAttributes() const override { return Attributes; }

private:
	TArray<TArray<OpList>> ListsOfOpLists;
	TArray<OpList> QueuedOpLists;
	FString WorkerId = TEXT("test_worker");
	TArray<FString> Attributes = { TEXT("test") };
};

class MockRPCExecutor : public RPCExecutorInterface
{
public:
	virtual FCrossServerRPCParams TryRetrieveCrossServerRPCParams(const Worker_Op& Op) override
	{
		return { FUnrealObjectRef(),
				 Op.op.command_request.request_id,
				 { 0, 0, static_cast<uint32>(Op.op.command_request.request_id), {} },
				 Op.span_id };
	}

	virtual bool ExecuteCommand(const FCrossServerRPCParams& Params) override
	{
		if (Params.RequestId == SuccessRequestId || Params.Timestamp + 0.5 < FDateTime::Now())
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

	ViewCoordinator Coordinator{ MakeUnique<ConnectionHandlerStub>(), nullptr };
	CrossServerRPCHandler Handler(Coordinator, MakeUnique<MockRPCExecutor>());
	EntityComponentOpListBuilder Builder;
	Builder.AddEntityCommandRequest(TestEntityId, SuccessRequestId, CreateCrossServerCommandRequest());
	const TArray<Worker_Op> Ops = MoveTemp(Builder).CreateOpArray();

	Handler.ProcessOps(ShortTimeAdvanced, Ops);
	const auto& QueuedRPCs = Handler.GetQueuedCrossServerRPCs();
	TestEqual("Number of queued up Cross Server RPCs", QueuedRPCs.Num(), 0);
	return true;
}

CROSSSERVERRPCHANDLER_TEST(GIVEN_rpc_WHEN_rpc_already_queued_THEN_discard)
{
	AddExpectedError(QueueingCommand, EAutomationExpectedErrorFlags::Exact);
	AddExpectedError(RPCInFlight, EAutomationExpectedErrorFlags::Exact);
	ViewCoordinator Coordinator{ MakeUnique<ConnectionHandlerStub>(), nullptr };
	CrossServerRPCHandler Handler(Coordinator, MakeUnique<MockRPCExecutor>());
	EntityComponentOpListBuilder Builder;
	Builder.AddEntityCommandRequest(TestEntityId, QueueingRequestId, CreateCrossServerCommandRequest());
	Handler.ProcessOps(ShortTimeAdvanced, MoveTemp(Builder).CreateOpArray());

	Builder = EntityComponentOpListBuilder();
	Builder.AddEntityCommandRequest(TestEntityId, QueueingRequestId, CreateCrossServerCommandRequest());
	Handler.ProcessOps(ShortTimeAdvanced, MoveTemp(Builder).CreateOpArray());
	const auto& QueuedRPCs = Handler.GetQueuedCrossServerRPCs();
	TestEqual("Number of queued up Cross Server RPCs", QueuedRPCs.Num(), 1);
	TestEqual("Number of queued up Cross Server RPCs", QueuedRPCs[TestEntityId].Num(), 1);
	return true;
}

CROSSSERVERRPCHANDLER_TEST(GIVEN_rpc_WHEN_resolved_and_queue_THEN_queue)
{
	AddExpectedError(QueueingCommand, EAutomationExpectedErrorFlags::Exact, 1);
	ViewCoordinator Coordinator{ MakeUnique<ConnectionHandlerStub>(), nullptr };
	CrossServerRPCHandler Handler(Coordinator, MakeUnique<MockRPCExecutor>());
	EntityComponentOpListBuilder Builder;
	Builder.AddEntityCommandRequest(TestEntityId, QueueingRequestId, CreateCrossServerCommandRequest());
	Handler.ProcessOps(ShortTimeAdvanced, MoveTemp(Builder).CreateOpArray());

	Builder = EntityComponentOpListBuilder();
	Builder.AddEntityCommandRequest(TestEntityId, SuccessRequestId, CreateCrossServerCommandRequest());
	Handler.ProcessOps(ShortTimeAdvanced, MoveTemp(Builder).CreateOpArray());
	const auto& QueuedRPCs = Handler.GetQueuedCrossServerRPCs();
	TestEqual("Number of queued up Cross Server RPCs", QueuedRPCs.Num(), 1);
	TestEqual("Number of queued up Cross Server RPCs", QueuedRPCs[TestEntityId].Num(), 2);
	return true;
}

CROSSSERVERRPCHANDLER_TEST(GIVEN_rpc_WHEN_unresolved_THEN_queue)
{
	AddExpectedError(QueueingCommand, EAutomationExpectedErrorFlags::Exact);
	ViewCoordinator Coordinator{ MakeUnique<ConnectionHandlerStub>(), nullptr };
	CrossServerRPCHandler Handler(Coordinator, MakeUnique<MockRPCExecutor>());
	EntityComponentOpListBuilder Builder;
	Builder.AddEntityCommandRequest(TestEntityId, QueueingRequestId, CreateCrossServerCommandRequest());
	Handler.ProcessOps(ShortTimeAdvanced, MoveTemp(Builder).CreateOpArray());
	const auto& QueuedRPCs = Handler.GetQueuedCrossServerRPCs();
	TestEqual("Number of queued up Cross Server RPCs", QueuedRPCs.Num(), 1);
	TestEqual("Number of queued up Cross Server RPCs", QueuedRPCs[TestEntityId].Num(), 1);
	return true;
}

CROSSSERVERRPCHANDLER_TEST(GIVEN_queued_rpc_WHEN_timeout_THEN_try_execute)
{
	AddExpectedError(QueueingCommand, EAutomationExpectedErrorFlags::Exact);
	AddExpectedError(ExecutingCommand, EAutomationExpectedErrorFlags::Exact);
	ViewCoordinator Coordinator{ MakeUnique<ConnectionHandlerStub>(), nullptr };
	CrossServerRPCHandler Handler(Coordinator, MakeUnique<MockRPCExecutor>());
	EntityComponentOpListBuilder Builder;
	Builder.AddEntityCommandRequest(TestEntityId, QueueingRequestId, CreateCrossServerCommandRequest());
	Handler.ProcessOps(ShortTimeAdvanced, MoveTemp(Builder).CreateOpArray());
	const auto& QueuedRPCs = Handler.GetQueuedCrossServerRPCs();
	TestEqual("Number of queued up Cross Server RPCs", QueuedRPCs.Num(), 1);
	TestEqual("Number of queued up Cross Server RPCs", QueuedRPCs[TestEntityId].Num(), 1);

	FPlatformProcess::Sleep(1.f);
	Handler.ProcessOps(LongTimeAdvanced, {});

	const auto& EmptyQueuedRPCs = Handler.GetQueuedCrossServerRPCs();
	TestEqual("Number of queued up Cross Server RPCs", EmptyQueuedRPCs.Num(), 0);
	return true;
}

} // namespace SpatialGDK
