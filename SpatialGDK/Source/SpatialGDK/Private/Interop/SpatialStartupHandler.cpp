#include "SpatialStartupHandler.h"

#include "Algo/AllOf.h"
#include "Algo/Copy.h"
#include "Algo/MinElement.h"
#include "Algo/Transform.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/SpatialPlayerSpawner.h"
#include "Schema/ServerWorker.h"
#include "SpatialView/EntityComponentTypes.h"
#include "Utils/EntityFactory.h"
#include "Utils/InterestFactory.h"

DEFINE_LOG_CATEGORY_STATIC(LogSpatialStartupHandler, Log, All);

namespace SpatialGDK
{
FSpatialStartupHandler::FSpatialStartupHandler(USpatialNetDriver& InNetDriver, const FInitialSetup& InSetup)
	: ClaimHandler(*InNetDriver.Connection)
	, Setup(InSetup)
	, NetDriver(&InNetDriver)

{
}

bool FSpatialStartupHandler::TryFinishStartup()
{
	if (Stage == EStage::CreateWorkerEntity)
	{
		if (!bCalledCreateEntity)
		{
			bCalledCreateEntity = true;
			ServerWorkerEntityCreator1.Emplace(*NetDriver, *NetDriver->Connection);
		}
		ServerWorkerEntityCreator1->ProcessOps(GetOps());
		if (ServerWorkerEntityCreator1->IsFinished())
		{
			WorkerEntityId = ServerWorkerEntityCreator1->GetWorkerEntityId();
			Stage = EStage::WaitForWorkerEntities;
		}
	}

	if (Stage == EStage::WaitForWorkerEntities)
	{
		TMap<Worker_EntityId_Key, const ComponentData*> WorkerComponents;

		for (const auto& EntityData : GetCoordinator().GetView())
		{
			const ComponentData* WorkerComponent =
				EntityData.Value.Components.FindByPredicate(ComponentIdEquality{ SpatialConstants::SERVER_WORKER_COMPONENT_ID });
			if (WorkerComponent != nullptr)
			{
				WorkerComponents.Emplace(EntityData.Key, WorkerComponent);
			}
		}
		if (WorkerComponents.Num() >= Setup.ExpectedServerWorkersCount)
		{
			ensureMsgf(WorkerComponents.Num() == Setup.ExpectedServerWorkersCount,
					   TEXT("There should never be more server workers connected than we expect. Expected %d, actually have %d"),
					   Setup.ExpectedServerWorkersCount, WorkerComponents.Num());
			WorkerComponents.GetKeys(WorkerEntityIds);
			Stage = EStage::WaitForGSMEntity;
		}
	}

	if (Stage == EStage::WaitForGSMEntity)
	{
		const EntityViewElement* GlobalStateManagerEntity = GetCoordinator().GetView().Find(GetGSM().GlobalStateManagerEntityId);
		if (GlobalStateManagerEntity != nullptr)
		{
			Stage = EStage::DeriveDeploymentRecoveryState;
		}
	}

	if (Stage == EStage::DeriveDeploymentRecoveryState)
	{
		const EntityViewElement& GlobalStateManagerEntity = GetCoordinator().GetView().FindChecked(GetGSM().GlobalStateManagerEntityId);
		const ComponentData* StartupActorManagerData = GlobalStateManagerEntity.Components.FindByPredicate(
			ComponentIdEquality{ SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID });
		if (StartupActorManagerData != nullptr)
		{
			if (Schema_GetBoolCount(StartupActorManagerData->GetFields(), SpatialConstants::STARTUP_ACTOR_MANAGER_CAN_BEGIN_PLAY_ID) != 0)
			{
				// StartupActorManager's CAN_BEGIN_PLAY is raised after startup is finished, and is false in the initial snapshot.
				// If it's true at this point, then this worker is either recovering or loading from a non-initial snapshot.
				bIsRecoveringOrSnapshot =
					GetBoolFromSchema(StartupActorManagerData->GetFields(), SpatialConstants::STARTUP_ACTOR_MANAGER_CAN_BEGIN_PLAY_ID);

				// We should only call BeginPlay on actors if this is a fresh deployment; otherwise, we assume that
				// BeginPlay has already been called on them before.
				NetDriver->GlobalStateManager->bCanSpawnWithAuthority = !bIsRecoveringOrSnapshot;

				Stage = EStage::TryClaimingGSMEntityAuthority;
			}
		}
	}

	if (Stage == EStage::TryClaimingGSMEntityAuthority)
	{
		const bool bDidClaimStartupPartition = TryClaimingStartupPartition();
		if (bDidClaimStartupPartition)
		{
			Stage = EStage::WaitForGSMEntityAuthority;
		}
		else
		{
			Stage = EStage::GetVirtualWorkerTranslationState;
			bHasGSMAuth = false;
		}
	}

	if (Stage == EStage::WaitForGSMEntityAuthority)
	{
		const EntityViewElement& GlobalStateManagerEntity = GetCoordinator().GetView().FindChecked(GetGSM().GlobalStateManagerEntityId);
		if (GlobalStateManagerEntity.Authority.Contains(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID))
		{
			bHasGSMAuth = true;
			// HACK
			GetGSM().SetDeploymentState();
			Stage = EStage::FillWorkerTranslationState;
		}
	}

	if (Stage == EStage::FillWorkerTranslationState)
	{
		TMap<VirtualWorkerId, SpatialVirtualWorkerTranslator::WorkerInformation> VirtualWorkerMapping;

		bool bFoundTranslator = false;
		bool bNeedToCreatePartitions = false;
		const EntityViewElement* VirtualWorkerTranslatorEntity =
			GetCoordinator().GetView().Find(SpatialConstants::INITIAL_VIRTUAL_WORKER_TRANSLATOR_ENTITY_ID);
		if (VirtualWorkerTranslatorEntity != nullptr)
		{
			const ComponentData* VirtualWorkerTranslatorComponent = VirtualWorkerTranslatorEntity->Components.FindByPredicate(
				ComponentIdEquality{ SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID });
			if (ensure(VirtualWorkerTranslatorComponent != nullptr))
			{
				bFoundTranslator = true;
				SpatialVirtualWorkerTranslator::ApplyMappingFromSchema(VirtualWorkerMapping,
																	   *VirtualWorkerTranslatorComponent->GetFields());
				Algo::Transform(VirtualWorkerMapping, WorkerPartitions,
								[](const TPair<VirtualWorkerId, SpatialVirtualWorkerTranslator::WorkerInformation>& Pair) {
									return Pair.Value.PartitionEntityId;
								});
				bNeedToCreatePartitions = WorkerPartitions.Num() != Setup.ExpectedServerWorkersCount;
			}
		}

		if (bFoundTranslator)
		{
			if (bNeedToCreatePartitions)
			{
				if (!bHasCalledPartitionEntityCreate)
				{
					bHasCalledPartitionEntityCreate = true;

					UE_LOG(LogSpatialStartupHandler, Log, TEXT("Spawning partition entities for %d virtual workers"),
						   Setup.ExpectedServerWorkersCount);
					for (VirtualWorkerId GeneratedVirtualWorkerId = 1;
						 GeneratedVirtualWorkerId <= static_cast<VirtualWorkerId>(Setup.ExpectedServerWorkersCount);
						 ++GeneratedVirtualWorkerId)
					{
						if (VirtualWorkerMapping.Contains(GeneratedVirtualWorkerId))
						{
							// This virtual worker mapping already exists, no need to create a partition.
							continue;
						}

						PhysicalWorkerName WorkerName = TEXT("DUMMY");

						const ComponentData* ServerWorkerData = GetCoordinator().GetComponent(WorkerEntityIds[GeneratedVirtualWorkerId - 1],
																							  SpatialConstants::SERVER_WORKER_COMPONENT_ID);
						if (ensure(ServerWorkerData != nullptr))
						{
							WorkerName = GetStringFromSchema(ServerWorkerData->GetFields(), SpatialConstants::SERVER_WORKER_NAME_ID);
						}

						const Worker_EntityId PartitionEntityId = NetDriver->PackageMap->AllocateNewEntityId();
						UE_LOG(LogSpatialStartupHandler, Log, TEXT("- Virtual Worker: %d. Entity: %lld. "), GeneratedVirtualWorkerId,
							   PartitionEntityId);
						SpawnPartitionEntity(PartitionEntityId, GeneratedVirtualWorkerId);
						WorkersToPartitions.Emplace(GeneratedVirtualWorkerId,
													SpatialVirtualWorkerTranslator::WorkerInformation{
														WorkerName, WorkerEntityIds[GeneratedVirtualWorkerId - 1], PartitionEntityId });
					}
				}
			}

			EntityHandler.ProcessOps(GetOps());

			if (PartitionsToCreate.Num() == 0)
			{
				TMap<Worker_EntityId_Key, const ComponentData*> WorkerComponents;
				Algo::Transform(WorkerEntityIds, WorkerComponents, [this](const Worker_EntityId EntityId) {
					return TPair<Worker_EntityId_Key, const ComponentData*>{
						EntityId, GetCoordinator().GetView()[EntityId].Components.FindByPredicate(
									  ComponentIdEquality{ SpatialConstants::SERVER_WORKER_COMPONENT_ID })
					};
				});

				// All partitions created.
				ComponentUpdate Update(SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID);
				for (const auto& Entry : WorkersToPartitions)
				{
					Schema_Object* EntryObject =
						Schema_AddObject(Update.GetFields(), SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID);
					Schema_AddUint32(EntryObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID, Entry.Key);
					AddStringToSchema(EntryObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME_ID, Entry.Value.WorkerName);
					Schema_AddEntityId(EntryObject, SpatialConstants::MAPPING_SERVER_WORKER_ENTITY_ID, Entry.Value.ServerWorkerEntityId);
					Schema_AddEntityId(EntryObject, SpatialConstants::MAPPING_PARTITION_ID, Entry.Value.PartitionEntityId);

					const ComponentData* ServerWorkerComponentData = WorkerComponents.FindChecked(Entry.Value.ServerWorkerEntityId);
					const ServerWorker ServerWorkerData(ServerWorkerComponentData->GetUnderlying());
					ClaimHandler.ClaimPartition(ServerWorkerData.SystemEntityId, Entry.Value.PartitionEntityId);
				}
				GetCoordinator().SendComponentUpdate(SpatialConstants::INITIAL_VIRTUAL_WORKER_TRANSLATOR_ENTITY_ID, MoveTemp(Update));
				Stage = EStage::GetVirtualWorkerTranslationState;
			}
		}
	}

	if (Stage == EStage::GetVirtualWorkerTranslationState)
	{
		const ComponentData* DeploymentMapComponent =
			GetCoordinator().GetView()[SpatialConstants::INITIAL_GLOBAL_STATE_MANAGER_ENTITY_ID].Components.FindByPredicate(
				ComponentIdEquality{ SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID });
		if (ensure(DeploymentMapComponent != nullptr))
		{
			const int32 ExistingSessionId =
				Schema_GetInt32(DeploymentMapComponent->GetFields(), SpatialConstants::DEPLOYMENT_MAP_SESSION_ID);

			GetGSM().ApplySessionId(ExistingSessionId);
		}

		const EntityViewElement* VirtualWorkerTranslatorEntity =
			GetCoordinator().GetView().Find(SpatialConstants::INITIAL_VIRTUAL_WORKER_TRANSLATOR_ENTITY_ID);
		if (VirtualWorkerTranslatorEntity != nullptr)
		{
			const ComponentData* WorkerTranslatorComponentData = VirtualWorkerTranslatorEntity->Components.FindByPredicate(
				ComponentIdEquality{ SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID });
			if (WorkerTranslatorComponentData != nullptr)
			{
				// We've received worker translation data.
				TMap<VirtualWorkerId, SpatialVirtualWorkerTranslator::WorkerInformation> VirtualWorkerMapping;
				SpatialVirtualWorkerTranslator::ApplyMappingFromSchema(VirtualWorkerMapping, *WorkerTranslatorComponentData->GetFields());
				for (const auto& Mapping : VirtualWorkerMapping)
				{
					if (Mapping.Value.ServerWorkerEntityId == WorkerEntityId)
					{
						LocalVirtualWorkerId = Mapping.Key;
						LocalPartitionId = Mapping.Value.PartitionEntityId;
						Stage = EStage::WaitForAssignedPartition;
					}
				}
			}
		}
	}

	if (Stage == EStage::WaitForAssignedPartition)
	{
		// ClaimHandler.ClaimPartition(NetDriver->Connection->GetWorkerSystemEntityId(), LocalPartitionId);
		Stage = EStage::WaitForAssignedPartition2;
	}

	if (Stage == EStage::WaitForAssignedPartition2)
	{
		const EntityViewElement* AssignedPartitionEntity = GetCoordinator().GetView().Find(LocalPartitionId);
		if (AssignedPartitionEntity != nullptr)
		{
			const EntityViewElement* VirtualWorkerTranslatorEntity =
				GetCoordinator().GetView().Find(SpatialConstants::INITIAL_VIRTUAL_WORKER_TRANSLATOR_ENTITY_ID);
			if (VirtualWorkerTranslatorEntity != nullptr)
			{
				const ComponentData* WorkerTranslatorComponentData = VirtualWorkerTranslatorEntity->Components.FindByPredicate(
					ComponentIdEquality{ SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID });
				if (WorkerTranslatorComponentData != nullptr)
				{
					// We've received worker translation data.
					NetDriver->VirtualWorkerTranslator->ApplyMappingFromSchema(WorkerTranslatorComponentData->GetFields());

					// NOTE: Consider if waiting for partition should be a separate step from sending ReadyToBeginPlay.
					ComponentUpdate MarkAsReady(SpatialConstants::SERVER_WORKER_COMPONENT_ID);
					Schema_AddBool(MarkAsReady.GetFields(), SpatialConstants::SERVER_WORKER_READY_TO_BEGIN_PLAY_ID, true);
					GetCoordinator().SendComponentUpdate(WorkerEntityId, MoveTemp(MarkAsReady));

					Stage = bHasGSMAuth ? EStage::DispatchGSMStartPlay : EStage::WaitForGSMStartPlay;
				}
			}
		}
	}

	if (Stage == EStage::DispatchGSMStartPlay)
	{
		const bool bAreAllWorkerEntitiesReady = Algo::AllOf(WorkerEntityIds, [this](const Worker_EntityId ServerWorkerEntityId) -> bool {
			const EntityViewElement& ServerWorkerEntity = GetCoordinator().GetView().FindChecked(ServerWorkerEntityId);
			const ComponentData* ServerWorkerComponent =
				ServerWorkerEntity.Components.FindByPredicate(ComponentIdEquality{ SpatialConstants::SERVER_WORKER_COMPONENT_ID });
			return GetBoolFromSchema(ServerWorkerComponent->GetFields(), SpatialConstants::SERVER_WORKER_READY_TO_BEGIN_PLAY_ID);
		});

		if (bAreAllWorkerEntitiesReady)
		{
			GetGSM().SendCanBeginPlayUpdate(true);

			NetDriver->CreateAndInitializeCoreClassesAfterStartup();

			GetGSM().TriggerBeginPlay();

			// Hmm - this seems necessary because unless we call this after NotifyBeginPlay has been triggered, it won't actually
			// do anything, because internally it checks that BeginPlay has actually been called. I'm not sure why we called
			// SetAcceptingPlayers above though unless it was only to catch the non-auth server instances. In which case the auth
			// server is failing to call SetAcceptingPlayers again at some later point.
			//
			// I've now removed it from the other places it used to be called, because I believe they were both neither no longer
			// valid. Above because the world tick won't have begun, and during the deployment man auth gained, for the same reason.
			// Leaving this comment block in for review reasons but will remove before merging.
			GetGSM().SetAcceptingPlayers(true);

			Stage = EStage::Finished;
		}
	}

	if (Stage == EStage::WaitForGSMStartPlay)
	{
		const EntityViewElement& GlobalStateManagerEntity = GetCoordinator().GetView().FindChecked(GetGSM().GlobalStateManagerEntityId);
		const ComponentData* StartupActorManagerData = GlobalStateManagerEntity.Components.FindByPredicate(
			ComponentIdEquality{ SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID });
		if (ensure(StartupActorManagerData != nullptr)
			&& Schema_GetBool(StartupActorManagerData->GetFields(), SpatialConstants::STARTUP_ACTOR_MANAGER_CAN_BEGIN_PLAY_ID))
		{
			NetDriver->CreateAndInitializeCoreClassesAfterStartup();

			GetGSM().TriggerBeginPlay();

			Stage = EStage::Finished;
		}
	}

	return Stage == EStage::Finished;
}

void FSpatialStartupHandler::SpawnPartitionEntity(Worker_EntityId PartitionEntityId, VirtualWorkerId VirtualWorkerId)
{
	const bool bRunStrategyWorker = USpatialStatics::IsStrategyWorkerEnabled();
	UAbstractLBStrategy* LoadBalanceStrategy = NetDriver->LoadBalanceStrategy;
	const bool bDirectAssignment = bRunStrategyWorker && !LoadBalanceStrategy->IsStrategyWorkerAware();

	UnrealServerInterestFactory* InterestF = nullptr;
	if (!bRunStrategyWorker || bDirectAssignment)
	{
		InterestF = NetDriver->InterestFactory.Get();
	}

	SpatialGDK::QueryConstraint LBConstraint = LoadBalanceStrategy->GetWorkerInterestQueryConstraint(VirtualWorkerId);

	TArray<FWorkerComponentData> Components = EntityFactory::CreatePartitionEntityComponents(
		TEXT("WorkerPartition"), PartitionEntityId, InterestF, LBConstraint, VirtualWorkerId, NetDriver->DebugCtx != nullptr);
	TArray<ComponentData> PartitionComponentsToCreate;
	Algo::Transform(Components, PartitionComponentsToCreate, [](const FWorkerComponentData& Component) {
		return ComponentData(OwningComponentDataPtr(Component.schema_type), Component.component_id);
	});
	const Worker_RequestId RequestId = GetCoordinator().SendCreateEntityRequest(MoveTemp(PartitionComponentsToCreate), PartitionEntityId,
																				SpatialGDK::RETRY_UNTIL_COMPLETE);

	PartitionsToCreate.Emplace(PartitionEntityId);

	CreateEntityDelegate OnCreateWorkerEntityResponse;
	OnCreateWorkerEntityResponse.BindLambda([this, PartitionEntityId](const Worker_CreateEntityResponseOp& Op) {
		PartitionsToCreate.Remove(PartitionEntityId);
	});

	EntityHandler.AddRequest(RequestId, MoveTemp(OnCreateWorkerEntityResponse));
}

bool FSpatialStartupHandler::TryClaimingStartupPartition()
{
	UGlobalStateManager* GlobalStateManager = &GetGSM();

	// Perform a naive leader election where we wait for the correct number of server workers to be present in the deployment, and then
	// whichever server has the lowest server worker entity ID becomes the leader and claims the snapshot partition.
	const Worker_EntityId LocalServerWorkerEntityId = WorkerEntityId;

	check(LocalServerWorkerEntityId != SpatialConstants::INVALID_ENTITY_ID);

	const Worker_EntityId_Key* LowestEntityId = Algo::MinElement(WorkerEntityIds);

	check(LowestEntityId != nullptr);

	if (LocalServerWorkerEntityId == *LowestEntityId)
	{
		UE_LOG(LogSpatialStartupHandler, Log, TEXT("MaybeClaimSnapshotPartition claiming snapshot partition"));
		GlobalStateManager->ClaimSnapshotPartition();
		return true;
	}
	UE_LOG(LogSpatialStartupHandler, Log, TEXT("Not claiming snapshot partition"));
	return false;
}

ViewCoordinator& FSpatialStartupHandler::GetCoordinator()
{
	return NetDriver->Connection->GetCoordinator();
}

const ViewCoordinator& FSpatialStartupHandler::GetCoordinator() const
{
	return NetDriver->Connection->GetCoordinator();
}

const TArray<Worker_Op>& FSpatialStartupHandler::GetOps() const
{
	return GetCoordinator().GetWorkerMessages();
}

UGlobalStateManager& FSpatialStartupHandler::GetGSM()
{
	return *NetDriver->GlobalStateManager;
}

FSpatialClientStartupHandler::FSpatialClientStartupHandler(USpatialNetDriver& InNetDriver, UGameInstance& InGameInstance,
														   const FInitialSetup& InSetup)
	: Setup(InSetup)
	, NetDriver(&InNetDriver)
	, GameInstance(&InGameInstance)
{
}

bool FSpatialClientStartupHandler::TryFinishStartup()
{
	if (!bQueriedGSM)
	{
		bQueriedGSM = true;
		QueryGSM();
	}

	TOptional<USpatialNetDriver::FPendingNetworkFailure> PendingNetworkFailure;

	QueryHandler.ProcessOps(GetOps());

	if (Stage == EStage::QueryGSM)
	{
		FString StartupClientDebugString;
		if (GSMData && SnapshotData && GSMData->bAcceptingPlayers)
		{
			// TODO: Get snapshot version from the GSM entity or whatever.
			const uint64 ServerSnapshotVersion = SnapshotData->SnapshotVersion; // NetDriver->GlobalStateManager->GetSnapshotVersion();
			const uint32 ServerSchemaHash = GSMData->SchemaHash;				// GlobalStateManager->GetSchemaHash();
			const uint32 LocalSchemaHash =
				NetDriver->ClassInfoManager->SchemaDatabase->SchemaBundleHash; // GlobalStateManager->GetSchemaHash();
			const uint32 LocalSessionId = NetDriver->ClientGetSessionId();
			if (GSMData->DeploymentSessionId != LocalSessionId)
			{
				StartupClientDebugString = FString::Printf(TEXT("GlobalStateManager session id mismatch - got (%d) expected (%d)."),
														   GSMData->DeploymentSessionId, LocalSessionId);
			}
			else if (SpatialConstants::SPATIAL_SNAPSHOT_VERSION != ServerSnapshotVersion)
			{
				UE_LOG(LogSpatialOSNetDriver, Error,
					   TEXT("Your client's snapshot version does not match your deployment's snapshot version. Client version: = '%llu', "
							"Server "
							"version = '%llu'"),
					   ServerSnapshotVersion, SpatialConstants::SPATIAL_SNAPSHOT_VERSION);

				PendingNetworkFailure = {
					ENetworkFailure::OutdatedClient,
					TEXT("Your snapshot version of the game does not match that of the server. Please try updating your game snapshot.")
				};
			}
			else if (LocalSchemaHash != ServerSchemaHash) // Are we running with the same schema hash as the server?
			{
				UE_LOG(LogSpatialOSNetDriver, Error,
					   TEXT("Your client's schema does not match your deployment's schema. Client hash: '%u' Server hash: '%u'"),
					   LocalSchemaHash, ServerSchemaHash);

				PendingNetworkFailure = {
					ENetworkFailure::OutdatedClient,
					TEXT("Your version of the game does not match that of the server. Please try updating your game version.")
				};
			}
			else
			{
				UWorld* CurrentWorld = NetDriver->GetWorld();
				const FString& DeploymentMapURL = GSMData->DeploymentMapURL;
				if (CurrentWorld == nullptr || UWorld::RemovePIEPrefix(DeploymentMapURL) != UWorld::RemovePIEPrefix(CurrentWorld->URL.Map))
				{
					Stage = EStage::WaitForMapLoad;

					PostMapLoadedDelegateHandle =
						FCoreUObjectDelegates::PostLoadMapWithWorld.AddRaw(this, &FSpatialClientStartupHandler::OnMapLoaded);

					// Load the correct map based on the GSM URL
					UE_LOG(LogSpatial, Log, TEXT("Welcomed by SpatialOS (Level: %s)"), *DeploymentMapURL);

					// Extract map name and options
					FWorldContext& WorldContext = GEngine->GetWorldContextFromPendingNetGameNetDriverChecked(NetDriver);
					FURL LastURL = WorldContext.PendingNetGame->URL;

					FURL RedirectURL = FURL(&LastURL, *DeploymentMapURL, (ETravelType)WorldContext.TravelType);
					RedirectURL.Host = LastURL.Host;
					RedirectURL.Port = LastURL.Port;
					RedirectURL.Portal = LastURL.Portal;

					// Usually the LastURL options are added to the RedirectURL in the FURL constructor.
					// However this is not the case when TravelType = TRAVEL_Absolute so we must do it explicitly here.
					if (WorldContext.TravelType == ETravelType::TRAVEL_Absolute)
					{
						RedirectURL.Op.Append(LastURL.Op);
					}

					RedirectURL.AddOption(*SpatialConstants::ClientsStayConnectedURLOption);

					WorldContext.PendingNetGame->bSuccessfullyConnected = true;
					WorldContext.PendingNetGame->bSentJoinRequest = false;
					WorldContext.PendingNetGame->URL = RedirectURL;
				}
				else
				{
					Stage = EStage::SendPlayerSpawnRequest;
				}
			}
		}
	}

	if (Stage == EStage::WaitForMapLoad)
	{
		if (bFinishedMapLoad)
		{
			Stage = EStage::SendPlayerSpawnRequest;
		}
	}

	if (Stage == EStage::SendPlayerSpawnRequest)
	{
		NetDriver->PlayerSpawner->SendPlayerSpawnRequest();
		Stage = EStage::Finished;
	}

	NetDriver->PendingNetworkFailure = PendingNetworkFailure;

	return Stage == EStage::Finished;
}

void FSpatialClientStartupHandler::OnMapLoaded(UWorld* LoadedWorld)
{
	if (LoadedWorld == nullptr)
	{
		return;
	}

	if (LoadedWorld->GetNetDriver() != NetDriver)
	{
		// In PIE, if we have more than 2 clients, then OnMapLoaded is going to be triggered once each client loads the world.
		// As the delegate is a global variable, it triggers all 3 USpatialNetDriver::OnMapLoaded callbacks. As a result, we should
		// make sure that the net driver of this world is in fact us.
		return;
	}

	FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostMapLoadedDelegateHandle);
	bFinishedMapLoad = true;
}

void FSpatialClientStartupHandler::QueryGSM()
{
	// Build a constraint for the GSM.
	Worker_ComponentConstraint GSMComponentConstraint{};
	GSMComponentConstraint.component_id = SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID;

	Worker_Constraint GSMConstraint{};
	GSMConstraint.constraint_type = WORKER_CONSTRAINT_TYPE_COMPONENT;
	GSMConstraint.constraint.component_constraint = GSMComponentConstraint;

	Worker_EntityQuery GSMQuery{};
	GSMQuery.constraint = GSMConstraint;
	static TArray<Worker_ComponentId> ComponentIds{
		SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID,
		SpatialConstants::SNAPSHOT_VERSION_COMPONENT_ID,
	};
	GSMQuery.snapshot_result_type_component_ids = ComponentIds.GetData();
	GSMQuery.snapshot_result_type_component_id_count = ComponentIds.Num();

	const Worker_RequestId RequestID = NetDriver->Connection->SendEntityQueryRequest(&GSMQuery, RETRY_UNTIL_COMPLETE);

	EntityQueryDelegate GSMQueryDelegate;
	GSMQueryDelegate.BindLambda([this](const Worker_EntityQueryResponseOp& Op) {
		if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
		{
			UE_LOG(LogGlobalStateManager, Warning, TEXT("Could not find GSM via entity query: %s"), UTF8_TO_TCHAR(Op.message));
		}
		else if (Op.result_count == 0)
		{
			UE_LOG(LogGlobalStateManager, Log, TEXT("GSM entity query shows the GSM does not yet exist in the world."));
		}
		else
		{
			if (ensure(Op.result_count == 1))
			{
				const Worker_Entity& GSMEntity = Op.results[0];
				if (ensure(GSMEntity.component_count == 2))
				{
					for (uint32 ComponentIndex = 0; ComponentIndex < GSMEntity.component_count; ++ComponentIndex)
					{
						const Worker_ComponentData& Component = GSMEntity.components[ComponentIndex];
						if (Component.component_id == SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID)
						{
							FDeploymentMapData ParsedData;
							if (GetFromComponentData(Component, ParsedData))
							{
								GSMData = ParsedData;
							}
						}
						else if (Component.component_id == SpatialConstants::SNAPSHOT_VERSION_COMPONENT_ID)
						{
							FSnapshotData ParsedData;
							if (GetFromComponentData(Component, ParsedData))
							{
								SnapshotData = ParsedData;
							}
						}
					}
				}
			}
		}

		if (Stage != EStage::Finished)
		{
			// Automatically retry.
			FTimerManager& TimerManager = GameInstance->GetTimerManager();
			FTimerHandle Dummy;
			TimerManager.SetTimer(
				Dummy,
				[this]() {
					QueryGSM();
				},
				0.1f, /*bInLoop =*/false);
		}
	});

	QueryHandler.AddRequest(RequestID, GSMQueryDelegate);
}

ViewCoordinator& FSpatialClientStartupHandler::GetCoordinator()
{
	return NetDriver->Connection->GetCoordinator();
}

const ViewCoordinator& FSpatialClientStartupHandler::GetCoordinator() const
{
	return NetDriver->Connection->GetCoordinator();
}
const TArray<Worker_Op>& FSpatialClientStartupHandler::GetOps() const
{
	return GetCoordinator().GetWorkerMessages();
}

bool FSpatialClientStartupHandler::GetFromComponentData(const Worker_ComponentData& Component, FDeploymentMapData& OutData)
{
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(Component.schema_type);

	int FieldsFound = 0;

	if (Schema_GetBytesCount(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_MAP_URL_ID) != 0)
	{
		++FieldsFound;
		OutData.DeploymentMapURL = GetStringFromSchema(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_MAP_URL_ID);
	}

	if (Schema_GetBoolCount(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_ACCEPTING_PLAYERS_ID) != 0)
	{
		++FieldsFound;
		OutData.bAcceptingPlayers = GetBoolFromSchema(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_ACCEPTING_PLAYERS_ID);
	}
	if (Schema_GetInt32Count(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_SESSION_ID) != 0)
	{
		++FieldsFound;
		OutData.DeploymentSessionId = Schema_GetInt32(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_SESSION_ID);
	}
	if (Schema_GetUint32Count(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_SCHEMA_HASH) != 0)
	{
		++FieldsFound;
		OutData.SchemaHash = Schema_GetUint32(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_SCHEMA_HASH);
	}
	return FieldsFound == 4;
}

bool FSpatialClientStartupHandler::GetFromComponentData(const Worker_ComponentData& Component, FSnapshotData& OutData)
{
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(Component.schema_type);

	int FieldsFound = 0;

	if (Schema_GetUint64(ComponentObject, SpatialConstants::SNAPSHOT_VERSION_NUMBER_ID))
	{
		++FieldsFound;
		OutData.SnapshotVersion = Schema_GetUint64(ComponentObject, SpatialConstants::SNAPSHOT_VERSION_NUMBER_ID);
	}

	return FieldsFound == 1;
}
} // namespace SpatialGDK
