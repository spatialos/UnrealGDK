// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/RPCs/SpatialRPCService_2.h"

#include "EngineClasses/SpatialNetDriver.h"

#include "SpatialGDKSettings.h"

DEFINE_LOG_CATEGORY(LogSpatialRPCService_2);

namespace SpatialGDK
{
RPCWritingContext::EntityWrite::EntityWrite(EntityWrite&& Write)
	: Ctx(Write.Ctx)
	, EntityId(Write.EntityId)
	, ComponentId(Write.ComponentId)
{
	Write.bActiveWriter = false;
}

RPCWritingContext::EntityWrite::~EntityWrite()
{
	if (bActiveWriter)
	{
		switch (Ctx.Kind)
		{
		case DataKind::Generic:
			break;
		case DataKind::Data:
			if (Ctx.DataWrittenCallback)
			{
				Ctx.DataWrittenCallback(EntityId, ComponentId, Data);
			}
			break;
		case DataKind::Update:
			if (Ctx.UpdateWrittenCallback)
			{
				Ctx.UpdateWrittenCallback(EntityId, ComponentId, Update);
			}
			break;
		case DataKind::CommandRequest:
			if (Ctx.RequestWrittenCallback)
			{
				Ctx.RequestWrittenCallback(EntityId, Request);
			}
			break;
		case DataKind::CommandResponse:
			if (Ctx.ResponseWrittenCallback)
			{
				Ctx.ResponseWrittenCallback(EntityId, Response);
			}
			break;
		}
	}
}

Schema_ComponentUpdate* RPCWritingContext::EntityWrite::GetComponentUpdateToWrite()
{
	check(Ctx.Kind == DataKind::Update);
	GetFieldsToWrite();
	return Update;
}

Schema_Object* RPCWritingContext::EntityWrite::GetFieldsToWrite()
{
	if (Fields == nullptr)
	{
		switch (Ctx.Kind)
		{
		case DataKind::Generic:
			GenData = Schema_CreateGenericData();
			Fields = Schema_GetGenericDataObject(GenData);
			break;
		case DataKind::Data:
			Data = Schema_CreateComponentData();
			Fields = Schema_GetComponentDataFields(Data);
			break;
		case DataKind::Update:
			Update = Schema_CreateComponentUpdate();
			Fields = Schema_GetComponentUpdateFields(Update);
			break;
		case DataKind::CommandRequest:
			Request = Schema_CreateCommandRequest();
			Fields = Schema_GetCommandRequestObject(Request);
			break;
		case DataKind::CommandResponse:
			Response = Schema_CreateCommandResponse();
			Fields = Schema_GetCommandResponseObject(Response);
			break;
		}
	}
	return Fields;
}

void RPCWritingContext::EntityWrite::RPCWritten(uint32 RPCId)
{
	if (Ctx.RPCWrittenCallback)
	{
		Ctx.RPCWrittenCallback(EntityId, ComponentId, RPCId);
	}
}

RPCWritingContext::EntityWrite::EntityWrite(RPCWritingContext& InCtx, Worker_EntityId InEntityId, Worker_ComponentId InComponentID)
	: EntityId(InEntityId)
	, ComponentId(InComponentID)
	, Ctx(InCtx)
{
}

RPCWritingContext::EntityWrite RPCWritingContext::WriteTo(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	return EntityWrite(*this, EntityId, ComponentId);
}

RPCWritingContext::RPCWritingContext(DataKind InKind)
	: Kind(InKind)
{
}

SpatialRPCService_2::SpatialRPCService_2(const FSubView& InRemoteSubView,
										 const FSubView& InLocalAuthSubView
										 /*, USpatialLatencyTracer* InSpatialLatencyTracer, SpatialEventTracer* InEventTracer*/
										 ,
										 USpatialNetDriver* InNetDriver)
	: RemoteSubView(&InRemoteSubView)
	, LocalAuthSubView(&InLocalAuthSubView)
{
}

void SpatialRPCService_2::AddRPCQueue(FName QueueName, RPCQueueDescription&& Desc)
{
	Queues.Add(QueueName, MoveTemp(Desc));
}

void SpatialRPCService_2::AddRPCReceiver(FName ReceiverName, RPCReceiverDescription&& Desc)
{
	Receivers.Add(ReceiverName, MoveTemp(Desc));
}

void SpatialRPCService_2::AdvanceView()
{
	// const FSubViewDelta* ViewDeltas[] = { &AuthSubView->GetViewDelta(), &ActorSubView->GetViewDelta() };

	auto HasReceiverAuthority = [](const RPCReceiverDescription& Desc, const EntityViewElement& ViewElement) {
		return Desc.Authority == 0 || ViewElement.Authority.Contains(Desc.Authority);
	};

	for (const EntityDelta& Delta : RemoteSubView->GetViewDelta().EntityDeltas)
	{
		switch (Delta.Type)
		{
		case EntityDelta::UPDATE:
		{
			const EntityViewElement& ViewElement = RemoteSubView->GetView().FindChecked(Delta.EntityId);
			for (const ComponentChange& Change : Delta.ComponentUpdates)
			{
				RPCReadingContext Ctx;
				Ctx.EntityId = Delta.EntityId;
				Ctx.ComponentId = Change.ComponentId;
				Ctx.Update = Change.Update;
				Ctx.Fields = Schema_GetComponentUpdateFields(Change.Update);

				for (auto& Entry : Receivers)
				{
					if (HasReceiverAuthority(Entry.Value, ViewElement))
					{
						Entry.Value.Receiver->OnUpdate(Ctx);
					}
				}
			}
			for (const ComponentChange& Change : Delta.ComponentsRefreshed)
			{
				RPCReadingContext Ctx;
				Ctx.EntityId = Delta.EntityId;
				Ctx.ComponentId = Change.ComponentId;
				Ctx.Fields = Schema_GetComponentDataFields(Change.CompleteUpdate.Data);

				for (auto& Entry : Receivers)
				{
					if (HasReceiverAuthority(Entry.Value, ViewElement))
					{
						Entry.Value.Receiver->OnUpdate(Ctx);
					}
				}
			}

			for (const AuthorityChange& Change : Delta.AuthorityLost)
			{
				for (auto& Entry : Receivers)
				{
					if (!HasReceiverAuthority(Entry.Value, ViewElement))
					{
						Entry.Value.Receiver->OnRemoved(Delta.EntityId);
					}
				}
			}
			for (const AuthorityChange& Change : Delta.AuthorityLostTemporarily)
			{
				for (auto& Entry : Receivers)
				{
					if (!HasReceiverAuthority(Entry.Value, ViewElement))
					{
						Entry.Value.Receiver->OnRemoved(Delta.EntityId);
					}
				}
			}
			for (const AuthorityChange& Change : Delta.AuthorityGained)
			{
				for (auto& Entry : Receivers)
				{
					if (HasReceiverAuthority(Entry.Value, ViewElement))
					{
						Entry.Value.Receiver->OnAdded(Delta.EntityId, ViewElement);
					}
				}
			}
			for (const AuthorityChange& Change : Delta.AuthorityLostTemporarily)
			{
				for (auto& Entry : Receivers)
				{
					if (HasReceiverAuthority(Entry.Value, ViewElement))
					{
						Entry.Value.Receiver->OnAdded(Delta.EntityId, ViewElement);
					}
				}
			}

			break;
		}
		case EntityDelta::ADD:
		{
			const EntityViewElement& ViewElement = RemoteSubView->GetView().FindChecked(Delta.EntityId);
			for (auto& Entry : Receivers)
			{
				if (HasReceiverAuthority(Entry.Value, ViewElement))
				{
					Entry.Value.Receiver->OnAdded(Delta.EntityId, ViewElement);
				}
			}
		}
		break;
		case EntityDelta::REMOVE:
			for (auto& Entry : Receivers)
			{
				Entry.Value.Receiver->OnRemoved(Delta.EntityId);
			}
			break;
		case EntityDelta::TEMPORARILY_REMOVED:
		{
			const EntityViewElement& ViewElement = RemoteSubView->GetView().FindChecked(Delta.EntityId);
			for (auto& Entry : Receivers)
			{
				Entry.Value.Receiver->OnRemoved(Delta.EntityId);
				if (HasReceiverAuthority(Entry.Value, ViewElement))
				{
					Entry.Value.Receiver->OnAdded(Delta.EntityId, ViewElement);
				}
			}
		}
		}
	}

	for (const EntityDelta& Delta : LocalAuthSubView->GetViewDelta().EntityDeltas)
	{
		switch (Delta.Type)
		{
		case EntityDelta::UPDATE:
		{
			for (const ComponentChange& Change : Delta.ComponentUpdates)
			{
				RPCReadingContext Ctx;
				Ctx.EntityId = Delta.EntityId;
				Ctx.ComponentId = Change.ComponentId;
				Ctx.Update = Change.Update;
				Ctx.Fields = Schema_GetComponentUpdateFields(Change.Update);

				for (auto& QueueEntry : Queues)
				{
					QueueEntry.Value.Sender->OnUpdate(Ctx);
				}
			}
			for (const ComponentChange& Change : Delta.ComponentsRefreshed)
			{
				RPCReadingContext Ctx;
				Ctx.EntityId = Delta.EntityId;
				Ctx.ComponentId = Change.ComponentId;
				Ctx.Fields = Schema_GetComponentDataFields(Change.CompleteUpdate.Data);

				for (auto& QueueEntry : Queues)
				{
					QueueEntry.Value.Sender->OnUpdate(Ctx);
				}
			}
			break;
		}
		case EntityDelta::ADD:
		{
			const EntityViewElement& ViewElement = LocalAuthSubView->GetView().FindChecked(Delta.EntityId);
			for (auto& QueueEntry : Queues)
			{
				if (ViewElement.Authority.Contains(QueueEntry.Value.Authority))
				{
					QueueEntry.Value.Queue->OnAuthGained(Delta.EntityId, ViewElement);
					QueueEntry.Value.Sender->OnAuthGained(Delta.EntityId, ViewElement);
				}
			}
		}
		break;
		case EntityDelta::REMOVE:
			for (auto& QueueEntry : Queues)
			{
				QueueEntry.Value.Queue->OnAuthLost(Delta.EntityId);
				QueueEntry.Value.Sender->OnAuthLost(Delta.EntityId);
			}
			break;
		case EntityDelta::TEMPORARILY_REMOVED:
		{
			const EntityViewElement& ViewElement = LocalAuthSubView->GetView().FindChecked(Delta.EntityId);
			for (auto& QueueEntry : Queues)
			{
				QueueEntry.Value.Queue->OnAuthLost(Delta.EntityId);
				QueueEntry.Value.Sender->OnAuthLost(Delta.EntityId);
				if (ViewElement.Authority.Contains(QueueEntry.Value.Authority))
				{
					QueueEntry.Value.Queue->OnAuthGained(Delta.EntityId, ViewElement);
					QueueEntry.Value.Sender->OnAuthGained(Delta.EntityId, ViewElement);
				}
			}
		}
		break;
		default:
			break;
		}
	}
}

TArray<SpatialRPCService_2::UpdateToSend> SpatialRPCService_2::GetRPCsAndAcksToSend()
{
	TArray<UpdateToSend> Updates;

	RPCWritingContext Ctx(RPCWritingContext::DataKind::Update);
	Ctx.UpdateWrittenCallback = [&Updates](Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentUpdate* InUpdate) {
		if (ensure(InUpdate != nullptr))
		{
			UpdateToSend Update;
			Update.EntityId = EntityId;
			Update.Update.component_id = ComponentId;
			Update.Update.schema_type = InUpdate;
			Updates.Add(Update);
		}
	};

	Ctx.RPCWrittenCallback = [](Worker_EntityId Entity, Worker_ComponentId ComponentId, uint32 RPCId) {
		// if (EventTracer != nullptr)
		//{
		//	EventTraceUniqueId LinearTraceId = EventTraceUniqueId::GenerateForRPC(EntityId, static_cast<uint8>(Type), NewRPCId);
		//	FSpatialGDKSpanId SpanId = EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateSendRPC(LinearTraceId),
		//		/* Causes */ Payload.SpanId.GetConstId(), /* NumCauses */ 1);
		//	RPCStore.AddSpanIdForComponentUpdate(EntityComponent, SpanId);
		//}
	};

	for (auto& QueueEntry : Queues)
	{
		RPCQueueDescription& Queue = QueueEntry.Value;
		Queue.Queue->FlushAll(Ctx);
	}

	for (auto& Entry : Receivers)
	{
		RPCReceiverDescription& Receiver = Entry.Value;
		Receiver.Receiver->FlushUpdates(Ctx);
	}

	return Updates;
}

TArray<FWorkerComponentData> SpatialRPCService_2::GetRPCComponentsOnEntityCreation(Worker_EntityId EntityId)
{
	TArray<FWorkerComponentData> CreationData;

	RPCWritingContext Ctx(RPCWritingContext::DataKind::Data);
	Ctx.DataWrittenCallback = [&CreationData](Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentData* InData) {
		if (ensure(InData != nullptr))
		{
			FWorkerComponentData Data;
			Data.component_id = ComponentId;
			Data.schema_type = InData;
			CreationData.Add(Data);
		}
	};

	for (auto& QueueEntry : Queues)
	{
		RPCQueueDescription& Queue = QueueEntry.Value;
		Queue.Queue->Flush(EntityId, Ctx);
	}
	return CreationData;
}

void SpatialRPCService_2::ClearPendingRPCs(Worker_EntityId EntityId) {}

} // namespace SpatialGDK
