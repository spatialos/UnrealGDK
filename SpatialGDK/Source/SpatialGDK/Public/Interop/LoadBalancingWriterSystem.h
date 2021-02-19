// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/LoadBalancingStuff.h"
#include "SpatialCommonTypes.h"
#include "UObject/WeakObjectPtr.h"

class USpatialNetDriver;

namespace SpatialGDK
{
class FSubView;
class LoadBalancingWriter
{
public:
	void Advance();
	void OnActorReplicated(AActor* Actor);
	LoadBalancingStuff GetOrCreateLoadBalancingData(const AActor* Actor);

public:
	struct LoadBalancingActorStuff
	{
		LoadBalancingActorStuff()
			: LoadBalancingActorStuff(SpatialGDK::LoadBalancingData{})
		{
		}

		LoadBalancingActorStuff(const LoadBalancingData& InLoadBalancingData)
			: LoadBalancingData(InLoadBalancingData)
		{
		}

		LoadBalancingData LoadBalancingData;
		TWeakObjectPtr<AActor> Actor;
	};
	const FSubView* SubView;
	TWeakObjectPtr<USpatialNetDriver> NetDriver;
	TMap<Worker_EntityId_Key, LoadBalancingActorStuff> DataStore;
};
} // namespace SpatialGDK