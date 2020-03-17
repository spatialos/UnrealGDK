// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/SpatialClassInfoManager.h"
#include "Schema/Interest.h"

/**
 * This class contains static functionality to create the required result types for query based interest.
 *
 * If the required result type will contain generated components, then there is a dependency on the ClassInfoManager to retrieve those.
 * Otherwise the result types are built using the predefined component sets in SpatialConstants.
 */

namespace
{

class SPATIALGDK_API ResultTypeCreators
{
public:

	static SpatialGDK::ResultType CreateClientNonAuthInterestResultType(USpatialClassInfoManager* ClassInfoManager);
	static SpatialGDK::ResultType CreateClientAuthInterestResultType(USpatialClassInfoManager* ClassInfoManager);
	static SpatialGDK::ResultType CreateServerNonAuthInterestResultType(USpatialClassInfoManager* ClassInfoManager);
	static SpatialGDK::ResultType CreateServerAuthInterestResultType();

};

}
