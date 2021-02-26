// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/ActorGroupMember.h"
#include "Schema/ActorSetMember.h"
#include "SpatialCommonTypes.h"
#include "SpatialView/EntityComponentTypes.h"
#include "UObject/WeakObjectPtr.h"

#include "SpatialView/SubView.h"

class USpatialNetDriver;

namespace SpatialGDK
{
class FLoadBalancingWriterBase
{
public:
	FLoadBalancingWriterBase(USpatialNetDriver* InNetDriver)
		: NetDriver(InNetDriver)
	{
	}

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

	void ReplicateActor(Worker_EntityId ActorEntityId, AActor* Actor)
	{
		LoadBalancingData* PresentDataPtr = DataStore.Find(ActorEntityId);
		const TComponent UpdatedData = GetLoadBalancingData(Actor);
		if (PresentDataPtr != nullptr)
		{
			if (!ShouldUpdateComponent(PresentDataPtr->Data, UpdatedData))
			{
				return;
			}
		}
		DataStore.Add(ActorEntityId, { UpdatedData });
		FWorkerComponentUpdate CreateComponentData = UpdatedData.CreateComponentUpdate();
		SendComponentUpdate(ActorEntityId, CreateComponentData);
	}

	virtual TComponent GetLoadBalancingData(AActor* Actor) const = 0;

	virtual bool ShouldUpdateComponent(const TComponent& PresentData, const TComponent& UpdatedData) const { return true; }

protected:
	struct LoadBalancingData
	{
		TComponent Data;
	};

private:
	TMap<Worker_EntityId_Key, LoadBalancingData> DataStore;
};

class LoadBalancingWriter
{
public:
	LoadBalancingWriter(USpatialNetDriver* InNetDriver);

	void OnActorReplicated(Worker_EntityId ActorEntityId, AActor* Actor);

	TWeakObjectPtr<USpatialNetDriver> NetDriver;

	TUniquePtr<TLoadBalancingWriter<ActorSetMember>> ActorSetWriter;
	TUniquePtr<TLoadBalancingWriter<ActorGroupMember>> ActorGroupWriter;
};
} // namespace SpatialGDK
