#include "Interop/ActorSubviews.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/InitialOnlyFilter.h"
#include "Schema/Restricted.h"
#include "Schema/Tombstone.h"
#include "Schema/UnrealMetadata.h"
#include "SpatialView/EntityComponentTypes.h"
#include "SpatialView/EntityView.h"
#include "SpatialView/SubView.h"
#include "SpatialView/ViewCoordinator.h"

namespace SpatialGDK
{
namespace ActorSubviews
{
namespace MainActorSubviewSetup
{
static bool IsActorEntity(const Worker_EntityId EntityId, const EntityViewElement& Entity, USpatialNetDriver& NetDriver);

static TArray<FDispatcherRefreshCallback> GetCallbacks(ViewCoordinator& Coordinator);
} // namespace MainActorSubviewSetup

namespace AuthoritySubviewSetup
{
static bool IsAuthorityActorEntity(const Worker_EntityId EntityId, const EntityViewElement& Element);

static TArray<FDispatcherRefreshCallback> GetCallbacks(ViewCoordinator& Coordinator);
} // namespace AuthoritySubviewSetup

namespace AutonomousSubviewSetup
{
static bool IsAutonomousActorEntity(const Worker_EntityId EntityId, const EntityViewElement& Element);

static TArray<FDispatcherRefreshCallback> GetCallbacks(ViewCoordinator& Coordinator);
} // namespace AutonomousSubviewSetup

namespace SimulatedSubviewSetup
{
static bool IsSimulatedActorEntity(const Worker_EntityId EntityId, const EntityViewElement& Entity);

static TArray<FDispatcherRefreshCallback> GetCallbacks(ViewCoordinator& Coordinator);
} // namespace SimulatedSubviewSetup

static TArray<FDispatcherRefreshCallback> CombineCallbacks(TArray<FDispatcherRefreshCallback> Lhs,
														   const TArray<FDispatcherRefreshCallback>& Rhs)
{
	Lhs.Append(Rhs);
	return Lhs;
}

bool MainActorSubviewSetup::IsActorEntity(const Worker_EntityId EntityId, const EntityViewElement& Entity, USpatialNetDriver& NetDriver)
{
	if (Entity.Components.ContainsByPredicate(ComponentIdEquality{ Tombstone::ComponentId }))
	{
		return false;
	}

	if (NetDriver.AsyncPackageLoadFilter != nullptr)
	{
		const UnrealMetadata Metadata(
			Entity.Components.FindByPredicate(ComponentIdEquality{ SpatialConstants::UNREAL_METADATA_COMPONENT_ID })->GetUnderlying());

		if (!NetDriver.AsyncPackageLoadFilter->IsAssetLoadedOrTriggerAsyncLoad(EntityId, Metadata.ClassPath))
		{
			return false;
		}
	}

	if (NetDriver.InitialOnlyFilter != nullptr)
	{
		if (Entity.Components.ContainsByPredicate(ComponentIdEquality{ SpatialConstants::INITIAL_ONLY_PRESENCE_COMPONENT_ID }))
		{
			if (!NetDriver.InitialOnlyFilter->HasInitialOnlyDataOrRequestIfAbsent(EntityId))
			{
				return false;
			}
		}
	}

	// If we see a player controller component on this entity and we're a server we should hold it back until we
	// also have the partition component.
	return !NetDriver.IsServer()
		   || Entity.Components.ContainsByPredicate(ComponentIdEquality{ SpatialConstants::PLAYER_CONTROLLER_COMPONENT_ID })
				  == Entity.Components.ContainsByPredicate(ComponentIdEquality{ SpatialConstants::PARTITION_COMPONENT_ID });
}

TArray<FDispatcherRefreshCallback> MainActorSubviewSetup::GetCallbacks(ViewCoordinator& Coordinator)
{
	return { Coordinator.CreateComponentExistenceRefreshCallback(Tombstone::ComponentId), Coordinator.CreateComponentExistenceRefreshCallback(Partition::ComponentId), Coordinator.CreateComponentExistenceRefreshCallback(SpatialConstants::PLAYER_CONTROLLER_COMPONENT_ID) };
}

bool AuthoritySubviewSetup::IsAuthorityActorEntity(const Worker_EntityId EntityId, const EntityViewElement& Element)
{
	return Element.Authority.Contains(SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID);
}

TArray<FDispatcherRefreshCallback> AuthoritySubviewSetup::GetCallbacks(ViewCoordinator& Coordinator)
{
	return CombineCallbacks(MainActorSubviewSetup::GetCallbacks(Coordinator),
							{
								Coordinator.CreateAuthorityChangeRefreshCallback(SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID),
							});
}

bool AutonomousSubviewSetup::IsAutonomousActorEntity(const Worker_EntityId EntityId, const EntityViewElement& Element)
{
	return !AuthoritySubviewSetup::IsAuthorityActorEntity(EntityId, Element)
		   && Element.Authority.Contains(SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID);
}

TArray<FDispatcherRefreshCallback> AutonomousSubviewSetup::GetCallbacks(ViewCoordinator& Coordinator)
{
	return CombineCallbacks(AuthoritySubviewSetup::GetCallbacks(Coordinator),
							{
								Coordinator.CreateAuthorityChangeRefreshCallback(SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID),
							});
}

bool SimulatedSubviewSetup::IsSimulatedActorEntity(const Worker_EntityId EntityId, const EntityViewElement& Entity)
{
	return !AuthoritySubviewSetup::IsAuthorityActorEntity(EntityId, Entity)
		   && !AutonomousSubviewSetup::IsAutonomousActorEntity(EntityId, Entity);
}

TArray<FDispatcherRefreshCallback> SimulatedSubviewSetup::GetCallbacks(ViewCoordinator& Coordinator)
{
	return AutonomousSubviewSetup::GetCallbacks(Coordinator);
}

FSubView& CreateActorSubView(USpatialNetDriver& NetDriver)
{
	return CreateCustomActorSubView(/*CustomComponentId =*/{}, /*CustomPredicate =*/{}, /*CustomRefresh =*/{}, NetDriver);
}

FSubView& CreateCustomActorSubView(TOptional<Worker_ComponentId> MaybeCustomComponentId, TOptional<FFilterPredicate> MaybeCustomPredicate,
								   TOptional<TArray<FDispatcherRefreshCallback>> MaybeCustomRefresh, USpatialNetDriver& NetDriver)
{
	if (!MaybeCustomComponentId)
	{
		MaybeCustomComponentId = SpatialConstants::ACTOR_TAG_COMPONENT_ID;
	}

	if (MaybeCustomPredicate)
	{
		MaybeCustomPredicate = [CustomPredicate = MaybeCustomPredicate.GetValue(), &NetDriver](const Worker_EntityId EntityId,
																							   const EntityViewElement& Entity) {
			if (!MainActorSubviewSetup::IsActorEntity(EntityId, Entity, NetDriver))
			{
				return false;
			}

			return CustomPredicate(EntityId, Entity);
		};
	}
	else
	{
		MaybeCustomPredicate = [&NetDriver](const Worker_EntityId EntityId, const EntityViewElement& Entity) {
			return MainActorSubviewSetup::IsActorEntity(EntityId, Entity, NetDriver);
		};
	}

	if (MaybeCustomRefresh)
	{
		MaybeCustomRefresh->Append(MainActorSubviewSetup::GetCallbacks(NetDriver.Connection->GetCoordinator()));
	}
	else
	{
		MaybeCustomRefresh = MainActorSubviewSetup::GetCallbacks(NetDriver.Connection->GetCoordinator());
	}

	return NetDriver.Connection->GetCoordinator().CreateSubView(*MaybeCustomComponentId, *MaybeCustomPredicate, *MaybeCustomRefresh);
}

FSubView& CreateActorAuthSubView(USpatialNetDriver& NetDriver)
{
	return NetDriver.Connection->GetCoordinator().CreateSubView(SpatialConstants::ACTOR_AUTH_TAG_COMPONENT_ID, FSubView::NoFilter,
																FSubView::NoDispatcherCallbacks);
}

template <typename TCallable>
FFilterPredicate GetRoleFilterPredicate(TCallable RolePredicate, USpatialNetDriver& NetDriver)
{
	return [RolePredicate, &NetDriver](const Worker_EntityId EntityId, const EntityViewElement& Entity) -> bool {
		if (!MainActorSubviewSetup::IsActorEntity(EntityId, Entity, NetDriver))
		{
			return false;
		}

		return Invoke(RolePredicate, EntityId, Entity);
	};
}

FSubView& CreateAuthoritySubView(USpatialNetDriver& NetDriver)
{
	return NetDriver.Connection->GetCoordinator().CreateSubView(
		SpatialConstants::ACTOR_AUTH_TAG_COMPONENT_ID, GetRoleFilterPredicate(&AuthoritySubviewSetup::IsAuthorityActorEntity, NetDriver),
		AuthoritySubviewSetup::GetCallbacks(NetDriver.Connection->GetCoordinator()));
}

FSubView& CreateAutonomousSubView(USpatialNetDriver& NetDriver)
{
	return NetDriver.Connection->GetCoordinator().CreateSubView(
		SpatialConstants::ACTOR_AUTH_TAG_COMPONENT_ID, GetRoleFilterPredicate(&AutonomousSubviewSetup::IsAutonomousActorEntity, NetDriver),
		AutonomousSubviewSetup::GetCallbacks(NetDriver.Connection->GetCoordinator()));
}

FSubView& CreateSimulatedSubView(USpatialNetDriver& NetDriver)
{
	return NetDriver.Connection->GetCoordinator().CreateSubView(
		SpatialConstants::ACTOR_TAG_COMPONENT_ID, GetRoleFilterPredicate(&SimulatedSubviewSetup::IsSimulatedActorEntity, NetDriver),
		SimulatedSubviewSetup::GetCallbacks(NetDriver.Connection->GetCoordinator()));
}

} // namespace ActorSubviews
} // namespace SpatialGDK
