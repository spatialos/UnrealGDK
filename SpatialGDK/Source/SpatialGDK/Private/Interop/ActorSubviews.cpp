#include "Interop/ActorSubviews.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/ActorSystem.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/InitialOnlyFilter.h"
#include "Interop/OwnershipCompletenessHandler.h"
#include "Schema/ActorSetMember.h"
#include "Schema/Restricted.h"
#include "Schema/Tombstone.h"
#include "Schema/UnrealMetadata.h"
#include "SpatialView/EntityComponentTypes.h"
#include "SpatialView/EntityView.h"
#include "SpatialView/SubView.h"
#include "SpatialView/ViewCoordinator.h"

#include "SpatialView/OpList/EntityComponentOpList.h"
#include "Tests/SpatialView/SpatialViewUtils.h"
#include "Tests/TestDefinitions.h"

namespace SpatialGDK
{
namespace ActorSubviews
{
namespace MainActorSubviewSetup
{
static bool IsActorEntity(const Worker_EntityId EntityId, const EntityViewElement& Entity, USpatialNetDriver* NetDriver);

static TArray<FDispatcherRefreshCallback> GetCallbacks(ViewCoordinator& Coordinator);
} // namespace MainActorSubviewSetup

namespace AuthoritySubviewSetup
{
static bool IsAuthorityActorEntity(const Worker_EntityId EntityId, const EntityViewElement& Element);

static TArray<FDispatcherRefreshCallback> GetCallbacks(ViewCoordinator& Coordinator);
} // namespace AuthoritySubviewSetup

namespace OwnershipSubviewSetup
{
static bool IsPlayerOwnedActorEntity(const Worker_EntityId EntityId, const EntityViewElement& Element);

static TArray<FDispatcherRefreshCallback> GetCallbacks(ViewCoordinator& Coordinator);
} // namespace OwnershipSubviewSetup

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

bool MainActorSubviewSetup::IsActorEntity(const Worker_EntityId EntityId, const EntityViewElement& Entity, USpatialNetDriver* InNetDriver)
{
	if (Entity.Components.ContainsByPredicate(ComponentIdEquality{ Tombstone::ComponentId }))
	{
		return false;
	}

	if (InNetDriver == nullptr)
	{
		return true;
	}

	USpatialNetDriver& NetDriver = *InNetDriver;

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
	return { Coordinator.CreateComponentExistenceRefreshCallback(Tombstone::ComponentId),
			 Coordinator.CreateComponentExistenceRefreshCallback(Partition::ComponentId),
			 Coordinator.CreateComponentExistenceRefreshCallback(SpatialConstants::PLAYER_CONTROLLER_COMPONENT_ID) };
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

bool OwnershipSubviewSetup::IsPlayerOwnedActorEntity(const Worker_EntityId EntityId, const EntityViewElement& Element)
{
	return !AuthoritySubviewSetup::IsAuthorityActorEntity(EntityId, Element)
		   && Element.Authority.Contains(SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID);
}

TArray<FDispatcherRefreshCallback> OwnershipSubviewSetup::GetCallbacks(ViewCoordinator& Coordinator)
{
	return CombineCallbacks(AuthoritySubviewSetup::GetCallbacks(Coordinator),
							{
								Coordinator.CreateAuthorityChangeRefreshCallback(SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID),
							});
}

bool SimulatedSubviewSetup::IsSimulatedActorEntity(const Worker_EntityId EntityId, const EntityViewElement& Entity)
{
	return !AuthoritySubviewSetup::IsAuthorityActorEntity(EntityId, Entity)
		   && !OwnershipSubviewSetup::IsPlayerOwnedActorEntity(EntityId, Entity);
}

TArray<FDispatcherRefreshCallback> SimulatedSubviewSetup::GetCallbacks(ViewCoordinator& Coordinator)
{
	return OwnershipSubviewSetup::GetCallbacks(Coordinator);
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
			if (!MainActorSubviewSetup::IsActorEntity(EntityId, Entity, &NetDriver))
			{
				return false;
			}

			return CustomPredicate(EntityId, Entity);
		};
	}
	else
	{
		MaybeCustomPredicate = [&NetDriver](const Worker_EntityId EntityId, const EntityViewElement& Entity) {
			return MainActorSubviewSetup::IsActorEntity(EntityId, Entity, &NetDriver);
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
PRAGMA_DISABLE_OPTIMIZATION
template <typename TCallable, typename... TValues>
FFilterPredicate GetRoleFilterPredicate(TCallable RolePredicate, USpatialNetDriver& NetDriver, TValues... Values)
{
	return [RolePredicate, &NetDriver, ExtraValues = TTuple<TValues...>(Values...)](const Worker_EntityId EntityId,
																					const EntityViewElement& Entity) -> bool {
		if (!MainActorSubviewSetup::IsActorEntity(EntityId, Entity, &NetDriver))
		{
			return false;
		}

		return ExtraValues.ApplyAfter(RolePredicate, EntityId, Entity);
	};
}

template <typename TCallable, typename... TValues>
FFilterPredicate GetMockRoleFilterPredicate(TCallable RolePredicate, TValues... Values)
{
	return [RolePredicate, ExtraValues = TTuple<TValues...>(Values...)](const Worker_EntityId EntityId,
																		const EntityViewElement& Entity) -> bool {
		return ExtraValues.ApplyAfter(RolePredicate, EntityId, Entity);
	};
}

FSubView& CreateAuthoritySubView(USpatialNetDriver& NetDriver)
{
	return NetDriver.Connection->GetCoordinator().CreateSubView(
		SpatialConstants::ACTOR_AUTH_TAG_COMPONENT_ID, GetRoleFilterPredicate(&AuthoritySubviewSetup::IsAuthorityActorEntity, NetDriver),
		AuthoritySubviewSetup::GetCallbacks(NetDriver.Connection->GetCoordinator()));
}

FSubView& CreateOwnershipSubView(USpatialNetDriver& NetDriver)
{
	return NetDriver.Connection->GetCoordinator().CreateSubView(
		SpatialConstants::ACTOR_AUTH_TAG_COMPONENT_ID, GetRoleFilterPredicate(&OwnershipSubviewSetup::IsPlayerOwnedActorEntity, NetDriver),
		OwnershipSubviewSetup::GetCallbacks(NetDriver.Connection->GetCoordinator()));
}

FSubView& CreateSimulatedSubView(USpatialNetDriver& NetDriver)
{
	return NetDriver.Connection->GetCoordinator().CreateSubView(
		SpatialConstants::ACTOR_TAG_COMPONENT_ID, GetRoleFilterPredicate(&SimulatedSubviewSetup::IsSimulatedActorEntity, NetDriver),
		SimulatedSubviewSetup::GetCallbacks(NetDriver.Connection->GetCoordinator()));
}

bool IsAutonomousAndOwnershipComplete(Worker_EntityId EntityId, const EntityViewElement& Entity,
									  const FOwnershipCompletenessHandler* CompletenessHandler)
{
	return OwnershipSubviewSetup::IsPlayerOwnedActorEntity(EntityId, Entity) && CompletenessHandler->IsOwnershipComplete(EntityId, Entity);
}

FSubView& CreateAutonomousOwnershipCompletenessSubView(ViewCoordinator& Coordinator, const FFilterPredicate& FilterPredicate,
													   FOwnershipCompletenessHandler& CompletenessHandler)
{
	FSubView& SubView = Coordinator.CreateSubView(
		SpatialConstants::ACTOR_AUTH_TAG_COMPONENT_ID, FilterPredicate,
		CombineCallbacks(FOwnershipCompletenessHandler::GetCallbacks(Coordinator), OwnershipSubviewSetup::GetCallbacks(Coordinator)));
	CompletenessHandler.AddSubView(SubView);
	return SubView;
}

FSubView& CreateAutonomousOwnershipCompletenessSubView(USpatialNetDriver& NetDriver)
{
	return CreateAutonomousOwnershipCompletenessSubView(
		NetDriver.Connection->GetCoordinator(),
		GetRoleFilterPredicate(&IsAutonomousAndOwnershipComplete, NetDriver, &NetDriver.OwnershipCompletenessHandler.GetValue()),
		NetDriver.OwnershipCompletenessHandler.GetValue());
}

FSubView& CreateAutonomousOwnershipCompletenessSubView(ViewCoordinator& Coordinator, FOwnershipCompletenessHandler& CompletenessHandler)
{
	return CreateAutonomousOwnershipCompletenessSubView(
		Coordinator, GetMockRoleFilterPredicate(&IsAutonomousAndOwnershipComplete, &CompletenessHandler), CompletenessHandler);
}

FSubView& CreateSimulatedSubView(ViewCoordinator& Coordinator)
{
	return Coordinator.CreateSubView(SpatialConstants::ACTOR_TAG_COMPONENT_ID,
									 GetMockRoleFilterPredicate(&SimulatedSubviewSetup::IsSimulatedActorEntity),
									 SimulatedSubviewSetup::GetCallbacks(Coordinator));
}

bool IsSimulatedAndOwnershipComplete(Worker_EntityId EntityId, const EntityViewElement& Entity,
									 const FOwnershipCompletenessHandler* CompletenessHandler)
{
	return SimulatedSubviewSetup::IsSimulatedActorEntity(EntityId, Entity) && CompletenessHandler->IsOwnershipComplete(EntityId, Entity);
}
PRAGMA_ENABLE_OPTIMIZATION

FSubView& CreateSimulatedOwnershipCompletenessSubView(ViewCoordinator& Coordinator, const FFilterPredicate& FilterPredicate,
													  FOwnershipCompletenessHandler& CompletenessHandler)
{
	FSubView& SubView = Coordinator.CreateSubView(
		SpatialConstants::ACTOR_TAG_COMPONENT_ID, FilterPredicate,
		CombineCallbacks(FOwnershipCompletenessHandler::GetCallbacks(Coordinator), SimulatedSubviewSetup::GetCallbacks(Coordinator)));
	CompletenessHandler.AddSubView(SubView);
	return SubView;
}

FSubView& CreateSimulatedOwnershipCompletenessSubView(ViewCoordinator& Coordinator, FOwnershipCompletenessHandler& CompletenessHandler)
{
	return CreateSimulatedOwnershipCompletenessSubView(
		Coordinator, GetMockRoleFilterPredicate(&IsSimulatedAndOwnershipComplete, &CompletenessHandler), CompletenessHandler);
}

FSubView& CreateSimulatedOwnershipCompletenessSubView(USpatialNetDriver& NetDriver)
{
	return CreateSimulatedOwnershipCompletenessSubView(
		NetDriver.Connection->GetCoordinator(),
		GetRoleFilterPredicate(&IsSimulatedAndOwnershipComplete, NetDriver, &NetDriver.OwnershipCompletenessHandler.GetValue()),
		NetDriver.OwnershipCompletenessHandler.GetValue());
}
} // namespace ActorSubviews

PRAGMA_DISABLE_OPTIMIZATION
GDK_TEST(Core, OwnershipCompleteness, SomeTest)
{
	using namespace ActorSubviews;

	// Construct Op list
	TUniquePtr<MockConnectionHandler> ConnHandlerUnique = MakeUnique<MockConnectionHandler>();
	MockConnectionHandler* ConnHandler = ConnHandlerUnique.Get();
	{
		TArray<TArray<OpList>> ListsOfOpLists;
		TArray<OpList> OpLists;
		EntityComponentOpListBuilder Builder;
		Builder.AddEntity(1)
			.AddComponent(1, ComponentData(SpatialConstants::ACTOR_TAG_COMPONENT_ID))
			.AddComponent(1, ActorOwnership(2).CreateComponentData());
		OpLists.Add(MoveTemp(Builder).CreateOpList());
		ListsOfOpLists.Add(MoveTemp(OpLists));
		ConnHandler->SetListsOfOpLists(MoveTemp(ListsOfOpLists));
	}

	// No queue: Handler should be able to execute it
	ViewCoordinator Coordinator(MoveTemp(ConnHandlerUnique), nullptr, FComponentSetData());
	FSubView& SimSubView = CreateSimulatedSubView(Coordinator);
	FOwnershipCompletenessHandler H(/*bIsServer =*/false);
	H.AddPlayerEntity(2);
	FSubView& SimComplSubView = CreateSimulatedOwnershipCompletenessSubView(Coordinator, H);
	H.AddSubView(SimComplSubView);

	Coordinator.Advance(1);
	TestTrue(TEXT("Entity was complete without considering ownership"), SimSubView.IsEntityComplete(1));
	TestFalse(TEXT("Entity was incomplete when considering ownership"), SimComplSubView.IsEntityComplete(1));

	{
		TArray<OpList> OpLists;
		EntityComponentOpListBuilder Builder;
		Builder.AddComponent(1, ComponentData(SpatialConstants::ACTOR_OWNER_ONLY_DATA_TAG_COMPONENT_ID));
		OpLists.Add(MoveTemp(Builder).CreateOpList());
		TArray<TArray<OpList>> ListsOfOpLists;
		ListsOfOpLists.Add(MoveTemp(OpLists));
		ConnHandler->SetListsOfOpLists(MoveTemp(ListsOfOpLists));
	}

	Coordinator.Advance(2);
	TestTrue(TEXT("Entity still complete without considering ownership"), SimSubView.IsEntityComplete(1));
	TestTrue(TEXT("Entity turned complete when considering ownership"), SimComplSubView.IsEntityComplete(1));

	return true;
}

PRAGMA_ENABLE_OPTIMIZATION

} // namespace SpatialGDK
