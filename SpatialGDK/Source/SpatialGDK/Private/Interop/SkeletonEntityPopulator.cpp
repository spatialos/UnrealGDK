#include "Interop/SkeletonEntityPopulator.h"

#include "Algo/AllOf.h"
#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/ActorSystem.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Schema/SkeletonEntityManifest.h"
#include "SpatialView/EntityComponentTypes.h"
#include "SpatialView/ViewCoordinator.h"
#include "Utils/ComponentFactory.h"

namespace SpatialGDK
{
static void ForEachCompleteEntity(bool& bIsFirstCall, const FSubView& SubView, const TFunction<void(Worker_EntityId)> Handler)
{
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

				if (Delta.Type == EntityDelta::ADD || Delta.Type == EntityDelta::UPDATE || Delta.Type == EntityDelta::TEMPORARILY_REMOVED)
			{
				const Worker_EntityId EntityId = Delta.EntityId;
				Handler(EntityId);
			}
		}
	}
}

FSkeletonEntityPopulator::FSkeletonEntityPopulator(USpatialNetDriver& InNetDriver)
	: NetDriver(&InNetDriver)
	, LocallyAuthSkeletonEntityManifestsSubview(
		  SkeletonEntityFunctions::CreateLocallyAuthManifestSubView(InNetDriver.Connection->GetCoordinator()))
	, SkeletonEntitiesRequiringPopulationSubview(InNetDriver.Connection->GetCoordinator().CreateSubView(
		  SpatialConstants::SKELETON_ENTITY_POPULATION_AUTH_TAG_COMPONENT_ID, FSubView::NoFilter, FSubView::NoDispatcherCallbacks))
{
}

void FSkeletonEntityPopulator::Advance(ISpatialOSWorker& Connection)
{
	const auto& ViewDelta = LocallyAuthSkeletonEntityManifestsSubview.GetViewDelta().EntityDeltas;
	for (const auto& Delta : ViewDelta)
	{
		if (Manifests.Contains(Delta.EntityId))
		{
			continue;
		}
		if (Delta.Type == EntityDelta::ADD)
		{
			ManifestProcessing ToProcess(NetDriver);
			ToProcess.ManifestEntityId = Delta.EntityId;
			const ComponentData* ManifestDataPtr = GetCoordinator().GetComponent(Delta.EntityId, FSkeletonEntityManifest::ComponentId);
			ToProcess.Manifest = FSkeletonEntityManifest(*ManifestDataPtr);
			ToProcess.Manifest.bAckedManifest = true;
			GetCoordinator().SendComponentUpdate(Delta.EntityId, ToProcess.Manifest.CreateComponentUpdate());
			Manifests.Add(Delta.EntityId, MoveTemp(ToProcess));
		}
	}

	for (auto Iterator = Manifests.CreateIterator(); Iterator; ++Iterator)
	{
		Iterator->Value.Advance(*this);
		if (Iterator->Value.Stage == ManifestProcessing::EStage::Finished)
		{
			Iterator.RemoveCurrent();
			++NumManifestProcessed;
		}
	}
}

void FSkeletonEntityPopulator::ManifestProcessing::Advance(FSkeletonEntityPopulator& Populator)
{
	if (Stage == EStage::DiscoverAssignedSkeletonEntities)
	{
		ForEachCompleteEntity(bIsFirstPopulatingEntitiesCall, Populator.SkeletonEntitiesRequiringPopulationSubview,
							  [this](const Worker_EntityId EntityId) {
								  const EntityViewElement* EntityPtr = NetDriver->Connection->GetCoordinator().GetView().Find(EntityId);

								  check(EntityPtr != nullptr);

								  ConsiderEntityPopulation(EntityId, *EntityPtr);
							  });

		GetCoordinator().SendComponentUpdate(ManifestEntityId, Manifest.CreateComponentUpdate(), {});

		GetCoordinator().RefreshEntityCompleteness(ManifestEntityId);

		if (Manifest.PopulatedEntities.Num() == Manifest.EntitiesToPopulate.Num())
		{
			UE_LOG(LogSpatialSkeletonEntityCreator, Log, TEXT("All %d entities populated"), Manifest.PopulatedEntities.Num());
			Stage = EStage::RefreshCompleteness;
		}
	}

	if (Stage == EStage::RefreshCompleteness)
	{
		// All locally populated entities need entity refreshes as this worker won't receive
		// deltas for all the components it has added, and these can impact entity completeness.
		for (const Worker_EntityId EntityId : Manifest.PopulatedEntities)
		{
			GetCoordinator().RefreshEntityCompleteness(EntityId);
		}

		Stage = EStage::Finished;
	}
}

void FSkeletonEntityPopulator::ManifestProcessing::ConsiderEntityPopulation(const Worker_EntityId EntityId,
																			const EntityViewElement& Element)
{
	const ComponentData* UnrealMetadataComponent = Element.Components.FindByPredicate(ComponentIdEquality{ UnrealMetadata::ComponentId });

	if (!ensure(UnrealMetadataComponent != nullptr))
	{
		return;
	}

	if (!Manifest.EntitiesToPopulate.Contains(EntityId))
	{
		UE_LOG(LogSpatialSkeletonEntityCreator, Verbose,
			   TEXT("Skeleton entity %lld seen on worker, but doesn't exist in the manifest. Should be an auth race"), EntityId);
		return;
	}

	UnrealMetadata ActorMetadata(UnrealMetadataComponent->GetUnderlying());
	UClass* Class = ActorMetadata.GetNativeEntityClass();

	if (ensure(FUnrealObjectRef::IsUniqueActorClass(Class)
			   || (ActorMetadata.bNetStartup.IsSet() && ActorMetadata.bNetStartup.GetValue() && ActorMetadata.StablyNamedRef.IsSet())))
	{
		FUnrealObjectRef ActorRef;
		if (FUnrealObjectRef::IsUniqueActorClass(Class))
		{
			ActorRef = FUnrealObjectRef::FromObjectPtr(Class, NetDriver->PackageMap);
			if (ActorRef.IsValid())
			{
				ActorRef.bUseClassPathToLoadObject = true;
			}
		}
		else
		{
			ActorRef = ActorMetadata.StablyNamedRef.GetValue();
		}

		bool bUnresolved = false;
		UObject* Object = FUnrealObjectRef::ToObjectPtr(ActorRef, NetDriver->PackageMap, bUnresolved);
		if (!ensure(!bUnresolved))
		{
			return;
		}

		AActor* StartupActor = Cast<AActor>(Object);
		PopulateEntity(EntityId, *StartupActor);

		Manifest.PopulatedEntities.Emplace(EntityId);
	}
}

void FSkeletonEntityPopulator::ManifestProcessing::PopulateEntity(Worker_EntityId SkeletonEntityId, AActor& SkeletonEntityStartupActor)
{
	NetDriver->PackageMap->ResolveEntityActorAndSubobjects(SkeletonEntityId, &SkeletonEntityStartupActor);

	SkeletonEntityStartupActor.Role = ROLE_Authority;
	SkeletonEntityStartupActor.RemoteRole = ROLE_SimulatedProxy;

	USpatialActorChannel* Channel = ActorSystem::SetUpActorChannel(NetDriver, &SkeletonEntityStartupActor, SkeletonEntityId);

	if (ensure(IsValid(Channel)))
	{
		// Mark this channel as creating new entity as we need to remember to populate it
		// with all components when we decide to replicate it for the first time.
		Channel->bCreatingNewEntity = true;
	}
}

bool FSkeletonEntityPopulator::IsReady() const
{
	return NumManifestProcessed > 0;
}

bool FSkeletonEntityPopulator::HasReceivedManifests() const
{
	return LocallyAuthSkeletonEntityManifestsSubview.GetCompleteEntities().Num() > 0;
}

ViewCoordinator& FSkeletonEntityPopulator::GetCoordinator()
{
	return NetDriver->Connection->GetCoordinator();
}

ViewCoordinator& FSkeletonEntityPopulator::ManifestProcessing::GetCoordinator()
{
	return NetDriver->Connection->GetCoordinator();
}
} // namespace SpatialGDK
