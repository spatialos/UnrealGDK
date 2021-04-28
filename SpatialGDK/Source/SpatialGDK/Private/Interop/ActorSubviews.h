#pragma once

#include "Misc/Optional.h"

#include "SpatialCommonTypes.h"
#include "SpatialView/SubView.h"

class USpatialNetDriver;

namespace SpatialGDK
{
namespace ActorSubviews
{
FSubView& CreateActorSubView(USpatialNetDriver& NetDriver);
FSubView& CreateCustomActorSubView(TOptional<Worker_ComponentId> MaybeCustomComponentId, TOptional<FFilterPredicate> MaybeCustomPredicate,
								   TOptional<TArray<FDispatcherRefreshCallback>> MaybeCustomRefresh, USpatialNetDriver& NetDriver);
FSubView& CreateActorAuthSubView(USpatialNetDriver& NetDriver);
FSubView& CreateAuthoritySubView(USpatialNetDriver& NetDriver);
FSubView& CreatePlayerOwnershipSubView(USpatialNetDriver& NetDriver);
FSubView& CreateSimulatedSubView(USpatialNetDriver& NetDriver);
} // namespace ActorSubviews

} // namespace SpatialGDK
