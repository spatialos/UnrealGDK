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
	uint64 Guid = 0;

public:
	void ForceExecute(bool bExecute) { bForceExecute = bExecute; }

	void SetGuid(int64 Id) { Guid = Id; }

	virtual TOptional<FCrossServerRPCParams> TryRetrieveCrossServerRPCParams(const Worker_Op& Op) override
	{
		return { { FUnrealObjectRef(), Op.op.command_request.request_id, { 0, 0, Guid, {} }, {} } };
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
	// Construct Op list
	TArray<TArray<OpList>> ListsOfOpLists;
	TUniquePtr<MockConnectionHandler> ConnHandler = MakeUnique<MockConnectionHandler>();

	TArray<OpList> OpLists;
	EntityComponentOpListBuilder Builder;
	Builder.AddEntityCommandRequest(TestEntityId, SuccessRequestId, CreateCrossServerCommandRequest());
	OpLists.Add(MoveTemp(Builder).CreateOpList());
	ListsOfOpLists.Add(MoveTemp(OpLists));
	ConnHandler->SetListsOfOpLists(MoveTemp(ListsOfOpLists));

	// No queue: Handler should be able to execute it
	ViewCoordinator Coordinator(MoveTemp(ConnHandler), nullptr, ComponentSetData);
	CrossServerRPCHandler Handler(Coordinator, MakeUnique<MockRPCExecutor>(), nullptr);
	Coordinator.Advance(AdvancedTime);
	Handler.ProcessMessages(Coordinator.GetViewDelta().GetWorkerMessages(), AdvancedTime);
	TestEqual("Number of queued up Cross Server RPCs", Handler.GetQueuedCrossServerRPCs().Num(), 0);
	TestEqual("Number of RPC Guids in flight", Handler.GetRPCGuidsInFlightCount(), 0);
	TestEqual("Number of RPC Guids to be deleted soon", Handler.GetRPCsToDeleteCount(), 1);
	return true;
}

CROSSSERVERRPCHANDLER_TEST(GIVEN_rpc_WHEN_rpc_already_queued_THEN_discard)
{
	// Construct Op Lists
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

	// Advance 1: RPC will be put into queue
	ListsOfOpLists.Add(MoveTemp(OpLists));
	ConnHandler->SetListsOfOpLists(MoveTemp(ListsOfOpLists));
	ViewCoordinator Coordinator(MoveTemp(ConnHandler), nullptr, ComponentSetData);
	CrossServerRPCHandler Handler(Coordinator, MakeUnique<MockRPCExecutor>(), nullptr);
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
		TestEqual("Number of RPC Guids in flight", Handler.GetRPCGuidsInFlightCount(), 1);
		TestEqual("Number of RPC Guids to be deleted soon", Handler.GetRPCsToDeleteCount(), 0);
	}

	// Advance 2: Same RPC is in op list again. Skip it
	Coordinator.Advance(AdvancedTime);
	Handler.ProcessMessages(Coordinator.GetViewDelta().GetWorkerMessages(), AdvancedTime);
	const auto& QueuedRPCs2 = Handler.GetQueuedCrossServerRPCs();
	TestEqual("Number of queued up Cross Server RPCs", QueuedRPCs2.Num(), 1);
	if (!QueuedRPCs2.Contains(TestEntityId))
	{
		TestTrue("TestEntityId not in queued up RPCs", false);
	}
	else
	{
		TestEqual("Number of queued up Cross Server RPCs", QueuedRPCs2[TestEntityId].Num(), 1);
		TestEqual("Number of RPC Guids in flight", Handler.GetRPCGuidsInFlightCount(), 1);
		TestEqual("Number of RPC Guids to be deleted soon", Handler.GetRPCsToDeleteCount(), 0);
	}

	return true;
}

CROSSSERVERRPCHANDLER_TEST(GIVEN_rpc_WHEN_resolved_and_queue_THEN_queue)
{
	// Construct Op list
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
	TUniquePtr<MockRPCExecutor> Executor = MakeUnique<MockRPCExecutor>();
	MockRPCExecutor* ExecutorPtr = Executor.Get();
	ViewCoordinator Coordinator((MoveTemp(ConnHandler)), nullptr, ComponentSetData);
	CrossServerRPCHandler Handler(Coordinator, MoveTemp(Executor), nullptr);

	// Advance 1: RPC can't be processed. Queue it
	Coordinator.Advance(AdvancedTime);
	Handler.ProcessMessages(Coordinator.GetViewDelta().GetWorkerMessages(), AdvancedTime);

	// Advance 2: RPC can be processed, but there is a queue. Queue it
	ExecutorPtr->SetGuid(1);
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
		TestEqual("Number of RPC Guids in flight", Handler.GetRPCGuidsInFlightCount(), 2);
		TestEqual("Number of RPC Guids to be deleted soon", Handler.GetRPCsToDeleteCount(), 0);
	}
	return true;
}

CROSSSERVERRPCHANDLER_TEST(GIVEN_rpc_WHEN_unresolved_THEN_queue)
{
	// Construct Op list
	TArray<TArray<OpList>> ListsOfOpLists;
	TUniquePtr<MockConnectionHandler> ConnHandler = MakeUnique<MockConnectionHandler>();

	TArray<OpList> OpLists;
	EntityComponentOpListBuilder Builder;
	Builder.AddEntityCommandRequest(TestEntityId, QueueingRequestId, CreateCrossServerCommandRequest());
	OpLists.Add(MoveTemp(Builder).CreateOpList());
	ListsOfOpLists.Add(MoveTemp(OpLists));

	ConnHandler->SetListsOfOpLists(MoveTemp(ListsOfOpLists));
	ViewCoordinator Coordinator((MoveTemp(ConnHandler)), nullptr, ComponentSetData);
	CrossServerRPCHandler Handler(Coordinator, MakeUnique<MockRPCExecutor>(), nullptr);

	// Advance 1: RPC is unresolved. Queue it
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
		TestEqual("Number of RPC Guids in flight", Handler.GetRPCGuidsInFlightCount(), 1);
		TestEqual("Number of RPC Guids to be deleted soon", Handler.GetRPCsToDeleteCount(), 0);
	}

	return true;
}

CROSSSERVERRPCHANDLER_TEST(GIVEN_queued_rpc_WHEN_timeout_THEN_try_execute)
{
	// Construct Op list
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
	CrossServerRPCHandler Handler(Coordinator, MoveTemp(Executor), nullptr);

	// Advance 1: Unresolved RPC. Queue it
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
		TestEqual("Number of RPC Guids in flight", Handler.GetRPCGuidsInFlightCount(), 1);
		TestEqual("Number of RPC Guids to be deleted soon", Handler.GetRPCsToDeleteCount(), 0);
	}

	// Advance 2: Queued RPC will be executed. Guid should still be kept for a while
	ExecutorPtr->ForceExecute(true);
	Coordinator.Advance(AdvancedTime);
	Handler.ProcessMessages(Coordinator.GetViewDelta().GetWorkerMessages(), AdvancedTime);
	const auto& EmptyQueuedRPCs = Handler.GetQueuedCrossServerRPCs();
	TestEqual("Number of queued up Cross Server RPCs", EmptyQueuedRPCs.Num(), 0);
	TestEqual("Number of RPC Guids in flight", Handler.GetRPCGuidsInFlightCount(), 0);
	TestEqual("Number of RPC Guids to be deleted soon", Handler.GetRPCsToDeleteCount(), 1);

	// Advance 3: Guid will be removed
	Coordinator.Advance(LongAdvancedTime);
	Handler.ProcessMessages(Coordinator.GetViewDelta().GetWorkerMessages(), LongAdvancedTime);
	TestEqual("Number of RPC Guids in flight", Handler.GetRPCGuidsInFlightCount(), 0);
	TestEqual("Number of RPC Guids to be deleted soon", Handler.GetRPCsToDeleteCount(), 0);

	return true;
}

CROSSSERVERRPCHANDLER_TEST(GIVEN_same_rpc_WHEN_rpc_just_executed_THEN_skip)
{
	// Construct Op list
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

	Builder = EntityComponentOpListBuilder();
	Builder.AddEntityCommandRequest(TestEntityId, QueueingRequestId, CreateCrossServerCommandRequest());
	OpLists = TArray<OpList>();
	OpLists.Add(MoveTemp(Builder).CreateOpList());
	ListsOfOpLists.Add(MoveTemp(OpLists));

	ConnHandler->SetListsOfOpLists(MoveTemp(ListsOfOpLists));
	ViewCoordinator Coordinator((MoveTemp(ConnHandler)), nullptr, ComponentSetData);
	TUniquePtr<MockRPCExecutor> Executor = MakeUnique<MockRPCExecutor>();
	MockRPCExecutor* ExecutorPtr = Executor.Get();
	CrossServerRPCHandler Handler(Coordinator, MoveTemp(Executor), nullptr);

	// Advance 1: Unresolved RPC. Queue it
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
		TestEqual("Number of RPC Guids in flight", Handler.GetRPCGuidsInFlightCount(), 1);
		TestEqual("Number of RPC Guids to be deleted soon", Handler.GetRPCsToDeleteCount(), 0);
	}

	// Advance 2: Queued RPC gets executed
	ExecutorPtr->ForceExecute(true);
	Coordinator.Advance(AdvancedTime);
	Handler.ProcessMessages(Coordinator.GetViewDelta().GetWorkerMessages(), AdvancedTime);
	TestEqual("Number of queued up Cross Server RPCs", Handler.GetQueuedCrossServerRPCs().Num(), 0);
	TestEqual("Number of RPC Guids in flight", Handler.GetRPCGuidsInFlightCount(), 0);
	TestEqual("Number of RPC Guids to be deleted soon", Handler.GetRPCsToDeleteCount(), 1);

	// Advance 3: Same RPC got sent again. Skip it, because we just executed it.
	ExecutorPtr->ForceExecute(false);
	ExecutorPtr->SetGuid(0);
	Coordinator.Advance(AdvancedTime);
	Handler.ProcessMessages(Coordinator.GetViewDelta().GetWorkerMessages(), AdvancedTime);
	TestEqual("Number of queued up Cross Server RPCs", Handler.GetQueuedCrossServerRPCs().Num(), 0);
	TestEqual("Number of RPC Guids in flight", Handler.GetRPCGuidsInFlightCount(), 0);
	TestEqual("Number of RPC Guids to be deleted soon", Handler.GetRPCsToDeleteCount(), 1);

	return true;
}
} // namespace SpatialGDK
