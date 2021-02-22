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
	FLoadBalancingWriterBase(USpatialNetDriver* InNetDriver)
		: NetDriver(InNetDriver)
	{
	}

	Worker_EntityId GetEntityId(AActor* Actor) const;
	bool IsCreatingNewEntity(Worker_EntityId EntityId) const;
	const EntityViewElement& GetEntityView(Worker_EntityId EntityId) const;
	void SendComponentUpdate(Worker_EntityId EntityId, FWorkerComponentUpdate& ComponentUpdate);

protected:
	USpatialNetDriver* NetDriver;
};

template <class TComponent>
class TLoadBalancingWriter : public FLoadBalancingWriterBase
{
public:
	TLoadBalancingWriter(USpatialNetDriver* NetDriver)
		: FLoadBalancingWriterBase(NetDriver)
	{
	}

	virtual ~TLoadBalancingWriter() {}

	void Advance(const FSubView& SubView)
	{
		for (const EntityDelta& EntityDelta : SubView.GetViewDelta().EntityDeltas)
		{
			switch (EntityDelta.Type)
			{
			case EntityDelta::ADD:
			case EntityDelta::TEMPORARILY_REMOVED:
			case EntityDelta::UPDATE:
				DataStore.Add(EntityDelta.EntityId, GetLoadBalancingData(SubView.GetView()[EntityDelta.EntityId]));
				break;
			case EntityDelta::REMOVE:
				DataStore.Remove(EntityDelta.EntityId);
				break;
			default:
				break;
			}
		}
	}

	void ReplicateActor(AActor* Actor)
	{
		// USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(Actor->GetNetDriver());
		const Worker_EntityId EntityId = GetEntityId(Actor); // NetDriver->PackageMap->GetEntityIdFromObject(Actor);
		if (!IsCreatingNewEntity(EntityId))
		{
			const TComponent& Component = DataStore.Add(EntityId, GetLoadBalancingData(Actor)).Data;
			FWorkerComponentUpdate CreateComponentData = Component.CreateComponentUpdate();
			SendComponentUpdate(EntityId, CreateComponentData);
		}
	}

protected:
	struct LoadBalancingData
	{
		TComponent Data;
	};

	static LoadBalancingData GetLoadBalancingData(const EntityViewElement& ViewElement)
	{
		const ComponentData* ComponentData = ViewElement.Components.FindByPredicate(ComponentIdEquality{ TComponent::ComponentId });
		check(ComponentData != nullptr);
		return LoadBalancingData{ ComponentData->GetUnderlying() };
	}

	virtual LoadBalancingData GetLoadBalancingData(AActor* Actor) const = 0;

	TMap<Worker_EntityId_Key, LoadBalancingData> DataStore;
};

class LoadBalancingWriter
{
public:
	LoadBalancingWriter(USpatialNetDriver* InNetDriver);

	void Advance();
	void OnActorReplicated(AActor* Actor);

	struct FLoadBalancingStuff
	{
		ActorGroupMember ActorGroup;
		ActorSetMember ActorSet;
	};
	FLoadBalancingStuff GetOrCreateLoadBalancingData(const AActor* Actor);

public:
	struct LoadBalancingActorStuff
	{
		LoadBalancingActorStuff()
			: LoadBalancingActorStuff(FLoadBalancingStuff{})
		{
		}

		LoadBalancingActorStuff(const FLoadBalancingStuff& InLoadBalancingData)
			: LoadBalancingData(InLoadBalancingData)
		{
		}

		FLoadBalancingStuff LoadBalancingData;
		TWeakObjectPtr<AActor> Actor;
	};

	TWeakObjectPtr<USpatialNetDriver> NetDriver;
	const FSubView* SubView;

	TUniquePtr<TLoadBalancingWriter<ActorSetMember>> ActorSetWriter;
	TUniquePtr<TLoadBalancingWriter<ActorGroupMember>> ActorGroupWriter;

	TMap<Worker_EntityId_Key, LoadBalancingActorStuff> DataStore;
};
} // namespace SpatialGDK