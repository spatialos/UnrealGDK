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
	FSpatialEntityId EntityId;
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
	TArray<TArray<Worker_ComponentData>> ComponentArrayStorage;
};

class EntityComponentOpListBuilder
{
public:
	EntityComponentOpListBuilder();

	EntityComponentOpListBuilder& AddEntity(FSpatialEntityId EntityId);
	EntityComponentOpListBuilder& RemoveEntity(FSpatialEntityId EntityId);
	EntityComponentOpListBuilder& AddComponent(FSpatialEntityId EntityId, ComponentData Data);
	EntityComponentOpListBuilder& UpdateComponent(FSpatialEntityId EntityId, ComponentUpdate Update);
	EntityComponentOpListBuilder& RemoveComponent(FSpatialEntityId EntityId, Worker_ComponentId ComponentId);
	EntityComponentOpListBuilder& SetAuthority(FSpatialEntityId EntityId, Worker_ComponentSetId ComponentSetId, Worker_Authority Authority,
											   TArray<ComponentData> Components);
	EntityComponentOpListBuilder& SetDisconnect(Worker_ConnectionStatusCode StatusCode, StringStorage DisconnectReason);
	EntityComponentOpListBuilder& StartCriticalSection();
	EntityComponentOpListBuilder& EndCriticalSection();
	EntityComponentOpListBuilder& AddCreateEntityCommandResponse(FSpatialEntityId EntityID, Worker_RequestId RequestId,
																 Worker_StatusCode StatusCode, StringStorage Message);
	EntityComponentOpListBuilder& AddEntityQueryCommandResponse(Worker_RequestId RequestId, TArray<OpListEntity> Results,
																Worker_StatusCode StatusCode, StringStorage Message);
	EntityComponentOpListBuilder& AddEntityCommandRequest(FSpatialEntityId EntityID, Worker_RequestId RequestId,
														  CommandRequest CommandRequest);
	EntityComponentOpListBuilder& AddEntityCommandResponse(FSpatialEntityId EntityID, Worker_RequestId RequestId,
														   Worker_StatusCode StatusCode, StringStorage Message);
	EntityComponentOpListBuilder& AddDeleteEntityCommandResponse(FSpatialEntityId EntityID, Worker_RequestId RequestId,
																 Worker_StatusCode StatusCode, StringStorage Message);
	EntityComponentOpListBuilder& AddReserveEntityIdsCommandResponse(FSpatialEntityId EntityID, uint32 NumberOfEntities,
																	 Worker_RequestId RequestId, Worker_StatusCode StatusCode,
																	 StringStorage Message);

	OpList CreateOpList() &&;

private:
	TUniquePtr<EntityComponentOpListData> OpListData;
	const char* StoreString(StringStorage Message) const;
	const Worker_Entity* StoreQueriedEntities(TArray<OpListEntity> Entities) const;
	const Worker_ComponentData* StoreComponentDataArray(TArray<ComponentData> Components) const;
};

} // namespace SpatialGDK
