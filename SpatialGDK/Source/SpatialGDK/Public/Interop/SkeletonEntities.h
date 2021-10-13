#pragma once

#include "Containers/Array.h"
#include "Containers/Set.h"

#include "Connection/SpatialWorkerConnection.h"
#include "CreateEntityHandler.h"
#include "Schema/SkeletonEntityManifest.h"

class AActor;
class UWorld;

class USpatialNetDriver;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialSkeletonEntityCreator, Log, All);

namespace SpatialGDK
{
class FSubView;

struct FManifestCreationData;
using FManifestCreationHandle = TSharedPtr<FManifestCreationData>;

namespace SkeletonEntityFunctions
{
bool IsCompleteSkeleton(const EntityViewElement& Entity);
TArray<FDispatcherRefreshCallback> GetSkeletonEntityRefreshCallbacks(ViewCoordinator& Coordinator);
Worker_EntityId CreateSkeletonEntityForActor(AActor& Actor, USpatialNetDriver& NetDriver, FCreateEntityHandler& CreateHandler,
											 FCreateEntityDelegate OnCreated);
const FSubView& CreateFilledManifestSubView(ViewCoordinator& Coordinator);
const FSubView& CreateLocallyAuthManifestSubView(ViewCoordinator& Coordinator);

} // namespace SkeletonEntityFunctions

} // namespace SpatialGDK
