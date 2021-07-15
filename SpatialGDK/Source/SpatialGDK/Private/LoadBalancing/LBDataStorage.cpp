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

		SpatialGDK::Coordinates Coords = GetCoordinateFromSchema(PositionObject, 1);
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
		SpatialGDK::Coordinates Coords = GetCoordinateFromSchema(PositionObject, 1);

		FVector* Data = Positions.Find(EntityId);
		if (!ensure(Data != nullptr))
		{
			return;
		}
		*Data = SpatialGDK::Coordinates::ToFVector(Coords);

		Modified.Add(EntityId);
	}
}

} // namespace SpatialGDK
