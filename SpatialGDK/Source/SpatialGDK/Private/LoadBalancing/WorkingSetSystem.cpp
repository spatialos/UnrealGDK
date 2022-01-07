// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LoadBalancing/WorkingSetSystem.h"

#include "Algo/AllOf.h"
#include "Algo/Copy.h"

#include "Interop/WorkingSetsCommon.h"
#include "LoadBalancing/LoadBalancingTypes.h"
#include "LoadBalancing/PartitionManager.h"
#include "Schema/StandardLibrary.h"
#include "Schema/WorkingSet.h"
#include "SpatialView/ComponentData.h"
#include "SpatialView/EntityComponentTypes.h"
#include "SpatialView/EntityDelta.h"
#include "SpatialView/SubView.h"
#include "SpatialView/ViewCoordinator.h"

DEFINE_LOG_CATEGORY_STATIC(LogSpatialWorkingSetSystem, Log, All);

namespace SpatialGDK
{
struct FWorkingSetSystem::FImpl
{
	struct FReadWorkingSetData
	{
		TFunction<const FWorkingSetState*(Worker_EntityId)> GetConfirmedWorkingSet;
		FEntityToWorkerMap EntityToOwningWorker;
		const TMap<Worker_EntityId_Key, Worker_EntityId>& ActorOwnership;
	};

	struct FWorkingSetDataUpdates
	{
		TMap<Worker_EntityId_Key, FWorkingSetState> WorkingSetUpdates;
		TMap<Worker_EntityId_Key, FWorkingSetCommonData> WorkingSetAdds;

		TSet<Worker_EntityId_Key> NewMarkers;
		TSet<Worker_EntityId_Key> RemovedMarkers;
	};

	struct FReadWorkingSetUpdates
	{
		TMap<Worker_EntityId_Key, TSet<Worker_EntityId_Key>> RequestedActorMembership;
		TMap<Worker_EntityId_Key, Worker_EntityId> RequestedActorMembershipReleases;

		TMap<Worker_EntityId_Key, Worker_EntityId> LeaderChanges;
	};

	static FWorkingSetDataUpdates GatherWorkingSetRequests(const FSubView& MarkerEntitiesSubview);

	static FReadWorkingSetUpdates GatherWorkingSetUpdates(const FReadWorkingSetData& In, const FWorkingSetDataUpdates& DataUpdates);

	static TSet<Worker_EntityId_Key> DeclineConflictingRequests(const FReadWorkingSetData& In, const FWorkingSetDataUpdates& DataUpdates,
																const FReadWorkingSetUpdates& Updates);

	struct FResolvedWorkingSetChanges
	{
		TMap<Worker_EntityId_Key, FWorkingSetState> WorkingSetUpdates;
		TMap<Worker_EntityId_Key, Worker_EntityId> ActorOwnershipUpdates;
		TSet<Worker_EntityId_Key> ReleasedEntities;
		TSet<Worker_EntityId_Key> CapturedEntities;
		TSet<Worker_EntityId_Key> NewLeaders;
	};

	static FResolvedWorkingSetChanges ResolveWorkingSetChanges(const FReadWorkingSetData& In, const FWorkingSetDataUpdates& DataUpdates,
															   const FReadWorkingSetUpdates& Updates,
															   const TSet<Worker_EntityId_Key>& DeclinedRequests);
};

FWorkingSetSystem::FEntityToWorkerMap FWorkingSetSystem::CreateEntityToOwningWorkerMap(const EntityView& View,
																					   const FPartitionManager& PartitionManager)
{
	return [&View, &PartitionManager](Worker_EntityId EntityId) -> Worker_EntityId {
		const EntityViewElement* Entity = View.Find(EntityId);
		if (!ensure(Entity != nullptr))
		{
			return SpatialConstants::INVALID_ENTITY_ID;
		}

		const ComponentData* AuthorityDelegationComponent =
			Entity->Components.FindByPredicate(ComponentIdEquality{ AuthorityDelegation::ComponentId });
		AuthorityDelegation Delegation(AuthorityDelegationComponent->GetUnderlying());

		const Worker_PartitionId ServerAuthPartitionId = Delegation.Delegations.FindChecked(SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID);

		{
			// This considers an edge case where an entity is still in staging partition, which the StrategyWorker
			// didn't create and thus doesn't have knowledge of.
			const EntityViewElement& PartitionEntity = View.FindChecked(ServerAuthPartitionId);

			const ComponentData* StagingPartitionComponent =
				PartitionEntity.Components.FindByPredicate(ComponentIdEquality{ SpatialConstants::STAGING_PARTITION_COMPONENT_ID });
			if (StagingPartitionComponent != nullptr)
			{
				return Schema_GetEntityId(StagingPartitionComponent->GetFields(), 1);
			}
		}

		const FPartitionHandle PartitionHandle = PartitionManager.GetPartition(ServerAuthPartitionId);
		check(PartitionHandle.IsValid());
		const FLBWorkerHandle WorkerHandle = PartitionManager.GetWorkerForPartition(PartitionHandle);
		check(WorkerHandle.IsValid());
		return PartitionManager.GetSystemWorkerEntityIdForWorker(WorkerHandle);
	};
}

FWorkingSetSystem::FWorkingSetSystem(FEntityToWorkerMap InEntityToOwningWorker)
	: EntityToOwningWorker(InEntityToOwningWorker)
{
}

FWorkingSetSystem::~FWorkingSetSystem() {}

void FWorkingSetSystem::Advance(const FSubView& MarkerEntitiesSubview)
{
	const FImpl::FReadWorkingSetData In{ [this](Worker_EntityId MarkerEntityId) -> const FWorkingSetState* {
											const FWorkingSetData* WorkingSet = WorkingSets.Find(MarkerEntityId);
											if (WorkingSet != nullptr)
											{
												return &WorkingSet->ApprovedState;
											}
											return nullptr;
										},
										 EntityToOwningWorker, ActorOwnership };

	FImpl::FWorkingSetDataUpdates DataUpdates = FImpl::GatherWorkingSetRequests(MarkerEntitiesSubview);
	{
		CurrentRequests.Append(DataUpdates.WorkingSetUpdates);
		for (const Worker_EntityId_Key RemovedMarker : DataUpdates.RemovedMarkers)
		{
			CurrentRequests.Remove(RemovedMarker);
		}
		DataUpdates.WorkingSetUpdates = CurrentRequests;
	}

	const FImpl::FReadWorkingSetUpdates Updates = FImpl::GatherWorkingSetUpdates(In, DataUpdates);

	for (const Worker_EntityId_Key RemovedMarkerEntityId : DataUpdates.RemovedMarkers)
	{
		// This is a marker entity being destroyed by an auth worker.
		FWorkingSetData SetBeingDeleted;
		verify(WorkingSets.RemoveAndCopyValue(RemovedMarkerEntityId, SetBeingDeleted));
		for (const Worker_EntityId_Key MemberEntityId : SetBeingDeleted.ApprovedState.MemberEntities)
		{
			ActorOwnership.Remove(MemberEntityId);
		}
		for (const Worker_EntityId_Key MemberEntityId : SetBeingDeleted.TargetState.MemberEntities)
		{
			ActorOwnership.Remove(MemberEntityId);
		}
	}

	const TSet<Worker_EntityId_Key> DeclinedRequests = FImpl::DeclineConflictingRequests(In, DataUpdates, Updates);

	const FImpl::FResolvedWorkingSetChanges ResolvedChanges = FImpl::ResolveWorkingSetChanges(In, DataUpdates, Updates, DeclinedRequests);

	TSet<Worker_EntityId_Key> AddedSets;
	for (const auto& WorkingSetUpdate : DataUpdates.WorkingSetAdds)
	{
		AddedSets.Emplace(WorkingSetUpdate.Key);
		WorkingSets.Add(WorkingSetUpdate.Key,
						FWorkingSetData{ WorkingSetUpdate.Value.ConfirmedState, WorkingSetUpdate.Value.ConfirmedState });
	}
	for (const auto& WorkingSetUpdate : ResolvedChanges.WorkingSetUpdates)
	{
		AddedSets.Remove(WorkingSetUpdate.Key);
		WorkingSets.FindChecked(WorkingSetUpdate.Key).TargetState = WorkingSetUpdate.Value;
	}
	ensureMsgf(AddedSets.Num() == 0, TEXT("Expected all added sets to receive an update but sets %s didn't receive them"),
			   *FString::JoinBy(AddedSets, TEXT(", "), [](const Worker_EntityId_Key SetWithoutUpdate) {
				   return FString::Printf(TEXT("%lld"), SetWithoutUpdate);
			   }));

	ProgressActorOwnership.Empty(ResolvedChanges.ActorOwnershipUpdates.Num());
	for (const auto& ActorOwnershipUpdate : ResolvedChanges.ActorOwnershipUpdates)
	{
		ProgressActorOwnership.Emplace(ActorOwnershipUpdate.Key, ActorOwnershipUpdate.Value);
	}
	for (const Worker_EntityId_Key ReleasedEntityId : ResolvedChanges.ReleasedEntities)
	{
		ActorOwnership.Remove(ReleasedEntityId);
	}

	FWorkingSetsChangeState NewChange;
	NewChange.EntitiesToForceLoadBalance = ResolvedChanges.ReleasedEntities;
	NewChange.EntitiesToForceLoadBalance.Append(ResolvedChanges.NewLeaders);
	NewChange.ReleasedEntities = ResolvedChanges.ReleasedEntities;
	NewChange.NewMembers = ResolvedChanges.CapturedEntities;
	NewChange.NewMarkers = DataUpdates.NewMarkers;

	ChangeDelta = NewChange.CreateDiff(LastChangeState);
	LastChangeState = NewChange;
}

FWorkingSetSystem::FWorkingSetsChangeState FWorkingSetSystem::FWorkingSetsChangeState::CreateDiff(
	const FWorkingSetsChangeState& Other) const
{
	FWorkingSetsChangeState Diff;
	Diff.EntitiesToForceLoadBalance = EntitiesToForceLoadBalance.Difference(Other.EntitiesToForceLoadBalance);
	Diff.ReleasedEntities = ReleasedEntities.Difference(Other.ReleasedEntities);
	Diff.NewMembers = NewMembers.Difference(Other.NewMembers);
	Diff.NewMarkers = NewMarkers.Difference(Other.NewMarkers);
	return Diff;
}

FWorkingSetSystem::FImpl::FWorkingSetDataUpdates FWorkingSetSystem::FImpl::GatherWorkingSetRequests(const FSubView& MarkerEntitiesSubview)
{
	FWorkingSetDataUpdates Updates;
	for (const EntityDelta& Delta : MarkerEntitiesSubview.GetViewDelta().EntityDeltas)
	{
		const Worker_EntityId MarkerEntityId = Delta.EntityId;

		switch (Delta.Type)
		{
		case EntityDelta::ADD:
		case EntityDelta::UPDATE:
		{
			const EntityViewElement& Entity = MarkerEntitiesSubview.GetView()[MarkerEntityId];

			FWorkingSetCommonData BaseData(Entity);

			Updates.WorkingSetUpdates.Emplace(MarkerEntityId, BaseData.RequestedState);

			if (Delta.Type == EntityDelta::ADD)
			{
				Updates.NewMarkers.Emplace(MarkerEntityId);
				Updates.WorkingSetAdds.Add(MarkerEntityId, BaseData);
			}
		}
		break;
		case EntityDelta::REMOVE:
		{
			Updates.RemovedMarkers.Emplace(MarkerEntityId);
		}
		break;
		default:
			checkNoEntry();
		}
	}
	return Updates;
}

FWorkingSetSystem::FImpl::FReadWorkingSetUpdates FWorkingSetSystem::FImpl::GatherWorkingSetUpdates(
	const FReadWorkingSetData& In, const FWorkingSetDataUpdates& DataUpdates)
{
	FReadWorkingSetUpdates Updates;
	for (const auto& Update : DataUpdates.WorkingSetUpdates)
	{
		const Worker_EntityId MarkerEntityId = Update.Key;
		const FWorkingSetState& RequestedSetState = Update.Value;

		const FWorkingSetState* ConfirmedSetState = In.GetConfirmedWorkingSet(MarkerEntityId);

		if (ConfirmedSetState == nullptr)
		{
			// This is a new marker entity.
			for (const Worker_EntityId_Key MemberEntityId : RequestedSetState.MemberEntities)
			{
				Updates.RequestedActorMembership.FindOrAdd(MemberEntityId).Emplace(MarkerEntityId);
			}
		}
		else
		{
			// This marker entity must be updated with new requests.
			const FWorkingSetState& ExistingSet = *ConfirmedSetState;

			const auto NewFreedEntities = ExistingSet.MemberEntities.Difference(RequestedSetState.MemberEntities);
			for (const Worker_EntityId_Key RemovedMember : NewFreedEntities)
			{
				checkf(!Updates.RequestedActorMembershipReleases.Contains(RemovedMember),
					   TEXT("Entities may only be released once from a given working set! DoubleReleasingMarkerEntityId: %lld "
							"ReleasedEntityId: %lld PreviouslyReleasingMarkerEntityId: %lld"),
					   MarkerEntityId, RemovedMember, Updates.RequestedActorMembershipReleases[RemovedMember]);

				Updates.RequestedActorMembershipReleases.Emplace(RemovedMember, MarkerEntityId);
			}

			const auto NewMemberEntities = RequestedSetState.MemberEntities.Difference(ExistingSet.MemberEntities);
			for (const Worker_EntityId_Key NewMember : NewMemberEntities)
			{
				Updates.RequestedActorMembership.FindOrAdd(NewMember).Emplace(MarkerEntityId);
			}

			if (ExistingSet.LeaderEntityId != RequestedSetState.LeaderEntityId)
			{
				Updates.LeaderChanges.Emplace(MarkerEntityId, RequestedSetState.LeaderEntityId);
			}
		}
	}
	for (const Worker_EntityId_Key RemovedMarker : DataUpdates.RemovedMarkers)
	{
		for (const Worker_EntityId_Key ReleasedEntity : In.GetConfirmedWorkingSet(RemovedMarker)->MemberEntities)
		{
			Updates.RequestedActorMembershipReleases.Emplace(ReleasedEntity, RemovedMarker);
		}
	}
	return Updates;
}

TSet<Worker_EntityId_Key> FWorkingSetSystem::FImpl::DeclineConflictingRequests(const FReadWorkingSetData& In,
																			   const FWorkingSetDataUpdates& DataUpdates,
																			   const FReadWorkingSetUpdates& Updates)
{
	struct FConflictingWorkingSetData
	{
		Worker_EntityId MarkerEntityId;
		TSet<Worker_EntityId_Key> ConflictingEntities;

		static bool Compare(const FConflictingWorkingSetData& Lhs, const FConflictingWorkingSetData& Rhs, const FWorkingSetState& LhsState,
							const FWorkingSetState& RhsState, FEntityToWorkerMap MapEntityToOwningWorker)
		{
			// Special case: prioritize sets when all entities in a set are owned by the same worker.
			auto IsOwnedByOneWorker = [&MapEntityToOwningWorker](const FWorkingSetState& State) {
				TSet<Worker_EntityId_Key> WorkersOwningSetMembers;
				Algo::Transform(State.MemberEntities, WorkersOwningSetMembers, MapEntityToOwningWorker);
				return WorkersOwningSetMembers.Num() == 1;
			};

			const bool bIsLhsOwnedByOneWorker = IsOwnedByOneWorker(LhsState);
			const bool bIsRhsOwnedByOneWorker = IsOwnedByOneWorker(RhsState);
			if (bIsLhsOwnedByOneWorker != bIsRhsOwnedByOneWorker)
			{
				return bIsLhsOwnedByOneWorker;
			}

			const TSet<Worker_EntityId_Key> SetOverlap = Lhs.ConflictingEntities.Intersect(Rhs.ConflictingEntities);
			if (SetOverlap.Num() > 0)
			{
				// The sets overlap, so let's pick the one with less conflicting entities.
				if (Lhs.ConflictingEntities.Num() != Rhs.ConflictingEntities.Num())
				{
					return Lhs.ConflictingEntities.Num() < Rhs.ConflictingEntities.Num();
				}
			}
			// The sets either don't overlap at all, or have the same number of conflicting entities;
			// any ordering will be fine, so let's disambiguate using marker entity IDs.
			return Lhs.MarkerEntityId < Rhs.MarkerEntityId;
		}
	};
	TMap<Worker_EntityId_Key, FConflictingWorkingSetData> ConflictingMarkerEntities;

	TSet<Worker_EntityId_Key> RequestsToDecline;

	for (const auto& ActorMembershipRequest : Updates.RequestedActorMembership)
	{
		const Worker_EntityId MemberEntityId = ActorMembershipRequest.Key;
		const TSet<Worker_EntityId_Key>& RequestingMarkerEntities = ActorMembershipRequest.Value;

		const Worker_EntityId* CurrentlyOwningMarkerEntityId = In.ActorOwnership.Find(MemberEntityId);
		if (CurrentlyOwningMarkerEntityId != nullptr)
		{
			// The only case where this set request could be approved is if this entity was just released.
			const Worker_EntityId* ReleasingMarkerEntityId = Updates.RequestedActorMembershipReleases.Find(MemberEntityId);
			const bool bWasReleasedByCurrentlyOwningSet =
				ReleasingMarkerEntityId != nullptr && ensure(*ReleasingMarkerEntityId == *CurrentlyOwningMarkerEntityId);
			if (!bWasReleasedByCurrentlyOwningSet)
			{
				// This entity is still held by a working set; all requests regarding this entity are declined.
				RequestsToDecline.Append(RequestingMarkerEntities);
				continue;
			}
		}

		if (RequestingMarkerEntities.Num() > 1)
		{
			for (const Worker_EntityId_Key ConflictingMarkerEntityId : RequestingMarkerEntities)
			{
				ConflictingMarkerEntities.FindOrAdd(ConflictingMarkerEntityId, { ConflictingMarkerEntityId })
					.ConflictingEntities.Emplace(MemberEntityId);
			}
		}
	}

	// The ordering here is for the sake of consistency and potentially better resolution of conflicts.
	ConflictingMarkerEntities.ValueSort([&DataUpdates, EntityToOwningWorker = In.EntityToOwningWorker](
											const FConflictingWorkingSetData& Lhs, const FConflictingWorkingSetData& Rhs) {
		return FConflictingWorkingSetData::Compare(Lhs, Rhs, DataUpdates.WorkingSetUpdates.FindChecked(Lhs.MarkerEntityId),
												   DataUpdates.WorkingSetUpdates.FindChecked(Rhs.MarkerEntityId), EntityToOwningWorker);
	});

	TSet<Worker_EntityId_Key> AssignedConflictingEntities;
	for (const auto& ConflictingMarkerEntity : ConflictingMarkerEntities)
	{
		const Worker_EntityId MarkerEntityId = ConflictingMarkerEntity.Key;
		bool bIsOverlappingWithAssignedEntities = false;
		for (const Worker_EntityId_Key RequestedEntityId : ConflictingMarkerEntity.Value.ConflictingEntities)
		{
			if (AssignedConflictingEntities.Contains(RequestedEntityId))
			{
				RequestsToDecline.Emplace(MarkerEntityId);
				bIsOverlappingWithAssignedEntities = true;
				break;
			}
		}
		if (!bIsOverlappingWithAssignedEntities)
		{
			AssignedConflictingEntities.Append(ConflictingMarkerEntity.Value.ConflictingEntities);
		}
	}
	return RequestsToDecline;
}

FWorkingSetSystem::FImpl::FResolvedWorkingSetChanges FWorkingSetSystem::FImpl::ResolveWorkingSetChanges(
	const FReadWorkingSetData& In, const FWorkingSetDataUpdates& DataUpdates, const FReadWorkingSetUpdates& Updates,
	const TSet<Worker_EntityId_Key>& DeclinedRequests)
{
	FResolvedWorkingSetChanges ResolvedChanges;

	TSet<Worker_EntityId_Key> NewlyCapturedEntities;
	TSet<Worker_EntityId_Key> NewlyReleasedEntities;
	TMap<Worker_EntityId_Key, Worker_EntityId>& ResolvedActorOwnershipUpdates = ResolvedChanges.ActorOwnershipUpdates;

	for (const auto& RequestedWorkingSet : DataUpdates.WorkingSetUpdates)
	{
		const Worker_EntityId MarkerEntityId = RequestedWorkingSet.Key;

		const FWorkingSetState& Data = RequestedWorkingSet.Value;
		if (DeclinedRequests.Contains(MarkerEntityId))
		{
			// No changes to internal LB state required as this request is declined.
			const FWorkingSetState* LastConfirmedState = In.GetConfirmedWorkingSet(MarkerEntityId);
			FWorkingSetState GeneratedTargetState = LastConfirmedState != nullptr ? *LastConfirmedState : FWorkingSetState();
			// Taking epoch from the request but the set itself from the response.
			GeneratedTargetState.Epoch = Data.Epoch;
			ResolvedChanges.WorkingSetUpdates.Emplace(MarkerEntityId, GeneratedTargetState);
		}
		else
		{
			TOptional<FWorkingSetState> MaybePreviousData;
			{
				const FWorkingSetState* PreviousDataPtr = In.GetConfirmedWorkingSet(MarkerEntityId);
				if (PreviousDataPtr != nullptr)
				{
					MaybePreviousData = *PreviousDataPtr;
				}
			}

			const FWorkingSetState& Updated = ResolvedChanges.WorkingSetUpdates.Emplace(MarkerEntityId, Data);

			TSet<Worker_EntityId_Key> SetReleasedEntities;
			TSet<Worker_EntityId_Key> SetCapturedEntities;
			if (MaybePreviousData)
			{
				SetReleasedEntities = MaybePreviousData->MemberEntities.Difference(Updated.MemberEntities);
				SetCapturedEntities = Updated.MemberEntities.Difference(MaybePreviousData->MemberEntities);
			}
			else
			{
				SetCapturedEntities = Updated.MemberEntities;
			}

			for (const Worker_EntityId_Key CapturedEntityId : SetCapturedEntities)
			{
				bool bAlreadyInSet = false;
				NewlyCapturedEntities.Emplace(CapturedEntityId, &bAlreadyInSet);
				check(!bAlreadyInSet);

				checkf(!ResolvedActorOwnershipUpdates.Contains(CapturedEntityId),
					   TEXT("Trying to assign an entity to multiple working sets! EntityId: %lld NewMarkerEntityId: %lld "
							"OldMarkerEntityId: %lld"),
					   CapturedEntityId, MarkerEntityId, ResolvedActorOwnershipUpdates[CapturedEntityId]);
				ResolvedActorOwnershipUpdates.Emplace(CapturedEntityId, MarkerEntityId);
			}
			for (const Worker_EntityId_Key ReleasedEntityId : SetReleasedEntities)
			{
				NewlyReleasedEntities.Emplace(ReleasedEntityId);
			}
			const Worker_EntityId* ChangedLeader = Updates.LeaderChanges.Find(MarkerEntityId);
			if (ChangedLeader != nullptr)
			{
				ResolvedChanges.NewLeaders.Emplace(*ChangedLeader);
			}
		}
	}

	for (const auto& ReleasedEntity : Updates.RequestedActorMembershipReleases)
	{
		NewlyReleasedEntities.Emplace(ReleasedEntity.Key);
	}

	auto UpdatedEntities = NewlyReleasedEntities.Intersect(NewlyCapturedEntities);
	auto CapturedEntities = NewlyCapturedEntities.Difference(NewlyReleasedEntities);
	auto ActuallyReleasedEntities = NewlyReleasedEntities.Difference(NewlyCapturedEntities);

	ResolvedChanges.ReleasedEntities = MoveTemp(ActuallyReleasedEntities);
	ResolvedChanges.CapturedEntities = MoveTemp(CapturedEntities);
	return ResolvedChanges;
}

void FWorkingSetSystem::UpdateMigratingEntities(const TSet<Worker_EntityId_Key>& CurrentlyMigratingEntities, ISpatialOSWorker& Connection)
{
	for (auto& WorkingSet : WorkingSets)
	{
		if (WorkingSet.Value.ApprovedState.Epoch == WorkingSet.Value.TargetState.Epoch)
		{
			// This working set is up to date.
			continue;
		}
		const FWorkingSetState& Request = WorkingSet.Value.TargetState;
		const TSet<Worker_EntityId_Key> RequestedMigratingEntities = CurrentlyMigratingEntities.Intersect(Request.MemberEntities);
		if (RequestedMigratingEntities.Num() != 0)
		{
			// Some entities from this requested set are still migrating, need to wait before confirming them.
			continue;
		}
		FWorkingSetState& Response = WorkingSet.Value.ApprovedState;
		Response = Request;

		for (const Worker_EntityId_Key MemberEntityId : Response.MemberEntities)
		{
			ActorOwnership.Emplace(MemberEntityId, WorkingSet.Key);
		}

		UE_LOG(LogSpatialWorkingSetSystem, Display,
			   TEXT("Approving working set after migrating all entities. MarkerEntityId: %lld Generation: %lld EntityCount: %d"),
			   WorkingSet.Key, Response.Epoch, Response.MemberEntities.Num());

		Connection.SendComponentUpdate(WorkingSet.Key, FWorkingSetMarkerResponse(Response).CreateComponentUpdate());

		for (const Worker_EntityId_Key MemberEntityId : Response.MemberEntities)
		{
			FWorkingSetMember Component;
			Component.WorkingSetMarkerEntityId = WorkingSet.Key;
			Connection.SendComponentUpdate(MemberEntityId, Component.CreateComponentUpdate());
		}
	}

	for (const Worker_EntityId_Key FreedEntityId : ChangeDelta.ReleasedEntities)
	{
		FWorkingSetMember Component;
		Component.WorkingSetMarkerEntityId = SpatialConstants::INVALID_ENTITY_ID;
		Connection.SendComponentUpdate(FreedEntityId, Component.CreateComponentUpdate());
	}
}

Worker_EntityId FWorkingSetSystem::GetOwningMarkerEntityId(Worker_EntityId MemberEntityId) const
{
	const Worker_EntityId* TransientOwningMarkerEntityId = ProgressActorOwnership.Find(MemberEntityId);
	if (TransientOwningMarkerEntityId != nullptr)
	{
		return *TransientOwningMarkerEntityId;
	}
	return ActorOwnership.FindRef(MemberEntityId);
}

bool FWorkingSetSystem::IsWorkingLeaderEntity(Worker_EntityId MemberEntityId) const
{
	const Worker_EntityId LeaderEntityId = ActorOwnership.FindRef(MemberEntityId);

	const auto* Set = WorkingSets.Find(LeaderEntityId);

	return Set != nullptr && Set->TargetState.LeaderEntityId == MemberEntityId;
}

bool FWorkingSetSystem::IsMarkerEntity(Worker_EntityId EntityId) const
{
	return WorkingSets.Contains(EntityId);
}

TOptional<FWorkingSetSystem::FWorkingSet> FWorkingSetSystem::GetSet(Worker_EntityId MarkerEntityId) const
{
	const auto* WorkingSet = WorkingSets.Find(MarkerEntityId);
	if (WorkingSet)
	{
		return FWorkingSet{ WorkingSet->TargetState.MemberEntities, WorkingSet->ApprovedState.MemberEntities,
							WorkingSet->TargetState.LeaderEntityId };
	}
	return {};
}
} // namespace SpatialGDK
