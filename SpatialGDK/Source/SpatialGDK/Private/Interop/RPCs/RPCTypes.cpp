// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/RPCs/RPCTypes.h"

namespace SpatialGDK
{
RPCWritingContext::EntityWrite::EntityWrite(EntityWrite&& Write)
	: EntityId(Write.EntityId)
	, ComponentId(Write.ComponentId)
	, Ctx(Write.Ctx)
	, Fields(Write.Fields)
{
	Data = Write.Data;
	Write.bActiveWriter = false;
	Write.Fields = nullptr;
	Write.Data = nullptr;
}

RPCWritingContext::EntityWrite::~EntityWrite()
{
	if (bActiveWriter)
	{
		switch (Ctx.Kind)
		{
		case DataKind::Generic:
			break;
		case DataKind::ComponentData:
			if (Ctx.DataWrittenCallback)
			{
				Ctx.DataWrittenCallback(EntityId, ComponentId, Data);
			}
			break;
		case DataKind::ComponentUpdate:
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
	check(Ctx.Kind == DataKind::ComponentUpdate);
	GetFieldsToWrite();
	return Update;
}

Schema_Object* RPCWritingContext::EntityWrite::GetFieldsToWrite()
{
	if (ensure(bActiveWriter) && Fields == nullptr)
	{
		switch (Ctx.Kind)
		{
		case DataKind::Generic:
			GenData = Schema_CreateGenericData();
			Fields = Schema_GetGenericDataObject(GenData);
			break;
		case DataKind::ComponentData:
			Data = Schema_CreateComponentData();
			Fields = Schema_GetComponentDataFields(Data);
			break;
		case DataKind::ComponentUpdate:
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

void RPCBufferSender::OnAuthGained(Worker_EntityId EntityId, EntityViewElement const& Element)
{
	RPCReadingContext readCtx;
	readCtx.EntityId = EntityId;
	for (const auto& Component : Element.Components)
	{
		if (ComponentsToReadOnAuthGained.Contains(Component.GetComponentId()))
		{
			readCtx.ComponentId = Component.GetComponentId();
			readCtx.Fields = Schema_GetComponentDataFields(Component.GetUnderlying());

			OnAuthGained_ReadComponent(readCtx);
		}
	}
}

void RPCBufferReceiver::OnAdded(Worker_EntityId EntityId, EntityViewElement const& Element)
{
	RPCReadingContext readCtx;
	readCtx.EntityId = EntityId;
	for (const auto& Component : Element.Components)
	{
		if (ComponentsToRead.Contains(Component.GetComponentId()))
		{
			readCtx.ComponentId = Component.GetComponentId();
			readCtx.Fields = Schema_GetComponentDataFields(Component.GetUnderlying());

			OnAdded_ReadComponent(readCtx);
		}
	}
}

void RPCQueue::OnAuthGained(Worker_EntityId EntityId, EntityViewElement const& Element)
{
	RPCReadingContext readCtx;
	readCtx.EntityId = EntityId;
	for (const auto& Component : Element.Components)
	{
		if (ComponentsToReadOnAuthGained.Contains(Component.GetComponentId()))
		{
			readCtx.ComponentId = Component.GetComponentId();
			readCtx.Fields = Schema_GetComponentDataFields(Component.GetUnderlying());

			OnAuthGained_ReadComponent(readCtx);
		}
	}
}
} // namespace SpatialGDK
