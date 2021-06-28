// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialNetDriverGameplayDebuggerContext.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "GameplayDebuggerCategoryReplicator.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "LoadBalancing/GameplayDebuggerLBStrategy.h"
#include "Schema/AuthorityIntent.h"
#include "SpatialConstants.h"
#include "SpatialView/ComponentData.h"
#include "SpatialView/EntityView.h"
#include "SpatialView/SubView.h"
#include "SpatialView/ViewDelta.h"
#include "Utils/SpatialActorUtils.h"
#endif // WITH_GAMEPLAY_DEBUGGER

DEFINE_LOG_CATEGORY_STATIC(LogSpatialNetDriverGameplayDebuggerContext, Log, All);

#if WITH_GAMEPLAY_DEBUGGER
USpatialNetDriverGameplayDebuggerContext::~USpatialNetDriverGameplayDebuggerContext()
{
	Reset();
}

void USpatialNetDriverGameplayDebuggerContext::Enable(const SpatialGDK::FSubView& InSubView, USpatialNetDriver& NetDriver)
{
	if (NetDriver.GameplayDebuggerCtx != nullptr)
	{
		UE_LOG(LogSpatialNetDriverGameplayDebuggerContext, Error, TEXT("Enabling GDKGameplayDebugger more than once"));
		return;
	}

	if (NetDriver.LoadBalanceStrategy == nullptr)
	{
		UE_LOG(LogSpatialNetDriverGameplayDebuggerContext, Error, TEXT("Enabling GDKGameplayDebugger too soon"));
		return;
	}

	NetDriver.GameplayDebuggerCtx = NewObject<USpatialNetDriverGameplayDebuggerContext>();
	NetDriver.GameplayDebuggerCtx->Init(InSubView, NetDriver);
}

void USpatialNetDriverGameplayDebuggerContext::Disable(USpatialNetDriver& NetDriver)
{
	if (NetDriver.GameplayDebuggerCtx == nullptr)
	{
		UE_LOG(LogSpatialNetDriverGameplayDebuggerContext, Error, TEXT("Disabling GDKGameplayDebugger before enabling it"));
		return;
	}

	NetDriver.GameplayDebuggerCtx = nullptr;
}

void USpatialNetDriverGameplayDebuggerContext::Init(const SpatialGDK::FSubView& InSubView, USpatialNetDriver& InNetDriver)
{
	SubView = &InSubView;
	NetDriver = &InNetDriver;

	check(NetDriver->Connection && NetDriver->Sender);

	if (NetDriver->LoadBalanceStrategy == nullptr)
	{
		UE_LOG(LogSpatialNetDriverGameplayDebuggerContext, Error, TEXT("Enabling GDKGameplayDebugger before LB strategy is setup"));
		return;
	}

	// Allocate and assign a specific "gameplay debugger LB strategy". This new
	// strategy wraps the existing default strategy, and allows "replicator actors"
	// to be intercepted and handled through specific gameplay debugger LB rules.
	// All other actors are handled by the (wrapped) default strategy.
	LBStrategy = NewObject<UGameplayDebuggerLBStrategy>();
	LBStrategy->Init(*this, *NetDriver->LoadBalanceStrategy);
	NetDriver->LoadBalanceStrategy = LBStrategy;
	NetDriver->Sender->UpdatePartitionEntityInterestAndPosition();

	TSet<VirtualWorkerId> VirtualWorkerIds = LBStrategy->GetVirtualWorkerIds();

	PhysicalToVirtualWorkerIdMap.Reserve(VirtualWorkerIds.Num());
	for (const auto& VirtualWorkerId : VirtualWorkerIds)
	{
		const FString* PhysicalWorkerName = NetDriver->VirtualWorkerTranslator->GetPhysicalWorkerForVirtualWorker(VirtualWorkerId);
		if (PhysicalWorkerName != nullptr)
		{
			PhysicalToVirtualWorkerIdMap.Add(*PhysicalWorkerName, VirtualWorkerId);
		}
		else
		{
			UE_LOG(LogSpatialNetDriverGameplayDebuggerContext, Error, TEXT("Failed to convert virtual worker to physical worker name"));
		}
	}
}

void USpatialNetDriverGameplayDebuggerContext::Reset()
{
	for (auto& TrackedEntity : TrackedEntities)
	{
		if (AGameplayDebuggerCategoryReplicator* Replicator =
				Cast<AGameplayDebuggerCategoryReplicator>(TrackedEntity.Value.ReplicatorWeakObjectPtr.Get()))
		{
			UnregisterServerRequestCallback(*Replicator, TrackedEntity.Value);
		}
	}

	TrackedEntities.Empty();
	ComponentsAdded.Empty();
	ComponentsUpdated.Empty();
	ActorsAdded.Empty();
}

TOptional<VirtualWorkerId> USpatialNetDriverGameplayDebuggerContext::GetActorDelegatedWorkerId(const AActor& InActor)
{
	check(NetDriver && NetDriver->PackageMap);

	FEntityData* EntityData = nullptr;

	Worker_EntityId EntityId = NetDriver->PackageMap->GetEntityIdFromObject(&InActor);
	if (EntityId != SpatialConstants::INVALID_ENTITY_ID)
	{
		EntityData = TrackedEntities.Find(EntityId);
	}

	if (EntityData == nullptr)
	{
		return {};
	}

	return EntityData->Component.DelegatedVirtualWorkerId;
}

void USpatialNetDriverGameplayDebuggerContext::AdvanceView()
{
	const SpatialGDK::FSubViewDelta& ViewDelta = SubView->GetViewDelta();
	for (const SpatialGDK::EntityDelta& Delta : ViewDelta.EntityDeltas)
	{
		switch (Delta.Type)
		{
		case SpatialGDK::EntityDelta::ADD:
			TrackEntity(Delta.EntityId);
			break;
		case SpatialGDK::EntityDelta::REMOVE:
			UntrackEntity(Delta.EntityId);
			break;
		case SpatialGDK::EntityDelta::TEMPORARILY_REMOVED:
			UntrackEntity(Delta.EntityId);
			TrackEntity(Delta.EntityId);
			break;
		case SpatialGDK::EntityDelta::UPDATE:
			for (const SpatialGDK::AuthorityChange& Change : Delta.AuthorityGained)
			{
				if (Change.ComponentSetId == SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID)
				{
					AddAuthority(Delta.EntityId, TrackedEntities.Find(Delta.EntityId));
				}
			}
			for (const SpatialGDK::AuthorityChange& Change : Delta.AuthorityLostTemporarily)
			{
				if (Change.ComponentSetId == SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID)
				{
					RemoveAuthority(Delta.EntityId, TrackedEntities.Find(Delta.EntityId));
				}
			}
			for (const SpatialGDK::AuthorityChange& Change : Delta.AuthorityLost)
			{
				if (Change.ComponentSetId == SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID)
				{
					RemoveAuthority(Delta.EntityId, TrackedEntities.Find(Delta.EntityId));
				}
			}
			break;
		}
	}
}

void USpatialNetDriverGameplayDebuggerContext::TickServer()
{
	check(NetDriver && NetDriver->Connection && NetDriver->Sender && NetDriver->PackageMap);

	for (const auto& EntityAdded : ComponentsAdded)
	{
		if (NetDriver->HasServerAuthority(EntityAdded))
		{
			if (FEntityData* EntityData = TrackedEntities.Find(EntityAdded))
			{
				NetDriver->Connection->GetCoordinator().RefreshEntityCompleteness(EntityAdded);
			}
		}
	}
	ComponentsAdded.Reset();

	for (const auto& EntityUpdated : ComponentsUpdated)
	{
		if (NetDriver->HasServerAuthority(EntityUpdated))
		{
			if (FEntityData* EntityData = TrackedEntities.Find(EntityUpdated))
			{
				FWorkerComponentUpdate ComponentUpdate = EntityData->Component.CreateComponentUpdate();
				NetDriver->Connection->SendComponentUpdate(EntityUpdated, &ComponentUpdate);
			}
		}
	}
	ComponentsUpdated.Reset();

	for (auto It = ActorsAdded.CreateIterator(); It; It++)
	{
		// If authority lost, then forget about this actor
		if (!NetDriver->HasServerAuthority(*It))
		{
			It.RemoveCurrent();
			continue;
		}

		// If entity lost, then forget about this actor
		FEntityData* EntityData = TrackedEntities.Find(*It);
		if (EntityData == nullptr)
		{
			It.RemoveCurrent();
			continue;
		}

		TWeakObjectPtr<UObject> EntityObjectWeakPtr = NetDriver->PackageMap->GetObjectFromEntityId(*It);
		AGameplayDebuggerCategoryReplicator* CategoryReplicator = Cast<AGameplayDebuggerCategoryReplicator>(EntityObjectWeakPtr.Get());
		if (CategoryReplicator == nullptr)
		{
			// Not unexpected - assume latency and wait for actor to appear
			continue;
		}

		// Update tracking mode, and if changed send an update
		const bool ShouldTrackPlayer = CategoryReplicator->GetServerTrackingMode() == EGameplayDebuggerServerTrackingMode::Player;
		if (EntityData->Component.TrackPlayer != ShouldTrackPlayer)
		{
			EntityData->Component.TrackPlayer = ShouldTrackPlayer;
			ComponentsUpdated.Add(*It);
		}

		EntityData->ReplicatorWeakObjectPtr = TWeakObjectPtr<AGameplayDebuggerCategoryReplicator>(CategoryReplicator);

		// If we have authority over this replicator:
		//	a. Provide a list of available servers to the replicator actor - this allows the replicator
		//		to provide an interface where an authorative server (from available servers) can be selected
		//	b. Register a callback with the replicator actor, to handle authorative server change requests
		if (CategoryReplicator->HasAuthority())
		{
			// a. Build and provide available servers
			TArray<FString> PhysicalWorkerIds;
			PhysicalToVirtualWorkerIdMap.GetKeys(PhysicalWorkerIds);
			CategoryReplicator->SetAvailableServers(PhysicalWorkerIds);

			// b. Register callback
			RegisterServerRequestCallback(*CategoryReplicator, *EntityData);
		}

		It.RemoveCurrent();
	}

	for (auto& Pair : TrackedEntities)
	{
		if (Pair.Value.Component.TrackPlayer)
		{
			if (!NetDriver->HasServerAuthority(Pair.Key))
			{
				// Do not track player if we don't have authority
				continue;
			}

			TWeakObjectPtr<AGameplayDebuggerCategoryReplicator> ReplicatorWeakObjectPtr = Pair.Value.ReplicatorWeakObjectPtr;
			AGameplayDebuggerCategoryReplicator* Replicator = ReplicatorWeakObjectPtr.Get();
			if (Replicator == nullptr)
			{
				// Not unexpected - assume latency and wait for actor to appear
				continue;
			}

			const AActor* PlayerControllerActor = Cast<AActor>(Replicator->GetReplicationOwner());
			if (PlayerControllerActor == nullptr)
			{
				// Not unexpected due to tests
				continue;
			}

			const VirtualWorkerId PlayerControllerVirtualWorkerId = GetActorVirtualWorkerId(*PlayerControllerActor);
			if (PlayerControllerVirtualWorkerId == SpatialConstants::INVALID_VIRTUAL_WORKER_ID)
			{
				// Not unexpected due to tests
				continue;
			}

			check(NetDriver->LoadBalanceStrategy);
			const VirtualWorkerId ReplicatorVirtualWorkerId = NetDriver->LoadBalanceStrategy->WhoShouldHaveAuthority(*Replicator);
			if (PlayerControllerVirtualWorkerId == ReplicatorVirtualWorkerId)
			{
				// No change in virtual worker, so nothing to do
				continue;
			}

			check(NetDriver && NetDriver->VirtualWorkerTranslator);

			const FString* PhysicalWorkerName =
				NetDriver->VirtualWorkerTranslator->GetPhysicalWorkerForVirtualWorker(PlayerControllerVirtualWorkerId);
			if (!ensureMsgf(PhysicalWorkerName != nullptr, TEXT("Serious error: Cannot find physical name for worker based")))
			{
				continue;
			}

			Pair.Value.Component.DelegatedVirtualWorkerId = PlayerControllerVirtualWorkerId;
			Pair.Value.CurrentWorkerId = *PhysicalWorkerName;
			Replicator->SetCurrentServer(*PhysicalWorkerName);
			ComponentsUpdated.Add(Pair.Key);
		}
	}
}

void USpatialNetDriverGameplayDebuggerContext::TrackEntity(Worker_EntityId InEntityId)
{
	check(NetDriver && NetDriver->VirtualWorkerTranslator);

	const SpatialGDK::EntityViewElement& Element = SubView->GetView().FindChecked(InEntityId);
	const SpatialGDK::ComponentData* Data = Element.Components.FindByPredicate([](const SpatialGDK::ComponentData& Component) {
		return Component.GetComponentId() == SpatialConstants::GDK_GAMEPLAY_DEBUGGER_COMPONENT_ID;
	});

	if (Data == nullptr)
	{
		UE_LOG(LogSpatialNetDriverGameplayDebuggerContext, Error, TEXT("Failed to access component data for entity %lld"), InEntityId);
		return;
	}

	Schema_ComponentData* ComponentData = Data->GetUnderlying();
	if (ComponentData == nullptr)
	{
		UE_LOG(LogSpatialNetDriverGameplayDebuggerContext, Error, TEXT("Failed to get underlying component data for entity %lld"),
			   InEntityId);
		return;
	}

	FEntityData* EntityData = TrackedEntities.Find(InEntityId);
	if (EntityData == nullptr)
	{
		EntityData = &TrackedEntities.Add(InEntityId);
		EntityData->Component = SpatialGDK::GameplayDebuggerComponent(*ComponentData);
		ComponentsAdded.Add(InEntityId);
		ActorsAdded.Add(InEntityId);
	}
	else
	{
		UE_LOG(LogSpatialNetDriverGameplayDebuggerContext, Error, TEXT("Tracking entity twice, where id = %lld"), InEntityId);
	}

	check(EntityData);

	const bool HasAuthority = NetDriver->HasServerAuthority(InEntityId);
	if (HasAuthority)
	{
		AddAuthority(InEntityId, EntityData);
	}
	else
	{
		RemoveAuthority(InEntityId, EntityData);
	}
}

void USpatialNetDriverGameplayDebuggerContext::UntrackEntity(Worker_EntityId InEntityId)
{
	check(NetDriver && NetDriver->PackageMap);

	RemoveAuthority(InEntityId, TrackedEntities.Find(InEntityId));

	TrackedEntities.Remove(InEntityId);
	ComponentsAdded.Remove(InEntityId);
	ComponentsUpdated.Remove(InEntityId);
	ActorsAdded.Remove(InEntityId);
}

void USpatialNetDriverGameplayDebuggerContext::AddAuthority(Worker_EntityId InEntityId, FEntityData* OptionalEntityData)
{
	check(NetDriver && NetDriver->VirtualWorkerTranslator);

	if (OptionalEntityData == nullptr)
	{
		return;
	}

	OptionalEntityData->Component.DelegatedVirtualWorkerId = LBStrategy->GetLocalVirtualWorkerId();
	OptionalEntityData->Component.TrackPlayer = false; // correct value is assigned actor is resolved (on authorative server)

	const FString* PhysicalWorkerName =
		NetDriver->VirtualWorkerTranslator->GetPhysicalWorkerForVirtualWorker(OptionalEntityData->Component.DelegatedVirtualWorkerId);
	if (PhysicalWorkerName != nullptr)
	{
		OptionalEntityData->CurrentWorkerId = *PhysicalWorkerName;
	}
	else
	{
		UE_LOG(LogSpatialNetDriverGameplayDebuggerContext, Error, TEXT("Physical worker name not found"));
	}

	ActorsAdded.AddUnique(InEntityId);
}

void USpatialNetDriverGameplayDebuggerContext::RemoveAuthority(Worker_EntityId InEntityId, FEntityData* InOptionalEntityData)
{
	if (InOptionalEntityData == nullptr)
	{
		return;
	}

	InOptionalEntityData->Component.DelegatedVirtualWorkerId = 0;
	InOptionalEntityData->CurrentWorkerId.Empty();

	if (AGameplayDebuggerCategoryReplicator* Replicator =
			Cast<AGameplayDebuggerCategoryReplicator>(InOptionalEntityData->ReplicatorWeakObjectPtr.Get()))
	{
		UnregisterServerRequestCallback(*Replicator, *InOptionalEntityData);
	}
}

void USpatialNetDriverGameplayDebuggerContext::RegisterServerRequestCallback(AGameplayDebuggerCategoryReplicator& InReplicator,
																			 FEntityData& InEntityData)
{
	if (!InEntityData.Handle.IsValid())
	{
		InEntityData.Handle =
			InReplicator.OnServerTrackingRequest().AddUObject(this, &USpatialNetDriverGameplayDebuggerContext::OnServerTrackingRequest);
	}
	else
	{
		UE_LOG(LogSpatialNetDriverGameplayDebuggerContext, Error, TEXT("Trying to bind change notification more than once"));
	}
}

void USpatialNetDriverGameplayDebuggerContext::UnregisterServerRequestCallback(AGameplayDebuggerCategoryReplicator& InReplicator,
																			   FEntityData& InEntityData)
{
	check(NetDriver && NetDriver->PackageMap);

	if (InEntityData.Handle.IsValid())
	{
		InReplicator.OnServerTrackingRequest().Remove(InEntityData.Handle);
		InEntityData.Handle.Reset();
	}
}

void USpatialNetDriverGameplayDebuggerContext::OnServerTrackingRequest(AGameplayDebuggerCategoryReplicator* InCategoryReplicator,
																	   bool InTrackPlayer, FString InOptionalServerWorkerId)
{
	check(NetDriver && NetDriver->PackageMap && NetDriver->Connection);

	if (InCategoryReplicator == nullptr)
	{
		UE_LOG(LogSpatialNetDriverGameplayDebuggerContext, Error, TEXT("Callback with null replicator"));
		return;
	}

	if (!InCategoryReplicator->HasAuthority())
	{
		UE_LOG(LogSpatialNetDriverGameplayDebuggerContext, Warning,
			   TEXT("Only expect to be registered and receive this callback when there is authority"));
		return;
	}

	Worker_EntityId EntityId = NetDriver->PackageMap->GetEntityIdFromObject(InCategoryReplicator);
	if (EntityId == SpatialConstants::INVALID_ENTITY_ID)
	{
		UE_LOG(LogSpatialNetDriverGameplayDebuggerContext, Warning, TEXT("Callback from an actor with no entity"));
		return;
	}

	FEntityData* EntityData = TrackedEntities.Find(EntityId);
	if (EntityData == nullptr)
	{
		UE_LOG(LogSpatialNetDriverGameplayDebuggerContext, Warning, TEXT("Callback from an entity we are not tracking"));
		return;
	}

	bool ShouldUpdateComponent = false;

	if (InTrackPlayer != EntityData->Component.TrackPlayer)
	{
		EntityData->Component.TrackPlayer = InTrackPlayer;
		ShouldUpdateComponent = true;
	}

	if ((InOptionalServerWorkerId.Len() > 0) && (EntityData->CurrentWorkerId != InOptionalServerWorkerId))
	{
		const uint32* VirtualWorkerId = PhysicalToVirtualWorkerIdMap.Find(InOptionalServerWorkerId);
		if (VirtualWorkerId != nullptr)
		{
			EntityData->Component.DelegatedVirtualWorkerId = *VirtualWorkerId;
			EntityData->CurrentWorkerId = InOptionalServerWorkerId;
			InCategoryReplicator->SetCurrentServer(InOptionalServerWorkerId);
			ShouldUpdateComponent = true;
		}
		else
		{
			UE_LOG(LogSpatialNetDriverGameplayDebuggerContext, Error, TEXT("Invalid server worker id provided ('%s')"));
		}
	}

	if (ShouldUpdateComponent)
	{
		ComponentsUpdated.Add(EntityId);
	}
}

VirtualWorkerId USpatialNetDriverGameplayDebuggerContext::GetActorVirtualWorkerId(const AActor& InActor) const
{
	check(SubView != nullptr && NetDriver != nullptr && NetDriver->PackageMap != nullptr);

	const Worker_EntityId EntityId = NetDriver->PackageMap->GetEntityIdFromObject(&InActor);
	if (EntityId == SpatialConstants::INVALID_ENTITY_ID)
	{
		return SpatialConstants::INVALID_VIRTUAL_WORKER_ID;
	}

	const SpatialGDK::EntityViewElement* EntityViewPtr = SubView->GetView().Find(EntityId);
	if (EntityViewPtr != nullptr)
	{
		return SpatialConstants::INVALID_VIRTUAL_WORKER_ID;
	}

	const SpatialGDK::ComponentData* IntentComponentData =
		EntityViewPtr->Components.FindByPredicate([](const SpatialGDK::ComponentData& Data) {
		return Data.GetComponentId() == SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID;
	});

	if (IntentComponentData == nullptr)
	{
		return SpatialConstants::INVALID_VIRTUAL_WORKER_ID;
	}

	SpatialGDK::AuthorityIntent Intent = SpatialGDK::AuthorityIntent(IntentComponentData->GetUnderlying());
	const VirtualWorkerId VirtualWorkerId = Intent.VirtualWorkerId;
	return VirtualWorkerId;
};

#endif // WITH_GAMEPLAY_DEBUGGER
