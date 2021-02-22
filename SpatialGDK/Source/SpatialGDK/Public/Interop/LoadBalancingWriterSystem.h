// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/LoadBalancingStuff.h"
#include "SpatialCommonTypes.h"
#include "SpatialView/EntityComponentTypes.h"
#include "UObject/WeakObjectPtr.h"

#include "SpatialView/SubView.h"

class USpatialNetDriver;

namespace SpatialGDK
{
class FSubView;

class FLoadBalancingWriterBase
{
public:
	FLoadBalancingWriterBase(USpatialNetDriver* InNetDriver, const FSubView* InSubView)
		: NetDriver(InNetDriver)
		, SubView(InSubView)
	{
	}

	Worker_EntityId GetEntityId(AActor* Actor) const;
	const EntityViewElement& GetEntityView(Worker_EntityId EntityId) const;
	void SendComponentUpdate(Worker_EntityId EntityId, FWorkerComponentUpdate& ComponentUpdate);

protected:
	USpatialNetDriver* NetDriver;
	const FSubView* SubView;
};

template <class TComponent>
class TLoadBalancingWriter : public FLoadBalancingWriterBase
{
public:
	TLoadBalancingWriter(USpatialNetDriver* NetDriver, const FSubView* InSubView)
		: FLoadBalancingWriterBase(NetDriver, InSubView)
	{
	}

	virtual ~TLoadBalancingWriter() {}

	void Advance()
	{
		for (const EntityDelta& EntityDelta : SubView->GetViewDelta().EntityDeltas)
		{
			switch (EntityDelta.Type)
			{
			case EntityDelta::ADD:
			case EntityDelta::TEMPORARILY_REMOVED:
			case EntityDelta::UPDATE:
				DataStore.Add(EntityDelta.EntityId, GetLoadBalancingData(GetEntityView(EntityDelta.EntityId)));
				break;
			case EntityDelta::REMOVE:
				DataStore.Remove(EntityDelta.EntityId);
				break;
			default:
				break;
			}
		}
	}

	void ReplicateActor(Worker_EntityId ActorEntityId, AActor* Actor)
	{
		const TComponent& Component = DataStore.Add(ActorEntityId, { GetLoadBalancingData(Actor) }).Data;
		FWorkerComponentUpdate CreateComponentData = Component.CreateComponentUpdate();
		SendComponentUpdate(ActorEntityId, CreateComponentData);
	}

	virtual TComponent GetLoadBalancingData(AActor* Actor) const = 0;

protected:
	struct LoadBalancingData
	{
		TComponent Data;
	};

private:
	static LoadBalancingData GetLoadBalancingData(const EntityViewElement& ViewElement)
	{
		const ComponentData* ComponentData = ViewElement.Components.FindByPredicate(ComponentIdEquality{ TComponent::ComponentId });
		check(ComponentData != nullptr);
		return LoadBalancingData{ ComponentData->GetUnderlying() };
	}

	TMap<Worker_EntityId_Key, LoadBalancingData> DataStore;
};

class LoadBalancingWriter
{
public:
	LoadBalancingWriter(USpatialNetDriver* InNetDriver, const FSubView* InSubView);

	void Advance();
	void OnActorReplicated(Worker_EntityId ActorEntityId, AActor* Actor);

	TWeakObjectPtr<USpatialNetDriver> NetDriver;
	const FSubView* SubView;

	TUniquePtr<TLoadBalancingWriter<ActorSetMember>> ActorSetWriter;
	TUniquePtr<TLoadBalancingWriter<ActorGroupMember>> ActorGroupWriter;
};
} // namespace SpatialGDK