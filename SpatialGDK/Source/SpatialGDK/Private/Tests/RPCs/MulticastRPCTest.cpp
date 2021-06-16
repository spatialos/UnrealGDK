// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "Interop/RPCs/RPCQueues.h"
#include "Interop/RPCs/RPCService.h"
#include "Interop/RPCs/RPC_MulticastRingBuffer_Receiver.h"
#include "Interop/RPCs/RPC_MulticastRingBuffer_Sender.h"

#include "SpatialView/MessagesToSend.h"
#include "Tests/SpatialView/TestWorker.h"

#define RPCRINGBUFFER_TEST(TestName) GDK_TEST(Core, MulticastRPC, TestName)

namespace MulticastRPCTestPrivate
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

struct MulticastComponent
{
	static constexpr int32 Capacity = 32;
	MulticastComponent() { Buffer.SetNum(Capacity); }
	TArray<Payload> Buffer;
	uint32 CountWritten = 0;
	uint32 InitialCount = 0;
};

struct EntityComponentMock
{
	TMap<Worker_EntityId_Key, MulticastComponent> Buffers;
};

constexpr Worker_ComponentId BufferComponentId = 1;
constexpr Worker_ComponentId AuthComponentTag = 2;
constexpr Worker_ComponentSetId AuthComponentSet = 1001;

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

	TOptional<uint64> ReadRPCCount(const RPCReadingContext& Ctx) { return GetBufferComponent(Ctx.EntityId).CountWritten; }

	TOptional<uint32> ReadInitialRPCCount(const RPCReadingContext& Ctx) { return GetBufferComponent(Ctx.EntityId).InitialCount; }

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

	void WriteInitialRPCCount(RPCWritingContext::EntityWrite& Ctx, uint32 Count)
	{
		check(Role == Writer);
		GetBufferComponent(Ctx.EntityId).InitialCount = Count;
	}

private:
	MulticastComponent& GetBufferComponent(Worker_EntityId EntityId) { return Data.Buffers[EntityId]; }

	EntityComponentMock& Data;
	LocalRole Role;
};

using Sender = MulticastRingBufferSender<Payload, Serializer>;
using Receiver = MulticastRingBufferReceiver<Payload, NullReceiveWrapper, Serializer>;
using UnboundedQueue = TRPCUnboundedQueue<Payload>;

struct FWorker;
struct FRuntime
{
	TArray<FWorker*> Workers;
	TMap<Worker_EntityId, int32> Auth;

	void AddEntity(Worker_EntityId EntityId, int32 Auth);
	void AdvanceAll();
	void FlushAll();
};

struct FWorker
{
	int32 WorkerId;
	FTestWorker TestWorker;
	RPCService RPCs;
	Receiver RPCReceiver;

	EntityComponentMock Data;

	void SendMessages(FRuntime& Runtime, MessagesToSend& Messages) const
	{
		for (auto Worker : Runtime.Workers)
		{
			if (Worker != this)
			{
				EntityComponentMock& TargetData = Worker->Data;
				FTargetView& TargetView = Worker->TestWorker.GetTargetView();
				for (OutgoingComponentMessage& Message : Messages.ComponentMessages)
				{
					TargetView.UpdateComponent(Message.EntityId, ComponentUpdate(Message.ComponentId));
					const MulticastComponent& SrcComponent = Data.Buffers.FindChecked(Message.EntityId);
					MulticastComponent* TargetComponent = TargetData.Buffers.Find(Message.EntityId);
					if (TargetComponent)
					{
						*TargetComponent = SrcComponent;
					}
				}
			}
		}
	}

	FWorker(uint32 BufferSize, FRuntime& Runtime)
		: WorkerId(Runtime.Workers.Num())
		, TestWorker(FTestWorker::Create({ AuthComponentSet }, FString(""), WorkerId))
		, RPCs(TestWorker.GetCoordinator().CreateSubView(BufferComponentId, FSubView::NoFilter, FSubView::NoDispatcherCallbacks),
			   TestWorker.GetCoordinator().CreateSubView(AuthComponentTag, FSubView::NoFilter, FSubView::NoDispatcherCallbacks))
		, RPCReceiver(Serializer(Data, Serializer::Reader), BufferSize, NullReceiveWrapper<Payload>())
	{
		Runtime.Workers.Add(this);
		TestWorker.SetSendMessageCallback([this, &Runtime](TUniquePtr<MessagesToSend> Messages) {
			SendMessages(Runtime, *Messages);
		});

		RPCService::RPCReceiverDescription Desc;
		Desc.Authority = 0;
		Desc.Receiver = &RPCReceiver;
		RPCs.AddRPCReceiver("Multicast", MoveTemp(Desc));
	}

	void Advance()
	{
		TestWorker.AdvanceToTargetView(1.0);
		RPCs.AdvanceView();
	}

	virtual void FlushSystems()
	{
		RPCWritingContext Ctx("", [this](Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentUpdate*) {
			TestWorker.GetCoordinator().SendComponentUpdate(EntityId, ComponentUpdate(ComponentId));
		});
		RPCReceiver.FlushUpdates(Ctx);
	}

	void Flush()
	{
		FlushSystems();
		TestWorker.GetCoordinator().FlushMessagesToSend();
	}
};

struct FServerWorker : FWorker
{
	Sender RPCSender;
	UnboundedQueue RPCQueue;

	FServerWorker(uint32 BufferSize, FRuntime& Runtime)
		: FWorker(BufferSize, Runtime)
		, RPCSender(Serializer(Data, Serializer::Writer), BufferSize)
		, RPCQueue("Multicast", RPCSender)
	{
		RPCService::RPCQueueDescription Desc;
		Desc.Authority = AuthComponentSet;
		Desc.Sender = &RPCSender;
		Desc.Queue = &RPCQueue;
		RPCs.AddRPCQueue("Multicast", MoveTemp(Desc));
	}

	virtual void FlushSystems() override
	{
		FWorker::FlushSystems();
		{
			RPCWritingContext Ctx("", [this](Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentUpdate*) {
				TestWorker.GetCoordinator().SendComponentUpdate(EntityId, ComponentUpdate(ComponentId));
			});
			RPCQueue.FlushAll(Ctx);
		}
	}
};

void FRuntime::AddEntity(Worker_EntityId EntityId, int32 AuthServer)
{
	for (auto Worker : Workers)
	{
		EntityComponentMock& TargetData = Worker->Data;
		FTargetView& TargetView = Worker->TestWorker.GetTargetView();

		TargetData.Buffers.Add(EntityId);
		TargetView.AddEntity(EntityId);
		TargetView.AddOrSetComponent(EntityId, ComponentData(BufferComponentId));
		if (Worker->WorkerId == AuthServer)
		{
			TargetView.AddOrSetComponent(EntityId, ComponentData(AuthComponentTag));
			TargetView.AddAuthority(EntityId, AuthComponentSet);
			Auth.Add(EntityId, AuthServer);
		}
	}
}

void FRuntime::AdvanceAll()
{
	for (auto Worker : Workers)
	{
		Worker->Advance();
	}
}

void FRuntime::FlushAll()
{
	for (auto Worker : Workers)
	{
		Worker->Flush();
	}
}

struct MulticastTest_Fixture
{
	FRuntime SpatialOS;
	FServerWorker ServerWorker1;
	FServerWorker ServerWorker2;
	FWorker ClientWorker1;
	FWorker ClientWorker2;

	MulticastTest_Fixture(uint32 BufferSize)
		: ServerWorker1(BufferSize, SpatialOS)
		, ServerWorker2(BufferSize, SpatialOS)
		, ClientWorker1(BufferSize, SpatialOS)
		, ClientWorker2(BufferSize, SpatialOS)
	{
	}
};

RPCRINGBUFFER_TEST(MulticastTest)
{
	const uint32 BufferSize = 1;
	MulticastTest_Fixture Fixture(BufferSize);

	Worker_EntityId EntityForTest = 1;

	Fixture.SpatialOS.AddEntity(EntityForTest, Fixture.ServerWorker2.WorkerId);

	Fixture.SpatialOS.AdvanceAll();

	Fixture.ServerWorker2.RPCQueue.Push(1, Payload(4));
	Fixture.ServerWorker2.Flush();

	Fixture.SpatialOS.AdvanceAll();

	auto CanExtractCallback = [](Worker_EntityId) {
		return true;
	};

	int32 ExpectedRpcId = 4;
	int32 Extracted = 0;
	int32 NumRPCToExtract = BufferSize;
	auto ExtractCallback = [this, &NumRPCToExtract, &Extracted, &ExpectedRpcId, EntityForTest](Worker_EntityId Entity, const Payload& Data,
																							   const RPCEmptyData&) {
		if (Extracted >= NumRPCToExtract)
		{
			return false;
		}
		TestEqual(TEXT("Test right entity has received"), Entity, EntityForTest);
		TestTrue(TEXT("Received a valid RPC"), Data.ValidRPC);
		TestEqual(TEXT("Read RPC in the expected order"), Data.Identifier, ExpectedRpcId);
		//++ExpectedRpcId;
		++Extracted;
		return true;
	};

	Fixture.ClientWorker1.RPCReceiver.ExtractReceivedRPCs(CanExtractCallback, ExtractCallback);
	TestEqual(TEXT("Extraction"), Extracted, 1);
	Extracted = 0;

	Fixture.ClientWorker2.RPCReceiver.ExtractReceivedRPCs(CanExtractCallback, ExtractCallback);
	TestEqual(TEXT("Extraction"), Extracted, 1);
	Extracted = 0;

	Fixture.ServerWorker1.RPCReceiver.ExtractReceivedRPCs(CanExtractCallback, ExtractCallback);
	TestEqual(TEXT("Extraction"), Extracted, 1);
	Extracted = 0;

	Fixture.ClientWorker1.RPCReceiver.ExtractReceivedRPCs(CanExtractCallback, ExtractCallback);
	TestEqual(TEXT("Extraction"), Extracted, 0);
	Fixture.ClientWorker2.RPCReceiver.ExtractReceivedRPCs(CanExtractCallback, ExtractCallback);
	TestEqual(TEXT("Extraction"), Extracted, 0);
	Fixture.ServerWorker1.RPCReceiver.ExtractReceivedRPCs(CanExtractCallback, ExtractCallback);
	TestEqual(TEXT("Extraction"), Extracted, 0);

	return true;
}

} // namespace MulticastRPCTestPrivate
