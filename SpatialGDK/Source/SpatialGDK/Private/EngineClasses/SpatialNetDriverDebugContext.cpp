// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialNetDriverDebugContext.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialSender.h"
#include "LoadBalancing/DebugLBStrategy.h"
#include "Utils/SpatialActorUtils.h"

namespace
{
// Utility function, extracted from TSet<T>::Intersect
template <typename T>
bool IsSetIntersectionEmpty(const TSet<T>& Set1, const TSet<T>& Set2)
{
	const bool b2Smaller = (Set1.Num() > Set2.Num());
	const TSet<T>& A = (b2Smaller ? Set2 : Set1);
	const TSet<T>& B = (b2Smaller ? Set1 : Set2);

	for (auto SetIt = A.CreateConstIterator(); SetIt; ++SetIt)
	{
		if (B.Contains(*SetIt))
		{
			return false;
		}
	}
	return true;
}
} // namespace

void USpatialNetDriverDebugContext::EnableDebugSpatialGDK(const SpatialGDK::FSubView& InSubView, USpatialNetDriver* NetDriver)
{
	if (NetDriver->DebugCtx == nullptr)
	{
		if (!ensureMsgf(NetDriver->LoadBalanceStrategy, TEXT("Enabling SpatialGDKDebug too soon")))
		{
			return;
		}
		NetDriver->DebugCtx = NewObject<USpatialNetDriverDebugContext>();
		NetDriver->DebugCtx->Init(InSubView, NetDriver);
	}
}

void USpatialNetDriverDebugContext::DisableDebugSpatialGDK(USpatialNetDriver* NetDriver)
{
	if (NetDriver->DebugCtx != nullptr)
	{
		NetDriver->DebugCtx->Cleanup();
	}
}

void USpatialNetDriverDebugContext::Init(const SpatialGDK::FSubView& InSubView, USpatialNetDriver* InNetDriver)
{
	SubView = &InSubView;
	NetDriver = InNetDriver;

	DebugStrategy = NewObject<UDebugLBStrategy>();
	DebugStrategy->InitDebugStrategy(this, NetDriver->LoadBalanceStrategy);
	NetDriver->LoadBalanceStrategy = DebugStrategy;

	NetDriver->Sender->UpdatePartitionEntityInterestAndPosition();
}

void USpatialNetDriverDebugContext::Cleanup()
{
	Reset();
	NetDriver->LoadBalanceStrategy = Cast<UDebugLBStrategy>(DebugStrategy)->GetWrappedStrategy();
	NetDriver->DebugCtx = nullptr;
	NetDriver->Sender->UpdatePartitionEntityInterestAndPosition();
}

void USpatialNetDriverDebugContext::AdvanceView()
{
	const SpatialGDK::FSubViewDelta& ViewDelta = SubView->GetViewDelta();
	for (const SpatialGDK::EntityDelta& Delta : ViewDelta.EntityDeltas)
	{
		switch (Delta.Type)
		{
		case SpatialGDK::EntityDelta::ADD:
			AddComponent(Delta.EntityId);
			break;
		case SpatialGDK::EntityDelta::REMOVE:
			RemoveComponent(Delta.EntityId);
			break;
		case SpatialGDK::EntityDelta::TEMPORARILY_REMOVED:
			RemoveComponent(Delta.EntityId);
			AddComponent(Delta.EntityId);
			break;
		case SpatialGDK::EntityDelta::UPDATE:
			for (const SpatialGDK::AuthorityChange& Change : Delta.AuthorityLostTemporarily)
			{
				if (Change.ComponentSetId == SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID)
				{
					AuthorityLost(Delta.EntityId);
				}
			}
			for (const SpatialGDK::AuthorityChange& Change : Delta.AuthorityLost)
			{
				if (Change.ComponentSetId == SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID)
				{
					AuthorityLost(Delta.EntityId);
				}
			}
			for (const SpatialGDK::ComponentChange& Change : Delta.ComponentUpdates)
			{
				if (Change.ComponentId == SpatialConstants::GDK_DEBUG_COMPONENT_ID)
				{
					OnComponentChange(Delta.EntityId, Change);
				}
			}
			for (const SpatialGDK::ComponentChange& Change : Delta.ComponentsRefreshed)
			{
				if (Change.ComponentId == SpatialConstants::GDK_DEBUG_COMPONENT_ID)
				{
					OnComponentChange(Delta.EntityId, Change);
				}
			}
			break;
		}
	}
}

void USpatialNetDriverDebugContext::OnComponentChange(FSpatialEntityId EntityId, const SpatialGDK::ComponentChange& Change)
{
	if (Change.Type == SpatialGDK::ComponentChange::UPDATE)
	{
		ApplyComponentUpdate(EntityId, Change.Update);
	}
	else if (Change.Type == SpatialGDK::ComponentChange::COMPLETE_UPDATE)
	{
		RemoveComponent(EntityId);
		AddComponent(EntityId);
	}
}

void USpatialNetDriverDebugContext::Reset()
{
	for (const auto& Entry : NetDriver->Connection->GetView())
	{
		const SpatialGDK::EntityViewElement& ViewElement = Entry.Value;
		if (ViewElement.Authority.Contains(SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID)
			&& ViewElement.Components.ContainsByPredicate([](const SpatialGDK::ComponentData& Data) {
				   return Data.GetComponentId() == SpatialConstants::GDK_DEBUG_COMPONENT_ID;
			   }))
		{
			NetDriver->Connection->SendRemoveComponent(Entry.Key, SpatialConstants::GDK_DEBUG_COMPONENT_ID);
		}
	}

	SemanticInterest.Empty();
	SemanticDelegations.Empty();
	CachedInterestSet.Empty();
	ActorDebugInfo.Empty();

	NetDriver->Sender->UpdatePartitionEntityInterestAndPosition();
}

USpatialNetDriverDebugContext::DebugComponentAuthData& USpatialNetDriverDebugContext::GetAuthDebugComponent(AActor* Actor)
{
	ensureAlwaysMsgf(Actor && Actor->HasAuthority(), TEXT("Called GetAuthDebugComponent without authority. Actor: %s"),
					 *GetNameSafe(Actor));

	SpatialGDK::DebugComponent* DbgComp = nullptr;

	const FSpatialEntityId Entity = NetDriver->PackageMap->GetEntityIdFromObject(Actor);
	if (Entity != SpatialConstants::INVALID_ENTITY_ID)
	{
		DbgComp = DebugComponents.Find(Entity);
	}

	DebugComponentAuthData& Comp = ActorDebugInfo.FindOrAdd(Actor);
	if (DbgComp && Comp.Entity == SpatialConstants::INVALID_ENTITY_ID)
	{
		Comp.Component = *DbgComp;
		Comp.bAdded = true;
	}
	Comp.Entity = Entity;

	return Comp;
}

void USpatialNetDriverDebugContext::AddActorTag(AActor* Actor, FName Tag)
{
	if (Actor->HasAuthority())
	{
		DebugComponentAuthData& Comp = GetAuthDebugComponent(Actor);
		Comp.Component.ActorTags.Add(Tag);
		if (SemanticInterest.Contains(Tag) && Comp.Entity != SpatialConstants::INVALID_ENTITY_ID)
		{
			AddEntityToWatch(Comp.Entity);
		}
		Comp.bDirty = true;
	}
}

void USpatialNetDriverDebugContext::RemoveActorTag(AActor* Actor, FName Tag)
{
	if (Actor->HasAuthority())
	{
		DebugComponentAuthData& Comp = GetAuthDebugComponent(Actor);
		Comp.Component.ActorTags.Remove(Tag);
		if (IsSetIntersectionEmpty(SemanticInterest, Comp.Component.ActorTags) && Comp.Entity != SpatialConstants::INVALID_ENTITY_ID)
		{
			RemoveEntityToWatch(Comp.Entity);
		}
		Comp.bDirty = true;
	}
}

void USpatialNetDriverDebugContext::AddComponent(FSpatialEntityId EntityId)
{
	const SpatialGDK::EntityViewElement& Element = SubView->GetView().FindChecked(EntityId);
	const SpatialGDK::ComponentData* Data = Element.Components.FindByPredicate([](const SpatialGDK::ComponentData& Component) {
		return Component.GetComponentId() == SpatialConstants::GDK_DEBUG_COMPONENT_ID;
	});
	if (!ensureAlwaysMsgf(Data != nullptr, TEXT("Failed to access Debug component data for entity %s"), *EntityId.ToString()))
	{
		return;
	}
	SpatialGDK::DebugComponent& DbgComp = DebugComponents.Add(EntityId, SpatialGDK::DebugComponent(Data->GetUnderlying()));

	if (!IsSetIntersectionEmpty(SemanticInterest, DbgComp.ActorTags))
	{
		AddEntityToWatch(EntityId);
	}
}

void USpatialNetDriverDebugContext::RemoveComponent(FSpatialEntityId EntityId)
{
	RemoveEntityToWatch(EntityId);
	DebugComponents.Remove(EntityId);
}

void USpatialNetDriverDebugContext::ApplyComponentUpdate(FSpatialEntityId Entity, Schema_ComponentUpdate* Update)
{
	SpatialGDK::DebugComponent* DbgComp = DebugComponents.Find(Entity);

	if (!ensureAlwaysMsgf(DbgComp != nullptr, TEXT("Failed to ApplyComponentUpdate for debug context because component data wasn't found")))
	{
		return;
	}

	DbgComp->ApplyComponentUpdate(Update);

	if (IsSetIntersectionEmpty(SemanticInterest, DbgComp->ActorTags))
	{
		RemoveEntityToWatch(Entity);
	}
	else
	{
		AddEntityToWatch(Entity);
	}
}

void USpatialNetDriverDebugContext::AuthorityLost(FSpatialEntityId EntityId)
{
	for (auto Iterator = ActorDebugInfo.CreateIterator(); Iterator; ++Iterator)
	{
		if (Iterator->Value.Entity == EntityId)
		{
			Iterator.RemoveCurrent();
			break;
		}
	}
}

void USpatialNetDriverDebugContext::AddEntityToWatch(FSpatialEntityId Entity)
{
	bool bAlreadyWatchingEntity = false;
	CachedInterestSet.Add(Entity, &bAlreadyWatchingEntity);
	bNeedToUpdateInterest |= !bAlreadyWatchingEntity;
}

void USpatialNetDriverDebugContext::RemoveEntityToWatch(FSpatialEntityId Entity)
{
	if (CachedInterestSet.Remove(Entity) > 0)
	{
		bNeedToUpdateInterest = true;
	}
}

void USpatialNetDriverDebugContext::AddInterestOnTag(FName Tag)
{
	bool bAlreadyInSet = false;
	SemanticInterest.Add(Tag, &bAlreadyInSet);

	if (!bAlreadyInSet)
	{
		for (auto& EntityView : DebugComponents)
		{
			if (EntityView.Value.ActorTags.Contains(Tag))
			{
				AddEntityToWatch(EntityView.Key);
			}
		}

		for (const auto& Item : ActorDebugInfo)
		{
			if (Item.Value.Component.ActorTags.Contains(Tag))
			{
				FSpatialEntityId Entity = NetDriver->PackageMap->GetEntityIdFromObject(Item.Key);
				if (Entity != SpatialConstants::INVALID_ENTITY_ID)
				{
					AddEntityToWatch(Entity);
				}
			}
		}
	}
}

void USpatialNetDriverDebugContext::RemoveInterestOnTag(FName Tag)
{
	if (SemanticInterest.Remove(Tag) > 0)
	{
		CachedInterestSet.Empty();
		bNeedToUpdateInterest = true;

		for (auto& EntityView : DebugComponents)
		{
			if (!IsSetIntersectionEmpty(EntityView.Value.ActorTags, SemanticInterest))
			{
				AddEntityToWatch(EntityView.Key);
			}
		}

		for (const auto& Item : ActorDebugInfo)
		{
			if (!IsSetIntersectionEmpty(Item.Value.Component.ActorTags, SemanticInterest))
			{
				FSpatialEntityId Entity = NetDriver->PackageMap->GetEntityIdFromObject(Item.Key);
				if (Entity != SpatialConstants::INVALID_ENTITY_ID)
				{
					AddEntityToWatch(Entity);
				}
			}
		}
	}
}

void USpatialNetDriverDebugContext::KeepActorOnLocalWorker(AActor* Actor)
{
	if (Actor->HasAuthority())
	{
		DebugComponentAuthData& Comp = GetAuthDebugComponent(Actor);
		Comp.Component.DelegatedWorkerId = DebugStrategy->GetLocalVirtualWorkerId();
		Comp.bDirty = true;
	}
}

void USpatialNetDriverDebugContext::DelegateTagToWorker(FName Tag, uint32 WorkerId)
{
	SemanticDelegations.Add(Tag, WorkerId);
}

void USpatialNetDriverDebugContext::RemoveTagDelegation(FName Tag)
{
	SemanticDelegations.Remove(Tag);
}

TOptional<VirtualWorkerId> USpatialNetDriverDebugContext::GetActorHierarchyExplicitDelegation(const AActor* Actor)
{
	const AActor* NetOwner = SpatialGDK::GetReplicatedHierarchyRoot(Actor);
	return GetActorHierarchyExplicitDelegation_Traverse(NetOwner);
}

TOptional<VirtualWorkerId> USpatialNetDriverDebugContext::GetActorHierarchyExplicitDelegation_Traverse(const AActor* Actor)
{
	TOptional<VirtualWorkerId> CandidateDelegation = GetActorExplicitDelegation(Actor);
	for (const AActor* Child : Actor->Children)
	{
		TOptional<VirtualWorkerId> ChildDelegation = GetActorHierarchyExplicitDelegation_Traverse(Child);
		if (ChildDelegation)
		{
			ensureMsgf(
				!CandidateDelegation.IsSet() || !ChildDelegation.IsSet() || CandidateDelegation.GetValue() == ChildDelegation.GetValue(),
				TEXT("Inconsistent delegation. Actor %s is delegated to %i but a child is delegated to %i"), *Actor->GetName(),
				CandidateDelegation.GetValue(), ChildDelegation.GetValue());

			CandidateDelegation = ChildDelegation;
		}
	}

	return CandidateDelegation;
}

TOptional<VirtualWorkerId> USpatialNetDriverDebugContext::GetActorExplicitDelegation(const AActor* Actor)
{
	SpatialGDK::DebugComponent* DbgComp = nullptr;
	if (DebugComponentAuthData* DebugInfo = ActorDebugInfo.Find(Actor))
	{
		DbgComp = &DebugInfo->Component;
	}
	else
	{
		FSpatialEntityId Entity = NetDriver->PackageMap->GetEntityIdFromObject(Actor);
		if (Entity != SpatialConstants::INVALID_ENTITY_ID)
		{
			DbgComp = DebugComponents.Find(Entity);
		}
	}

	if (!DbgComp)
	{
		return {};
	}

	if (DbgComp->DelegatedWorkerId)
	{
		return *DbgComp->DelegatedWorkerId;
	}

	TOptional<VirtualWorkerId> CandidateDelegation;
	for (auto Tag : DbgComp->ActorTags)
	{
		if (VirtualWorkerId* Worker = SemanticDelegations.Find(Tag))
		{
			if (ensureMsgf(!CandidateDelegation.IsSet() || CandidateDelegation.GetValue() == *Worker,
						   TEXT("Inconsistent delegation. Actor %s delegated to both %i and %i"), *Actor->GetName(),
						   CandidateDelegation.GetValue(), *Worker))
			{
				CandidateDelegation = *Worker;
			}
		}
	}

	return CandidateDelegation;
}

void USpatialNetDriverDebugContext::TickServer()
{
	for (auto& Entry : ActorDebugInfo)
	{
		AActor* Actor = Entry.Key;
		DebugComponentAuthData& Data = Entry.Value;
		if (!Data.bAdded)
		{
			FSpatialEntityId Entity = NetDriver->PackageMap->GetEntityIdFromObject(Actor);
			if (Entity != SpatialConstants::INVALID_ENTITY_ID)
			{
				if (!IsSetIntersectionEmpty(Data.Component.ActorTags, SemanticInterest))
				{
					AddEntityToWatch(Entity);
				}

				// There is a requirement of readiness before we can use SendAddComponent
				if (IsActorReady(Actor))
				{
					FWorkerComponentData CompData = Data.Component.CreateDebugComponent();
					NetDriver->Connection->SendAddComponent(Entity, &CompData);
					NetDriver->Connection->GetCoordinator().RefreshEntityCompleteness(Entity);

					Data.Entity = Entity;
					Data.bAdded = true;
				}
			}
		}
		else if (Data.bDirty)
		{
			FWorkerComponentUpdate CompUpdate = Data.Component.CreateDebugComponentUpdate();
			NetDriver->Connection->SendComponentUpdate(Data.Entity, &CompUpdate);
			Data.bDirty = false;
		}
	}

	if (NeedEntityInterestUpdate())
	{
		NetDriver->Sender->UpdatePartitionEntityInterestAndPosition();
	}
}

bool USpatialNetDriverDebugContext::IsActorReady(AActor* Actor)
{
	FSpatialEntityId Entity = NetDriver->PackageMap->GetEntityIdFromObject(Actor);
	if (Entity != SpatialConstants::INVALID_ENTITY_ID)
	{
		return NetDriver->HasServerAuthority(Entity);
	}
	return false;
}

SpatialGDK::QueryConstraint USpatialNetDriverDebugContext::ComputeAdditionalEntityQueryConstraint() const
{
	SpatialGDK::QueryConstraint EntitiesConstraint;
	for (FSpatialEntityId Entity : CachedInterestSet)
	{
		SpatialGDK::QueryConstraint EntityQuery;
		EntityQuery.EntityIdConstraint = Entity;
		EntitiesConstraint.OrConstraint.Add(EntityQuery);
	}

	return EntitiesConstraint;
}
