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

bool RPCService::HasReceiverAuthority(const RPCReceiverDescription& Desc, const EntityViewElement& ViewElement)
{
	return Desc.Authority == NoAuthorityNeeded || ViewElement.Authority.Contains(Desc.Authority);
}

bool RPCService::IsReceiverAuthoritySet(const RPCReceiverDescription& Desc, Worker_ComponentSetId ComponentSet)
{
	return Desc.Authority != NoAuthorityNeeded && ComponentSet == Desc.Authority;
}

void RPCService::ProcessUpdatesToSender(Worker_EntityId EntityId, ComponentSpan<ComponentChange> Updates)
{
	for (const ComponentChange& Change : Updates)
	{
		RPCReadingContext Ctx;
		Ctx.EntityId = EntityId;
		Ctx.ComponentId = Change.ComponentId;
		if (Change.Type == ComponentChange::COMPLETE_UPDATE)
		{
			Ctx.Fields = Schema_GetComponentDataFields(Change.CompleteUpdate.Data);
		}
		else
		{
			Ctx.Update = Change.Update;
			Ctx.Fields = Schema_GetComponentUpdateFields(Change.Update);
		}

		for (auto& QueueEntry : Queues)
		{
			if (QueueEntry.Value.Sender->GetComponentsToReadOnUpdate().Contains(Ctx.ComponentId))
			{
				QueueEntry.Value.Sender->OnUpdate(Ctx);
			}
		}
	}
}

void RPCService::AdvanceSenderQueues()
{
	for (const EntityDelta& Delta : LocalAuthSubView->GetViewDelta().EntityDeltas)
	{
		switch (Delta.Type)
		{
		case EntityDelta::UPDATE:
		{
			ProcessUpdatesToSender(Delta.EntityId, Delta.ComponentUpdates);
			ProcessUpdatesToSender(Delta.EntityId, Delta.ComponentsRefreshed);
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
				if (ensure(ViewElement.Authority.Contains(QueueEntry.Value.Authority)))
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

void RPCService::ProcessUpdatesToReceivers(Worker_EntityId EntityId, const EntityViewElement& ViewElement,
										   ComponentSpan<ComponentChange> Updates)
{
	for (const ComponentChange& Change : Updates)
	{
		RPCReadingContext Ctx;
		Ctx.EntityId = EntityId;
		Ctx.ComponentId = Change.ComponentId;
		if (Change.Type == ComponentChange::COMPLETE_UPDATE)
		{
			Ctx.Fields = Schema_GetComponentDataFields(Change.CompleteUpdate.Data);
		}
		else
		{
			Ctx.Update = Change.Update;
			Ctx.Fields = Schema_GetComponentUpdateFields(Change.Update);
		}

		for (auto& Entry : Receivers)
		{
			if (HasReceiverAuthority(Entry.Value, ViewElement) && Entry.Value.Receiver->GetComponentsToRead().Contains(Ctx.ComponentId))
			{
				Entry.Value.Receiver->OnUpdate(Ctx);
			}
		}
	}
}

void RPCService::HandleReceiverAuthorityGained(Worker_EntityId EntityId, const EntityViewElement& ViewElement,
											   ComponentSpan<AuthorityChange> AuthChanges)
{
	for (const AuthorityChange& Change : AuthChanges)
	{
		for (auto& Entry : Receivers)
		{
			const RPCReceiverDescription& Desc = Entry.Value;
			if (IsReceiverAuthoritySet(Desc, Change.ComponentSetId))
			{
				Entry.Value.Receiver->OnAdded(EntityId, ViewElement);
			}
		}
	}
}

void RPCService::HandleReceiverAuthorityLost(Worker_EntityId EntityId, ComponentSpan<AuthorityChange> AuthChanges)
{
	for (const AuthorityChange& Change : AuthChanges)
	{
		for (auto& Entry : Receivers)
		{
			const RPCReceiverDescription& Desc = Entry.Value;
			if (IsReceiverAuthoritySet(Desc, Change.ComponentSetId))
			{
				Entry.Value.Receiver->OnRemoved(EntityId);
			}
		}
	}
}

void RPCService::AdvanceReceivers()
{
	for (const EntityDelta& Delta : RemoteSubView->GetViewDelta().EntityDeltas)
	{
		switch (Delta.Type)
		{
		case EntityDelta::UPDATE:
		{
			const EntityViewElement& ViewElement = RemoteSubView->GetView().FindChecked(Delta.EntityId);
			ProcessUpdatesToReceivers(Delta.EntityId, ViewElement, Delta.ComponentUpdates);
			ProcessUpdatesToReceivers(Delta.EntityId, ViewElement, Delta.ComponentsRefreshed);

			HandleReceiverAuthorityLost(Delta.EntityId, Delta.AuthorityLost);
			HandleReceiverAuthorityLost(Delta.EntityId, Delta.AuthorityLostTemporarily);
			HandleReceiverAuthorityGained(Delta.EntityId, ViewElement, Delta.AuthorityGained);
			HandleReceiverAuthorityGained(Delta.EntityId, ViewElement, Delta.AuthorityLostTemporarily);

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

RPCCallbacks::DataWritten RPCService::MakeDataWriteCallback(TArray<FWorkerComponentData>& OutArray) const
{
	return [&OutArray](Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentData* InData) {
		if (ensure(InData != nullptr))
		{
			FWorkerComponentData Data;
			Data.component_id = ComponentId;
			Data.schema_type = InData;
			OutArray.Add(Data);
		}
	};
}

RPCCallbacks::UpdateWritten RPCService::MakeUpdateWriteCallback(TArray<UpdateToSend>& OutUpdates) const
{
	return [&OutUpdates](Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentUpdate* InUpdate) {
		if (ensure(InUpdate != nullptr))
		{
			UpdateToSend Update;
			Update.EntityId = EntityId;
			Update.Update.component_id = ComponentId;
			Update.Update.schema_type = InUpdate;
			OutUpdates.Add(Update);
		}
	};
}

TArray<RPCService::UpdateToSend> RPCService::GetRPCsAndAcksToSend()
{
	TArray<UpdateToSend> Updates;

	RPCWritingContext Ctx(MakeUpdateWriteCallback(Updates));

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

TArray<RPCService::UpdateToSend> RPCService::FlushSenderForEntity(Worker_EntityId EntityId)
{
	TArray<UpdateToSend> Updates;

	RPCWritingContext Ctx(MakeUpdateWriteCallback(Updates));

	for (auto& QueueEntry : Queues)
	{
		RPCQueueDescription& Queue = QueueEntry.Value;
		Queue.Queue->Flush(EntityId, Ctx);
	}

	return Updates;
}

TArray<FWorkerComponentData> RPCService::GetRPCComponentsOnEntityCreation(Worker_EntityId EntityId)
{
	TArray<FWorkerComponentData> CreationData;

	RPCWritingContext Ctx(MakeDataWriteCallback(CreationData));

	for (auto& QueueEntry : Queues)
	{
		RPCQueueDescription& Queue = QueueEntry.Value;
		Queue.Queue->Flush(EntityId, Ctx);
	}
	return CreationData;
}

} // namespace SpatialGDK
