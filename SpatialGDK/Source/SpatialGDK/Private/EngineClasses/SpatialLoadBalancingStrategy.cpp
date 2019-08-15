// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialLoadBalancingStrategy.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetDriver.h"

void USpatialLoadBalancingStrategy::Init(const ASpatialVirtualWorkerTranslator* InTranslator)
{
	Translator = InTranslator;
}

USpatialLoadBalancingStrategy::~USpatialLoadBalancingStrategy()
{
	Translator = nullptr;
}

FString USingleWorkerLoadBalancingStrategy::GetAuthoritativeVirtualWorkerId(const AActor& Actor) const
{
	return Translator->GetVirtualWorkerAssignments()[0];
}


void UGridBasedLoadBalancingStrategy::Init(const ASpatialVirtualWorkerTranslator* InTranslator)
{
	USpatialLoadBalancingStrategy::Init(InTranslator);
	// TODO: init the cell dims based on world size.
}

bool UGridBasedLoadBalancingStrategy::ShouldChangeAuthority(const AActor& Actor) const
{
	// TODO
	return false;
}

FString UGridBasedLoadBalancingStrategy::GetAuthoritativeVirtualWorkerId(const AActor& Actor) const
{
	// TODO
	return Translator->GetVirtualWorkerAssignments()[0];
}
