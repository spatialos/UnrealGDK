// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/CrossServerRPCHandler.h"
#include "Interop/RPCExecutor.h"
#include "Tests/TestDefinitions.h"

#include "SpatialView/OpList/EntityComponentOpList.h"
#include "Tests/SpatialView/SpatialViewUtils.h"

#define CROSSSERVERRPCHANDLER_TEST(TestName) GDK_TEST(Core, CrossServerRPCHandler, TestName)

namespace SpatialGDK
{
namespace
{
const float AdvancedTime = 0.5f;
const float LongAdvancedTime = 5.5f;
const Worker_EntityId TestEntityId = 1;
const Worker_RequestId SuccessRequestId = 1;
const Worker_RequestId QueueingRequestId = 2;

const FComponentSetData ComponentSetData = {};
} // anonymous namespace

class MockConnectionHandler : public AbstractConnectionHandler
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

	virtual Worker_EntityId GetWorkerSystemEntityId() const override { return WorkerSystemEntityId; }

private:
	TArray<TArray<OpList>> ListsOfOpLists;
	TArray<OpList> QueuedOpLists;
	Worker_EntityId WorkerSystemEntityId = 1;
	FString WorkerId = TEXT("test_worker");
};

class MockRPCExecutor : public RPCExecutorInterface
{
private:
	bool bForceExecute = false;

public:
	void ForceExecute(bool bExecute) { bForceExecute = bExecute; }

	virtual TOptional<FCrossServerRPCParams> TryRetrieveCrossServerRPCParams(const Worker_Op& Op) override
	{
		return { { FUnrealObjectRef(),
				   Op.op.command_request.request_id,
				   { 0, 0, static_cast<uint32>(Op.op.command_request.request_id), {} },
				   {} } };
	}

	virtual bool ExecuteCommand(const FCrossServerRPCParams& Params) override
	{
		if (Params.RequestId == SuccessRequestId || bForceExecute)
		{
			return true;
		}

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
	TArray<TArray<OpList>> ListsOfOpLists;
	TUniquePtr<MockConnectionHandler> ConnHandler = MakeUnique<MockConnectionHandler>();

	TArray<OpList> OpLists;
	EntityComponentOpListBuilder Builder;
	Builder.AddEntityCommandRequest(TestEntityId, SuccessRequestId, CreateCrossServerCommandRequest());
	OpLists.Add(MoveTemp(Builder).CreateOpList());
	ListsOfOpLists.Add(MoveTemp(OpLists));
	ConnHandler->SetListsOfOpLists(MoveTemp(ListsOfOpLists));

	ViewCoordinator Coordinator(MoveTemp(ConnHandler), nullptr, ComponentSetData);
	CrossServerRPCHandler Handler(Coordinator, MakeUnique<MockRPCExecutor>());
	Coordinator.Advance(AdvancedTime);
	Handler.ProcessMessages(Coordinator.GetViewDelta().GetWorkerMessages(), AdvancedTime);
	const auto& QueuedRPCs = Handler.GetQueuedCrossServerRPCs();
	TestEqual("Number of queued up Cross Server RPCs", QueuedRPCs.Num(), 0);
	return true;
}

CROSSSERVERRPCHANDLER_TEST(GIVEN_rpc_WHEN_rpc_already_queued_THEN_discard)
{
	TArray<TArray<OpList>> ListsOfOpLists;
	TUniquePtr<MockConnectionHandler> ConnHandler = MakeUnique<MockConnectionHandler>();

	TArray<OpList> OpLists;
	EntityComponentOpListBuilder Builder;
	Builder.AddEntityCommandRequest(TestEntityId, QueueingRequestId, CreateCrossServerCommandRequest());
	OpLists.Add(MoveTemp(Builder).CreateOpList());
	ListsOfOpLists.Add(MoveTemp(OpLists));

	OpLists = TArray<OpList>();
	Builder = EntityComponentOpListBuilder();
	Builder.AddEntityCommandRequest(TestEntityId, QueueingRequestId, CreateCrossServerCommandRequest());
	OpLists.Add(MoveTemp(Builder).CreateOpList());

	ListsOfOpLists.Add(MoveTemp(OpLists));
	ConnHandler->SetListsOfOpLists(MoveTemp(ListsOfOpLists));
	ViewCoordinator Coordinator(MoveTemp(ConnHandler), nullptr, ComponentSetData);
	CrossServerRPCHandler Handler(Coordinator, MakeUnique<MockRPCExecutor>());
	Coordinator.Advance(AdvancedTime);
	Handler.ProcessMessages(Coordinator.GetViewDelta().GetWorkerMessages(), AdvancedTime);

	Coordinator.Advance(AdvancedTime);
	Handler.ProcessMessages(Coordinator.GetViewDelta().GetWorkerMessages(), AdvancedTime);
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
	TArray<TArray<OpList>> ListsOfOpLists;
	TUniquePtr<MockConnectionHandler> ConnHandler = MakeUnique<MockConnectionHandler>();

	TArray<OpList> OpLists;
	EntityComponentOpListBuilder Builder;
	Builder.AddEntityCommandRequest(TestEntityId, QueueingRequestId, CreateCrossServerCommandRequest());
	OpLists.Add(MoveTemp(Builder).CreateOpList());
	ListsOfOpLists.Add(MoveTemp(OpLists));

	Builder = EntityComponentOpListBuilder();
	Builder.AddEntityCommandRequest(TestEntityId, SuccessRequestId, CreateCrossServerCommandRequest());
	OpLists = TArray<OpList>();
	OpLists.Add(MoveTemp(Builder).CreateOpList());

	ListsOfOpLists.Add(MoveTemp(OpLists));
	ConnHandler->SetListsOfOpLists(MoveTemp(ListsOfOpLists));
	ViewCoordinator Coordinator((MoveTemp(ConnHandler)), nullptr, ComponentSetData);
	CrossServerRPCHandler Handler(Coordinator, MakeUnique<MockRPCExecutor>());
	Coordinator.Advance(AdvancedTime);
	Handler.ProcessMessages(Coordinator.GetViewDelta().GetWorkerMessages(), AdvancedTime);

	Coordinator.Advance(AdvancedTime);
	Handler.ProcessMessages(Coordinator.GetViewDelta().GetWorkerMessages(), AdvancedTime);
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
	TArray<TArray<OpList>> ListsOfOpLists;
	TUniquePtr<MockConnectionHandler> ConnHandler = MakeUnique<MockConnectionHandler>();

	TArray<OpList> OpLists;
	EntityComponentOpListBuilder Builder;
	Builder.AddEntityCommandRequest(TestEntityId, QueueingRequestId, CreateCrossServerCommandRequest());
	OpLists.Add(MoveTemp(Builder).CreateOpList());
	ListsOfOpLists.Add(MoveTemp(OpLists));

	ConnHandler->SetListsOfOpLists(MoveTemp(ListsOfOpLists));
	ViewCoordinator Coordinator((MoveTemp(ConnHandler)), nullptr, ComponentSetData);
	CrossServerRPCHandler Handler(Coordinator, MakeUnique<MockRPCExecutor>());
	Coordinator.Advance(AdvancedTime);
	Handler.ProcessMessages(Coordinator.GetViewDelta().GetWorkerMessages(), AdvancedTime);
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
	TArray<TArray<OpList>> ListsOfOpLists;
	TUniquePtr<MockConnectionHandler> ConnHandler = MakeUnique<MockConnectionHandler>();

	TArray<OpList> OpLists;
	EntityComponentOpListBuilder Builder;
	Builder.AddEntityCommandRequest(TestEntityId, QueueingRequestId, CreateCrossServerCommandRequest());
	OpLists.Add(MoveTemp(Builder).CreateOpList());
	ListsOfOpLists.Add(MoveTemp(OpLists));

	OpLists = TArray<OpList>();
	OpLists.Add(EntityComponentOpListBuilder().CreateOpList());
	ListsOfOpLists.Add(MoveTemp(OpLists));

	OpLists = TArray<OpList>();
	OpLists.Add(EntityComponentOpListBuilder().CreateOpList());
	ListsOfOpLists.Add(MoveTemp(OpLists));

	ConnHandler->SetListsOfOpLists(MoveTemp(ListsOfOpLists));
	ViewCoordinator Coordinator((MoveTemp(ConnHandler)), nullptr, ComponentSetData);
	TUniquePtr<MockRPCExecutor> Executor = MakeUnique<MockRPCExecutor>();
	MockRPCExecutor* ExecutorPtr = Executor.Get();
	CrossServerRPCHandler Handler(Coordinator, MoveTemp(Executor));

	Coordinator.Advance(AdvancedTime);
	Handler.ProcessMessages(Coordinator.GetViewDelta().GetWorkerMessages(), AdvancedTime);
	const auto& QueuedRPCs = Handler.GetQueuedCrossServerRPCs();
	TestEqual("Number of queued up Cross Server RPCs", QueuedRPCs.Num(), 1);
	TestEqual("Number of RPC Guids in flight", Handler.GetRPCGuidsInFlightCount(), 1);
	TestEqual("Number of RPC Guids to be deleted soon", Handler.GetRPCsToDeleteCount(), 0);
	if (!QueuedRPCs.Contains(TestEntityId))
	{
		TestTrue("TestEntityId not in queued up RPCs", false);
	}
	else
	{
		TestEqual("Number of queued up Cross Server RPCs", QueuedRPCs[TestEntityId].Num(), 1);
	}

	ExecutorPtr->ForceExecute(true);
	Coordinator.Advance(AdvancedTime);
	Handler.ProcessMessages(Coordinator.GetViewDelta().GetWorkerMessages(), AdvancedTime);
	const auto& EmptyQueuedRPCs = Handler.GetQueuedCrossServerRPCs();
	TestEqual("Number of queued up Cross Server RPCs", EmptyQueuedRPCs.Num(), 0);
	TestEqual("Number of RPC Guids in flight", Handler.GetRPCGuidsInFlightCount(), 0);
	TestEqual("Number of RPC Guids to be deleted soon", Handler.GetRPCsToDeleteCount(), 1);

	Coordinator.Advance(LongAdvancedTime);
	Handler.ProcessMessages(Coordinator.GetViewDelta().GetWorkerMessages(), LongAdvancedTime);
	TestEqual("Number of RPC Guids in flight", Handler.GetRPCGuidsInFlightCount(), 0);
	TestEqual("Number of RPC Guids to be deleted soon", Handler.GetRPCsToDeleteCount(), 0);

	return true;
}
} // namespace SpatialGDK
