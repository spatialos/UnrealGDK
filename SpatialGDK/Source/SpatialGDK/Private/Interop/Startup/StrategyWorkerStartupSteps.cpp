#include "Interop/Startup/StrategyWorkerStartupSteps.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"

#include "Interop/SkeletonEntityPopulator.h"

#include "Interop/GlobalStateManager.h"
#include "Utils/EntityFactory.h"

namespace SpatialGDK
{
FCreateStagingPartition::FCreateStagingPartition(TSharedRef<FServerWorkerStartupContext> InState, USpatialNetDriver& InNetDriver,
												 UGlobalStateManager& InGlobalStateManager)
	: State(InState)
	, NetDriver(InNetDriver)
	, GlobalStateManager(InGlobalStateManager)
{
	StepName = TEXT("Create local worker staging partition");
}

void FCreateStagingPartition::Start()
{
	auto& Coordinator = NetDriver.Connection->GetCoordinator();
	StagingPartitionId = NetDriver.PackageMap->AllocateNewEntityId();
	TArray<FWorkerComponentData> Components = SpatialGDK::EntityFactory::CreatePartitionEntityComponents(
		TEXT("WorkerStagingPartition"), StagingPartitionId, nullptr, QueryConstraint(), 0, true);

	TArray<ComponentData> PartitionComponentsToCreate;
	Algo::Transform(Components, PartitionComponentsToCreate, [](const FWorkerComponentData& Component) {
		return ComponentData(OwningComponentDataPtr(Component.schema_type), Component.component_id);
	});

	Worker_RequestId CreationRequest =
		Coordinator.SendCreateEntityRequest(MoveTemp(PartitionComponentsToCreate), StagingPartitionId, RETRY_UNTIL_COMPLETE);
	CommandsHandler.AddRequest(CreationRequest, [this](const Worker_CreateEntityResponseOp& Op) {
		auto& Coordinator = NetDriver.Connection->GetCoordinator();
		CommandsHandler.ClaimPartition(Coordinator, Coordinator.GetWorkerSystemEntityId(), StagingPartitionId,
									   [this](const Worker_CommandResponseOp& Op) {
										   bPartitionClaimed = true;
									   });
	});
}

bool FCreateStagingPartition::TryFinish()
{
	CommandsHandler.ProcessOps(NetDriver.Connection->GetCoordinator().GetWorkerMessages());
	if (!bPartitionClaimed)
	{
		return false;
	}

	auto& Coordinator = NetDriver.Connection->GetCoordinator();
	if (!Coordinator.GetView().Contains(StagingPartitionId))
	{
		return false;
	}

	NetDriver.StagingPartitionId = StagingPartitionId;
	ComponentUpdate MarkAsReady(SpatialConstants::SERVER_WORKER_COMPONENT_ID);
	Schema_AddBool(MarkAsReady.GetFields(), SpatialConstants::SERVER_WORKER_READY_TO_BEGIN_PLAY_ID, true);
	Coordinator.SendComponentUpdate(*State->WorkerEntityId, MoveTemp(MarkAsReady));
	return true;
}

FWaitForGSMAuthOrInitialManifest::FWaitForGSMAuthOrInitialManifest(TSharedRef<FServerWorkerStartupContext> InState,
																   USpatialNetDriver& InNetDriver,
																   UGlobalStateManager& InGlobalStateManager)
	: State(InState)
	, NetDriver(InNetDriver)
	, GlobalStateManager(InGlobalStateManager)
{
	StepName = TEXT("Wait for GSM Auth or manifest");
}

void FWaitForGSMAuthOrInitialManifest::Start()
{
	// Initialize all actors role to simulated while the skeletons are created.
	for (AActor* Actor : TActorRange<AActor>(NetDriver.GetWorld()))
	{
		if (!IsValid(Actor))
		{
			continue;
		}

		if (!Actor->GetIsReplicated())
		{
			continue;
		}

		const bool bIsStartupActor = Actor->IsNetStartupActor();
		const bool bIsStablyNamedAndReplicated = Actor->IsNameStableForNetworking();

		if (!bIsStablyNamedAndReplicated && !FUnrealObjectRef::IsUniqueActorClass(Actor->GetClass()))
		{
			continue;
		}

		Actor->Role = ROLE_SimulatedProxy;
		Actor->RemoteRole = ROLE_Authority;
	}
}

bool FWaitForGSMAuthOrInitialManifest::TryFinish()
{
	// The strategy worker will decide which worker is the GSM auth worker.
	if (GlobalStateManager.HasAuthority())
	{
		State->bHasGSMAuth = true;
		GlobalStateManager.SetDeploymentState();
		return true;
	}

	return NetDriver.SkeletonPopulator->HasReceivedManifests();
}

} // namespace SpatialGDK
