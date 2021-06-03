#include "Interop/SkeletonEntities.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetDriverRPC.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "EngineUtils.h"
#include "Interop/ActorSystem.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialSender.h"
#include "Interop/WellKnownEntitySystem.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Schema/SkeletonEntityManifest.h"
#include "SpatialView/ViewCoordinator.h"
#include "Utils/ComponentFactory.h"
#include "Utils/EntityFactory.h"
#include "Utils/EntityPool.h"
#include "Utils/InterestFactory.h"

namespace SpatialGDK
{
DEFINE_LOG_CATEGORY_STATIC(LogSpatialSkeletonEntityCreator, Log, All);

static ComponentData Convert(const FWorkerComponentData& WorkerComponentData)
{
	return ComponentData(OwningComponentDataPtr(WorkerComponentData.schema_type), WorkerComponentData.component_id);
};

FDistributedStartupActorSkeletonEntityCreator::FDistributedStartupActorSkeletonEntityCreator(USpatialNetDriver& InNetDriver)
	: NetDriver(&InNetDriver)
	, ManifestSubView(&NetDriver->Connection->GetCoordinator().CreateSubView(
		  FSkeletonEntityManifest::ComponentId, FSubView::NoFilter,
		  { NetDriver->Connection->GetCoordinator().CreateComponentChangedRefreshCallback(FSkeletonEntityManifest::ComponentId) }))
{
}

void FDistributedStartupActorSkeletonEntityCreator::CreateSkeletonEntitiesForLevel(UWorld& World)
{
	ensure(Stage == EStage::Initial);

	UE_LOG(LogSpatialSkeletonEntityCreator, Log, TEXT("Creating skeleton entities for world %s"), *World.GetName())

	Stage = EStage::CreatingEntities;

	TSet<Worker_EntityId_Key> SkeletonEntityIds;

	for (AActor* Actor : TActorRange<AActor>(&World))
	{
		if (!IsValid(Actor))
		{
			continue;
		}

		if (!(Actor->IsNameStableForNetworking() && Actor->GetIsReplicated()))
		{
			continue;
		}

		const Worker_EntityId SkeletonEntityId = CreateSkeletonEntityForActor(*Actor);

		UE_LOG(LogSpatialSkeletonEntityCreator, Log, TEXT("EntityId: %lld Creating skeleton entity for %s"), SkeletonEntityId,
			   *Actor->GetName());

		RemainingSkeletonEntities.Emplace(SkeletonEntityId);
		SkeletonEntityIds.Emplace(SkeletonEntityId);
	}

	if (RemainingSkeletonEntities.Num() == 0)
	{
		UE_LOG(LogSpatialSkeletonEntityCreator, Log, TEXT("No skeleton entities must be created for %s"), *World.GetName())

		Stage = EStage::DelegatingEntities;
	}
}

Worker_EntityId FDistributedStartupActorSkeletonEntityCreator::CreateSkeletonEntityForActor(AActor& Actor)
{
	const Worker_EntityId ActorEntityId = NetDriver->PackageMap->AllocateEntityId();

	EntityFactory Factory(NetDriver, NetDriver->PackageMap, NetDriver->ClassInfoManager, NetDriver->RPCService.Get());

	TArray<FWorkerComponentData> EntityComponents = Factory.CreateSkeletonEntityComponents(&Actor);

	// LB components also contain authority delegation, giving this worker ServerAuth.
	Factory.WriteLBComponents(EntityComponents, &Actor);

	// RPC components.
	auto RPCService = NetDriver->RPCService.Get();

	checkf(RPCService != nullptr, TEXT("Attempting to create an entity with a null RPCService."));
	EntityComponents.Append(RPCService->GetRPCComponentsOnEntityCreation(ActorEntityId));
	EntityComponents.Append(NetDriver->RPCs->GetRPCComponentsOnEntityCreation(ActorEntityId));

	// Skeleton entity markers		.
	EntityComponents.Emplace(ComponentFactory::CreateEmptyComponentData(SpatialConstants::FLESHOUT_QUERY_TAG_COMPONENT_ID));
	EntityComponents.Emplace(ComponentFactory::CreateEmptyComponentData(SpatialConstants::FLESHOUT_REQUIRED_TAG_COMPONENT_ID));

	EntityComponents.Add(NetDriver->InterestFactory->CreateInterestData(
		&Actor, NetDriver->ClassInfoManager->GetOrCreateClassInfoByObject(&Actor), ActorEntityId));

	TArray<ComponentData> SkeletonEntityComponentDatas;
	Algo::Transform(EntityComponents, SkeletonEntityComponentDatas, Convert);

	ViewCoordinator& Coordinator = NetDriver->Connection->GetCoordinator();
	const Worker_RequestId CreateEntityRequestId =
		Coordinator.SendCreateEntityRequest(MoveTemp(SkeletonEntityComponentDatas), ActorEntityId);

	CreateEntityDelegate OnCreated = CreateEntityDelegate::CreateLambda(
		[this, ActorEntityId, WeakActor = MakeWeakObjectPtr(&Actor)](const Worker_CreateEntityResponseOp&) {
			RemainingSkeletonEntities.Remove(ActorEntityId);
			if (WeakActor.IsValid())
			{
				SkeletonEntitiesToDelegate.Emplace(MakeTuple(ActorEntityId, WeakActor));
				UE_LOG(LogSpatialSkeletonEntityCreator, Log, TEXT("EntityId: %lld Created skeleton entity"), ActorEntityId);
			}
			if (RemainingSkeletonEntities.Num() == 0)
			{
				Stage = EStage::DelegatingEntities;
			}
		});
	CreateHandler.AddRequest(CreateEntityRequestId, MoveTemp(OnCreated));

	return ActorEntityId;
}

void FDistributedStartupActorSkeletonEntityCreator::Advance()
{
	CreateHandler.ProcessOps(NetDriver->Connection->GetWorkerMessages());

	if (Stage == EStage::DelegatingEntities)
	{
		for (auto Kvp : SkeletonEntitiesToDelegate)
		{
			if (!Kvp.Value.IsValid())
			{
				continue;
			}

			const Worker_EntityId CreatedSkeletonEntityId = Kvp.Key;
			AActor& SkeletonEntityActor = *Kvp.Value;

			const VirtualWorkerId AuthWorkerId = NetDriver->LoadBalanceStrategy->WhoShouldHaveAuthority(SkeletonEntityActor);

			const Worker_PartitionId AuthWorkerPartition =
				NetDriver->VirtualWorkerTranslator->GetPartitionEntityForVirtualWorker(AuthWorkerId);

			UE_LOG(LogSpatialSkeletonEntityCreator, Log,
				   TEXT("EntityId: %lld Delegating skeleton entity to VirtualWorker %d Partition %lld"), CreatedSkeletonEntityId,
				   AuthWorkerId, AuthWorkerPartition);

			AuthorityDelegation AuthorityDelegationComponent;
			AuthorityDelegationComponent.Delegations.Emplace(SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID, AuthWorkerPartition);

			EntitiesToPopulatingWorkers.Emplace(CreatedSkeletonEntityId, AuthWorkerId);

			NetDriver->Connection->GetCoordinator().SendComponentUpdate(
				CreatedSkeletonEntityId,
				ComponentUpdate(OwningComponentUpdatePtr(AuthorityDelegationComponent.CreateAuthorityDelegationUpdate().schema_type),
								AuthorityDelegation::ComponentId),
				{});
		}

		SkeletonEntitiesToDelegate.Empty();

		UE_LOG(LogSpatialSkeletonEntityCreator, Log, TEXT("Finished delegating entities, moving on to manifest"));

		Stage = EStage::SigningManifest;
	}

	if (Stage == EStage::SigningManifest)
	{
		TMap<VirtualWorkerId, TArray<Worker_EntityId>> WorkerToEntitiesToPopulate;
		// Every worker should get a manifest to proceed through the skeleton entity flow.
		for (const auto& VirtualWorker : NetDriver->WellKnownEntitySystem->VirtualWorkerTranslationManager->GetVirtualWorkerMapping())
		{
			WorkerToEntitiesToPopulate.FindOrAdd(VirtualWorker.Key);
		}
		for (const auto& SkeletonEntityToPopulatingWorker : EntitiesToPopulatingWorkers)
		{
			WorkerToEntitiesToPopulate.FindOrAdd(SkeletonEntityToPopulatingWorker.Value).Add(SkeletonEntityToPopulatingWorker.Key);
		}
		for (const auto& WorkerToEntitiesToPopulateEl : WorkerToEntitiesToPopulate)
		{
			const VirtualWorkerId WorkerId = WorkerToEntitiesToPopulateEl.Key;
			const TArray<Worker_EntityId>& EntitiesToPopulate = WorkerToEntitiesToPopulateEl.Value;

			TArray<ComponentData> SkeletonEntityManifestComponents;

			{
				FSkeletonEntityManifest Manifest;
				Manifest.EntitiesToPopulate = EntitiesToPopulate;

				SkeletonEntityManifestComponents.Emplace(Manifest.CreateComponentData());
			}

			const Worker_PartitionId ServerWorkerPartitionId =
				NetDriver->VirtualWorkerTranslator->GetPartitionEntityForVirtualWorker(WorkerId);

			{
				const Worker_PartitionId ThisWorkerPartitionId = NetDriver->VirtualWorkerTranslator->GetClaimedPartitionId();
				AuthorityDelegation Authority;
				Authority.Delegations.Emplace(SpatialConstants::SKELETON_ENTITY_MANIFEST_AUTH_COMPONENT_SET_ID, ServerWorkerPartitionId);
				Authority.Delegations.Emplace(SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID, ThisWorkerPartitionId);

				SkeletonEntityManifestComponents.Emplace(
					ComponentData(OwningComponentDataPtr(Authority.CreateComponentData().schema_type), AuthorityDelegation::ComponentId));
			}

			{
				Query Q;
				Q.Constraint.bSelfConstraint = true;
				Q.ResultComponentIds.Append(
					{ SpatialConstants::SKELETON_ENTITY_MANIFEST_COMPONENT_ID, SpatialConstants::AUTHORITY_DELEGATION_COMPONENT_ID });

				ComponentSetInterest Interest1{ { Q } };

				Interest ManifestInterest;
				ManifestInterest.ComponentInterestMap.Emplace(SpatialConstants::SKELETON_ENTITY_MANIFEST_AUTH_COMPONENT_SET_ID, Interest1);
				ManifestInterest.ComponentInterestMap.Emplace(SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID, Interest1);

				SkeletonEntityManifestComponents.Emplace(
					ComponentData(OwningComponentDataPtr(ManifestInterest.CreateComponentData().schema_type), Interest::ComponentId));
			}

			SkeletonEntityManifestComponents.Emplace(ComponentData(Position::ComponentId));

			ViewCoordinator& Coordinator = NetDriver->Connection->GetCoordinator();
			Coordinator.SendCreateEntityRequest(MoveTemp(SkeletonEntityManifestComponents), /*EntityId =*/{});

			UE_LOG(LogSpatialSkeletonEntityCreator, Log, TEXT("Created manifest for VirtualWorker %d WorkerPartition %lld"), WorkerId,
				   ServerWorkerPartitionId);
		}

		ManifestsCreatedCount = WorkerToEntitiesToPopulate.Num();

		UE_LOG(LogSpatialSkeletonEntityCreator, Log, TEXT("Finished creating manifests"));
		Stage = EStage::WaitingForPopulation;
	}

	if (Stage == EStage::WaitingForPopulation)
	{
		auto ProcessEntities = [](bool& bIsFirstCall, const FSubView& SubView, const TFunction<void(Worker_EntityId)> Handler) {
			if (bIsFirstCall)
			{
				bIsFirstCall = false;
				for (const Worker_EntityId EntityId : SubView.GetCompleteEntities())
				{
					Handler(EntityId);
				}
			}
			else
			{
				for (const EntityDelta& Delta : SubView.GetViewDelta().EntityDeltas)
				{
					checkf(Delta.Type != EntityDelta::REMOVE,
						   TEXT("Skeleton entity manifest must never be removed, but was removed from entity %lld"), Delta.EntityId)

						if (Delta.Type == EntityDelta::ADD || Delta.Type == EntityDelta::UPDATE
							|| Delta.Type == EntityDelta::TEMPORARILY_REMOVED)
					{
						const Worker_EntityId EntityId = Delta.EntityId;
						Handler(EntityId);
					}
				}
			}
		};

		auto ProcessManifest = [this](const Worker_EntityId ManifestEntityId) {
			const EntityViewElement* Element = ManifestSubView->GetView().Find(ManifestEntityId);

			const ComponentData* ManifestComponent =
				Element->Components.FindByPredicate(ComponentIdEquality{ FSkeletonEntityManifest::ComponentId });

			checkf(ManifestComponent != nullptr, TEXT("Skeleton entity manifest must never be removed, but was removed from entity %lld"),
				   ManifestEntityId);

			const FSkeletonEntityManifest Manifest(*ManifestComponent);

			Manifests.Add(ManifestEntityId, Manifest);
		};

		ProcessEntities(bIsFirstTimeProcessingManifests, *ManifestSubView, ProcessManifest);

		if (Manifests.Num() == ManifestsCreatedCount)
		{
			const bool bAllManifestsFinished =
				Algo::AllOf(Manifests, [](const TPair<Worker_EntityId_Key, FSkeletonEntityManifest>& ManifestPair) {
					return ManifestPair.Value.PopulatedEntities.Num() == ManifestPair.Value.EntitiesToPopulate.Num();
				});

			if (bAllManifestsFinished)
			{
				Stage = EStage::FinalizingSkeletons;
			}
		}
	}

	if (Stage == EStage::FinalizingSkeletons)
	{
		for (const Worker_EntityId_Key SkeletonEntityId : CreatedSkeletonEntities)
		{
			ViewCoordinator& Coordinator = NetDriver->Connection->GetCoordinator();

			Coordinator.SendRemoveComponent(SkeletonEntityId, SpatialConstants::FLESHOUT_REQUIRED_TAG_COMPONENT_ID, {});
			Coordinator.SendRemoveComponent(SkeletonEntityId, SpatialConstants::FLESHOUT_QUERY_TAG_COMPONENT_ID, {});
		}

		Stage = EStage::Finished;
	}
}

bool FDistributedStartupActorSkeletonEntityCreator::IsReady() const
{
	return Stage == EStage::Finished;
}

void FDistributedStartupActorSkeletonEntityCreator::HackAddManifest(const Worker_EntityId EntityId, const FSkeletonEntityManifest& Manifest)
{
	ensure(Stage == EStage::WaitingForPopulation);

	UE_LOG(LogSpatialSkeletonEntityCreator, Log, TEXT("Adding manifest with %d entities to %lld as a hack..."),
		   Manifest.PopulatedEntities.Num(), EntityId);

	Manifests.Add(EntityId, Manifest);
}

FSkeletonEntityPopulator::FSkeletonEntityPopulator(USpatialNetDriver& InNetDriver)
	: NetDriver(&InNetDriver)
	, SubView(&InNetDriver.Connection->GetCoordinator().CreateSubView(SpatialConstants::FLESHOUT_REQUIRED_TAG_COMPONENT_ID,
																	  FSubView::NoFilter, FSubView::NoDispatcherCallbacks))
{
	Stage = EStage::ReceivingManifest;
}

void FSkeletonEntityPopulator::ConsiderEntityPopulation(const Worker_EntityId EntityId, const EntityViewElement& Element)
{
	const ComponentData* UnrealMetadataComponent = Element.Components.FindByPredicate(ComponentIdEquality{ UnrealMetadata::ComponentId });

	if (!ensure(UnrealMetadataComponent != nullptr))
	{
		return;
	}

	if (!ensure(Manifest->EntitiesToPopulate.Contains(EntityId)))
	{
		return;
	}

	const UnrealMetadata ActorMetadata(UnrealMetadataComponent->GetUnderlying());

	if (ActorMetadata.bNetStartup.IsSet() && ActorMetadata.bNetStartup.GetValue() && ActorMetadata.StablyNamedRef.IsSet())
	{
		const FUnrealObjectRef& ActorRef = ActorMetadata.StablyNamedRef.GetValue();
		bool bUnresolved = false;
		UObject* Object = FUnrealObjectRef::ToObjectPtr(ActorRef, NetDriver->PackageMap, bUnresolved);
		if (!ensure(!bUnresolved))
		{
			return;
		}

		AActor* StartupActor = Cast<AActor>(Object);
		PopulateEntity(EntityId, *StartupActor);

		Manifest->PopulatedEntities.Emplace(EntityId);
	}

	EntitiesFleshedOut.Emplace(EntityId);
}

void FSkeletonEntityPopulator::Advance()
{
	if (Stage == EStage::ReceivingManifest)
	{
		for (const auto& Kvp : GetCoordinator().GetView())
		{
			const Worker_EntityId EntityId = Kvp.Key;
			const ComponentData* ManifestPtr = GetCoordinator().GetComponent(EntityId, FSkeletonEntityManifest::ComponentId);
			if (ManifestPtr == nullptr)
			{
				continue;
			}
			if (!Kvp.Value.Authority.Contains(SpatialConstants::SKELETON_ENTITY_MANIFEST_AUTH_COMPONENT_SET_ID))
			{
				continue;
			}
			check(!ManifestEntityId);
			ManifestEntityId = EntityId;
			Manifest = FSkeletonEntityManifest(*ManifestPtr);
		}
		if (Manifest)
		{
			Stage = EStage::PopulatingEntities;
		}
	}

	if (Stage == EStage::PopulatingEntities)
	{
		if (!bStartedPopulatingEntities)
		{
			bStartedPopulatingEntities = true;

			for (const Worker_EntityId_Key CompleteEntities : SubView->GetCompleteEntities())
			{
				ConsiderEntityPopulation(CompleteEntities, NetDriver->Connection->GetCoordinator().GetView()[CompleteEntities]);
			}
		}
		else
		{
			for (const EntityDelta& Delta : SubView->GetViewDelta().EntityDeltas)
			{
				if (Delta.Type == EntityDelta::ADD)
				{
					const Worker_EntityId EntityId = Delta.EntityId;
					const EntityViewElement& Element = SubView->GetView()[Delta.EntityId];

					ConsiderEntityPopulation(EntityId, Element);
				}
			}
		}

		if (OnManifestUpdated)
		{
			OnManifestUpdated(*ManifestEntityId, *Manifest);
		}
		else
		{
			GetCoordinator().SendComponentUpdate(*ManifestEntityId, Manifest->CreateComponentUpdate(), {});
		}

		if (Manifest->PopulatedEntities.Num() == Manifest->EntitiesToPopulate.Num())
		{
			UE_LOG(LogSpatialSkeletonEntityCreator, Log, TEXT("All %d entities populated"), Manifest->PopulatedEntities.Num());
			Stage = EStage::SigningManifest;
		}
	}

	if (Stage == EStage::SigningManifest)
	{
		for (const Worker_EntityId EntityId : Manifest->PopulatedEntities)
		{
			GetCoordinator().RefreshEntityCompleteness(EntityId);
		}

		Stage = EStage::Finished;
	}
}

void FSkeletonEntityPopulator::PopulateEntity(Worker_EntityId SkeletonEntityId, AActor& SkeletonEntityStartupActor)
{
	EntityFactory Factory(NetDriver, NetDriver->PackageMap, NetDriver->ClassInfoManager, NetDriver->RPCService.Get());

	NetDriver->PackageMap->ResolveEntityActor(&SkeletonEntityStartupActor, SkeletonEntityId);

	TArray<FWorkerComponentData> Components;

	// Hack: EntityFactory::WriteUnrealComponents expects an UnrealMetadata to be present in the components array,
	// but since we aren't creating a complete entities but only fleshing out an existing one, we have to get this
	// specific component from the entity in view.
	Components.Emplace(NetDriver->Connection->GetCoordinator()
						   .GetView()[SkeletonEntityId]
						   .Components.FindByPredicate(ComponentIdEquality{ UnrealMetadata::ComponentId })
						   ->GetWorkerComponentData());

	USpatialActorChannel* Channel = NetDriver->ActorSystem->SetUpActorChannel(&SkeletonEntityStartupActor, SkeletonEntityId);

	check(IsValid(Channel));

	uint32 BytesWrittenDummy = 0;
	Factory.WriteUnrealComponents(Components, Channel, BytesWrittenDummy);
	Components.Add(ActorSystem::CreateLevelComponentData(SkeletonEntityStartupActor, *NetDriver->GetWorld(), *NetDriver->ClassInfoManager));

	UE_LOG(LogSpatialSkeletonEntityCreator, Log, TEXT("EntityId: %lld Fleshing out with %d components"), SkeletonEntityId,
		   Components.Num());

	Components.RemoveAll([](const FWorkerComponentData& C) {
		return C.component_id == UnrealMetadata::ComponentId;
	});

	for (const FWorkerComponentData& ComponentData : Components)
	{
		NetDriver->Connection->GetCoordinator().SendAddComponent(SkeletonEntityId, Convert(ComponentData), {});
	}

	{
		ViewCoordinator& Coordinator = NetDriver->Connection->GetCoordinator();

		Coordinator.SendRemoveComponent(SkeletonEntityId, SpatialConstants::FLESHOUT_REQUIRED_TAG_COMPONENT_ID, {});
		Coordinator.SendRemoveComponent(SkeletonEntityId, SpatialConstants::FLESHOUT_QUERY_TAG_COMPONENT_ID, {});
	}
}

bool FSkeletonEntityPopulator::IsReady() const
{
	return Stage == EStage::Finished;
}

ViewCoordinator& FSkeletonEntityPopulator::GetCoordinator()
{
	return NetDriver->Connection->GetCoordinator();
}
} // namespace SpatialGDK
