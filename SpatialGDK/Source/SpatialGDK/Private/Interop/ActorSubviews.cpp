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
struct FMainActorSubviewSetup
{
	static bool IsActorEntity(const Worker_EntityId EntityId, const EntityViewElement& Entity, USpatialNetDriver& NetDriver)
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

	static TArray<FDispatcherRefreshCallback> GetCallbacks(ViewCoordinator& Coordinator)
	{
		return { Coordinator.CreateComponentExistenceRefreshCallback(Tombstone::ComponentId) };
	}
};

struct FAuthoritySubviewSetup
{
	static bool IsAuthorityActorEntity(const Worker_EntityId EntityId, const EntityViewElement& Element);

	static TArray<FDispatcherRefreshCallback> GetCallbacks(ViewCoordinator& Coordinator);
};

struct FAutonomousSubviewSetup
{
	static bool IsAutonomousActorEntity(const Worker_EntityId EntityId, const EntityViewElement& Element);

	static TArray<FDispatcherRefreshCallback> GetCallbacks(ViewCoordinator& Coordinator);
};

struct FSimulatedSubviewSetup
{
	static bool IsSimulatedActorEntity(const Worker_EntityId EntityId, const EntityViewElement& Entity);

	static TArray<FDispatcherRefreshCallback> GetCallbacks(ViewCoordinator& Coordinator);
};

template <typename TCallable>
FFilterPredicate GetRoleFilterPredicate(TCallable RolePredicate, USpatialNetDriver& NetDriver)
{
	return [RolePredicate, NetDriver = &NetDriver](const Worker_EntityId EntityId, const EntityViewElement& Entity) -> bool {
		if (!FMainActorSubviewSetup::IsActorEntity(EntityId, Entity, *NetDriver))
		{
			return false;
		}

		return Invoke(RolePredicate, EntityId, Entity);
	};
}

static TArray<FDispatcherRefreshCallback> CombineCallbacks(TArray<FDispatcherRefreshCallback> Lhs,
														   const TArray<FDispatcherRefreshCallback>& Rhs)
{
	Lhs.Append(Rhs);
	return Rhs;
}

bool FAuthoritySubviewSetup::IsAuthorityActorEntity(const Worker_EntityId EntityId, const EntityViewElement& Element)
{
	return Element.Authority.Contains(SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID);
}

TArray<FDispatcherRefreshCallback> FAuthoritySubviewSetup::GetCallbacks(ViewCoordinator& Coordinator)
{
	return CombineCallbacks(FMainActorSubviewSetup::GetCallbacks(Coordinator),
							{
								Coordinator.CreateComponentExistenceRefreshCallback(SpatialConstants::PLAYER_CONTROLLER_COMPONENT_ID),
								Coordinator.CreateComponentExistenceRefreshCallback(Partition::ComponentId),
								Coordinator.CreateAuthorityChangeRefreshCallback(SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID),
							});
}

bool FAutonomousSubviewSetup::IsAutonomousActorEntity(const Worker_EntityId EntityId, const EntityViewElement& Element)
{
	return !FAuthoritySubviewSetup::IsAuthorityActorEntity(EntityId, Element)
		   && Element.Authority.Contains(SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID);
}

TArray<FDispatcherRefreshCallback> FAutonomousSubviewSetup::GetCallbacks(ViewCoordinator& Coordinator)
{
	return CombineCallbacks(FAuthoritySubviewSetup::GetCallbacks(Coordinator),
							{
								Coordinator.CreateAuthorityChangeRefreshCallback(SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID),
							});
}

bool FSimulatedSubviewSetup::IsSimulatedActorEntity(const Worker_EntityId EntityId, const EntityViewElement& Entity)
{
	return !FAuthoritySubviewSetup::IsAuthorityActorEntity(EntityId, Entity)
		   && !FAutonomousSubviewSetup::IsAutonomousActorEntity(EntityId, Entity);
}

TArray<FDispatcherRefreshCallback> FSimulatedSubviewSetup::GetCallbacks(ViewCoordinator& Coordinator)
{
	return FAutonomousSubviewSetup::GetCallbacks(Coordinator);
}

FSubView& CreateActorSubView(USpatialNetDriver& NetDriver)
{
	return CreateCustomActorSubView({}, {}, {}, NetDriver);
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
		MaybeCustomPredicate = [CustomPredicate = MaybeCustomPredicate.GetValue(), NetDriver = &NetDriver](
								   const Worker_EntityId EntityId, const EntityViewElement& Entity) {
			if (!FMainActorSubviewSetup::IsActorEntity(EntityId, Entity, *NetDriver))
			{
				return false;
			}

			return CustomPredicate(EntityId, Entity);
		};
	}
	else
	{
		MaybeCustomPredicate = [NetDriver = &NetDriver](const Worker_EntityId EntityId, const EntityViewElement& Entity) {
			return FMainActorSubviewSetup::IsActorEntity(EntityId, Entity, *NetDriver);
		};
	}

	if (MaybeCustomRefresh)
	{
		MaybeCustomRefresh->Append(FMainActorSubviewSetup::GetCallbacks(NetDriver.Connection->GetCoordinator()));
	}
	else
	{
		MaybeCustomRefresh = FMainActorSubviewSetup::GetCallbacks(NetDriver.Connection->GetCoordinator());
	}

	return NetDriver.Connection->GetCoordinator().CreateSubView(*MaybeCustomComponentId, *MaybeCustomPredicate, *MaybeCustomRefresh);
}

FSubView& CreateActorAuthSubView(USpatialNetDriver& NetDriver)
{
	return NetDriver.Connection->GetCoordinator().CreateSubView(
		SpatialConstants::ACTOR_AUTH_TAG_COMPONENT_ID, FSubView::NoFilter,
		FAutonomousSubviewSetup::GetCallbacks(NetDriver.Connection->GetCoordinator()));
}

FSubView& CreateAuthoritySubView(USpatialNetDriver& NetDriver)
{
	return NetDriver.Connection->GetCoordinator().CreateSubView(
		SpatialConstants::ACTOR_AUTH_TAG_COMPONENT_ID, GetRoleFilterPredicate(&FAuthoritySubviewSetup::IsAuthorityActorEntity, NetDriver),
		FAuthoritySubviewSetup::GetCallbacks(NetDriver.Connection->GetCoordinator()));
}

FSubView& CreateAutonomousSubView(USpatialNetDriver& NetDriver)
{
	return NetDriver.Connection->GetCoordinator().CreateSubView(
		SpatialConstants::ACTOR_AUTH_TAG_COMPONENT_ID, GetRoleFilterPredicate(&FAutonomousSubviewSetup::IsAutonomousActorEntity, NetDriver),
		FAutonomousSubviewSetup::GetCallbacks(NetDriver.Connection->GetCoordinator()));
}

FSubView& CreateSimulatedSubView(USpatialNetDriver& NetDriver)
{
	return NetDriver.Connection->GetCoordinator().CreateSubView(
		SpatialConstants::ACTOR_TAG_COMPONENT_ID, GetRoleFilterPredicate(&FSimulatedSubviewSetup::IsSimulatedActorEntity, NetDriver),
		FSimulatedSubviewSetup::GetCallbacks(NetDriver.Connection->GetCoordinator()));
}

} // namespace ActorSubviews
} // namespace SpatialGDK
