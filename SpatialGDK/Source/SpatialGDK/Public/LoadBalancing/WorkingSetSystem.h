// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Containers/Set.h"
#include "Misc/Optional.h"

#include "Interop/WorkingSetsCommon.h"
#include "SpatialCommonTypes.h"
#include "SpatialView/EntityView.h"

namespace SpatialGDK
{
class FSubView;
class ISpatialOSWorker;
struct EntityViewElement;
class ViewCoordinator;

class FPartitionManager;

class FWorkingSetSystem
{
public:
	using FEntityToWorkerMap = TFunction<Worker_EntityId(Worker_EntityId)>;

	static FEntityToWorkerMap CreateEntityToOwningWorkerMap(const EntityView& View, const FPartitionManager& PartitionManager);

	explicit FWorkingSetSystem(FEntityToWorkerMap EntityToOwningWorker);
	~FWorkingSetSystem();

	void Advance(const FSubView& MarkerEntitiesSubview);
	void UpdateMigratingEntities(const TSet<Worker_EntityId_Key>& CurrentlyMigratingEntities, ISpatialOSWorker& Connection);

	Worker_EntityId GetOwningMarkerEntityId(Worker_EntityId MemberEntityId) const;
	bool IsWorkingLeaderEntity(Worker_EntityId MemberEntityId) const;
	bool IsMarkerEntity(Worker_EntityId EntityId) const;

	const TSet<Worker_EntityId_Key>& GetEntitiesToAttach() const { return ChangeDelta.NewMembers; }
	const TSet<Worker_EntityId_Key>& GetEntitiesToForceLoadBalance() const { return ChangeDelta.EntitiesToForceLoadBalance; }
	const TSet<Worker_EntityId_Key>& GetNewMarkerEntities() const { return ChangeDelta.NewMarkers; }
	const TSet<Worker_EntityId_Key>& GetReleasedEntities() const { return ChangeDelta.ReleasedEntities; }

	struct FWorkingSet
	{
		const TSet<Worker_EntityId_Key>& TargetEntities;
		const TSet<Worker_EntityId_Key>& ConfirmedEntities;
		Worker_EntityId LeaderEntity;
	};
	TOptional<FWorkingSet> GetSet(Worker_EntityId MarkerEntityId) const;

private:
	struct FImpl;

	struct FWorkingSetData
	{
		FWorkingSetState TargetState;
		FWorkingSetState ApprovedState;
	};

	TMap<Worker_EntityId_Key, FWorkingSetState> CurrentRequests;

	TMap<Worker_EntityId_Key, FWorkingSetData> WorkingSets;
	TMap<Worker_EntityId_Key, Worker_EntityId> ActorOwnership;
	TMap<Worker_EntityId_Key, Worker_EntityId> ProgressActorOwnership;

	struct FWorkingSetsChangeState
	{
		TSet<Worker_EntityId_Key> NewMembers;
		TSet<Worker_EntityId_Key> EntitiesToForceLoadBalance;
		TSet<Worker_EntityId_Key> NewMarkers;
		TSet<Worker_EntityId_Key> ReleasedEntities;

		FWorkingSetsChangeState CreateDiff(const FWorkingSetsChangeState& Other) const;
	};

	FWorkingSetsChangeState LastChangeState;
	FWorkingSetsChangeState ChangeDelta;
};

} // namespace SpatialGDK
