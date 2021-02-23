// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/RPCs/RPCService.h"

namespace SpatialGDK
{
RPCService::RPCService(const FSubView& InRemoteSubView, const FSubView& InLocalAuthSubView)
	: RemoteSubView(&InRemoteSubView)
	, LocalAuthSubView(&InLocalAuthSubView)
{
}

void RPCService::AddRPCQueue(FName QueueName, RPCQueueDescription&& Desc)
{
	Queues.Add(QueueName, MoveTemp(Desc));
}

void RPCService::AddRPCReceiver(FName ReceiverName, RPCReceiverDescription&& Desc)
{
	Receivers.Add(ReceiverName, MoveTemp(Desc));
}

void RPCService::AdvanceSenderQueues()
{
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
			checkNoEntry();
			break;
		}
	}
}

void RPCService::AdvanceReceivers()
{
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
		default:
			checkNoEntry();
			break;
		}
	}
}

void RPCService::AdvanceView()
{
	AdvanceReceivers();
	AdvanceSenderQueues();
}

TArray<RPCService::UpdateToSend> RPCService::GetRPCsAndAcksToSend()
{
	TArray<UpdateToSend> Updates;

	RPCWritingContext Ctx(RPCWritingContext::DataKind::ComponentUpdate);
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

TArray<FWorkerComponentData> RPCService::GetRPCComponentsOnEntityCreation(Worker_EntityId EntityId)
{
	TArray<FWorkerComponentData> CreationData;

	RPCWritingContext Ctx(RPCWritingContext::DataKind::ComponentData);
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

} // namespace SpatialGDK
