// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/SpatialClassInfoManager.h"
#include "Schema/Interest.h"

/**
 * Result types!
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
