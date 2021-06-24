#include "LoadBalancing/LBDataStorage.h"
#include "Schema/StandardLibrary.h"
#include "Utils/SchemaUtils.h"

namespace SpatialGDK
{
void FLBDataCollection::Advance()
{
	for (const EntityDelta& Delta : SubView.GetViewDelta().EntityDeltas)
	{
		switch (Delta.Type)
		{
		case EntityDelta::UPDATE:
		{
			for (const auto& CompleteUpdate : Delta.ComponentsRefreshed)
			{
				for (auto& Storage : DataStorages)
				{
					if (Storage->GetComponentsToWatch().Contains(CompleteUpdate.ComponentId))
					{
						Storage->OnComponentRefreshed(Delta.EntityId, CompleteUpdate.ComponentId, CompleteUpdate.Data);
					}
				}
			}
			for (const auto& Update : Delta.ComponentUpdates)
			{
				for (auto& Storage : DataStorages)
				{
					if (Storage->GetComponentsToWatch().Contains(Update.ComponentId))
					{
						Storage->OnUpdate(Delta.EntityId, Update.ComponentId, Update.Update);
					}
				}
			}
		}
		break;
		case EntityDelta::ADD:
		{
			const SpatialGDK::EntityViewElement& Element = SubView.GetView().FindChecked(Delta.EntityId);
			for (auto& Storage : DataStorages)
			{
				Storage->OnAdded(Delta.EntityId, Element);
			}
		}
		break;
		case EntityDelta::REMOVE:
		{
			for (auto& Storage : DataStorages)
			{
				Storage->OnRemoved(Delta.EntityId);
			}
		}
		break;
		case EntityDelta::TEMPORARILY_REMOVED:
		{
			for (auto& Storage : DataStorages)
			{
				Storage->OnRemoved(Delta.EntityId);
			}
			const SpatialGDK::EntityViewElement& Element = SubView.GetView().FindChecked(Delta.EntityId);
			for (auto& Storage : DataStorages)
			{
				Storage->OnAdded(Delta.EntityId, Element);
			}
		}
		break;
		default:
			break;
		}
	}
}

TSet<Worker_ComponentId> FLBDataCollection::GetComponentsToWatch() const
{
	TSet<Worker_ComponentId> Components;
	for (auto Storage : DataStorages)
	{
		Components = Components.Union(Storage->GetComponentsToWatch());
	}

	return Components;
}

FSpatialPositionStorage::FSpatialPositionStorage()
{
	Components.Add(SpatialConstants::POSITION_COMPONENT_ID);
}

void FSpatialPositionStorage::OnComponentAdded(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentData* Data)
{
	if (ensureAlways(SpatialConstants::POSITION_COMPONENT_ID == ComponentId))
	{
		Schema_Object* PositionObject = Schema_GetComponentDataFields(Data);

		Schema_Object* CoordsObject = Schema_GetObject(PositionObject, 1);
		SpatialGDK::Coordinates Coords;
		Coords.X = Schema_GetDouble(CoordsObject, 1);
		Coords.Y = Schema_GetDouble(CoordsObject, 2);
		Coords.Z = Schema_GetDouble(CoordsObject, 3);

		FVector Position = SpatialGDK::Coordinates::ToFVector(Coords);

		Modified.Add(EntityId);

		Positions.Add(EntityId, Position);
	}
}

void FSpatialPositionStorage::OnRemoved(Worker_EntityId EntityId)
{
	Modified.Remove(EntityId);
	Positions.Remove(EntityId);
}

void FSpatialPositionStorage::OnUpdate(Worker_EntityId EntityId, Worker_ComponentId InComponentId, Schema_ComponentUpdate* Update)
{
	if (InComponentId == SpatialConstants::POSITION_COMPONENT_ID)
	{
		Schema_Object* PositionObject = Schema_GetComponentUpdateFields(Update);
		Schema_Object* CoordsObject = Schema_GetObject(PositionObject, 1);
		SpatialGDK::Coordinates Coords;
		Coords.X = Schema_GetDouble(CoordsObject, 1);
		Coords.Y = Schema_GetDouble(CoordsObject, 2);
		Coords.Z = Schema_GetDouble(CoordsObject, 3);

		FVector* Data = Positions.Find(EntityId);
		if (!ensure(Data != nullptr))
		{
			return;
		}
		*Data = SpatialGDK::Coordinates::ToFVector(Coords);

		Modified.Add(EntityId);
	}
}

FActorGroupStorage::FActorGroupStorage()
{
	Components.Add(SpatialConstants::ACTOR_GROUP_MEMBER_COMPONENT_ID);
}

void FActorGroupStorage::OnComponentAdded(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentData* Data)
{
	if (ensureAlways(SpatialConstants::ACTOR_GROUP_MEMBER_COMPONENT_ID == ComponentId))
	{
		Schema_Object* GroupObject = Schema_GetComponentDataFields(Data);
		int32 GroupId = Schema_GetUint32(GroupObject, SpatialConstants::ACTOR_GROUP_MEMBER_COMPONENT_ACTOR_GROUP_ID);
		Groups.Add(EntityId, GroupId);
		Modified.Add(EntityId);
	}
}

void FActorGroupStorage::OnRemoved(Worker_EntityId EntityId)
{
	Groups.Remove(EntityId);
	Modified.Remove(EntityId);
}

void FActorGroupStorage::OnUpdate(Worker_EntityId EntityId, Worker_ComponentId InComponentId, Schema_ComponentUpdate* Update)
{
	Schema_Object* GroupObject = Schema_GetComponentUpdateFields(Update);
	int32 GroupId = Schema_GetUint32(GroupObject, SpatialConstants::ACTOR_GROUP_MEMBER_COMPONENT_ACTOR_GROUP_ID);
	Groups.Add(EntityId, GroupId);
	Modified.Add(EntityId);
}

} // namespace SpatialGDK
