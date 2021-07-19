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

	virtual void SetIsUpdateRequired(const bool bUpdateRequired) PURE_VIRTUAL(, );
	virtual bool IsUpdateRequired() const PURE_VIRTUAL(ISpatialInterestProvider::IsUpdateRequired, return false;);

	virtual bool GetUseNetCullDistanceSquaredForCheckoutRadius() const PURE_VIRTUAL(ISpatialInterestProvider::GetUseNetCullDistanceSquaredForCheckoutRadius, return false;);
};
