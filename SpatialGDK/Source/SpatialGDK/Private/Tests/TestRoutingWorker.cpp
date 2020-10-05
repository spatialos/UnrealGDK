// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CoreMinimal.h"

// Engine
#include "Engine/Engine.h"
#include "GameFramework/GameStateBase.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/TestActor.h"
#include "Tests/TestDefinitions.h"
#include "Tests/TestingComponentViewHelpers.h"

// GDK
#include "Interop/Connection/SpatialOSWorkerInterface.h"
#include "Interop/SpatialRPCService.h"
#include "Interop/SpatialRoutingSystem.h"
#include "Interop/SpatialStaticComponentView.h"
#include "Schema/CrossServerEndpoint.h"
#include "SpatialView/OpList/EntityComponentOpList.h"
#include "SpatialView/ViewCoordinator.h"

#include "improbable/c_schema.h"

class SpatialOSWorkerConnectionSpy : public SpatialOSWorkerInterface
{
public:
	SpatialOSWorkerConnectionSpy();

	virtual const TArray<SpatialGDK::EntityDelta>& GetEntityDeltas() override;
	virtual const TArray<Worker_Op>& GetWorkerMessages() override;
	virtual Worker_RequestId SendReserveEntityIdsRequest(uint32_t NumOfEntities) override;
	virtual Worker_RequestId SendCreateEntityRequest(TArray<FWorkerComponentData> Components, const Worker_EntityId* EntityId) override;
	virtual Worker_RequestId SendDeleteEntityRequest(Worker_EntityId EntityId) override;
	virtual void SendAddComponent(Worker_EntityId EntityId, FWorkerComponentData* ComponentData) override;
	virtual void SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId) override;
	virtual void SendComponentUpdate(Worker_EntityId EntityId, FWorkerComponentUpdate* ComponentUpdate) override;
	virtual Worker_RequestId SendCommandRequest(Worker_EntityId EntityId, Worker_CommandRequest* Request, uint32_t CommandId) override;
	virtual void SendCommandResponse(Worker_RequestId RequestId, Worker_CommandResponse* Response) override;
	virtual void SendCommandFailure(Worker_RequestId RequestId, const FString& Message) override;
	virtual void SendLogMessage(uint8_t Level, const FName& LoggerName, const TCHAR* Message) override;
	virtual void SendComponentInterest(Worker_EntityId EntityId, TArray<Worker_InterestOverride>&& ComponentInterest) override;
	virtual Worker_RequestId SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery) override;
	virtual void SendMetrics(SpatialGDK::SpatialMetrics Metrics) override;

	// The following methods are used to query for state in tests.
	const Worker_EntityQuery* GetLastEntityQuery();
	void ClearLastEntityQuery();

	Worker_RequestId GetLastRequestId();

	USpatialStaticComponentView* ComponentView;
	TSet<TPair<Worker_EntityId_Key, Worker_ComponentId>> Updates;

private:
	Worker_RequestId NextRequestId;

	const Worker_EntityQuery* LastEntityQuery;

	TArray<SpatialGDK::EntityDelta> PlaceholderEntityDeltas;
	TArray<Worker_Op> PlaceholderWorkerMessages;
};

SpatialOSWorkerConnectionSpy::SpatialOSWorkerConnectionSpy()
	: NextRequestId(0)
	, LastEntityQuery(nullptr)
{
	ComponentView = NewObject<USpatialStaticComponentView>();
}

const TArray<SpatialGDK::EntityDelta>& SpatialOSWorkerConnectionSpy::GetEntityDeltas()
{
	return PlaceholderEntityDeltas;
}

const TArray<Worker_Op>& SpatialOSWorkerConnectionSpy::GetWorkerMessages()
{
	return PlaceholderWorkerMessages;
}

Worker_RequestId SpatialOSWorkerConnectionSpy::SendReserveEntityIdsRequest(uint32_t NumOfEntities)
{
	return NextRequestId++;
}

Worker_RequestId SpatialOSWorkerConnectionSpy::SendCreateEntityRequest(TArray<FWorkerComponentData> Components,
																	   const Worker_EntityId* EntityId)
{
	return NextRequestId++;
}

Worker_RequestId SpatialOSWorkerConnectionSpy::SendDeleteEntityRequest(Worker_EntityId EntityId)
{
	ComponentView->OnRemoveEntity(EntityId);
	return NextRequestId++;
}

void SpatialOSWorkerConnectionSpy::SendAddComponent(Worker_EntityId EntityId, FWorkerComponentData* ComponentData) {}

void SpatialOSWorkerConnectionSpy::SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	Worker_RemoveComponentOp Op;
	Op.entity_id = EntityId;
	Op.component_id = ComponentId;
	ComponentView->OnRemoveComponent(Op);
}

void SpatialOSWorkerConnectionSpy::SendComponentUpdate(Worker_EntityId EntityId, FWorkerComponentUpdate* ComponentUpdate)
{
	Worker_ComponentUpdateOp Update;
	Update.entity_id = EntityId;
	Update.update.component_id = ComponentUpdate->component_id;
	Update.update.schema_type = ComponentUpdate->schema_type;
	ComponentView->OnComponentUpdate(Update);

	Updates.Add(MakeTuple(EntityId, ComponentUpdate->component_id));
}

Worker_RequestId SpatialOSWorkerConnectionSpy::SendCommandRequest(Worker_EntityId EntityId, Worker_CommandRequest* Request,
																  uint32_t CommandId)
{
	return NextRequestId++;
}

void SpatialOSWorkerConnectionSpy::SendCommandResponse(Worker_RequestId RequestId, Worker_CommandResponse* Response) {}

void SpatialOSWorkerConnectionSpy::SendCommandFailure(Worker_RequestId RequestId, const FString& Message) {}

void SpatialOSWorkerConnectionSpy::SendLogMessage(uint8_t Level, const FName& LoggerName, const TCHAR* Message) {}

void SpatialOSWorkerConnectionSpy::SendComponentInterest(Worker_EntityId EntityId, TArray<Worker_InterestOverride>&& ComponentInterest) {}

Worker_RequestId SpatialOSWorkerConnectionSpy::SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery)
{
	LastEntityQuery = EntityQuery;
	return NextRequestId++;
}

void SpatialOSWorkerConnectionSpy::SendMetrics(SpatialGDK::SpatialMetrics Metrics) {}

const Worker_EntityQuery* SpatialOSWorkerConnectionSpy::GetLastEntityQuery()
{
	return LastEntityQuery;
}

void SpatialOSWorkerConnectionSpy::ClearLastEntityQuery()
{
	LastEntityQuery = nullptr;
}

Worker_RequestId SpatialOSWorkerConnectionSpy::GetLastRequestId()
{
	return NextRequestId - 1;
}

#define ROUTING_SERVICE_TEST(TestName) GDK_TEST(Core, SpatialRPCService, TestName)
namespace SpatialGDK
{
class ConnectionHandlerStub : public AbstractConnectionHandler
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

	virtual const TArray<FString>& GetWorkerAttributes() const override { return Attributes; }

private:
	TArray<TArray<OpList>> ListsOfOpLists;
	TArray<OpList> QueuedOpLists;
	FString WorkerId = TEXT("test_worker");
	TArray<FString> Attributes = { TEXT("test") };
};

void AddEntityAndCrossServerComponents(EntityComponentOpListBuilder& Builder, Worker_EntityId Id)
{
	Builder.AddEntity(Id);
	Builder.AddComponent(Id, ComponentData{ SpatialConstants::CROSSSERVER_SENDER_ENDPOINT_COMPONENT_ID });
	Builder.AddComponent(Id, ComponentData{ SpatialConstants::CROSSSERVER_SENDER_ACK_ENDPOINT_COMPONENT_ID });
	Builder.AddComponent(Id, ComponentData{ SpatialConstants::CROSSSERVER_RECEIVER_ENDPOINT_COMPONENT_ID });
	Builder.AddComponent(Id, ComponentData{ SpatialConstants::CROSSSERVER_RECEIVER_ACK_ENDPOINT_COMPONENT_ID });
	Builder.AddComponent(Id, ComponentData{ SpatialConstants::ROUTINGWORKER_TAG_COMPONENT_ID });

	Builder.SetAuthority(Id, SpatialConstants::CROSSSERVER_SENDER_ENDPOINT_COMPONENT_ID, WORKER_AUTHORITY_NOT_AUTHORITATIVE);
	Builder.SetAuthority(Id, SpatialConstants::CROSSSERVER_SENDER_ACK_ENDPOINT_COMPONENT_ID, WORKER_AUTHORITY_AUTHORITATIVE);
	Builder.SetAuthority(Id, SpatialConstants::CROSSSERVER_RECEIVER_ENDPOINT_COMPONENT_ID, WORKER_AUTHORITY_AUTHORITATIVE);
	Builder.SetAuthority(Id, SpatialConstants::CROSSSERVER_RECEIVER_ACK_ENDPOINT_COMPONENT_ID, WORKER_AUTHORITY_NOT_AUTHORITATIVE);
	Builder.SetAuthority(Id, SpatialConstants::ROUTINGWORKER_TAG_COMPONENT_ID, WORKER_AUTHORITY_NOT_AUTHORITATIVE);
}

void AddEntityAndCrossServerComponents(USpatialStaticComponentView& View, Worker_EntityId Id)
{
	Worker_AddComponentOp Op;
	Op.entity_id = Id;
	Op.data.schema_type = Schema_CreateComponentData();

	Op.data.component_id = SpatialConstants::CROSSSERVER_SENDER_ENDPOINT_COMPONENT_ID;
	View.OnAddComponent(Op);

	Op.data.component_id = SpatialConstants::CROSSSERVER_SENDER_ACK_ENDPOINT_COMPONENT_ID;
	View.OnAddComponent(Op);

	Op.data.component_id = SpatialConstants::CROSSSERVER_RECEIVER_ENDPOINT_COMPONENT_ID;
	View.OnAddComponent(Op);

	Op.data.component_id = SpatialConstants::CROSSSERVER_RECEIVER_ACK_ENDPOINT_COMPONENT_ID;
	View.OnAddComponent(Op);

	Worker_AuthorityChangeOp AuthOp;
	AuthOp.entity_id = Id;
	AuthOp.authority = WORKER_AUTHORITY_AUTHORITATIVE;

	AuthOp.component_id = SpatialConstants::CROSSSERVER_SENDER_ENDPOINT_COMPONENT_ID;
	View.OnAuthorityChange(AuthOp);

	AuthOp.component_id = SpatialConstants::CROSSSERVER_RECEIVER_ACK_ENDPOINT_COMPONENT_ID;
	View.OnAuthorityChange(AuthOp);

	AuthOp.authority = WORKER_AUTHORITY_NOT_AUTHORITATIVE;
	AuthOp.component_id = SpatialConstants::CROSSSERVER_SENDER_ACK_ENDPOINT_COMPONENT_ID;
	View.OnAuthorityChange(AuthOp);

	AuthOp.component_id = SpatialConstants::CROSSSERVER_RECEIVER_ENDPOINT_COMPONENT_ID;
	View.OnAuthorityChange(AuthOp);
}

void RemoveEntityAndCrossServerComponents(EntityComponentOpListBuilder& Builder, Worker_EntityId Id)
{
	// Builder.SetAuthority(Id, SpatialConstants::CROSSSERVER_SENDER_ACK_ENDPOINT_COMPONENT_ID, WORKER_AUTHORITY_NOT_AUTHORITATIVE);
	// Builder.SetAuthority(Id, SpatialConstants::CROSSSERVER_RECEIVER_ENDPOINT_COMPONENT_ID, WORKER_AUTHORITY_NOT_AUTHORITATIVE);

	Builder.RemoveComponent(Id, SpatialConstants::CROSSSERVER_SENDER_ENDPOINT_COMPONENT_ID);
	Builder.RemoveComponent(Id, SpatialConstants::CROSSSERVER_SENDER_ACK_ENDPOINT_COMPONENT_ID);
	Builder.RemoveComponent(Id, SpatialConstants::CROSSSERVER_RECEIVER_ENDPOINT_COMPONENT_ID);
	Builder.RemoveComponent(Id, SpatialConstants::CROSSSERVER_RECEIVER_ACK_ENDPOINT_COMPONENT_ID);
	Builder.RemoveComponent(Id, SpatialConstants::ROUTINGWORKER_TAG_COMPONENT_ID);
	Builder.RemoveEntity(Id);
}

void RemoveEntityAndCrossServerComponents(SpatialOSWorkerConnectionSpy& Connection, Worker_EntityId Id)
{
	Connection.SendRemoveComponent(Id, SpatialConstants::CROSSSERVER_SENDER_ENDPOINT_COMPONENT_ID);
	Connection.SendRemoveComponent(Id, SpatialConstants::CROSSSERVER_SENDER_ACK_ENDPOINT_COMPONENT_ID);
	Connection.SendRemoveComponent(Id, SpatialConstants::CROSSSERVER_RECEIVER_ENDPOINT_COMPONENT_ID);
	Connection.SendRemoveComponent(Id, SpatialConstants::CROSSSERVER_RECEIVER_ACK_ENDPOINT_COMPONENT_ID);
	Connection.SendRemoveComponent(Id, SpatialConstants::ROUTINGWORKER_TAG_COMPONENT_ID);
	Connection.SendDeleteEntityRequest(Id);

	for (auto Iterator = Connection.Updates.CreateIterator(); Iterator; ++Iterator)
	{
		if (Iterator->Key == Id)
		{
			Iterator.RemoveCurrent();
		}
	}
}

bool ExtractUpdatesFromRPCService(ConnectionHandlerStub& Stub, SpatialGDK::SpatialRPCService& RPCService)
{
	bool bHadUpdates = false;
	EntityComponentOpListBuilder Builder;
	TArray<TArray<OpList>> ListsOfOpLists;
	TArray<OpList> OpLists;

	auto ListOfUpdates = RPCService.GetRPCsAndAcksToSend();

	for (const auto& Update : ListOfUpdates)
	{
		Builder.UpdateComponent(Update.EntityId,
								ComponentUpdate(OwningComponentUpdatePtr(Update.Update.schema_type), Update.Update.component_id));
		bHadUpdates = true;
	}

	OpLists.Add(MoveTemp(Builder).CreateOpList());
	ListsOfOpLists.Add(MoveTemp(OpLists));
	Stub.SetListsOfOpLists(MoveTemp(ListsOfOpLists));

	return bHadUpdates;
}

bool ProcessRPCUpdates(TSet<TPair<Worker_EntityId_Key, Worker_ComponentId>>& Updates, SpatialGDK::SpatialRPCService& RPCService,
					   TSet<Worker_EntityId_Key> FilterUpdates = TSet<Worker_EntityId_Key>())
{
	bool bHadUpdates = false;
	for (const auto& Update : Updates)
	{
		if (FilterUpdates.Num() != 0 && !FilterUpdates.Contains(Update.Get<0>()))
		{
			continue;
		}

		switch (Update.Get<1>())
		{
		case SpatialConstants::CROSSSERVER_RECEIVER_ENDPOINT_COMPONENT_ID:
			RPCService.ExtractRPCsForEntity(Update.Get<0>(), Update.Get<1>());
			bHadUpdates = true;
			break;
		case SpatialConstants::CROSSSERVER_SENDER_ACK_ENDPOINT_COMPONENT_ID:
			RPCService.UpdateMergedACKs(Update.Get<0>());
			bHadUpdates = true;
			break;
		default:
			checkNoEntry();
		}
	}
	Updates.Empty();

	return bHadUpdates;
}

struct RPCToSend
{
	RPCToSend(Worker_EntityId InSender, Worker_EntityId InTarget, uint32 InPayloadId)
		: Sender(InSender)
		, Target(InTarget)
		, PayloadId(InPayloadId)
	{
	}

	Worker_EntityId Sender;
	Worker_EntityId Target;
	uint32 PayloadId;
};

struct Components
{
	Components(Worker_EntityId Entity, USpatialStaticComponentView* View)
	{
		Sender = View->GetComponentData<CrossServerEndpointSender>(Entity);
		Receiver = View->GetComponentData<CrossServerEndpointReceiver>(Entity);
		SenderACK = View->GetComponentData<CrossServerEndpointSenderACK>(Entity);
		ReceiverACK = View->GetComponentData<CrossServerEndpointReceiverACK>(Entity);
	}

	bool CheckSettled()
	{
		for (auto& Slot : Sender->ReliableRPCBuffer.Counterpart)
		{
			if (Slot.IsSet())
			{
				return false;
			}
		}
		for (auto& Slot : SenderACK->ACKArray)
		{
			if (Slot.IsSet())
			{
				return false;
			}
		}
		for (auto& Slot : Receiver->ReliableRPCBuffer.Counterpart)
		{
			if (Slot.IsSet())
			{
				return false;
			}
		}
		for (auto& Slot : ReceiverACK->ACKArray)
		{
			if (Slot.IsSet())
			{
				return false;
			}
		}
		return true;
	}

	CrossServerEndpointSender* Sender;
	CrossServerEndpointReceiver* Receiver;
	CrossServerEndpointSenderACK* SenderACK;
	CrossServerEndpointReceiverACK* ReceiverACK;
};

struct TestRoutingFixture
{
	TestRoutingFixture()
		: Handler(MakeUnique<ConnectionHandlerStub>())
		, Stub(Handler.Get())
		, Coordinator(MoveTemp(Handler))
		, RoutingSystem(Coordinator.CreateSubView(SpatialConstants::ROUTINGWORKER_TAG_COMPONENT_ID,
												  [](const Worker_EntityId, const SpatialGDK::EntityViewElement&) {
													  return true;
												  },
												  {}),
						TEXT(""))
	{
	}

	bool Step()
	{
		Coordinator.Advance();

		RoutingSystem.Advance(&Spy);
		RoutingSystem.Flush(&Spy);

		return Spy.Updates.Num() > 0;
	}

private:
	TUniquePtr<ConnectionHandlerStub> Handler;

public:
	ConnectionHandlerStub* Stub;
	ViewCoordinator Coordinator;

	SpatialGDK::SpatialRoutingSystem RoutingSystem;

	SpatialOSWorkerConnectionSpy Spy;
};

ROUTING_SERVICE_TEST(TestRouting_BasicOp)
{
	TArray<Worker_EntityId> Entities;
	for (uint32 i = 0; i < 4; ++i)
	{
		Entities.Add((i + 1) * 10);
	}

	TArray<TArray<OpList>> ListsOfOpLists;
	{
		TArray<OpList> OpLists;
		EntityComponentOpListBuilder Builder;
		for (auto Entity : Entities)
		{
			AddEntityAndCrossServerComponents(Builder, Entity);
		}
		OpLists.Add(MoveTemp(Builder).CreateOpList());
		ListsOfOpLists.Add(MoveTemp(OpLists));
	}

	TestRoutingFixture TestFixture;

	SpatialOSWorkerConnectionSpy& Spy = TestFixture.Spy;
	ConnectionHandlerStub* Stub = TestFixture.Stub;
	Stub->SetListsOfOpLists(MoveTemp(ListsOfOpLists));
	TestFixture.Step();

	for (auto Entity : Entities)
	{
		AddEntityAndCrossServerComponents(*Spy.ComponentView, Entity);
	}

	Worker_EntityId Entity1 = Entities[0];
	Worker_EntityId Entity2 = Entities[1];

	CrossServerEndpointReceiver& Receiver2 = *Spy.ComponentView->GetComponentData<CrossServerEndpointReceiver>(Entity2);
	CrossServerEndpointSenderACK& SenderACK1 = *Spy.ComponentView->GetComponentData<CrossServerEndpointSenderACK>(Entity1);
	CrossServerEndpointReceiverACK& ReceiverACK2 = *Spy.ComponentView->GetComponentData<CrossServerEndpointReceiverACK>(Entity2);

	RPCPayload Payload(0, 1337, TArray<uint8>());

	SpatialGDK::SpatialRPCService* RPCServiceBackptr = nullptr;

	TSet<TPair<Worker_EntityId, uint32>> ExpectedRPCs = { MakeTuple(Entity2, 1337u) };

	auto ExtractRPCCallback = [this, &ExpectedRPCs, &RPCServiceBackptr](Worker_EntityId Target, const FUnrealObjectRef& SenderRef,
																		ERPCType Type, const SpatialGDK::RPCPayload& Payload,
																		uint64 SenderRev) {
		int32 NumRemoved = ExpectedRPCs.Remove(MakeTuple(Target, Payload.Index));
		FString DebugText = FString::Printf(TEXT("RPC %i from %llu to %llu was unexpected"), Payload.Index, SenderRef.Entity, Target);
		TestTrue(*DebugText, NumRemoved > 0);

		RPCServiceBackptr->WriteCrossServerACKFor(Target, SenderRef.Entity, SenderRef.Offset, SenderRev, Type);
		return true;
	};

	SpatialGDK::SpatialRPCService RPCService(ExtractRPCDelegate::CreateLambda(ExtractRPCCallback), Spy.ComponentView, nullptr);
	RPCServiceBackptr = &RPCService;
	for (auto Entity : Entities)
	{
		RPCService.OnEndpointAuthorityGained(Entity, SpatialConstants::CROSSSERVER_SENDER_ENDPOINT_COMPONENT_ID);
		RPCService.OnEndpointAuthorityGained(Entity, SpatialConstants::CROSSSERVER_RECEIVER_ACK_ENDPOINT_COMPONENT_ID);
	}

	RPCService.PushRPC(Entity1, FUnrealObjectRef(Entity2, 0), ERPCType::CrossServerSender, Payload, false);
	ExtractUpdatesFromRPCService(*Stub, RPCService);

	TestFixture.Step();

	TestTrue(TEXT("SentRPC"), Receiver2.ReliableRPCBuffer.Counterpart.Num() > 0);
	TestTrue(TEXT("SentRPC"), Receiver2.ReliableRPCBuffer.Counterpart[0].IsSet());
	const FUnrealObjectRef& SenderBackRef = Receiver2.ReliableRPCBuffer.Counterpart[0].GetValue();
	TestTrue(TEXT("SentRPC"), SenderBackRef.Entity == Entity1);

	ProcessRPCUpdates(Spy.Updates, RPCService);

	TestTrue(TEXT("ReceivedRPC"), ExpectedRPCs.Num() == 0);

	ExtractUpdatesFromRPCService(*Stub, RPCService);

	TestFixture.Step();

	TestTrue(TEXT("ACKWritten"), SenderACK1.ACKArray.Num() > 0);
	TestTrue(TEXT("ACKWritten"), SenderACK1.ACKArray[0].IsSet());
	const ACKItem& ACK = SenderACK1.ACKArray[0].GetValue();
	TestTrue(TEXT("ACKWritten"), ACK.Sender == Entity1);

	ProcessRPCUpdates(Spy.Updates, RPCService);
	ExtractUpdatesFromRPCService(*Stub, RPCService);

	TestFixture.Step();

	for (auto& Slot : SenderACK1.ACKArray)
	{
		TestTrue(TEXT("SenderACK cleanued up"), !Slot.IsSet());
	}

	for (auto& Slot : Receiver2.ReliableRPCBuffer.Counterpart)
	{
		TestTrue(TEXT("Receiver cleaned up"), !Slot.IsSet());
	}

	ProcessRPCUpdates(Spy.Updates, RPCService);
	ExtractUpdatesFromRPCService(*Stub, RPCService);

	TestFixture.Step();

	for (auto& Slot : ReceiverACK2.ACKArray)
	{
		TestTrue(TEXT("Receiver ACK cleaned up"), !Slot.IsSet());
	}

	TArray<RPCToSend> RPCs;
	uint32 RPCId = 1;
	for (uint32 i = 0; i < 8; ++i)
	{
		for (int32 j = 0; j < Entities.Num(); ++j)
			for (int32 k = j + 1; k < Entities.Num(); ++k)
			{
				RPCs.Add(RPCToSend(Entities[j], Entities[k], RPCId++));
				RPCs.Add(RPCToSend(Entities[k], Entities[j], RPCId++));
			}
	}

	for (auto& RPC : RPCs)
	{
		ExpectedRPCs.Add(MakeTuple(RPC.Target, RPC.PayloadId));
	}

	bool bHasProcessedMessages = true;
	while (RPCs.Num() != 0 || bHasProcessedMessages)
	{
		if (RPCs.Num() > 0)
		{
			RPCToSend RPC = RPCs.Last();
			RPCPayload DummyPayload(0, RPC.PayloadId, TArray<uint8>());
			if (RPCService.PushRPC(RPC.Sender, FUnrealObjectRef(RPC.Target, 0), ERPCType::CrossServerSender, DummyPayload, false)
				== EPushRPCResult::Success)
			{
				RPCs.Pop();
			}
		}

		bHasProcessedMessages = false;
		bHasProcessedMessages |= ProcessRPCUpdates(Spy.Updates, RPCService);
		bHasProcessedMessages |= ExtractUpdatesFromRPCService(*Stub, RPCService);
		TestFixture.Step();
	}

	TestTrue(TEXT("All RPCs sent and accounted for"), ExpectedRPCs.Num() == 0);
	for (auto Entity : Entities)
	{
		Components Comps(Entity, Spy.ComponentView);
		TestTrue(TEXT("Settled"), Comps.CheckSettled());
	}

	return true;
}

ROUTING_SERVICE_TEST(TestRouting_Delete)
{
	const uint32 Delays = 8;

	TArray<Worker_EntityId> Entities;
	for (uint32 i = 0; i < 4 * Delays * 2; ++i)
	{
		Entities.Add((i + 1) * 10);
	}

	TArray<TArray<OpList>> ListsOfOpLists;
	{
		TArray<OpList> OpLists;
		EntityComponentOpListBuilder Builder;
		for (auto Entity : Entities)
		{
			AddEntityAndCrossServerComponents(Builder, Entity);
		}
		OpLists.Add(MoveTemp(Builder).CreateOpList());
		ListsOfOpLists.Add(MoveTemp(OpLists));
	}

	TestRoutingFixture TestFixture;

	SpatialOSWorkerConnectionSpy& Spy = TestFixture.Spy;
	ConnectionHandlerStub* Stub = TestFixture.Stub;
	Stub->SetListsOfOpLists(MoveTemp(ListsOfOpLists));
	TestFixture.Step();

	for (auto Entity : Entities)
	{
		AddEntityAndCrossServerComponents(*Spy.ComponentView, Entity);
	}

	SpatialGDK::SpatialRPCService* RPCServiceBackptr = nullptr;

	TSet<TPair<Worker_EntityId, uint32>> ExpectedRPCs;

	auto ExtractRPCCallback = [this, &ExpectedRPCs, &RPCServiceBackptr](Worker_EntityId Target, const FUnrealObjectRef& SenderRef,
																		ERPCType Type, const SpatialGDK::RPCPayload& Payload,
																		uint64 SenderRev) {
		RPCServiceBackptr->WriteCrossServerACKFor(Target, SenderRef.Entity, SenderRef.Offset, SenderRev, Type);
		return true;
	};

	SpatialGDK::SpatialRPCService RPCService(ExtractRPCDelegate::CreateLambda(ExtractRPCCallback), Spy.ComponentView, nullptr);
	RPCServiceBackptr = &RPCService;
	for (auto Entity : Entities)
	{
		RPCService.OnEndpointAuthorityGained(Entity, SpatialConstants::CROSSSERVER_SENDER_ENDPOINT_COMPONENT_ID);
		RPCService.OnEndpointAuthorityGained(Entity, SpatialConstants::CROSSSERVER_RECEIVER_ACK_ENDPOINT_COMPONENT_ID);
	}

	for (uint32 Attempt = 0; Attempt < 4; ++Attempt)
		for (uint32 CurDelay = 0; CurDelay < Delays; ++CurDelay)
		{
			Worker_EntityId Sender = Entities[2 * (Attempt * Delays + CurDelay) + 0];
			Worker_EntityId Receiver = Entities[2 * (Attempt * Delays + CurDelay) + 1];

			Worker_EntityId ToRemove = (Attempt / 2) == 0 ? Sender : Receiver;
			Worker_EntityId ToCheck = (Attempt / 2) == 1 ? Sender : Receiver;

			RPCPayload DummyPayload(0, 0, TArray<uint8>());
			RPCService.PushRPC(Sender, FUnrealObjectRef(Receiver, 0), ERPCType::CrossServerSender, DummyPayload, false);

			uint32 Delay = 0;
			bool bHasProcessedMessages = true;
			while (bHasProcessedMessages)
			{
				auto PerformDeletion = [&] {
					RPCService.OnEndpointAuthorityLost(ToRemove, SpatialConstants::CROSSSERVER_SENDER_ENDPOINT_COMPONENT_ID);
					RPCService.OnEndpointAuthorityLost(ToRemove, SpatialConstants::CROSSSERVER_RECEIVER_ACK_ENDPOINT_COMPONENT_ID);
					RemoveEntityAndCrossServerComponents(Spy, ToRemove);

					TArray<OpList> OpLists;
					EntityComponentOpListBuilder Builder;
					RemoveEntityAndCrossServerComponents(Builder, ToRemove);
					OpLists.Add(MoveTemp(Builder).CreateOpList());
					ListsOfOpLists.Add(MoveTemp(OpLists));
					Stub->SetListsOfOpLists(MoveTemp(ListsOfOpLists));
					TestFixture.Step();
				};

				if (Delay == CurDelay && Attempt % 2 == 0)
				{
					PerformDeletion();
				}

				bHasProcessedMessages = false;
				bHasProcessedMessages |= ProcessRPCUpdates(Spy.Updates, RPCService);

				if (Delay == CurDelay && Attempt % 2 == 1)
				{
					PerformDeletion();
				}

				bHasProcessedMessages |= ExtractUpdatesFromRPCService(*Stub, RPCService);
				bHasProcessedMessages |= TestFixture.Step();

				++Delay;
			}

			Components Comps(ToCheck, Spy.ComponentView);
			bool Settled = Comps.CheckSettled();
			if (!Settled)
			{
				FString Debug = FString::Printf(TEXT("Attempt %i for delay %i is not settled"), Attempt, Delay);
				TestTrue(Debug, Settled);
			}
		}

	return true;
}

ROUTING_SERVICE_TEST(TestRouting_Capacity)
{
	TArray<Worker_EntityId> Entities;
	for (uint32 i = 0; i < 4; ++i)
	{
		Entities.Add((i + 1) * 10);
	}

	TArray<TArray<OpList>> ListsOfOpLists;
	{
		TArray<OpList> OpLists;
		EntityComponentOpListBuilder Builder;
		for (auto Entity : Entities)
		{
			AddEntityAndCrossServerComponents(Builder, Entity);
		}
		OpLists.Add(MoveTemp(Builder).CreateOpList());
		ListsOfOpLists.Add(MoveTemp(OpLists));
	}

	TestRoutingFixture TestFixture;

	SpatialOSWorkerConnectionSpy& Spy = TestFixture.Spy;
	ConnectionHandlerStub* Stub = TestFixture.Stub;
	Stub->SetListsOfOpLists(MoveTemp(ListsOfOpLists));
	TestFixture.Step();

	for (auto Entity : Entities)
	{
		AddEntityAndCrossServerComponents(*Spy.ComponentView, Entity);
	}

	SpatialGDK::SpatialRPCService* RPCServiceBackptr = nullptr;

	TSet<TPair<Worker_EntityId, uint32>> ExpectedRPCs;

	auto ExtractRPCCallback = [this, &ExpectedRPCs, &RPCServiceBackptr](Worker_EntityId Target, const FUnrealObjectRef& SenderRef,
																		ERPCType Type, const SpatialGDK::RPCPayload& Payload,
																		uint64 SenderRev) {
		int32 NumRemoved = ExpectedRPCs.Remove(MakeTuple(Target, Payload.Index));
		FString DebugText = FString::Printf(TEXT("RPC %i from %llu to %llu was unexpected"), Payload.Index, SenderRef.Entity, Target);
		TestTrue(*DebugText, NumRemoved > 0);
		RPCServiceBackptr->WriteCrossServerACKFor(Target, SenderRef.Entity, SenderRef.Offset, SenderRev, Type);
		return true;
	};

	SpatialGDK::SpatialRPCService RPCService(ExtractRPCDelegate::CreateLambda(ExtractRPCCallback), Spy.ComponentView, nullptr);
	RPCServiceBackptr = &RPCService;
	for (auto Entity : Entities)
	{
		RPCService.OnEndpointAuthorityGained(Entity, SpatialConstants::CROSSSERVER_SENDER_ENDPOINT_COMPONENT_ID);
		RPCService.OnEndpointAuthorityGained(Entity, SpatialConstants::CROSSSERVER_RECEIVER_ACK_ENDPOINT_COMPONENT_ID);
	}

	uint32 MaxCapacity = SpatialGDK::RPCRingBufferUtils::GetRingBufferSize(ERPCType::CrossServerSender);

	TArray<uint32> TargetIdx;
	for (int32 idx = 0; idx < 4; ++idx)
	{
		TargetIdx.Add((idx + 1) % 4);
	}

	// Should take 2 steps to fan out and receive updates.
	uint32 RPCPerStep = MaxCapacity / 2;
	uint32 RPCAlloc = 0;
	for (uint32 BatchNum = 0; BatchNum < 16; ++BatchNum)
	{
		ProcessRPCUpdates(Spy.Updates, RPCService);

		for (uint32 i = 0; i < 4; ++i)
		{
			Worker_EntityId Sender = Entities[i];
			Worker_EntityId Receiver = Entities[TargetIdx[i]];
			for (uint32 j = 0; j < RPCPerStep; ++j)
			{
				RPCPayload DummyPayload(0, RPCAlloc, TArray<uint8>());
				ExpectedRPCs.Add(MakeTuple(Receiver, RPCAlloc));
				++RPCAlloc;
				SpatialGDK::EPushRPCResult Result =
					RPCService.PushRPC(Sender, FUnrealObjectRef(Receiver, 0), ERPCType::CrossServerSender, DummyPayload, false);
				TestTrue(TEXT("Did not run out of capacity"), Result == EPushRPCResult::Success);
				do
				{
					TargetIdx[i] = (TargetIdx[i] + 1) % 4;
					Receiver = Entities[TargetIdx[i]];
				} while (Receiver == Sender);
			}
		}
		ExtractUpdatesFromRPCService(*Stub, RPCService);
		TestFixture.Step();
	}
	bool bHasProcessedMessages = true;
	while (bHasProcessedMessages)
	{
		bHasProcessedMessages = false;
		bHasProcessedMessages |= ProcessRPCUpdates(Spy.Updates, RPCService);
		bHasProcessedMessages |= ExtractUpdatesFromRPCService(*Stub, RPCService);
		bHasProcessedMessages |= TestFixture.Step();
	}
	TestTrue(TEXT("All RPC were received"), ExpectedRPCs.Num() == 0);

	return true;
}

ROUTING_SERVICE_TEST(TestRouting_Migration)
{
	return true;
}

} // namespace SpatialGDK
