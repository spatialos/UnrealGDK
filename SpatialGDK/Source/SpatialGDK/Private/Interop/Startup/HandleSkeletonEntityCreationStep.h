#pragma once

#include "Interop/SkeletonEntityCreationStep.h"
#include "Interop/Startup/SpatialStartupCommon.h"

namespace SpatialGDK
{
class FCreateSkeletonEntities : public FStartupStep
{
public:
	FCreateSkeletonEntities(USpatialNetDriver& InNetDriver);

	virtual bool TryFinish() override;

private:
	FSkeletonEntityCreationStartupStep Implementation;
};
} // namespace SpatialGDK
