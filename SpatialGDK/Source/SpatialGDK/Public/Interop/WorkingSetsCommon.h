// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/WorkingSet.h"
#include "SpatialCommonTypes.h"

namespace SpatialGDK
{
struct EntityDelta;
struct EntityViewElement;
class FSubView;
class ViewCoordinator;

struct FWorkingSetCommonData
{
	FWorkingSetCommonData() = default;
	explicit FWorkingSetCommonData(const EntityViewElement& Entity);
	void Update(const EntityDelta& Delta);

	FWorkingSetState RequestedState;
	FWorkingSetState ConfirmedState;
};

class FWorkingSetDataStorage
{
public:
	const TMap<Worker_EntityId_Key, FWorkingSetCommonData>& GetWorkingSets() const { return DataStorage; }
	TMap<Worker_EntityId_Key, FWorkingSetCommonData>& GetWorkingSets() { return DataStorage; }

	void Advance(const FSubView& WorkingSetMarkerEntitiesSubview);
	bool HasMarkerEntity(Worker_EntityId MarkerEntityId) const { return DataStorage.Contains(MarkerEntityId); }
	const TSet<Worker_EntityId_Key>& GetUpdatedWorkingSets() const { return UpdatedWorkingSets; }
	const TSet<Worker_EntityId_Key>& GetRemovedWorkingSets() const { return RemovedWorkingSets; }
	const TSet<Worker_EntityId_Key>& GetUpdatedEntities() const { return EntitiesToRefresh; }
	const Worker_EntityId* GetOwningMarkerEntityId(Worker_EntityId MemberEntityId) const
	{
		return EntityWorkingSetMembership.Find(MemberEntityId);
	}

private:
	TMap<Worker_EntityId_Key, FWorkingSetCommonData> DataStorage;
	TSet<Worker_EntityId_Key> UpdatedWorkingSets;
	TSet<Worker_EntityId_Key> RemovedWorkingSets;
	TSet<Worker_EntityId_Key> EntitiesToRefresh;
	TMap<Worker_EntityId_Key, Worker_EntityId> EntityWorkingSetMembership;
};

TArray<ComponentData> CreateWorkingSetComponents(const Worker_PartitionId AuthServerWorkerPartitionId,
												 const FWorkingSetMarkerRequest& Request);
FSubView& CreateWorkingSetMarkersSubview(ViewCoordinator& Coordinator);
} // namespace SpatialGDK
