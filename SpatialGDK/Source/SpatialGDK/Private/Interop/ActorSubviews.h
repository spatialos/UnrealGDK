#pragma once

#include "Misc/Optional.h"

#include "SpatialCommonTypes.h"
#include "SpatialView/SubView.h"

class USpatialNetDriver;

namespace SpatialGDK
{
class ViewCoordinator;
class FOwnershipCompletenessHandler;

namespace ActorSubviews
{
using FActorFilterPredicateFactory = TFunction<FFilterPredicate(FFilterPredicate)>;
using FActorRefreshCallbackPredicateFactory = TFunction<TArray<FDispatcherRefreshCallback>(TArray<FDispatcherRefreshCallback>)>;

struct FActorSubviewExtension
{
	TOptional<FActorFilterPredicateFactory> PredicateFactory;
	TOptional<FActorRefreshCallbackPredicateFactory> RefreshCallbackFactory;

	FFilterPredicate ExtendPredicate(FFilterPredicate Predicate) const
	{
		if (PredicateFactory)
		{
			return (*PredicateFactory)(Predicate);
		}
		return Predicate;
	}

	TArray<FDispatcherRefreshCallback> ExtendCallbacks(TArray<FDispatcherRefreshCallback> Callbacks) const
	{
		if (RefreshCallbackFactory)
		{
			return (*RefreshCallbackFactory)(Callbacks);
		}
		return Callbacks;
	}
};

FSubView& CreateActorSubView(USpatialNetDriver& NetDriver);
FSubView& CreateCustomActorSubView(TOptional<Worker_ComponentId> MaybeCustomComponentId, TOptional<FFilterPredicate> MaybeCustomPredicate,
								   TOptional<TArray<FDispatcherRefreshCallback>> MaybeCustomRefresh, USpatialNetDriver& NetDriver);

FSubView& CreateActorAuthSubView(USpatialNetDriver& NetDriver);

FSubView& CreateAuthoritySubView(USpatialNetDriver& NetDriver);

FSubView& CreatePlayerOwnershipSubView(USpatialNetDriver& NetDriver);
FSubView& CreatePlayerOwnershipSubView(ViewCoordinator& Coordinator, FOwnershipCompletenessHandler& OwnershipHandler,
									   const FActorSubviewExtension& Extension = {});

FSubView& CreateSimulatedSubView(USpatialNetDriver& NetDriver);
FSubView& CreateSimulatedSubView(ViewCoordinator& Coordinator, FOwnershipCompletenessHandler& OwnershipHandler,
								 const FActorSubviewExtension& Extension = {});
} // namespace ActorSubviews

} // namespace SpatialGDK
