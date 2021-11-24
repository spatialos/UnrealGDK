// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/WorkingSetsHandler.h"

#include "SpatialView/EntityComponentTypes.h"
#include "SpatialView/EntityDelta.h"
#include "SpatialView/EntityView.h"
#include "SpatialView/SubView.h"
#include "SpatialView/ViewCoordinator.h"

DEFINE_LOG_CATEGORY(LogSpatialWorkingSetsHandler);

namespace SpatialGDK
{
bool FWorkingSetCompletenessHandler::IsWorkingSetComplete(const FWorkingSetCommonData& WorkingSet) const
{
	const TSet<Worker_EntityId_Key>& ConfirmedSetMembers = WorkingSet.ConfirmedState.MemberEntities;

	const TSet<Worker_EntityId_Key> LocallyAuthConfirmedSetMembers = ConfirmedSetMembers.Intersect(*AuthEntities);

	return ConfirmedSetMembers.Num() == LocallyAuthConfirmedSetMembers.Num();
}

void FWorkingSetCompletenessHandler::Advance(ViewCoordinator& Coordinator)
{
	for (const Worker_EntityId_Key UpdatedWorkingSet : DataStorage->GetUpdatedWorkingSets())
	{
		WorkingSetCompleteness.Emplace(UpdatedWorkingSet, false);
	}

	for (const Worker_EntityId_Key RemovedWorkingSet : DataStorage->GetRemovedWorkingSets())
	{
		WorkingSetCompleteness.Remove(RemovedWorkingSet);
	}

	bool bDidChangeAnyAuthEntities = false;
	for (const EntityDelta& Delta : BaseAuthoritySubview->GetViewDelta().EntityDeltas)
	{
		switch (Delta.Type)
		{
		case EntityDelta::ADD:
			AuthEntities->Emplace(Delta.EntityId);
			bDidChangeAnyAuthEntities = true;
			break;
		case EntityDelta::REMOVE:
			AuthEntities->Remove(Delta.EntityId);
			bDidChangeAnyAuthEntities = true;
			break;
		default:
			// Ignored.
			break;
		}
	}

	TSet<Worker_EntityId_Key> EntitiesToRefresh = DataStorage->GetUpdatedEntities();

	if (bDidChangeAnyAuthEntities)
	{
		for (const auto& WorkingSet : DataStorage->GetWorkingSets())
		{
			bool& bWasWorkingSetComplete = WorkingSetCompleteness[WorkingSet.Key];
			const bool bHasSetBecomeComplete = IsWorkingSetComplete(WorkingSet.Value);
			if (bHasSetBecomeComplete != bWasWorkingSetComplete)
			{
				bWasWorkingSetComplete = bHasSetBecomeComplete;

				UE_LOG(LogSpatialWorkingSetsHandler, Verbose,
					   TEXT("Authority changes caused working set to update. MarkerEntityId: %lld EntityCount: %d"), WorkingSet.Key,
					   WorkingSet.Value.ConfirmedState.MemberEntities.Num());
				EntitiesToRefresh.Append(WorkingSet.Value.ConfirmedState.MemberEntities);
			}
		}
	}

	for (const Worker_EntityId_Key PotentiallyUpdatedEntity : EntitiesToRefresh)
	{
		UE_LOG(LogSpatialWorkingSetsHandler, Verbose, TEXT("Refreshing entity due to working set changes. EntityId: %lld"),
			   PotentiallyUpdatedEntity);

		Coordinator.RefreshEntityCompleteness(PotentiallyUpdatedEntity);
	}
}

const FWorkingSetCommonData* FWorkingSetCompletenessHandler::GetOwningSet(const Worker_EntityId MaybeMemberEntityId) const
{
	const Worker_EntityId* MarkerEntityId = DataStorage->GetOwningMarkerEntityId(MaybeMemberEntityId);
	if (MarkerEntityId != nullptr)
	{
		return GetMarkerEntitySet(*MarkerEntityId);
	}
	return nullptr;
}

const FWorkingSetCommonData* FWorkingSetCompletenessHandler::GetMarkerEntitySet(const Worker_EntityId MarkerEntityId) const
{
	return DataStorage->GetWorkingSets().Find(MarkerEntityId);
}
} // namespace SpatialGDK
