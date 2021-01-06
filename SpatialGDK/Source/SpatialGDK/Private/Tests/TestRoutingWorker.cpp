// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CoreMinimal.h"

// Engine
#include "Engine/Engine.h"
#include "GameFramework/GameStateBase.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/TestActor.h"
#include "Tests/TestDefinitions.h"
//#include "Tests/TestingComponentViewHelpers.h"

// GDK
#include "Interop/Connection/SpatialOSWorkerInterface.h"
#include "Interop/RPCs/SpatialRPCService.h"
#include "Interop/SpatialRoutingSystem.h"
#include "Interop/SpatialStaticComponentView.h"
#include "Schema/CrossServerEndpoint.h"
#include "SpatialView/OpList/EntityComponentOpList.h"
#include "SpatialView/ViewCoordinator.h"

#include "improbable/c_schema.h"

SpatialGDK::OwningComponentUpdatePtr FullyCopyComponentUpdate(Schema_ComponentUpdate* SrcUpdate)
{
	SpatialGDK::OwningComponentUpdatePtr Copy(Schema_CopyComponentUpdate(SrcUpdate));
	uint32 ClearCount = Schema_GetComponentUpdateClearedFieldCount(SrcUpdate);
	TArray<Schema_FieldId> ClearedFields;
	ClearedFields.SetNum(ClearCount);
	Schema_GetComponentUpdateClearedFieldList(SrcUpdate, ClearedFields.GetData());
	for (uint32 i = 0; i < ClearCount; ++i)
	{
		Schema_AddComponentUpdateClearedField(Copy.Get(), ClearedFields[i]);
	}

	return MoveTemp(Copy);
}

class SpatialOSWorkerConnectionSpy : public SpatialOSWorkerInterface
{
public:
	SpatialOSWorkerConnectionSpy();

	virtual const TArray<SpatialGDK::EntityDelta>& GetEntityDeltas() override;
	virtual const TArray<Worker_Op>& GetWorkerMessages() override;
	virtual Worker_RequestId SendReserveEntityIdsRequest(uint32_t NumOfEntities, const SpatialGDK::FRetryData& Data) override;
	virtual Worker_RequestId SendCreateEntityRequest(TArray<FWorkerComponentData> Components, const Worker_EntityId* EntityId,
													 const SpatialGDK::FRetryData& Data, const FSpatialGDKSpanId& SpanId = {}) override;
	virtual Worker_RequestId SendDeleteEntityRequest(Worker_EntityId EntityId, const SpatialGDK::FRetryData& Data,
													 const FSpatialGDKSpanId& SpanId = {}) override;
	virtual void SendAddComponent(Worker_EntityId EntityId, FWorkerComponentData* ComponentData,
								  const FSpatialGDKSpanId& SpanId = {}) override;
	virtual void SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId,
									 const FSpatialGDKSpanId& SpanId = {}) override;
	virtual void SendComponentUpdate(Worker_EntityId EntityId, FWorkerComponentUpdate* ComponentUpdate,
									 const FSpatialGDKSpanId& SpanId = {}) override;
	virtual Worker_RequestId SendCommandRequest(Worker_EntityId EntityId, Worker_CommandRequest* Request,
												const SpatialGDK::FRetryData& Data, const FSpatialGDKSpanId& SpanId = {}) override;
	virtual void SendCommandResponse(Worker_RequestId RequestId, Worker_CommandResponse* Response,
									 const FSpatialGDKSpanId& SpanId = {}) override;
	virtual void SendCommandFailure(Worker_RequestId RequestId, const FString& Message, const FSpatialGDKSpanId& SpanId = {}) override;
	virtual void SendLogMessage(uint8_t Level, const FName& LoggerName, const TCHAR* Message) override;
	virtual Worker_RequestId SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery, const SpatialGDK::FRetryData& Data) override;
	virtual void SendMetrics(SpatialGDK::SpatialMetrics Metrics) override;

	// The following methods are used to query for state in tests.
	const Worker_EntityQuery* GetLastEntityQuery();
	void ClearLastEntityQuery();

	Worker_RequestId GetLastRequestId();

	SpatialGDK::ViewCoordinator* Coordinator;
	SpatialGDK::EntityComponentOpListBuilder Builder;
	bool bHadUpdates = false;

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
}

const TArray<SpatialGDK::EntityDelta>& SpatialOSWorkerConnectionSpy::GetEntityDeltas()
{
	return PlaceholderEntityDeltas;
}

const TArray<Worker_Op>& SpatialOSWorkerConnectionSpy::GetWorkerMessages()
{
	return PlaceholderWorkerMessages;
}

Worker_RequestId SpatialOSWorkerConnectionSpy::SendReserveEntityIdsRequest(uint32_t NumOfEntities, const SpatialGDK::FRetryData& Data)
{
	return NextRequestId++;
}

Worker_RequestId SpatialOSWorkerConnectionSpy::SendCreateEntityRequest(TArray<FWorkerComponentData> Components,
																	   const Worker_EntityId* EntityId, const SpatialGDK::FRetryData& Data,
																	   const FSpatialGDKSpanId& SpanId)
{
	return NextRequestId++;
}

Worker_RequestId SpatialOSWorkerConnectionSpy::SendDeleteEntityRequest(Worker_EntityId EntityId, const SpatialGDK::FRetryData& Data,
																	   const FSpatialGDKSpanId& SpanId)
{
	if (Coordinator)
	{
		Coordinator->SendDeleteEntityRequest(EntityId);
	}
	Builder.RemoveEntity(EntityId);
	return NextRequestId++;
}

void SpatialOSWorkerConnectionSpy::SendAddComponent(Worker_EntityId EntityId, FWorkerComponentData* ComponentData,
													const FSpatialGDKSpanId& SpanId)
{
}

void SpatialOSWorkerConnectionSpy::SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId,
													   const FSpatialGDKSpanId& SpanId)
{
	if (Coordinator)
	{
		Coordinator->SendRemoveComponent(EntityId, ComponentId, SpanId);
	}
	Builder.RemoveComponent(EntityId, ComponentId);
}

void SpatialOSWorkerConnectionSpy::SendComponentUpdate(Worker_EntityId EntityId, FWorkerComponentUpdate* ComponentUpdate,
													   const FSpatialGDKSpanId& SpanId)
{
	bHadUpdates = true;

	SpatialGDK::OwningComponentUpdatePtr UpdateData(ComponentUpdate->schema_type);
	if (Coordinator)
	{
		SpatialGDK::OwningComponentUpdatePtr UpdateDataCopy = FullyCopyComponentUpdate(ComponentUpdate->schema_type);

		Coordinator->SendComponentUpdate(EntityId, SpatialGDK::ComponentUpdate(MoveTemp(UpdateDataCopy), ComponentUpdate->component_id),
										 SpanId);
	}
	Builder.UpdateComponent(EntityId, SpatialGDK::ComponentUpdate(MoveTemp(UpdateData), ComponentUpdate->component_id));
}

Worker_RequestId SpatialOSWorkerConnectionSpy::SendCommandRequest(Worker_EntityId EntityId, Worker_CommandRequest* Request,
																  const SpatialGDK::FRetryData& Data, const FSpatialGDKSpanId& SpanId)
{
	return NextRequestId++;
}

void SpatialOSWorkerConnectionSpy::SendCommandResponse(Worker_RequestId RequestId, Worker_CommandResponse* Response,
													   const FSpatialGDKSpanId& SpanId)
{
}

void SpatialOSWorkerConnectionSpy::SendCommandFailure(Worker_RequestId RequestId, const FString& Message, const FSpatialGDKSpanId& SpanId)
{
}

void SpatialOSWorkerConnectionSpy::SendLogMessage(uint8_t Level, const FName& LoggerName, const TCHAR* Message) {}

Worker_RequestId SpatialOSWorkerConnectionSpy::SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery,
																	  const SpatialGDK::FRetryData& Data)
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
	void SetFromBuilder(EntityComponentOpListBuilder& Builder)
	{
		check(!HasNextOpList());
		TArray<TArray<OpList>> NewListsOfOpLists;
		TArray<OpList> OpLists;
		OpLists.Add(Builder.Move().CreateOpList());
		NewListsOfOpLists.Add(MoveTemp(OpLists));
		SetListsOfOpLists(MoveTemp(NewListsOfOpLists));
	}
	void SetListsOfOpLists(TArray<TArray<OpList>> List) { ListsOfOpLists = MoveTemp(List); }

	bool HasNextOpList() { return ListsOfOpLists.Num() > 0; }

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

	virtual Worker_EntityId GetWorkerSystemEntityId() const override { return 0; }

private:
	TArray<TArray<OpList>> ListsOfOpLists;
	TArray<OpList> QueuedOpLists;
	FString WorkerId = TEXT("test_worker");
	TArray<FString> Attributes = { TEXT("test") };
};

static FComponentSetData const& GetComponentSetData()
{
	static FComponentSetData s_ComponentSetData = [] {
		FComponentSetData Data;
		auto& ServerSet = Data.ComponentSets.Add(SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID);
		ServerSet.Add(SpatialConstants::CROSSSERVER_SENDER_ENDPOINT_COMPONENT_ID);
		ServerSet.Add(SpatialConstants::CROSSSERVER_RECEIVER_ACK_ENDPOINT_COMPONENT_ID);

		auto& RoutingSet = Data.ComponentSets.Add(SpatialConstants::ROUTING_WORKER_AUTH_COMPONENT_SET_ID);
		RoutingSet.Add(SpatialConstants::CROSSSERVER_SENDER_ACK_ENDPOINT_COMPONENT_ID);
		RoutingSet.Add(SpatialConstants::CROSSSERVER_RECEIVER_ENDPOINT_COMPONENT_ID);

		return Data;
	}();

	return s_ComponentSetData;
}

void AddEntityAndCrossServerComponents(EntityComponentOpListBuilder& Builder, Worker_EntityId Id)
{
	Builder.AddEntity(Id);
	Builder.AddComponent(Id, ComponentData{ SpatialConstants::CROSSSERVER_SENDER_ENDPOINT_COMPONENT_ID });
	Builder.AddComponent(Id, ComponentData{ SpatialConstants::CROSSSERVER_SENDER_ACK_ENDPOINT_COMPONENT_ID });
	Builder.AddComponent(Id, ComponentData{ SpatialConstants::CROSSSERVER_RECEIVER_ENDPOINT_COMPONENT_ID });
	Builder.AddComponent(Id, ComponentData{ SpatialConstants::CROSSSERVER_RECEIVER_ACK_ENDPOINT_COMPONENT_ID });
	Builder.AddComponent(Id, ComponentData{ SpatialConstants::ROUTINGWORKER_TAG_COMPONENT_ID });
	Builder.AddComponent(Id, ComponentData{ SpatialConstants::ACTOR_AUTH_TAG_COMPONENT_ID });
}

void AddComponentAuthForRoutingWorker(EntityComponentOpListBuilder& Builder, Worker_EntityId Id)
{
	TArray<ComponentData> CanonicalData;
	CanonicalData.Emplace(ComponentData{ SpatialConstants::CROSSSERVER_SENDER_ACK_ENDPOINT_COMPONENT_ID });
	CanonicalData.Emplace(ComponentData{ SpatialConstants::CROSSSERVER_RECEIVER_ENDPOINT_COMPONENT_ID });
	Builder.SetAuthority(Id, SpatialConstants::ROUTING_WORKER_AUTH_COMPONENT_SET_ID, WORKER_AUTHORITY_AUTHORITATIVE,
						 MoveTemp(CanonicalData));
}

void AddComponentAuthForServerWorker(EntityComponentOpListBuilder& Builder, Worker_EntityId Id)
{
	TArray<ComponentData> CanonicalData;
	CanonicalData.Emplace(ComponentData{ SpatialConstants::CROSSSERVER_SENDER_ENDPOINT_COMPONENT_ID });
	CanonicalData.Emplace(ComponentData{ SpatialConstants::CROSSSERVER_RECEIVER_ACK_ENDPOINT_COMPONENT_ID });
	Builder.SetAuthority(Id, SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID, WORKER_AUTHORITY_AUTHORITATIVE, MoveTemp(CanonicalData));
}

void RemoveEntityAndCrossServerComponents(ViewCoordinator& Coordinator, EntityComponentOpListBuilder& Builder, Worker_EntityId Id)
{
	Coordinator.SendRemoveComponent(Id, SpatialConstants::CROSSSERVER_SENDER_ENDPOINT_COMPONENT_ID, {});
	Builder.RemoveComponent(Id, SpatialConstants::CROSSSERVER_SENDER_ENDPOINT_COMPONENT_ID);
	Coordinator.SendRemoveComponent(Id, SpatialConstants::CROSSSERVER_SENDER_ACK_ENDPOINT_COMPONENT_ID, {});
	Builder.RemoveComponent(Id, SpatialConstants::CROSSSERVER_SENDER_ACK_ENDPOINT_COMPONENT_ID);
	Coordinator.SendRemoveComponent(Id, SpatialConstants::CROSSSERVER_RECEIVER_ENDPOINT_COMPONENT_ID, {});
	Builder.RemoveComponent(Id, SpatialConstants::CROSSSERVER_RECEIVER_ENDPOINT_COMPONENT_ID);
	Coordinator.SendRemoveComponent(Id, SpatialConstants::CROSSSERVER_RECEIVER_ACK_ENDPOINT_COMPONENT_ID, {});
	Builder.RemoveComponent(Id, SpatialConstants::CROSSSERVER_RECEIVER_ACK_ENDPOINT_COMPONENT_ID);
	Coordinator.SendRemoveComponent(Id, SpatialConstants::ROUTINGWORKER_TAG_COMPONENT_ID, {});
	Builder.RemoveComponent(Id, SpatialConstants::ROUTINGWORKER_TAG_COMPONENT_ID);
	Coordinator.SendRemoveComponent(Id, SpatialConstants::ACTOR_AUTH_TAG_COMPONENT_ID, {});
	Builder.RemoveComponent(Id, SpatialConstants::ACTOR_AUTH_TAG_COMPONENT_ID);
	Coordinator.SendDeleteEntityRequest(Id);
	Builder.RemoveEntity(Id);
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
	Components(Worker_EntityId Entity, const SpatialGDK::EntityView& View)
	{
		const SpatialGDK::EntityViewElement& Element = View.FindChecked(Entity);

		for (auto& Data : Element.Components)
		{
			if (Data.GetComponentId() == SpatialConstants::CROSSSERVER_SENDER_ENDPOINT_COMPONENT_ID)
			{
				Sender.Emplace(CrossServerEndpoint(Data.GetUnderlying()));
			}
			if (Data.GetComponentId() == SpatialConstants::CROSSSERVER_SENDER_ACK_ENDPOINT_COMPONENT_ID)
			{
				SenderACK.Emplace(CrossServerEndpointACK(Data.GetUnderlying()));
			}
			if (Data.GetComponentId() == SpatialConstants::CROSSSERVER_RECEIVER_ENDPOINT_COMPONENT_ID)
			{
				Receiver.Emplace(CrossServerEndpoint(Data.GetUnderlying()));
			}
			if (Data.GetComponentId() == SpatialConstants::CROSSSERVER_RECEIVER_ACK_ENDPOINT_COMPONENT_ID)
			{
				ReceiverACK.Emplace(CrossServerEndpointACK(Data.GetUnderlying()));
			}
		}
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

	TOptional<CrossServerEndpoint> Sender;
	TOptional<CrossServerEndpoint> Receiver;
	TOptional<CrossServerEndpointACK> SenderACK;
	TOptional<CrossServerEndpointACK> ReceiverACK;
};

struct WorkerMock
{
	WorkerMock()
		: Handler(MakeUnique<ConnectionHandlerStub>())
		, Stub(Handler.Get())
		, Coordinator(MoveTemp(Handler), nullptr, GetComponentSetData())
	{
	}

	void Step() { Coordinator.Advance(0.0); }

private:
	TUniquePtr<ConnectionHandlerStub> Handler;

public:
	ConnectionHandlerStub* Stub;
	ViewCoordinator Coordinator;
};

struct RoutingWorkerMock : WorkerMock
{
	RoutingWorkerMock()
		: RoutingSystem(Coordinator.CreateSubView(SpatialConstants::ROUTINGWORKER_TAG_COMPONENT_ID,
												  [](const Worker_EntityId, const SpatialGDK::EntityViewElement&) {
													  return true;
												  },
												  {}),
						0)
	{
		Spy.Coordinator = &Coordinator;
	}

	bool Step()
	{
		WorkerMock::Step();

		Spy.bHadUpdates = false;
		RoutingSystem.Advance(&Spy);
		RoutingSystem.Flush(&Spy);

		return Spy.bHadUpdates;
	}

	SpatialGDK::SpatialRoutingSystem RoutingSystem;
	SpatialOSWorkerConnectionSpy Spy;
};

struct ServerWorkerMock : WorkerMock
{
	ServerWorkerMock()
		: SubView(Coordinator.CreateSubView(SpatialConstants::ACTOR_AUTH_TAG_COMPONENT_ID,
											[](const Worker_EntityId, const SpatialGDK::EntityViewElement&) {
												return true;
											},
											{}))
	{
	}

	bool Step(CrossServerRPCService& Service, FRPCStore& Store)
	{
		WorkerMock::Step();

		Service.AdvanceView();
		Service.ProcessChanges();

		return Store.PendingComponentUpdatesToSend.Num() > 0;
	}

	SpatialGDK::FSubView& SubView;
};

struct TestRoutingFixture
{
	TestRoutingFixture()
	{
		CanExtractDelegate.BindLambda([](Worker_EntityId) {
			return true;
		});
	}

	void Init(const TArray<Worker_EntityId>& Entities)
	{
		{
			EntityComponentOpListBuilder Builder;
			for (auto Entity : Entities)
			{
				AddEntityAndCrossServerComponents(Builder, Entity);
				AddComponentAuthForRoutingWorker(Builder, Entity);
			}
			RoutingWorker.Stub->SetFromBuilder(Builder);
		}
		RoutingWorker.Step();

		{
			EntityComponentOpListBuilder Builder;
			for (auto Entity : Entities)
			{
				AddEntityAndCrossServerComponents(Builder, Entity);
				AddComponentAuthForServerWorker(Builder, Entity);
			}
			ServerWorker.Stub->SetFromBuilder(Builder);
		}

		ServerWorker.WorkerMock::Step();
	}

	void TransferRoutingWorkerUpdates() { ServerWorker.Stub->SetFromBuilder(RoutingWorker.Spy.Builder); }

	void TransferServerWorkerUpdates(CrossServerRPCService& RPCService, FRPCStore& RPCStore)
	{
		for (auto& It : RPCStore.PendingComponentUpdatesToSend)
		{
			RPCService.FlushPendingClearedFields(It);
		}

		EntityComponentOpListBuilder Builder;

		for (const auto& Update : RPCStore.PendingComponentUpdatesToSend)
		{
			SpatialGDK::OwningComponentUpdatePtr UpdateData(Update.Value.Update);
			SpatialGDK::OwningComponentUpdatePtr UpdateDataCopy = FullyCopyComponentUpdate(Update.Value.Update);

			ServerWorker.Coordinator.SendComponentUpdate(
				Update.Key.EntityId, SpatialGDK::ComponentUpdate(SpatialGDK::ComponentUpdate(MoveTemp(UpdateData), Update.Key.ComponentId)),
				{});
			Builder.UpdateComponent(Update.Key.EntityId, SpatialGDK::ComponentUpdate(MoveTemp(UpdateDataCopy), Update.Key.ComponentId));
		}
		RPCStore.PendingComponentUpdatesToSend.Empty();

		RoutingWorker.Stub->SetFromBuilder(Builder);
	}

	ActorCanExtractRPCDelegate CanExtractDelegate;

	RoutingWorkerMock RoutingWorker;
	ServerWorkerMock ServerWorker;
};

ROUTING_SERVICE_TEST(TestRoutingWorker_WhiteBox_SendOneMessage)
{
	TArray<Worker_EntityId> Entities;
	for (uint32 i = 0; i < 2; ++i)
	{
		Entities.Add((i + 1) * 10);
	}

	TestRoutingFixture TestFixture;
	TestFixture.Init(Entities);

	Worker_EntityId Entity1 = Entities[0];
	Worker_EntityId Entity2 = Entities[1];

	RPCPayload Payload(0, 1337, {}, TArray<uint8>(), {});

	SpatialGDK::CrossServerRPCService* RPCServiceBackptr = nullptr;

	TSet<TPair<Worker_EntityId_Key, uint32>> ExpectedRPCs = { MakeTuple((Worker_EntityId_Key)Entity2, 1337u) };

	auto ExtractRPCCallback = [this, &ExpectedRPCs, &RPCServiceBackptr](const FUnrealObjectRef& Target, const RPCSender& Sender,
																		SpatialGDK::RPCPayload Payload, TOptional<uint64>) {
		int32 NumRemoved = ExpectedRPCs.Remove(MakeTuple((Worker_EntityId_Key)Target.Entity, Payload.Index));
		FString DebugText = FString::Printf(TEXT("RPC %i from %llu to %llu was unexpected"), Payload.Index, Sender.Entity, Target.Entity);
		TestTrue(*DebugText, NumRemoved > 0);

		RPCServiceBackptr->WriteCrossServerACKFor(Target.Entity, Sender);
	};

	SpatialGDK::FRPCStore RPCStore;
	SpatialGDK::CrossServerRPCService RPCService(TestFixture.CanExtractDelegate, ExtractRPCDelegate::CreateLambda(ExtractRPCCallback),
												 TestFixture.ServerWorker.SubView, RPCStore);

	RPCServiceBackptr = &RPCService;
	RPCService.AdvanceView();
	RPCService.ProcessChanges();

	RPCService.PushCrossServerRPC(Entity2, RPCSender(Entity1, 0), Payload, false);
	TestFixture.TransferServerWorkerUpdates(RPCService, RPCStore);

	TestFixture.RoutingWorker.Step();
	TestFixture.TransferRoutingWorkerUpdates();
	TestFixture.ServerWorker.Step(RPCService, RPCStore);

	{
		Components Entity2Comps(Entity2, TestFixture.ServerWorker.Coordinator.GetView());

		TestTrue(TEXT("SentRPC"), Entity2Comps.Receiver->ReliableRPCBuffer.Counterpart.Num() > 0);
		TestTrue(TEXT("SentRPC"), Entity2Comps.Receiver->ReliableRPCBuffer.Counterpart[0].IsSet());
		const CrossServerRPCInfo& SenderBackRef = Entity2Comps.Receiver->ReliableRPCBuffer.Counterpart[0].GetValue();
		TestTrue(TEXT("SentRPC"), SenderBackRef.Entity == Entity1);
	}

	TestTrue(TEXT("ReceivedRPC"), ExpectedRPCs.Num() == 0);

	TestFixture.TransferServerWorkerUpdates(RPCService, RPCStore);
	TestFixture.RoutingWorker.Step();
	TestFixture.TransferRoutingWorkerUpdates();
	TestFixture.ServerWorker.Step(RPCService, RPCStore);

	{
		Components Entity1Comps(Entity1, TestFixture.ServerWorker.Coordinator.GetView());

		TestTrue(TEXT("ACKWritten"), Entity1Comps.SenderACK->ACKArray.Num() > 0);
		TestTrue(TEXT("ACKWritten"), Entity1Comps.SenderACK->ACKArray[0].IsSet());
		const ACKItem& ACK = Entity1Comps.SenderACK->ACKArray[0].GetValue();
		TestTrue(TEXT("ACKWritten"), ACK.Sender == Entity1);
	}

	TestFixture.TransferServerWorkerUpdates(RPCService, RPCStore);
	TestFixture.RoutingWorker.Step();
	TestFixture.TransferRoutingWorkerUpdates();
	TestFixture.ServerWorker.Step(RPCService, RPCStore);

	{
		Components Entity1Comps(Entity1, TestFixture.ServerWorker.Coordinator.GetView());
		Components Entity2Comps(Entity2, TestFixture.ServerWorker.Coordinator.GetView());
		for (auto& Slot : Entity1Comps.SenderACK->ACKArray)
		{
			TestTrue(TEXT("SenderACK cleanued up"), !Slot.IsSet());
		}

		for (auto& Slot : Entity2Comps.Receiver->ReliableRPCBuffer.Counterpart)
		{
			TestTrue(TEXT("Receiver cleaned up"), !Slot.IsSet());
		}
	}

	TestFixture.TransferServerWorkerUpdates(RPCService, RPCStore);
	TestFixture.RoutingWorker.Step();
	TestFixture.TransferRoutingWorkerUpdates();
	TestFixture.ServerWorker.Step(RPCService, RPCStore);

	{
		Components Entity2Comps(Entity2, TestFixture.ServerWorker.Coordinator.GetView());
		for (auto& Slot : Entity2Comps.ReceiverACK->ACKArray)
		{
			TestTrue(TEXT("Receiver ACK cleaned up"), !Slot.IsSet());
		}
	}

	return true;
}

ROUTING_SERVICE_TEST(TestRoutingWorker_BlackBox_SendSeveralMessagesToSeveralEntities)
{
	TArray<Worker_EntityId> Entities;
	for (uint32 i = 0; i < 4; ++i)
	{
		Entities.Add((i + 1) * 10);
	}

	TestRoutingFixture TestFixture;
	TestFixture.Init(Entities);

	SpatialGDK::CrossServerRPCService* RPCServiceBackptr = nullptr;

	TSet<TPair<Worker_EntityId_Key, uint32>> ExpectedRPCs;

	auto ExtractRPCCallback = [this, &ExpectedRPCs, &RPCServiceBackptr](const FUnrealObjectRef& Target, const RPCSender& Sender,
																		SpatialGDK::RPCPayload Payload, TOptional<uint64>) {
		int32 NumRemoved = ExpectedRPCs.Remove(MakeTuple((Worker_EntityId_Key)Target.Entity, Payload.Index));
		FString DebugText = FString::Printf(TEXT("RPC %i from %llu to %llu was unexpected"), Payload.Index, Sender.Entity, Target.Entity);
		TestTrue(*DebugText, NumRemoved > 0);

		RPCServiceBackptr->WriteCrossServerACKFor(Target.Entity, Sender);
	};

	SpatialGDK::FRPCStore RPCStore;
	SpatialGDK::CrossServerRPCService RPCService(TestFixture.CanExtractDelegate, ExtractRPCDelegate::CreateLambda(ExtractRPCCallback),
												 TestFixture.ServerWorker.SubView, RPCStore);
	RPCServiceBackptr = &RPCService;
	RPCService.AdvanceView();
	RPCService.ProcessChanges();

	// Push dummy OpList for the loop.
	TestFixture.TransferRoutingWorkerUpdates();

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
		ExpectedRPCs.Add(MakeTuple((Worker_EntityId_Key)RPC.Target, RPC.PayloadId));
	}

	bool bHasUpdatesToProcess = true;
	while (RPCs.Num() != 0 || bHasUpdatesToProcess)
	{
		if (RPCs.Num() > 0)
		{
			RPCToSend RPC = RPCs.Last();
			RPCPayload DummyPayload(0, RPC.PayloadId, {}, TArray<uint8>());
			if (RPCService.PushCrossServerRPC(RPC.Target, RPCSender(RPC.Sender, 0), DummyPayload, false) == EPushRPCResult::Success)
			{
				RPCs.Pop();
			}
		}
		bHasUpdatesToProcess = false;
		bHasUpdatesToProcess |= TestFixture.ServerWorker.Step(RPCService, RPCStore);
		TestFixture.TransferServerWorkerUpdates(RPCService, RPCStore);
		bHasUpdatesToProcess |= TestFixture.RoutingWorker.Step();
		TestFixture.TransferRoutingWorkerUpdates();
	}

	TestTrue(TEXT("All RPCs sent and accounted for"), ExpectedRPCs.Num() == 0);
	for (auto Entity : Entities)
	{
		Components Comps(Entity, TestFixture.ServerWorker.Coordinator.GetView());
		TestTrue(TEXT("Settled"), Comps.CheckSettled());
	}

	return true;
}

ROUTING_SERVICE_TEST(TestRoutingWorker_BlackBox_SendOneMessageBetweenDeletedEntities)
{
	const uint32 Delays = 4;

	TArray<Worker_EntityId> Entities;
	for (uint32 i = 0; i < 4 * Delays * 2; ++i)
	{
		Entities.Add((i + 1) * 10);
	}

	TestRoutingFixture TestFixture;
	TestFixture.Init(Entities);

	SpatialGDK::CrossServerRPCService* RPCServiceBackptr = nullptr;

	auto ExtractRPCCallback = [this, &RPCServiceBackptr](const FUnrealObjectRef& Target, const RPCSender& Sender,
														 SpatialGDK::RPCPayload Payload, TOptional<uint64>) {
		RPCServiceBackptr->WriteCrossServerACKFor(Target.Entity, Sender);
	};

	SpatialGDK::FRPCStore RPCStore;
	SpatialGDK::CrossServerRPCService RPCService(TestFixture.CanExtractDelegate, ExtractRPCDelegate::CreateLambda(ExtractRPCCallback),
												 TestFixture.ServerWorker.SubView, RPCStore);

	RPCServiceBackptr = &RPCService;
	RPCService.AdvanceView();
	RPCService.ProcessChanges();

	for (uint32 Attempt = 0; Attempt < 2; ++Attempt)
		for (uint32 CurDelay = 0; CurDelay < Delays; ++CurDelay)
		{
			Worker_EntityId Sender = Entities[2 * (Attempt * Delays + CurDelay) + 0];
			Worker_EntityId Receiver = Entities[2 * (Attempt * Delays + CurDelay) + 1];

			Worker_EntityId ToRemove = (Attempt % 2) == 0 ? Sender : Receiver;
			Worker_EntityId ToCheck = (Attempt % 2) == 1 ? Sender : Receiver;

			RPCPayload DummyPayload(0, 0, {}, TArray<uint8>());
			RPCService.PushCrossServerRPC(Receiver, RPCSender(Sender, 0), DummyPayload, false);

			uint32 Delay = 0;
			bool bHasUpdatesToProcess = true;
			while (bHasUpdatesToProcess)
			{
				bHasUpdatesToProcess = false;
				auto PerformDeletion = [&] {
					EntityComponentOpListBuilder Builder;
					RemoveEntityAndCrossServerComponents(TestFixture.ServerWorker.Coordinator, Builder, ToRemove);
					TestFixture.ServerWorker.Stub->SetFromBuilder(Builder);
					bHasUpdatesToProcess |= TestFixture.ServerWorker.Step(RPCService, RPCStore);

					RemoveEntityAndCrossServerComponents(TestFixture.RoutingWorker.Coordinator, Builder, ToRemove);
					TestFixture.RoutingWorker.Stub->SetFromBuilder(Builder);
					bHasUpdatesToProcess |= TestFixture.RoutingWorker.Step();
					TestFixture.TransferRoutingWorkerUpdates();
					bHasUpdatesToProcess |= TestFixture.ServerWorker.Step(RPCService, RPCStore);
				};

				if (Delay == CurDelay)
				{
					PerformDeletion();
				}

				TestFixture.TransferServerWorkerUpdates(RPCService, RPCStore);
				bHasUpdatesToProcess |= TestFixture.RoutingWorker.Step();
				TestFixture.TransferRoutingWorkerUpdates();
				bHasUpdatesToProcess |= TestFixture.ServerWorker.Step(RPCService, RPCStore);

				++Delay;
			}

			Components Comps(ToCheck, TestFixture.ServerWorker.Coordinator.GetView());
			bool Settled = Comps.CheckSettled();
			if (!Settled)
			{
				FString Debug = FString::Printf(TEXT("Attempt %i for delay %i is not settled"), Attempt, CurDelay);
				TestTrue(Debug, Settled);
			}
		}

	return true;
}

ROUTING_SERVICE_TEST(TestRoutingWorker_BlackBox_SendMoreMessagesThanRingBufferCapacityBetweenSeveralEntities)
{
	TArray<Worker_EntityId> Entities;
	for (uint32 i = 0; i < 4; ++i)
	{
		Entities.Add((i + 1) * 10);
	}

	TestRoutingFixture TestFixture;
	TestFixture.Init(Entities);

	SpatialGDK::CrossServerRPCService* RPCServiceBackptr = nullptr;

	TSet<TPair<Worker_EntityId_Key, uint32>> ExpectedRPCs;

	auto ExtractRPCCallback = [this, &ExpectedRPCs, &RPCServiceBackptr](const FUnrealObjectRef& Target, const RPCSender& Sender,
																		SpatialGDK::RPCPayload Payload, TOptional<uint64>) {
		int32 NumRemoved = ExpectedRPCs.Remove(MakeTuple((Worker_EntityId_Key)Target.Entity, Payload.Index));
		FString DebugText = FString::Printf(TEXT("RPC %i from %llu to %llu was unexpected"), Payload.Index, Sender.Entity, Target.Entity);
		TestTrue(*DebugText, NumRemoved > 0);
		RPCServiceBackptr->WriteCrossServerACKFor(Target.Entity, Sender);
	};

	SpatialGDK::FRPCStore RPCStore;
	SpatialGDK::CrossServerRPCService RPCService(TestFixture.CanExtractDelegate, ExtractRPCDelegate::CreateLambda(ExtractRPCCallback),
												 TestFixture.ServerWorker.SubView, RPCStore);

	RPCServiceBackptr = &RPCService;

	uint32 MaxCapacity = SpatialGDK::RPCRingBufferUtils::GetRingBufferSize(ERPCType::CrossServer);
	RPCService.AdvanceView();
	RPCService.ProcessChanges();

	// Push dummy OpList for the loop.
	TestFixture.TransferRoutingWorkerUpdates();

	TArray<uint32> TargetIdx;
	for (int32 idx = 0; idx < 4; ++idx)
	{
		TargetIdx.Add((idx + 1) % 4);
	}

	// Should take 2 steps to fan out, receive updates and free slots.
	uint32 RPCPerStep = MaxCapacity / 2;
	uint32 RPCAlloc = 0;
	for (uint32 BatchNum = 0; BatchNum < 16; ++BatchNum)
	{
		TestFixture.ServerWorker.Step(RPCService, RPCStore);

		for (uint32 i = 0; i < 4; ++i)
		{
			Worker_EntityId Sender = Entities[i];
			Worker_EntityId Receiver = Entities[TargetIdx[i]];
			for (uint32 j = 0; j < RPCPerStep; ++j)
			{
				RPCPayload DummyPayload(0, RPCAlloc, {}, TArray<uint8>());
				ExpectedRPCs.Add(MakeTuple((Worker_EntityId_Key)Receiver, RPCAlloc));
				++RPCAlloc;
				SpatialGDK::EPushRPCResult Result = RPCService.PushCrossServerRPC(Receiver, RPCSender(Sender, 0), DummyPayload, false);
				TestTrue(TEXT("Did not run out of capacity"), Result == EPushRPCResult::Success);
				do
				{
					TargetIdx[i] = (TargetIdx[i] + 1) % 4;
					Receiver = Entities[TargetIdx[i]];
				} while (Receiver == Sender);
			}
		}
		TestFixture.TransferServerWorkerUpdates(RPCService, RPCStore);
		TestFixture.RoutingWorker.Step();
		TestFixture.TransferRoutingWorkerUpdates();
	}
	bool bHasUpdatesToProcess = true;
	while (bHasUpdatesToProcess)
	{
		bHasUpdatesToProcess = false;
		bHasUpdatesToProcess |= TestFixture.ServerWorker.Step(RPCService, RPCStore);
		TestFixture.TransferServerWorkerUpdates(RPCService, RPCStore);
		bHasUpdatesToProcess |= TestFixture.RoutingWorker.Step();
		TestFixture.TransferRoutingWorkerUpdates();
	}
	TestTrue(TEXT("All RPC were received"), ExpectedRPCs.Num() == 0);

	return true;
}

} // namespace SpatialGDK
