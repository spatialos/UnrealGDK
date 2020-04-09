// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/SpatialClassInfoManager.h"
#include "Schema/Interest.h"

/**
 * This file contains functionality to create the required result types for query based interest.
 *
 * If the required result type will contain generated components, then there is a dependency on the ClassInfoManager to retrieve those.
 * Otherwise the result types are built using the predefined component sets in SpatialConstants.
 */


// These are marked part of the Spatial GDK API in order to be accessible by tests.
SPATIALGDK_API SpatialGDK::ResultType CreateClientNonAuthInterestResultType(USpatialClassInfoManager* ClassInfoManager);
SPATIALGDK_API SpatialGDK::ResultType CreateClientAuthInterestResultType(USpatialClassInfoManager* ClassInfoManager);
SPATIALGDK_API SpatialGDK::ResultType CreateServerNonAuthInterestResultType(USpatialClassInfoManager* ClassInfoManager);
SPATIALGDK_API SpatialGDK::ResultType CreateServerAuthInterestResultType();
