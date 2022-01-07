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
void FWorkingSetChangesHandler::Advance()
{
	for (const Worker_EntityId_Key UpdatedWorkingSetEntityId : DataStorage->GetUpdatedWorkingSets())
	{
		FLocalWorkingSetData& WorkingSetRequest = LocalWorkingSetRequests.FindOrAdd(UpdatedWorkingSetEntityId);
		if (WorkingSetRequest.bWasSetCreationLocallyRequested)
		{
			WorkingSetRequest.bWasSetCreationLocallyRequested = false;

			// Skipping over the first request as it's the initial set create.
			for (int32 PendingUpdateIndex = 1; PendingUpdateIndex < WorkingSetRequest.SetData.Num(); ++PendingUpdateIndex)
			{
				const FLocalWorkingSetRequest& PendingRequest = WorkingSetRequest.SetData[PendingUpdateIndex];
				Connection->SendComponentUpdate(UpdatedWorkingSetEntityId, PendingRequest.Request.CreateComponentUpdate(), {});
			}
		}
	}

	for (const Worker_EntityId RemovedWorkingSetEntityId : DataStorage->GetRemovedWorkingSets())
	{
		FLocalWorkingSetData& WorkingSetRequest = LocalWorkingSetRequests.FindChecked(RemovedWorkingSetEntityId);
		for (const FLocalWorkingSetRequest& PendingRequest : WorkingSetRequest.SetData)
		{
			PendingRequest.Callback.ExecuteIfBound(/*bWasSuccessful =*/false);
		}
		LocalWorkingSetRequests.Remove(RemovedWorkingSetEntityId);
	}

	for (auto& WorkingSetRequests : LocalWorkingSetRequests)
	{
		const Worker_EntityId MarkerEntityId = WorkingSetRequests.Key;

		const FWorkingSetCommonData* WorkingSet = DataStorage->GetWorkingSets().Find(MarkerEntityId);
		if (WorkingSet == nullptr)
		{
			// We didn't receive this working set yet.
			continue;
		}
		for (auto PendingLocalChangeIterator = WorkingSetRequests.Value.SetData.CreateIterator(); PendingLocalChangeIterator;
			 ++PendingLocalChangeIterator)
		{
			const FWorkingSetMarkerRequest& PendingRequest = PendingLocalChangeIterator->Request;
			const FWorkingSetState& Response = WorkingSet->ConfirmedState;
			if (PendingRequest.RequestedState.Epoch > Response.Epoch)
			{
				// The request is yet to be confirmed by the server.
				continue;
			}
			auto IsRequestFulfilled = [&PendingRequest, &Response] {
				return PendingRequest.RequestedState.MemberEntities.Num() == Response.MemberEntities.Num()
					   && PendingRequest.RequestedState.MemberEntities.Intersect(Response.MemberEntities).Num()
							  == PendingRequest.RequestedState.MemberEntities.Num();
			};
			const bool bWasRequestFulfilled = Response.Epoch == PendingRequest.RequestedState.Epoch && IsRequestFulfilled();

			// The request is complete.
			PendingLocalChangeIterator->Callback.ExecuteIfBound(bWasRequestFulfilled);
			PendingLocalChangeIterator.RemoveCurrent();
		}
	}
}

void FWorkingSetChangesHandler::ApplyLocalWorkingSetUpdateRequest(Worker_EntityId WorkingSetEntityId,
																  const FWorkingSetMarkerRequest& Request, FOnWorkingSetEvent Callback)
{
	FWorkingSetCommonData* WorkingSet = DataStorage->GetWorkingSets().Find(WorkingSetEntityId);
	if (WorkingSet == nullptr)
	{
		UE_LOG(LogSpatialWorkingSetsHandler, Warning, TEXT("Trying to update a non-existent working set. MarkerEntityId: %lld"),
			   WorkingSetEntityId);
		return;
	}
	auto* PendingLocalRequests = LocalWorkingSetRequests.Find(WorkingSetEntityId);
	if (!ensureMsgf(PendingLocalRequests != nullptr,
					TEXT("ChangesHandler's internal state should contain all working sets in view! MarkerEntityId: %lld"),
					WorkingSetEntityId))
	{
		return;
	}
	WorkingSet->RequestedState = Request.RequestedState;
	PendingLocalRequests->SetData.Emplace(FLocalWorkingSetRequest{ Request, Callback });
	if (!PendingLocalRequests->bWasSetCreationLocallyRequested)
	{
		Connection->SendComponentUpdate(WorkingSetEntityId, Request.CreateComponentUpdate());
	}
}

void FWorkingSetChangesHandler::ApplyLocalWorkingSetCreationRequest(Worker_EntityId WorkingSetEntityId,
																	const FWorkingSetMarkerRequest& Request, FOnWorkingSetEvent Callback)
{
	const FWorkingSetCommonData* WorkingSet = DataStorage->GetWorkingSets().Find(WorkingSetEntityId);

	if (!ensure(WorkingSet == nullptr))
	{
		return;
	}

	FLocalWorkingSetRequest PendingRequest;
	PendingRequest.Callback = Callback;
	PendingRequest.Request = Request;

	FLocalWorkingSetData PendingCreationWorkingSetData;
	PendingCreationWorkingSetData.SetData = { PendingRequest };
	PendingCreationWorkingSetData.bWasSetCreationLocallyRequested = true;

	LocalWorkingSetRequests.Add(WorkingSetEntityId, PendingCreationWorkingSetData);

	Connection->SendCreateEntityRequest(CreateWorkingSetComponents(WorkerStagingPartitionId, Request), WorkingSetEntityId);
}

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
