#pragma once

#include "EngineClasses/SpatialNetDriver.h"
#include "Schema/ActorGroupMember.h"
#include "Schema/AuthorityIntent.h"
#include "Utils/ComponentReader.h"

namespace SpatialGDK
{
class FLBDataStorage
{
public:
	virtual ~FLBDataStorage() = default;

	virtual void OnAdded(Worker_EntityId EntityId, const SpatialGDK::EntityViewElement& Element)
	{
		for (const auto& Component : Element.Components)
		{
			if (Components.Contains(Component.GetComponentId()))
			{
				OnComponentAdded(EntityId, Component.GetComponentId(), Component.GetUnderlying());
			}
		}
	}

	virtual void OnRefresh(Worker_EntityId EntityId, const SpatialGDK::EntityViewElement& Element)
	{
		OnRemoved(EntityId);
		OnAdded(EntityId, Element);
	}

	virtual void OnComponentRefreshed(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentData* Data)
	{
		OnComponentAdded(EntityId, ComponentId, Data);
	}

	virtual void OnComponentAdded(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentData* Data) = 0;
	virtual void OnRemoved(Worker_EntityId EntityId) = 0;
	virtual void OnUpdate(Worker_EntityId EntityId, Worker_ComponentId InComponentId, Schema_ComponentUpdate* Update) = 0;

	void ClearModified() { Modified.Empty(); }

	const TSet<Worker_EntityId_Key>& GetModifiedEntities() const { return Modified; }

	const TSet<Worker_ComponentId>& GetComponentsToWatch() const { return Components; }

protected:
	TSet<Worker_EntityId_Key> Modified;
	TSet<Worker_ComponentId> Components;
};

struct FLBDataCollection
{
	FLBDataCollection(const FSubView& InSubView)
		: SubView(InSubView)
	{
	}
	void Advance();
	TSet<Worker_ComponentId> GetComponentsToWatch() const;
	TArray<FLBDataStorage*> DataStorages;
	const FSubView& SubView;
};

class FSpatialPositionStorage : public FLBDataStorage
{
public:
	FSpatialPositionStorage();

	virtual void OnComponentAdded(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentData* Data) override;
	virtual void OnRemoved(Worker_EntityId EntityId) override;
	virtual void OnUpdate(Worker_EntityId EntityId, Worker_ComponentId InComponentId, Schema_ComponentUpdate* Update) override;

	TMap<Worker_EntityId_Key, FVector> const& GetPositions() const { return Positions; }

protected:
	TMap<Worker_EntityId_Key, FVector> Positions;
};

template <typename T>
class TLBDataStorage : public FLBDataStorage
{
public:
	TLBDataStorage() { Components.Add(T::ComponentId); }

	virtual void OnComponentAdded(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentData* Data) override
	{
		if (ensureAlways(T::ComponentId == ComponentId))
		{
			T NewSchemaObject(Data);
			SchemaObjects.Add(EntityId, NewSchemaObject);
			Modified.Add(EntityId);
		}
	}
	virtual void OnRemoved(Worker_EntityId EntityId) override
	{
		SchemaObjects.Remove(EntityId);
		Modified.Remove(EntityId);
	}
	virtual void OnUpdate(Worker_EntityId EntityId, Worker_ComponentId InComponentId, Schema_ComponentUpdate* Update) override
	{
		T* Object = SchemaObjects.Find(EntityId);
		if (!ensure(Object != nullptr))
		{
			return;
		}

		Object->ApplyComponentUpdate(Update);
		Modified.Add(EntityId);
	}

	TMap<Worker_EntityId_Key, T> const& GetObjects() const { return SchemaObjects; }

protected:
	TMap<Worker_EntityId_Key, T> SchemaObjects;
};

class FActorGroupStorage : public TLBDataStorage<ActorGroupMember>
{
};

class FDirectAssignmentStorage : public TLBDataStorage<AuthorityIntent>
{
};

} // namespace SpatialGDK
