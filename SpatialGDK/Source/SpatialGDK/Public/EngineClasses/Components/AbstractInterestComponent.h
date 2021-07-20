// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Components/ActorComponent.h"
#include "Interfaces/ISpatialInterestProvider.h"
#include "AbstractInterestComponent.generated.h"

UCLASS(ClassGroup = (SpatialGDK), abstract, NotSpatialType)
class SPATIALGDK_API UAbstractInterestComponent : public UActorComponent, public ISpatialInterestProvider
{
	GENERATED_BODY()

protected:
	void NotifyChannelUpdateRequired();
};
