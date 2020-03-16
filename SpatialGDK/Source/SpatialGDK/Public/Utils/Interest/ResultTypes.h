// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/SpatialClassInfoManager.h"
#include "Schema/Interest.h"

/**
 * This class contains static functionality to build the required result types for query based interest.
 *
 * If the required result type will contain generated components, then there is a dependency on the ClassInfoManager to retrieve those.
 * Otherwise the result types are built using the predefined component sets in SpatialConstants.
 */

namespace SpatialGDK
{

class SPATIALGDK_API ResultTypes
{
public:

	static ResultType CreateClientNonAuthInterestResultType(USpatialClassInfoManager* ClassInfoManager);
	static ResultType CreateClientAuthInterestResultType(USpatialClassInfoManager* ClassInfoManager);
	static ResultType CreateServerNonAuthInterestResultType(USpatialClassInfoManager* ClassInfoManager);
	static ResultType CreateServerAuthInterestResultType();

};

} // namespace SpatialGDK
