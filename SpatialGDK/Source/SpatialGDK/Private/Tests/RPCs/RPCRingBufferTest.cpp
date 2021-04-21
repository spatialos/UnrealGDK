// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "Interop/RPCs/RPCQueues.h"
#include "Interop/RPCs/RPC_RingBufferWithACK_Receiver.h"
#include "Interop/RPCs/RPC_RingBufferWithACK_Sender.h"

#define RPCRINGBUFFER_TEST(TestName) GDK_TEST(Core, RPCRingBuffer, TestName)

namespace RPCRingBufferTestPrivate
{
using namespace SpatialGDK;

struct Payload
{
	Payload() = default;
	Payload(uint32 Id)
		: Identifier(Id)
		, ValidRPC(true)
	{
	}

	uint32 Identifier;
	bool ValidRPC = false;
};

struct RingBufferComponent
{
	static constexpr int32 Capacity = 32;
	RingBufferComponent() { Buffer.SetNum(Capacity); }
	TArray<Payload> Buffer;
	uint32 CountWritten = 0;
};

struct ACKComponent
{
	uint32 ACKCount = 0;
};

struct EntityComponentMock
{
	TMap<Worker_EntityId_Key, RingBufferComponent> Buffers;
	TMap<Worker_EntityId_Key, ACKComponent> ACKs;
};

constexpr Worker_ComponentId BufferComponentId = 1;
constexpr Worker_ComponentId ACKComponentId = 2;

struct Serializer
{
public:
	enum LocalRole
	{
		Reader,
		Writer
	};

	Serializer(EntityComponentMock& InData, LocalRole InRole)
		: Data(InData)
		, Role(InRole)
	{
	}

	Worker_ComponentId GetComponentId() { return BufferComponentId; }

	Worker_ComponentId GetACKComponentId() { return ACKComponentId; }

	TOptional<uint64> ReadRPCCount(const RPCReadingContext& Ctx) { return GetBufferComponent(Ctx.EntityId).CountWritten; }

	TOptional<uint64> ReadACKCount(const RPCReadingContext& Ctx) { return GetACKComponent(Ctx.EntityId).ACKCount; }

	void ReadRPC(const RPCReadingContext& Ctx, uint32 Slot, Payload& OutPayload)
	{
		OutPayload = GetBufferComponent(Ctx.EntityId).Buffer[Slot];
	}

	void WriteRPC(RPCWritingContext::EntityWrite& Ctx, uint32 Slot, const Payload& InPayload)
	{
		check(Role == Writer);
		GetBufferComponent(Ctx.EntityId).Buffer[Slot] = InPayload;
	}

	void WriteRPCCount(RPCWritingContext::EntityWrite& Ctx, uint64 Count)
	{
		check(Role == Writer);
		GetBufferComponent(Ctx.EntityId).CountWritten = Count;
	}

	void WriteACKCount(RPCWritingContext::EntityWrite& Ctx, uint64 Count)
	{
		check(Role == Reader);
		GetACKComponent(Ctx.EntityId).ACKCount = Count;
	}

private:
	RingBufferComponent& GetBufferComponent(Worker_EntityId EntityId) { return Data.Buffers[EntityId]; }

	ACKComponent& GetACKComponent(Worker_EntityId EntityId) { return Data.ACKs[EntityId]; }

	EntityComponentMock& Data;
	LocalRole Role;
};

using Sender = MonotonicRingBufferWithACKSender<Payload, Serializer>;
using Receiver = MonotonicRingBufferWithACKReceiver<Payload, NullReceiveWrapper, Serializer>;
using UnboundedQueue = TRPCUnboundedQueue<Payload>;

struct RPCRingBufferTest_Fixture
{
	EntityComponentMock Data;

	Sender ServerWorker;
	Receiver ClientWorker;
	UnboundedQueue ServerQueue;

	RPCRingBufferTest_Fixture(uint32 BufferSize)
		: ServerWorker(Serializer(Data, Serializer::Writer), BufferSize)
		, ClientWorker(Serializer(Data, Serializer::Reader), BufferSize, NullReceiveWrapper<Payload>())
		, ServerQueue(FName(TEXT("DummyQueue")), ServerWorker)
	{
	}

	void AddEntity(Worker_EntityId EntityId)
	{
		Data.Buffers.Add(EntityId);
		Data.ACKs.Add(EntityId);
		EntityViewElement Element;
		Element.Components.Add(ComponentData(BufferComponentId));
		Element.Components.Add(ComponentData(ACKComponentId));
		ServerWorker.OnAuthGained(EntityId, Element);
		ServerQueue.OnAuthGained(EntityId, Element);
		ClientWorker.OnAdded(FName(TEXT("DummyName")), EntityId, Element);
	}
};

RPCRINGBUFFER_TEST(TestRingBufferRPCCapacityUpdate)
{
	const uint32 BufferSize = 1;
	RPCRingBufferTest_Fixture Fixture(BufferSize);

	const Worker_EntityId Entity = 1;

	Fixture.AddEntity(Entity);
	Fixture.ServerQueue.Push(Entity, Payload(1));

	bool bSentRPC = false;
	auto SentRPCCallback = [&bSentRPC](FName, Worker_EntityId, Worker_ComponentId, uint64, const RPCEmptyData&) {
		bSentRPC = true;
	};

	RPCWritingContext WritingCtx(Fixture.ServerQueue.Name, RPCCallbacks::UpdateWritten());
	Fixture.ServerQueue.FlushAll(WritingCtx, SentRPCCallback);

	TestTrue(TEXT("RPC Sent with available capacity"), bSentRPC);
	bSentRPC = false;

	Fixture.ServerQueue.FlushAll(WritingCtx, SentRPCCallback);
	TestFalse(TEXT("RPC Sent only once"), bSentRPC);

	auto CanExtractCallback = [](Worker_EntityId) {
		return true;
	};
	bool bExtractedRPC = false;
	auto ExtractCallback = [this, &bExtractedRPC](Worker_EntityId, const Payload& Data, const RPCEmptyData&) {
		bExtractedRPC = true;
		return true;
	};

	Fixture.ClientWorker.ExtractReceivedRPCs(CanExtractCallback, ExtractCallback);
	TestFalse(TEXT("No extraction done before update"), bExtractedRPC);

	RPCReadingContext BufferUpdateReadingCtx;
	BufferUpdateReadingCtx.EntityId = Entity;
	BufferUpdateReadingCtx.ComponentId = BufferComponentId;

	RPCReadingContext ACKUpdateReadingCtx;
	ACKUpdateReadingCtx.EntityId = Entity;
	ACKUpdateReadingCtx.ComponentId = ACKComponentId;

	Fixture.ClientWorker.OnUpdate(BufferUpdateReadingCtx);
	Fixture.ClientWorker.ExtractReceivedRPCs(CanExtractCallback, ExtractCallback);
	TestTrue(TEXT("Extracted RPC after update"), bExtractedRPC);
	bExtractedRPC = false;

	Fixture.ServerQueue.Push(Entity, Payload(1));

	Fixture.ServerQueue.FlushAll(WritingCtx, SentRPCCallback);
	TestFalse(TEXT("No RPC sent without capacity"), bSentRPC);

	Fixture.ClientWorker.OnUpdate(BufferUpdateReadingCtx);
	Fixture.ClientWorker.ExtractReceivedRPCs(CanExtractCallback, ExtractCallback);
	TestFalse(TEXT("No writes before ACK sent"), bExtractedRPC);

	Fixture.ClientWorker.FlushUpdates(WritingCtx);

	Fixture.ServerQueue.FlushAll(WritingCtx, SentRPCCallback);
	TestFalse(TEXT("No RPC sent without capacity"), bSentRPC);

	Fixture.ServerWorker.OnUpdate(ACKUpdateReadingCtx);
	Fixture.ServerQueue.FlushAll(WritingCtx, SentRPCCallback);
	TestTrue(TEXT("RPC sent after receiving ACK"), bSentRPC);

	Fixture.ClientWorker.OnUpdate(BufferUpdateReadingCtx);
	Fixture.ClientWorker.ExtractReceivedRPCs(CanExtractCallback, ExtractCallback);
	TestTrue(TEXT("Extracted second RPC after ACK"), bExtractedRPC);

	return true;
}

RPCRINGBUFFER_TEST(TestRingBufferPartialExtractionAndOverflow)
{
	const uint32 BufferSize = 4;
	RPCRingBufferTest_Fixture Fixture(BufferSize);

	const Worker_EntityId EntityForTest = 1;

	Fixture.AddEntity(EntityForTest);
	Fixture.AddEntity(EntityForTest + 1);

	bool bQueueOverflowed = false;
	auto QueueErrorCallback = [&bQueueOverflowed, EntityForTest, this](FName, Worker_EntityId Entity, QueueError Error) {
		TestEqual(TEXT("Test right entity is reported"), Entity, EntityForTest);
		TestEqual(TEXT("Test Queue error is overflow"), Error, QueueError::BufferOverflow);
		bQueueOverflowed = true;
	};
	Fixture.ServerQueue.SetErrorCallback(QueueErrorCallback);

	int32 RpcId = 1;

	for (uint32 i = 0; i < BufferSize; ++i)
	{
		Fixture.ServerQueue.Push(EntityForTest, Payload(RpcId++));
	}
	TestFalse(TEXT("Queue did not overflow 1"), bQueueOverflowed);

	RPCWritingContext WritingCtx(Fixture.ServerQueue.Name, RPCCallbacks::UpdateWritten());
	Fixture.ServerQueue.FlushAll(WritingCtx);

	auto CanExtractCallback = [](Worker_EntityId) {
		return true;
	};

	int32 ExpectedRpcId = 1;
	int32 Extracted = 0;
	int32 NumRPCToExtract = BufferSize / 2;
	auto ExtractCallback = [this, &NumRPCToExtract, &Extracted, &ExpectedRpcId, EntityForTest](Worker_EntityId Entity, const Payload& Data,
																							   const RPCEmptyData&) {
		if (Extracted >= NumRPCToExtract)
		{
			return false;
		}
		TestEqual(TEXT("Test right entity has received"), Entity, EntityForTest);
		TestTrue(TEXT("Received a valid RPC"), Data.ValidRPC);
		TestEqual(TEXT("Read RPC in the expected order"), Data.Identifier, ExpectedRpcId);
		++ExpectedRpcId;
		++Extracted;
		return true;
	};

	RPCReadingContext BufferUpdateReadingCtx;
	BufferUpdateReadingCtx.EntityId = EntityForTest;
	BufferUpdateReadingCtx.ComponentId = BufferComponentId;

	RPCReadingContext ACKUpdateReadingCtx;
	ACKUpdateReadingCtx.EntityId = EntityForTest;
	ACKUpdateReadingCtx.ComponentId = ACKComponentId;

	Fixture.ClientWorker.OnUpdate(BufferUpdateReadingCtx);
	Fixture.ClientWorker.ExtractReceivedRPCs(CanExtractCallback, ExtractCallback);
	TestEqual(TEXT("Partial extraction step 1 (1/2 buffer size)"), Extracted, BufferSize / 2);
	Extracted = 0;

	Fixture.ClientWorker.ExtractReceivedRPCs(CanExtractCallback, ExtractCallback);
	TestEqual(TEXT("Partial extraction step 2 (1 buffer size)"), Extracted, BufferSize / 2);
	Extracted = 0;

	Fixture.ClientWorker.FlushUpdates(WritingCtx);
	Fixture.ServerWorker.OnUpdate(ACKUpdateReadingCtx);
	for (uint32 i = 0; i < BufferSize; ++i)
	{
		Fixture.ServerQueue.Push(EntityForTest, Payload(RpcId++));
	}
	TestFalse(TEXT("Queue did not overflow 2"), bQueueOverflowed);
	Fixture.ServerQueue.FlushAll(WritingCtx);
	Fixture.ClientWorker.OnUpdate(BufferUpdateReadingCtx);
	Fixture.ClientWorker.ExtractReceivedRPCs(CanExtractCallback, ExtractCallback);
	TestEqual(TEXT("Partial extraction step 3 (1.5 buffer size)"), Extracted, BufferSize / 2);
	Extracted = 0;

	Fixture.ClientWorker.FlushUpdates(WritingCtx);
	Fixture.ServerWorker.OnUpdate(ACKUpdateReadingCtx);
	// Write 4 additional RPC, 2 will overflow
	for (uint32 i = 0; i < BufferSize; ++i)
	{
		Fixture.ServerQueue.Push(EntityForTest, Payload(RpcId++));
	}
	Fixture.ServerQueue.FlushAll(WritingCtx);
	TestTrue(TEXT("Queue did overflow"), bQueueOverflowed);
	bQueueOverflowed = false;

	NumRPCToExtract += BufferSize;
	Fixture.ClientWorker.OnUpdate(BufferUpdateReadingCtx);
	Fixture.ClientWorker.ExtractReceivedRPCs(CanExtractCallback, ExtractCallback);
	TestEqual(TEXT("Full extraction step 4 (2.5 buffer size)"), Extracted, BufferSize);
	Extracted = 0;

	Fixture.ClientWorker.FlushUpdates(WritingCtx);
	Fixture.ServerWorker.OnUpdate(ACKUpdateReadingCtx);

	Fixture.ServerQueue.FlushAll(WritingCtx);
	TestFalse(TEXT("Queue did not overflow 3"), bQueueOverflowed);

	Fixture.ClientWorker.OnUpdate(BufferUpdateReadingCtx);
	Fixture.ClientWorker.ExtractReceivedRPCs(CanExtractCallback, ExtractCallback);
	TestEqual(TEXT("Full extraction step 5 (3 buffer size)"), Extracted, BufferSize / 2);
	Extracted = 0;

	return true;
}

} // namespace RPCRingBufferTestPrivate
