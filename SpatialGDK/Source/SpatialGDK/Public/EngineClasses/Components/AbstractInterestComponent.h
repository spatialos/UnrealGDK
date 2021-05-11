// NWX_BEGIN - https://improbableio.atlassian.net/browse/NWX-18843 - [IMPROVEMENT] Support for custom Interest components
// NWX_MORE - https://improbableio.atlassian.net/browse/NWX-18915 - [IMPROVEMENT] InterestSettingsComponent
// NWX_MORE - https://improbableio.atlassian.net/browse/NWX-19238 - [IMPROVEMENT] Move UpdateRequired logic to SpatialActorChannel
// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Components/ActorComponent.h"
#include "Interfaces/ISpatialInterestProvider.h"
#include "AbstractInterestComponent.generated.h"

UCLASS(ClassGroup = (SpatialGDK), abstract, SpatialType = ServerOnly)
class SPATIALGDK_API UAbstractInterestComponent : public UActorComponent, public ISpatialInterestProvider
{
	GENERATED_BODY()

protected:
	void NotifyChannelUpdateRequired();
};
