#include "Interop/ActorSubviews.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/ActorSystem.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/InitialOnlyFilter.h"
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
FFilterPredicate GetRoleFilterPredicate(TCallable RolePredicate, USpatialNetDriver* NetDriver, TValues... Values)
{
	return [RolePredicate, NetDriver, ExtraValues = TTuple<TValues...>(Values...)](const Worker_EntityId EntityId,
																				   const EntityViewElement& Entity) -> bool {
		if (!MainActorSubviewSetup::IsActorEntity(EntityId, Entity, NetDriver))
		{
			return false;
		}

		return ExtraValues.ApplyAfter(RolePredicate, EntityId, Entity);
	};
}
FSubView& CreateAuthoritySubView(USpatialNetDriver& NetDriver)
{
	return NetDriver.Connection->GetCoordinator().CreateSubView(
		SpatialConstants::ACTOR_AUTH_TAG_COMPONENT_ID, GetRoleFilterPredicate(&AuthoritySubviewSetup::IsAuthorityActorEntity, &NetDriver),
		AuthoritySubviewSetup::GetCallbacks(NetDriver.Connection->GetCoordinator()));
}

FSubView& CreateOwnershipSubView(USpatialNetDriver& NetDriver)
{
	return NetDriver.Connection->GetCoordinator().CreateSubView(
		SpatialConstants::ACTOR_AUTH_TAG_COMPONENT_ID, GetRoleFilterPredicate(&AutonomousSubviewSetup::IsAutonomousActorEntity, &NetDriver),
		AutonomousSubviewSetup::GetCallbacks(NetDriver.Connection->GetCoordinator()));
}

FSubView& CreateSimulatedSubView(USpatialNetDriver& NetDriver)
{
	return NetDriver.Connection->GetCoordinator().CreateSubView(
		SpatialConstants::ACTOR_TAG_COMPONENT_ID, GetRoleFilterPredicate(&SimulatedSubviewSetup::IsSimulatedActorEntity, &NetDriver),
		SimulatedSubviewSetup::GetCallbacks(NetDriver.Connection->GetCoordinator()));
}

bool IsAutonomousAndOwnershipComplete(Worker_EntityId EntityId, const EntityViewElement& Entity,
									  FOwnershipCompletenessHandler* CompletenessHandler)
{
	return AutonomousSubviewSetup::IsAutonomousActorEntity(EntityId, Entity) && CompletenessHandler->IsOwnershipComplete(EntityId, Entity);
}

FSubView& CreateAutonomousOwnershipCompletenessSubView(USpatialNetDriver& NetDriver)
{
	return NetDriver.Connection->GetCoordinator().CreateSubView(
		SpatialConstants::ACTOR_AUTH_TAG_COMPONENT_ID,
		GetRoleFilterPredicate(&IsAutonomousAndOwnershipComplete, &NetDriver,
							   /*const_cast<const FOwnershipCompletenessHandler*>*/ (&NetDriver.OwnershipCompletenessHandler)),
		FOwnershipCompletenessHandler::GetCallbacks(NetDriver.Connection->GetCoordinator()));
}

FSubView& CreateSimulatedSubView(ViewCoordinator& Coordinator)
{
	return Coordinator.CreateSubView(SpatialConstants::ACTOR_TAG_COMPONENT_ID,
									 GetRoleFilterPredicate(&SimulatedSubviewSetup::IsSimulatedActorEntity, nullptr),
									 SimulatedSubviewSetup::GetCallbacks(Coordinator));
}

bool IsSimulatedAndOwnershipComplete(Worker_EntityId EntityId, const EntityViewElement& Entity,
									 FOwnershipCompletenessHandler* CompletenessHandler)
{
	return SimulatedSubviewSetup::IsSimulatedActorEntity(EntityId, Entity) && CompletenessHandler->IsOwnershipComplete(EntityId, Entity);
}
PRAGMA_ENABLE_OPTIMIZATION

FSubView& CreateSimulatedOwnershipCompletenessSubView(USpatialNetDriver& NetDriver)
{
	return NetDriver.Connection->GetCoordinator().CreateSubView(
		SpatialConstants::ACTOR_TAG_COMPONENT_ID,
		GetRoleFilterPredicate(&IsSimulatedAndOwnershipComplete, &NetDriver,
							   /*const_cast<const FOwnershipCompletenessHandler*>*/ (&NetDriver.OwnershipCompletenessHandler)),
		FOwnershipCompletenessHandler::GetCallbacks(NetDriver.Connection->GetCoordinator()));
}

FSubView& CreateSimulatedOwnershipCompletenessSubView(ViewCoordinator& Coordinator, FOwnershipCompletenessHandler& CompletenessHandler)
{
	return Coordinator.CreateSubView(SpatialConstants::ACTOR_TAG_COMPONENT_ID,
									 GetRoleFilterPredicate(&IsSimulatedAndOwnershipComplete, nullptr, &CompletenessHandler),
									 FOwnershipCompletenessHandler::GetCallbacks(Coordinator));
}
} // namespace ActorSubviews

class MockConnectionHandler : public AbstractConnectionHandler
{
public:
	void SetListsOfOpLists(TArray<TArray<OpList>> List) { ListsOfOpLists = MoveTemp(List); }

	virtual void Advance() override
	{
		QueuedOpLists = MoveTemp(ListsOfOpLists[0]);
		ListsOfOpLists.RemoveAt(0);
	}

	virtual uint32 GetOpListCount() override { return QueuedOpLists.Num(); }

	virtual OpList GetNextOpList() override
	{
		OpList Temp = MoveTemp(QueuedOpLists[0]);
		QueuedOpLists.RemoveAt(0);
		return Temp;
	}

	virtual void SendMessages(TUniquePtr<MessagesToSend> Messages) override {}

	virtual const FString& GetWorkerId() const override { return WorkerId; }

	virtual Worker_EntityId GetWorkerSystemEntityId() const override { return WorkerSystemEntityId; }

private:
	TArray<TArray<OpList>> ListsOfOpLists;
	TArray<OpList> QueuedOpLists;
	Worker_EntityId WorkerSystemEntityId = 1;
	FString WorkerId = TEXT("test_worker");
};

PRAGMA_DISABLE_OPTIMIZATION
GDK_TEST(Core, OwnershipCompleteness, SomeTest)
{
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
	FSubView& SimSubView = ActorSubviews::CreateSimulatedSubView(Coordinator);
	FOwnershipCompletenessHandler H;
	H.PlayerOwnedEntities.Emplace(2);
	FSubView& SimComplSubView = ActorSubviews::CreateSimulatedOwnershipCompletenessSubView(Coordinator, H);

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

bool FOwnershipCompletenessHandler::IsOwnershipComplete(Worker_EntityId EntityId, const EntityViewElement& Entity)
{
	const ActorOwnership Value = *Entity.Components.FindByPredicate(ComponentIdEquality{ ActorOwnership::ComponentId });

	EntitiesPossiblyOwned.Emplace(EntityId);

	const bool bIsPlayerOwned = Value.OwnerActorEntityId != SpatialConstants::INVALID_ENTITY_ID
								&& (Value.OwnerActorEntityId == EntityId || PlayerOwnedEntities.Contains(EntityId)
									|| PlayerOwnedEntities.Contains(Value.OwnerActorEntityId));

	const bool bHasOwnerOnlyComponents =
		Entity.Components.ContainsByPredicate(ComponentIdEquality{ SpatialConstants::ACTOR_OWNER_ONLY_DATA_TAG_COMPONENT_ID });

	return bIsPlayerOwned == bHasOwnerOnlyComponents;
}

TArray<FDispatcherRefreshCallback> FOwnershipCompletenessHandler::GetCallbacks(ViewCoordinator& Coordinator)
{
	using namespace ActorSubviews;
	return CombineCallbacks(
		SimulatedSubviewSetup::GetCallbacks(Coordinator),
		{ Coordinator.CreateComponentChangedRefreshCallback(ActorOwnership::ComponentId),
		  Coordinator.CreateComponentExistenceRefreshCallback(SpatialConstants::ACTOR_OWNER_ONLY_DATA_TAG_COMPONENT_ID) });
}
} // namespace SpatialGDK
