// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/WorkingSetsCommon.h"

#include "SpatialView/EntityComponentTypes.h"
#include "SpatialView/EntityDelta.h"
#include "SpatialView/EntityView.h"

#include "Schema/ActorGroupMember.h"
#include "Schema/AuthorityIntent.h"
#include "Schema/Interest.h"
#include "Schema/StandardLibrary.h"
#include "SpatialView/ComponentData.h"
#include "SpatialView/ViewCoordinator.h"

namespace SpatialGDK
{
DEFINE_LOG_CATEGORY_STATIC(LogSpatialWorkingSetsCommon, Display, All);

FWorkingSetCommonData::FWorkingSetCommonData(const EntityViewElement& Entity)
	: RequestedState(
		FWorkingSetMarkerRequest(*Entity.Components.FindByPredicate(ComponentIdEquality{ FWorkingSetMarkerRequest::ComponentId }))
			.RequestedState)
	, ConfirmedState(
		  FWorkingSetMarkerResponse(*Entity.Components.FindByPredicate(ComponentIdEquality{ FWorkingSetMarkerResponse::ComponentId }))
			  .ConfirmedState)
{
}

void FWorkingSetCommonData::Update(const EntityDelta& Delta)
{
	if (!ensureMsgf(Delta.Type == EntityDelta::UPDATE, TEXT("FWorkingSet::Update must be called with UPDATE entity deltas only!")))
	{
		return;
	}
	for (const ComponentChange& Change : Delta.ComponentUpdates)
	{
		const Schema_Object* UpdatedObject = nullptr;
		switch (Change.Type)
		{
		case ComponentChange::UPDATE:
			UpdatedObject = Schema_GetComponentUpdateFields(Change.Update);
			break;
		case ComponentChange::COMPLETE_UPDATE:
			UpdatedObject = Schema_GetComponentDataFields(Change.CompleteUpdate.Data);
			break;
		default:
			unimplemented();
		}
		if (!ensure(UpdatedObject != nullptr))
		{
			continue;
		}
		if (Change.ComponentId == FWorkingSetMarkerResponse::ComponentId)
		{
			ConfirmedState.ApplySchema(*UpdatedObject);
		}
		else if (Change.ComponentId == FWorkingSetMarkerRequest::ComponentId)
		{
			RequestedState.ApplySchema(*UpdatedObject);
		}
	}
}

void FWorkingSetDataStorage::Advance(const FSubView& WorkingSetMarkerEntitiesSubview)
{
	EntitiesToRefresh.Empty();
	UpdatedWorkingSets.Empty();
	RemovedWorkingSets.Empty();
	for (const EntityDelta& Delta : WorkingSetMarkerEntitiesSubview.GetViewDelta().EntityDeltas)
	{
		const Worker_EntityId MarkerEntityId = Delta.EntityId;
		switch (Delta.Type)
		{
		case EntityDelta::TEMPORARILY_REMOVED:
		case EntityDelta::ADD:
		{
			UpdatedWorkingSets.Emplace(MarkerEntityId);
			const EntityViewElement& Entity = WorkingSetMarkerEntitiesSubview.GetView()[MarkerEntityId];
			const auto& AddedSet = DataStorage.Emplace(MarkerEntityId, Entity);

			UE_LOG(LogSpatialWorkingSetsCommon, Display, TEXT("New working set discovered. MarkerEntityId: %lld EntityCount: %d"),
				   MarkerEntityId, AddedSet.ConfirmedState.MemberEntities.Num());

			for (const Worker_EntityId MemberEntity : AddedSet.ConfirmedState.MemberEntities)
			{
				EntityWorkingSetMembership.Emplace(MemberEntity, MarkerEntityId);

				EntitiesToRefresh.Emplace(MemberEntity);
			}
		}
		break;
		case EntityDelta::REMOVE:
			RemovedWorkingSets.Emplace(MarkerEntityId);
			UE_LOG(LogSpatialWorkingSetsCommon, Display, TEXT("Working set lost. MarkerEntityId: %lld EntityCount: %d"), MarkerEntityId,
				   DataStorage[MarkerEntityId].ConfirmedState.MemberEntities.Num());

			for (const Worker_EntityId MemberEntity : DataStorage[MarkerEntityId].ConfirmedState.MemberEntities)
			{
				EntityWorkingSetMembership.Remove(MemberEntity);

				EntitiesToRefresh.Emplace(MemberEntity);
			}
			DataStorage.Remove(MarkerEntityId);
			break;
		case EntityDelta::UPDATE:
		{
			UpdatedWorkingSets.Emplace(MarkerEntityId);
			auto& AddedSet = DataStorage[MarkerEntityId];

			const auto MembersBeforeUpdate = AddedSet.ConfirmedState.MemberEntities;

			AddedSet.Update(Delta);

			const auto& MembersAfterUpdate = AddedSet.ConfirmedState.MemberEntities;

			const auto ReleasedEntities = MembersBeforeUpdate.Difference(MembersAfterUpdate);
			for (const Worker_EntityId_Key ReleasedEntityId : ReleasedEntities)
			{
				EntityWorkingSetMembership.Remove(ReleasedEntityId);

				EntitiesToRefresh.Emplace(ReleasedEntityId);
			}

			const auto NewMemberEntities = MembersAfterUpdate.Difference(MembersBeforeUpdate);
			for (const Worker_EntityId_Key MemberEntity : NewMemberEntities)
			{
				EntityWorkingSetMembership.Emplace(MemberEntity, MarkerEntityId);

				EntitiesToRefresh.Emplace(MemberEntity);
			}
		}
		break;
		}
	}
}

TArray<ComponentData> CreateWorkingSetComponents(const Worker_PartitionId AuthServerWorkerPartitionId,
												 const FWorkingSetMarkerRequest& Request)
{
	TArray<ComponentData> Components;

	Components.Add(ComponentData(SpatialConstants::WORKING_SET_MARKER_TAG_COMPONENT_ID));
	Components.Emplace(Request.CreateComponentData());
	Components.Emplace(FWorkingSetMarkerResponse().CreateComponentData());

	auto Convert = [](Worker_ComponentData Component) {
		ComponentData Converted(OwningComponentDataPtr(Component.schema_type), Component.component_id);
		return Converted;
	};

	{
		Components.Emplace(Convert(Metadata(TEXT("WorkingSetMarker")).CreateComponentData()));
	}

	{
		// Give the auth UnrealWorker authority over the Request component, and the strategy worker auth over the Response component
		AuthorityDelegation Delegation;
		Delegation.Delegations.Emplace(SpatialConstants::STRATEGY_WORKER_AUTH_COMPONENT_SET_ID,
									   SpatialConstants::INITIAL_STRATEGY_PARTITION_ENTITY_ID);
		Delegation.Delegations.Emplace(SpatialConstants::LB_DELEGATION_AUTH_COMPONENT_SET_ID,
									   SpatialConstants::INITIAL_STRATEGY_PARTITION_ENTITY_ID);
		Delegation.Delegations.Emplace(SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID, AuthServerWorkerPartitionId);

		Components.Emplace(Convert(Delegation.CreateComponentData()));
	}
	{
		auto ApplyStrategyWorkerInterest = [](Query InOutQuery) {
			InOutQuery.ResultComponentIds = {
				FWorkingSetMarkerRequest::ComponentId,
				AuthorityIntentACK::ComponentId,
				SpatialConstants::STRATEGYWORKER_TAG_COMPONENT_ID,
				SpatialConstants::WORKING_SET_MARKER_TAG_COMPONENT_ID,
			};
			return InOutQuery;
		};

		auto ApplyServerWorkerInterest = [](Query InOutQuery) {
			InOutQuery.ResultComponentIds = {
				FWorkingSetMarkerResponse::ComponentId,
				SpatialConstants::LB_TAG_COMPONENT_ID,
				AuthorityIntentV2::ComponentId,
				SpatialConstants::WORKING_SET_MARKER_TAG_COMPONENT_ID,
			};

			return InOutQuery;
		};

		auto CreateSelfInterestQuery = [] {
			Query SelfInterestQuery;
			SelfInterestQuery.Constraint.bSelfConstraint = true;
			return SelfInterestQuery;
		};

		// Give the strategy worker and the auth UnrealWorker interest over the request/response components.
		Interest In;
		In.ComponentInterestMap.Add(SpatialConstants::STRATEGY_WORKER_AUTH_COMPONENT_SET_ID,
									{ { ApplyStrategyWorkerInterest(CreateSelfInterestQuery()) } });
		In.ComponentInterestMap.Add(SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID,
									{ { ApplyServerWorkerInterest(CreateSelfInterestQuery()) } });

		Components.Emplace(Convert(In.CreateComponentData()));
	}
	{
		Position Dummy;
		Components.Emplace(Convert(Dummy.CreateComponentData()));
	}
	{
		Components.Add(Convert(AuthorityIntentV2().CreateComponentData()));
		Components.Add(Convert(AuthorityIntentACK().CreateComponentData()));
		Components.Add(ComponentData(SpatialConstants::STRATEGYWORKER_TAG_COMPONENT_ID));
		Components.Add(ComponentData(SpatialConstants::LB_TAG_COMPONENT_ID));
	}

	return Components;
}

FSubView& CreateWorkingSetMarkersSubview(ViewCoordinator& Coordinator)
{
	return Coordinator.CreateSubView(SpatialConstants::WORKING_SET_MARKER_TAG_COMPONENT_ID, FSubView::NoFilter,
									 FSubView::NoDispatcherCallbacks);
}
} // namespace SpatialGDK
