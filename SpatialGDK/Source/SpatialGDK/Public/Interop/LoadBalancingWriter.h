// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/ActorGroupMember.h"
#include "Schema/ActorSetMember.h"
#include "SpatialCommonTypes.h"
#include "SpatialView/EntityComponentTypes.h"
#include "UObject/WeakObjectPtr.h"

#include "SpatialView/SubView.h"

class USpatialNetDriver;
class UAbstractLBStrategy;

namespace SpatialGDK
{
class ViewCoordinator;

class LoadBalancingWriterActorSet
{
public:
	explicit LoadBalancingWriterActorSet(ViewCoordinator& InCoordinator, const USpatialPackageMapClient& InPackageMap)
		: Coordinator(InCoordinator)
		, PackageMap(InPackageMap)
	{
	}

	ActorSetMember GetActorSetData(AActor* Actor) const;

	void UpdateActorSetComponent(Worker_EntityId ActorEntityId, AActor* Actor) const;

private:
	void SendComponentUpdate(Worker_EntityId EntityId, const ComponentUpdate& ComponentUpdate) const;

	ViewCoordinator& Coordinator;
	const USpatialPackageMapClient& PackageMap;
};

class LoadBalancingWriterActorGroup
{
public:
	explicit LoadBalancingWriterActorGroup(const UAbstractLBStrategy& InLoadBalancingStrategy)
		: LoadBalancingStrategy(InLoadBalancingStrategy)
	{
	}

	ActorGroupMember GetActorGroupData(AActor* Actor) const;

private:
	const UAbstractLBStrategy& LoadBalancingStrategy;
};
} // namespace SpatialGDK
