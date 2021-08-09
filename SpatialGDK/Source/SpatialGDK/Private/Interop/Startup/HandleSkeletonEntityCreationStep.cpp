#include "Interop/Startup/HandleSkeletonEntityCreationStep.h"

namespace SpatialGDK
{
FCreateSkeletonEntities::FCreateSkeletonEntities(USpatialNetDriver& InNetDriver)
	: Implementation(InNetDriver)
{
	StepName = TEXT("Creating skeleton entities");
}

bool FCreateSkeletonEntities::TryFinish()
{
	return Implementation.TryFinish();
}
} // namespace SpatialGDK
