#include "Interop/Startup/SpatialClientWorkerStartupHandler.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialPlayerSpawner.h"
#include "Schema/ServerWorker.h"
#include "SpatialView/EntityComponentTypes.h"
#include "Utils/EntityFactory.h"
#include "Utils/InterestFactory.h"

DEFINE_LOG_CATEGORY_STATIC(LogSpatialClientWorkerStartupHandler, Log, All);

namespace SpatialGDK
{
FSpatialClientStartupHandler::FSpatialClientStartupHandler(USpatialNetDriver& InNetDriver, UGameInstance& InGameInstance,
														   const FInitialSetup& InSetup)
	: Setup(InSetup)
	, NetDriver(&InNetDriver)
	, GameInstance(&InGameInstance)
{
}

FSpatialClientStartupHandler::~FSpatialClientStartupHandler()
{
	if (GSMQueryRetryTimer.IsValid())
	{
		if (GameInstance.IsValid())
		{
			GameInstance->GetTimerManager().ClearTimer(GSMQueryRetryTimer);
		}
	}
}

bool FSpatialClientStartupHandler::TryFinishStartup()
{
	if (!bQueriedGSM)
	{
		bQueriedGSM = true;
		QueryGSM();
	}

	TOptional<USpatialNetDriver::FPendingNetworkFailure> PendingNetworkFailure;
	StartupClientExtraState.Reset();

	QueryHandler.ProcessOps(GetOps());

	if (Stage == EStage::QueryGSM)
	{
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
				UE_LOG(LogSpatialClientWorkerStartupHandler, VeryVerbose,
					   TEXT("GlobalStateManager session id mismatch - got (%d) expected (%d)."), GSMData->DeploymentSessionId,
					   LocalSessionId);

				StartupClientExtraState = FString::Printf(TEXT("GlobalStateManager session id mismatch - got (%d) expected (%d)."),
														  GSMData->DeploymentSessionId, LocalSessionId);
			}
			else if (SpatialConstants::SPATIAL_SNAPSHOT_VERSION != ServerSnapshotVersion)
			{
				UE_LOG(LogSpatialClientWorkerStartupHandler, Error,
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
				UE_LOG(LogSpatialClientWorkerStartupHandler, Error,
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
					UE_LOG(LogSpatialClientWorkerStartupHandler, Log, TEXT("Welcomed by SpatialOS (Level: %s)"), *DeploymentMapURL);

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

	if (PendingNetworkFailure)
	{
		NetDriver->PendingNetworkFailure = PendingNetworkFailure;
	}

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

	FEntityQueryDelegate GSMQueryDelegate = [this](const Worker_EntityQueryResponseOp& Op) {
		if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
		{
			UE_LOG(LogSpatialClientWorkerStartupHandler, Warning, TEXT("Could not find GSM via entity query: %s"),
				   UTF8_TO_TCHAR(Op.message));
		}
		else if (Op.result_count == 0)
		{
			UE_LOG(LogSpatialClientWorkerStartupHandler, Log, TEXT("GSM entity query shows the GSM does not yet exist in the world."));
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
			if (!bStartedRetrying)
			{
				bStartedRetrying = true;
				// Automatically retry.
				FTimerManager& TimerManager = GameInstance->GetTimerManager();
				TimerManager.SetTimer(
					GSMQueryRetryTimer,
					[this]() {
						QueryGSM();
					},
					0.1f, /*bInLoop =*/true);
			}
		}
	};

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

FString FSpatialClientStartupHandler::GetStartupStateDescription() const
{
	switch (Stage)
	{
	case EStage::QueryGSM:
		return FString::Printf(TEXT("Querying GSM entity for the initial deployment data: %s"),
							   *StartupClientExtraState.Get(/*DefaultValue =*/TEXT("Nominal")));
	case EStage::WaitForMapLoad:
		return TEXT("Waiting for the map to be loaded");
	case EStage::SendPlayerSpawnRequest:
		return TEXT("Sending player spawn request");
	case EStage::Finished:
		return TEXT("Finished");
	}
	return TEXT("Invalid state");
}

} // namespace SpatialGDK
