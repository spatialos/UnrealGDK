// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/Array.h"
#include "SpatialView/CommandRequest.h"
#include "SpatialView/ComponentData.h"
#include "SpatialView/ComponentUpdate.h"
#include "SpatialView/OpList/OpList.h"
#include "StringStorage.h"
#include "Templates/UniquePtr.h"

namespace SpatialGDK
{
struct OpListEntity
{
	Worker_EntityId EntityId;
	TArray<ComponentData> Components;
};

// Data for a set of ops representing
struct EntityComponentOpListData : OpListData
{
	TArray<Worker_Op> Ops;
	TArray<ComponentData> DataStorage;
	TArray<ComponentUpdate> UpdateStorage;
	TArray<StringStorage> MessageStorage;
	TArray<TArray<Worker_Entity>> QueriedEntities;
	TArray<TArray<Worker_ComponentData>> QueriedComponents;
};

class EntityComponentOpListBuilder
{
public:
	EntityComponentOpListBuilder();

	EntityComponentOpListBuilder& AddEntity(Worker_EntityId EntityId);
	EntityComponentOpListBuilder& RemoveEntity(Worker_EntityId EntityId);
	EntityComponentOpListBuilder& AddComponent(Worker_EntityId EntityId, ComponentData Data);
	EntityComponentOpListBuilder& UpdateComponent(Worker_EntityId EntityId, ComponentUpdate Update);
	EntityComponentOpListBuilder& RemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId);
	EntityComponentOpListBuilder& SetAuthority(Worker_EntityId EntityId, Worker_ComponentSetId ComponentSetId, Worker_Authority Authority);
	EntityComponentOpListBuilder& SetDisconnect(Worker_ConnectionStatusCode StatusCode, StringStorage DisconnectReason);
	EntityComponentOpListBuilder& AddCreateEntityCommandResponse(Worker_EntityId EntityID, Worker_RequestId RequestId,
																 Worker_StatusCode StatusCode, StringStorage Message);
	EntityComponentOpListBuilder& AddEntityQueryCommandResponse(Worker_RequestId RequestId, TArray<OpListEntity> Results,
																Worker_StatusCode StatusCode, StringStorage Message);
	EntityComponentOpListBuilder& AddEntityCommandRequest(Worker_EntityId EntityID, Worker_RequestId RequestId,
														  CommandRequest CommandRequest);
	EntityComponentOpListBuilder& AddEntityCommandResponse(Worker_EntityId EntityID, Worker_RequestId RequestId,
														   Worker_StatusCode StatusCode, StringStorage Message);
	EntityComponentOpListBuilder& AddDeleteEntityCommandResponse(Worker_EntityId EntityID, Worker_RequestId RequestId,
																 Worker_StatusCode StatusCode, StringStorage Message);
	EntityComponentOpListBuilder& AddReserveEntityIdsCommandResponse(Worker_EntityId EntityID, uint32 NumberOfEntities,
																	 Worker_RequestId RequestId, Worker_StatusCode StatusCode,
																	 StringStorage Message);

	OpList CreateOpList() &&;

private:
	TUniquePtr<EntityComponentOpListData> OpListData;
	const char* StoreString(StringStorage Message) const;
	const Worker_Entity* StoreQueriedEntities(TArray<OpListEntity> Entities) const;
};

} // namespace SpatialGDK
