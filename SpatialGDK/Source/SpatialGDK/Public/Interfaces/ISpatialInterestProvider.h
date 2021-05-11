// NWX_BEGIN - https://improbableio.atlassian.net/browse/NWX-18843 - [IMPROVEMENT] Support for custom Interest components
// NWX_MORE - https://improbableio.atlassian.net/browse/NWX-18915 - [IMPROVEMENT] InterestSettingsComponent
// NWX_MORE - https://improbableio.atlassian.net/browse/NWX-19238 - [IMPROVEMENT] Move UpdateRequired logic to SpatialActorChannel
// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "UObject/Interface.h"
#include "Interop/SpatialClassInfoManager.h"
#include "Schema/Interest.h"
#include "ISpatialInterestProvider.generated.h"

UINTERFACE()
class SPATIALGDK_API USpatialInterestProvider : public UInterface
{
	GENERATED_BODY()
};

class SPATIALGDK_API ISpatialInterestProvider
{
	GENERATED_BODY()

public:

	virtual void PopulateFrequencyToConstraintsMap(const USpatialClassInfoManager& ClassInfoManager,
		SpatialGDK::FrequencyToConstraintsMap& OutFrequencyToQueryConstraints) const PURE_VIRTUAL(, );
};
