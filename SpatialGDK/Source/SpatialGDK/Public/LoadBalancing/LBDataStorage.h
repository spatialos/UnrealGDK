#pragma once

#include "EngineClasses/SpatialNetDriver.h"
#include "Schema/AuthorityIntent.h"
#include "Utils/ComponentReader.h"

namespace SpatialGDK
{
class FLBDataStorage
{
public:
	virtual ~FLBDataStorage() = default;

	virtual void OnAdded(Worker_EntityId EntityId, SpatialGDK::EntityViewElement const& Element) = 0;
	virtual void OnRemoved(Worker_EntityId EntityId) = 0;
	virtual void OnUpdate(Worker_EntityId EntityId, Worker_ComponentId InComponentId, Schema_ComponentUpdate* Update) = 0;

	void ClearModified() { Modified.Empty(); }

	const TSet<Worker_EntityId_Key>& GetModifiedEntities() const { return Modified; }

	const TSet<Worker_ComponentId>& GetComponentsToWatch() const { return Components; }

protected:
	TSet<Worker_EntityId_Key> Modified;
	TSet<Worker_ComponentId> Components;
};

class FSpatialPositionStorage : public FLBDataStorage
{
public:
	FSpatialPositionStorage();

	virtual void OnAdded(Worker_EntityId EntityId, SpatialGDK::EntityViewElement const& Element) override;
	virtual void OnRemoved(Worker_EntityId EntityId) override;
	virtual void OnUpdate(Worker_EntityId EntityId, Worker_ComponentId InComponentId, Schema_ComponentUpdate* Update) override;

	TMap<Worker_EntityId_Key, FVector> const& GetPositions() const { return Positions; }

protected:
	TMap<Worker_EntityId_Key, FVector> Positions;
};

class FActorGroupStorage : public FLBDataStorage
{
public:
	FActorGroupStorage();

	virtual void OnAdded(Worker_EntityId EntityId, SpatialGDK::EntityViewElement const& Element) override;
	virtual void OnRemoved(Worker_EntityId EntityId) override;
	virtual void OnUpdate(Worker_EntityId EntityId, Worker_ComponentId InComponentId, Schema_ComponentUpdate* Update) override;

	TMap<Worker_EntityId_Key, int32> const& GetGroups() const { return Groups; }

protected:
	TMap<Worker_EntityId_Key, int32> Groups;
};

class FDirectAssignmentStorage : public FLBDataStorage
{
public:
	FDirectAssignmentStorage();

	virtual void OnAdded(Worker_EntityId EntityId, SpatialGDK::EntityViewElement const& Element) override;
	virtual void OnRemoved(Worker_EntityId EntityId) override;
	virtual void OnUpdate(Worker_EntityId EntityId, Worker_ComponentId InComponentId, Schema_ComponentUpdate* Update) override;

	TMap<Worker_EntityId_Key, AuthorityIntent> const& GetAssignments() const { return Intents; }

protected:
	TMap<Worker_EntityId_Key, AuthorityIntent> Intents;
};

void ComputeFieldAndComponents(USpatialNetDriver* NetDriver, UClass* InClass, GDK_PROPERTY(Property) * InProperty,
							   TSet<Worker_ComponentId>&, Schema_FieldId&);

template <typename T>
class TLBDataStorage : public FLBDataStorage
{
public:
	TLBDataStorage(USpatialNetDriver* InNetDriver, UClass* InClass, GDK_PROPERTY(Property) * InProperty)
		: NetDriver(InNetDriver)
		, Class(InClass)
		, Property(InProperty)
	{
		check(Property->GetSize() == sizeof(T));
		ComputeFieldAndComponents(NetDriver, InClass, InProperty, this->Components, FieldId);
	}

	void OnAdded(Worker_EntityId EntityId, SpatialGDK::EntityViewElement const& Element) override
	{
		for (const auto& Component : Element.Components)
		{
			if (this->Components.Contains(Component.GetComponentId()))
			{
				OnAdded_ReadComponent(EntityId, Component.GetUnderlying());
			}
		}
	}
	void OnAdded_ReadComponent(Worker_EntityId EntityId, Schema_ComponentData* InData)
	{
		T& EntryData = Data.Add(EntityId);

		FObjectReferencesMap Map;
		ComponentReader Reader(NetDriver, Map, nullptr);
		Reader.ExtractField(Schema_GetComponentDataFields(InData), Class, FieldId, reinterpret_cast<uint8*>(&EntryData));
		Modified.Add(EntityId);
	}
	void OnRemoved(Worker_EntityId EntityId) override
	{
		Data.Remove(EntityId);
		Modified.Remove(EntityId);
	}
	void OnUpdate(Worker_EntityId EntityId, Worker_ComponentId InComponentId, Schema_ComponentUpdate* Update) override
	{
		if (!this->Components.Contains(InComponentId))
		{
			return;
		}
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update);

		TArray<uint32> UpdatedIds;
		UpdatedIds.SetNumUninitialized(Schema_GetUniqueFieldIdCount(ComponentObject));
		Schema_GetUniqueFieldIds(ComponentObject, UpdatedIds.GetData());

		if (!UpdatedIds.Contains(FieldId))
		{
			return;
		}

		FObjectReferencesMap Map;
		ComponentReader Reader(NetDriver, Map, nullptr);

		T* EntryData = Data.Find(EntityId);
		if (!ensureAlways(EntryData != nullptr))
		{
			return;
		}
		Reader.ExtractField(ComponentObject, Class, FieldId, reinterpret_cast<uint8*>(EntryData));
		Modified.Add(EntityId);
	}

	const T* GetDataForEntity(Worker_EntityId EntityId) const { return Data.Find(EntityId) }

	TMap<Worker_EntityId_Key, T> const& GetData() const
	{
		return Data;
	}

protected:
	USpatialNetDriver* NetDriver;
	UClass* Class;
	GDK_PROPERTY(Property) * Property;
	Schema_FieldId FieldId;

	TMap<Worker_EntityId_Key, T> Data;
};

} // namespace SpatialGDK
