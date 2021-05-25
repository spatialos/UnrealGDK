#include "LoadBalancing/LBDataStorage.h"
#include "Schema/StandardLibrary.h"
#include "Utils/SchemaUtils.h"

namespace SpatialGDK
{
void ComputeFieldAndComponents(USpatialNetDriver* NetDriver, UClass* Class, GDK_PROPERTY(Property) * Property,
							   TSet<Worker_ComponentId>& Components, Schema_FieldId& FieldId)
{
	// for (TObjectIterator< UClass > ClassIt; ClassIt; ++ClassIt)
	//{
	//	UClass* Class = *ClassIt;
	//	if (!Class->IsChildOf(Class))
	//	{
	//		continue;
	//	}
	//	Worker_ComponentId Component = NetDriver->ClassInfoManager->GetComponentIdForClass(*Class);
	//	if (Component != 0)
	//	{
	//		Components.Add(Component);
	//	}
	//}
	USchemaDatabase* Database = NetDriver->ClassInfoManager->SchemaDatabase;
	for (auto const& ComponentEntry : Database->ComponentIdToClassPath)
	{
		FString ClassPath = ComponentEntry.Value;
		UClass* ActorClass = StaticLoadClass(AActor::StaticClass(), nullptr, *ClassPath);
		if (ActorClass && ActorClass->IsChildOf(Class))
		{
			Components.Add(ComponentEntry.Key);
		}

		// UClass* ActorComponentClass = StaticLoadClass(UActorComponent::StaticClass(), nullptr, *ClassPath);
		// if()
	}

	if (Components.Num() > 0)
	{
		FObjectReferencesMap Map;
		ComponentReader Reader(NetDriver, Map, nullptr);
		FieldId = Reader.GetFieldFromProperty(Class, Property, *Components.CreateConstIterator());
	}
}

FSpatialPositionStorage::FSpatialPositionStorage()
{
	Components.Add(SpatialConstants::POSITION_COMPONENT_ID);
}

void FSpatialPositionStorage::OnAdded(Worker_EntityId EntityId, SpatialGDK::EntityViewElement const& Element)
{
	for (const auto& Component : Element.Components)
	{
		if (SpatialConstants::POSITION_COMPONENT_ID == Component.GetComponentId())
		{
			Schema_Object* PositionObject = Schema_GetComponentDataFields(Component.GetUnderlying());

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

void FActorGroupStorage::OnAdded(Worker_EntityId EntityId, SpatialGDK::EntityViewElement const& Element)
{
	for (const auto& Component : Element.Components)
	{
		if (SpatialConstants::ACTOR_GROUP_MEMBER_COMPONENT_ID == Component.GetComponentId())
		{
			Schema_Object* GroupObject = Schema_GetComponentDataFields(Component.GetUnderlying());
			int32 GroupId = Schema_GetUint32(GroupObject, SpatialConstants::ACTOR_GROUP_MEMBER_COMPONENT_ACTOR_GROUP_ID);
			Groups.Add(EntityId, GroupId);
			Modified.Add(EntityId);
		}
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

FDirectAssignmentStorage::FDirectAssignmentStorage()
{
	Components.Add(SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID);
}

void FDirectAssignmentStorage::OnAdded(Worker_EntityId EntityId, SpatialGDK::EntityViewElement const& Element)
{
	for (const auto& Component : Element.Components)
	{
		if (SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID == Component.GetComponentId())
		{
			AuthorityIntent NewIntent(Component.GetUnderlying());
			Intents.Add(EntityId, NewIntent);
			Modified.Add(EntityId);
		}
	}
}

void FDirectAssignmentStorage::OnRemoved(Worker_EntityId EntityId)
{
	Intents.Remove(EntityId);
	Modified.Remove(EntityId);
}

void FDirectAssignmentStorage::OnUpdate(Worker_EntityId EntityId, Worker_ComponentId InComponentId, Schema_ComponentUpdate* Update)
{
	AuthorityIntent* Intent = Intents.Find(EntityId);
	if (!ensure(Intent != nullptr))
	{
		return;
	}

	Intent->ApplyComponentUpdate(Update);
	Modified.Add(EntityId);
}

} // namespace SpatialGDK
