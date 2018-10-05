// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView.h"

#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/SpatialReceiver.h"
#include "Interop/SpatialSender.h"

DEFINE_LOG_CATEGORY(LogSpatialView);

void USpatialView::Init(USpatialNetDriver* InNetDriver)
{
	NetDriver = InNetDriver;
	Receiver = InNetDriver->Receiver;
	Sender = InNetDriver->Sender;
}

void USpatialView::ProcessOps(Worker_OpList* OpList)
{
	TArray<Worker_Op*> QueuedComponentUpdateOps;

	for (size_t i = 0; i < OpList->op_count; ++i)
	{
		Worker_Op* Op = &OpList->ops[i];
		switch (Op->op_type)
		{
		// Critical Section
		case WORKER_OP_TYPE_CRITICAL_SECTION:
			Receiver->OnCriticalSection(Op->critical_section.in_critical_section != 0);
			break;

		// Entity Lifetime
		case WORKER_OP_TYPE_ADD_ENTITY:
			Receiver->OnAddEntity(Op->add_entity);
			break;
		case WORKER_OP_TYPE_REMOVE_ENTITY:
			Receiver->OnRemoveEntity(Op->remove_entity);
			OnRemoveEntity(Op->remove_entity);
			break;

		// Components
		case WORKER_OP_TYPE_ADD_COMPONENT:
			OnAddComponent(Op->add_component);
			Receiver->OnAddComponent(Op->add_component);
			break;
		case WORKER_OP_TYPE_REMOVE_COMPONENT:
			break;
		case WORKER_OP_TYPE_COMPONENT_UPDATE:
			QueuedComponentUpdateOps.Add(Op);
			OnComponentUpdate(Op->component_update);
			break;

		// Commands
		case WORKER_OP_TYPE_COMMAND_REQUEST:
			Receiver->OnCommandRequest(Op->command_request);
			break;
		case WORKER_OP_TYPE_COMMAND_RESPONSE:
			Receiver->OnCommandResponse(Op->command_response);
			break;

		// Authority Change
		case WORKER_OP_TYPE_AUTHORITY_CHANGE:
			OnAuthorityChange(Op->authority_change);
			Receiver->OnAuthorityChange(Op->authority_change);
			break;

		// World Command Responses
		case WORKER_OP_TYPE_RESERVE_ENTITY_ID_RESPONSE:
			Receiver->OnReserveEntityIdResponse(Op->reserve_entity_id_response);
			break;
		case WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE:
			break;
		case WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE:
			Receiver->OnCreateEntityIdResponse(Op->create_entity_response);
			break;
		case WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE:
			break;
		case WORKER_OP_TYPE_ENTITY_QUERY_RESPONSE:
			break;

		case WORKER_OP_TYPE_FLAG_UPDATE:
			break;
		case WORKER_OP_TYPE_LOG_MESSAGE:
			UE_LOG(LogSpatialView, Log, TEXT("SpatialOS Worker Log: %s"), UTF8_TO_TCHAR(Op->log_message.message));
			break;
		case WORKER_OP_TYPE_METRICS:
			break;
		case WORKER_OP_TYPE_DISCONNECT:
			UE_LOG(LogSpatialView, Warning, TEXT("Disconnecting from SpatialOS: %s"), UTF8_TO_TCHAR(Op->disconnect.reason));
			break;

		default:
			break;
		}
	}

	for (Worker_Op* Op : QueuedComponentUpdateOps)
	{
		Receiver->OnComponentUpdate(Op->component_update);
	}

	Receiver->ProcessQueuedResolvedObjects();

	// Check every channel for net ownership changes (determines ACL and component interest)
	for (auto& Pair : NetDriver->GetSpatialOSNetConnection()->ActorChannels)
	{
		if (USpatialActorChannel* Channel = Cast<USpatialActorChannel>(Pair.Value))
		{
			Channel->SpatialViewTick();
		}
	}
}

Worker_Authority USpatialView::GetAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	if (TMap<Worker_ComponentId, Worker_Authority>* ComponentAuthorityMap = EntityComponentAuthorityMap.Find(EntityId))
	{
		if (Worker_Authority* Authority = ComponentAuthorityMap->Find(ComponentId))
		{
			return *Authority;
		}
	}

	return WORKER_AUTHORITY_NOT_AUTHORITATIVE;
}

template <typename T>
typename T* USpatialView::GetComponentData(Worker_EntityId EntityId)
{
	if (TMap<Worker_ComponentId, TSharedPtr<ComponentStorageBase>>* ComponentStorageMap = EntityComponentMap.Find(EntityId))
	{
		if (TSharedPtr<improbable::ComponentStorageBase>* Component = ComponentStorageMap->Find(T::ComponentId))
		{
			return &static_cast<improbable::ComponentStorage<T>&>(*(Component->Get())).Get();
		}
	}

	return nullptr;
}

void USpatialView::OnAddComponent(const Worker_AddComponentOp& Op)
{
	TSharedPtr<improbable::ComponentStorageBase> Data;
	switch (Op.data.component_id)
	{
	case SpatialConstants::ENTITY_ACL_COMPONENT_ID:
		Data = MakeShared<improbable::ComponentStorage<improbable::EntityAcl>>(Op.data);
		break;
	case SpatialConstants::METADATA_COMPONENT_ID:
		Data = MakeShared<improbable::ComponentStorage<improbable::Metadata>>(Op.data);
		break;
	case SpatialConstants::POSITION_COMPONENT_ID:
		Data = MakeShared<improbable::ComponentStorage<improbable::Position>>(Op.data);
		break;
	case SpatialConstants::PERSISTENCE_COMPONENT_ID:
		Data = MakeShared<improbable::ComponentStorage<improbable::Persistence>>(Op.data);
		break;
	case SpatialConstants::ROTATION_COMPONENT_ID:
		Data = MakeShared<improbable::ComponentStorage<improbable::Rotation>>(Op.data);
		break;
	case SpatialConstants::UNREAL_METADATA_COMPONENT_ID:
		Data = MakeShared<improbable::ComponentStorage<improbable::UnrealMetadata>>(Op.data);
		break;
	default:
		return;
	}

	EntityComponentMap.FindOrAdd(Op.entity_id).FindOrAdd(Op.data.component_id) = Data;
}

void USpatialView::OnRemoveEntity(const Worker_RemoveEntityOp& Op)
{
	EntityComponentMap.Remove(Op.entity_id);
}

void USpatialView::OnComponentUpdate(const Worker_ComponentUpdateOp& Op)
{
	switch (Op.update.component_id)
	{
	case SpatialConstants::ENTITY_ACL_COMPONENT_ID:
		if (improbable::EntityAcl* aclComponent = GetComponentData<improbable::EntityAcl>(Op.entity_id))
		{
			aclComponent->ApplyComponentUpdate(Op.update);
		}
		break;
	case SpatialConstants::POSITION_COMPONENT_ID:
		if (improbable::Position* positionComponent = GetComponentData<improbable::Position>(Op.entity_id))
		{
			positionComponent->ApplyComponentUpdate(Op.update);
		}
		break;
	case SpatialConstants::ROTATION_COMPONENT_ID:
		if (improbable::Rotation* rotationComponent = GetComponentData<improbable::Rotation>(Op.entity_id))
		{
			rotationComponent->ApplyComponentUpdate(Op.update);
		}
		break;
	default:
		return;
	}
}

void USpatialView::OnAuthorityChange(const Worker_AuthorityChangeOp& Op)
{
	EntityComponentAuthorityMap.FindOrAdd(Op.entity_id).FindOrAdd(Op.component_id) = (Worker_Authority)Op.authority;
}
