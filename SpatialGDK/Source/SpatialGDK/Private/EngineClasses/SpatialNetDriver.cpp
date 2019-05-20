// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialNetDriver.h"

#include "EngineGlobals.h"
#include "Engine/ActorChannel.h"
#include "Engine/ChildConnection.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Engine/NetworkObjectList.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameNetworkManager.h"
#include "Net/DataReplication.h"
#include "Net/RepLayout.h"
#include "SocketSubsystem.h"
#include "UObject/UObjectIterator.h"

#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/GlobalStateManager.h"
#include "Interop/SnapshotManager.h"
#include "Interop/SpatialClassInfoManager.h"
#include "Interop/SpatialPlayerSpawner.h"
#include "Interop/SpatialReceiver.h"
#include "Interop/SpatialSender.h"
#include "Interop/SpatialDispatcher.h"
#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialGameInstance.h"
#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "EngineClasses/SpatialPendingNetGame.h"
#include "SpatialConstants.h"
#include "SpatialGDKSettings.h"
#include "Utils/EngineVersionCheck.h"
#include "Utils/EntityPool.h"
#include "Utils/SpatialMetrics.h"

DEFINE_LOG_CATEGORY(LogSpatialOSNetDriver);

DECLARE_CYCLE_STAT(TEXT("ServerReplicateActors"), STAT_SpatialServerReplicateActors, STATGROUP_SpatialNet);
DEFINE_STAT(STAT_SpatialConsiderList);

bool USpatialNetDriver::InitBase(bool bInitAsClient, FNetworkNotify* InNotify, const FURL& URL, bool bReuseAddressAndPort, FString& Error)
{
	if (!Super::InitBase(bInitAsClient, InNotify, URL, bReuseAddressAndPort, Error))
	{
		return false;
	}

	// This is a temporary measure until we can look into replication graph support, required due to UNR-832
	checkf(!GetReplicationDriver(), TEXT("Replication Driver not supported, please remove it from config"));

	bConnectAsClient = bInitAsClient;
	bAuthoritativeDestruction = true;

	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &USpatialNetDriver::OnMapLoaded);

	FWorldDelegates::LevelAddedToWorld.AddUObject(this, &USpatialNetDriver::OnLevelAddedToWorld);

	// Make absolutely sure that the actor channel that we are using is our Spatial actor channel
	ChannelClasses[CHTYPE_Actor] = USpatialActorChannel::StaticClass();

	// Extract the snapshot to load (if any) from the map URL so that once we are connected to a deployment we can load that snapshot into the Spatial deployment.
	SnapshotToLoad = URL.GetOption(*SpatialConstants::SnapshotURLOption, TEXT(""));

	// We do this here straight away to trigger LoadMap.
	if (bInitAsClient)
	{
		// If the URL has not specified to keep the clients connection then we should create a new one.
		bPersistSpatialConnection = URL.HasOption(*SpatialConstants::ClientsStayConnectedURLOption);
	}
	else
	{
		// Servers should never disconnect from a deployment.
		bPersistSpatialConnection = true;
	}

	// Initialize ClassInfoManager here because it needs to load SchemaDatabase.
	// We shouldn't do that in CreateAndInitializeCoreClasses because it is called
	// from OnConnectedToSpatialOS callback which could be executed with the async
	// loading thread suspended (e.g. when resuming rendering thread), in which
	// case we'll crash upon trying to load SchemaDatabase.
	ClassInfoManager = NewObject<USpatialClassInfoManager>();
	ClassInfoManager->Init(this);

	InitiateConnectionToSpatialOS(URL);

	return true;
}

void USpatialNetDriver::PostInitProperties()
{
	Super::PostInitProperties();

	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		// GuidCache will be allocated as an FNetGUIDCache above. To avoid an engine code change, we re-do it with the Spatial equivalent.
		GuidCache = MakeShareable(new FSpatialNetGUIDCache(this));
	}
}

void USpatialNetDriver::InitiateConnectionToSpatialOS(const FURL& URL)
{
	USpatialGameInstance* GameInstance = nullptr;

	// A client does not have a world at this point, so we use the WorldContext
	// to get a reference to the GameInstance
	if (bConnectAsClient)
	{
		const FWorldContext& WorldContext = GEngine->GetWorldContextFromPendingNetGameNetDriverChecked(this);
		GameInstance = Cast<USpatialGameInstance>(WorldContext.OwningGameInstance);
	}
	else
	{
		GameInstance = Cast<USpatialGameInstance>(GetWorld()->GetGameInstance());
	}

	if (GameInstance == nullptr)
	{
		UE_LOG(LogSpatialOSNetDriver, Error, TEXT("A SpatialGameInstance is required. Make sure your game's GameInstance inherits from SpatialGameInstance"));
		return;
	}

	if (!bPersistSpatialConnection)
	{
		// Destroy the old connection
		GameInstance->GetSpatialWorkerConnection()->DestroyConnection();

		// Create a new SpatialWorkerConnection in the SpatialGameInstance.
		GameInstance->CreateNewSpatialWorkerConnection();
	}

	Connection = GameInstance->GetSpatialWorkerConnection();

	if (URL.HasOption(TEXT("locator")))
	{
		// Obtain PIT and LT.
		Connection->LocatorConfig.PlayerIdentityToken = URL.GetOption(TEXT("playeridentity="), TEXT(""));
		Connection->LocatorConfig.LoginToken = URL.GetOption(TEXT("login="), TEXT(""));
		Connection->LocatorConfig.UseExternalIp = true;
		Connection->LocatorConfig.WorkerType = GameInstance->GetSpatialWorkerType();
	}
	else // Using Receptionist
	{
		Connection->ReceptionistConfig.WorkerType = GameInstance->GetSpatialWorkerType();

		// Check for overrides in the travel URL.
		if (!URL.Host.IsEmpty())
		{
			Connection->ReceptionistConfig.ReceptionistHost = URL.Host;
		}

		bool bHasUseExternalIpOption = URL.HasOption(TEXT("useExternalIpForBridge"));

		if (bHasUseExternalIpOption)
		{
			FString UseExternalIpOption = URL.GetOption(TEXT("useExternalIpForBridge"), TEXT(""));
			if (UseExternalIpOption.Equals(TEXT("false"), ESearchCase::IgnoreCase))
			{
				Connection->ReceptionistConfig.UseExternalIp = false;
			}
			else
			{
				Connection->ReceptionistConfig.UseExternalIp = true;
			}
		}
	}

	Connection->Connect(bConnectAsClient);
}

void USpatialNetDriver::OnConnectedToSpatialOS()
{
	// If we're the server, we will spawn the special Spatial connection that will route all updates to SpatialOS.
	// There may be more than one of these connections in the future for different replication conditions.
	if (IsServer())
	{
		CreateServerSpatialOSNetConnection();
	}

	CreateAndInitializeCoreClasses();

	// Query the GSM to figure out what map to load
	if (!IsServer())
	{
		QueryGSMToLoadMap();
	}

	if (IsServer())
	{
		HandleOngoingServerTravel();
	}
}

void USpatialNetDriver::InitializeSpatialOutputDevice()
{
	int32 PIEIndex = -1; // -1 is Unreal's default index when not using PIE
#if WITH_EDITOR
	if (IsServer())
	{
		PIEIndex = GEngine->GetWorldContextFromWorldChecked(GetWorld()).PIEInstance;
	}
	else
	{
		PIEIndex = GEngine->GetWorldContextFromPendingNetGameNetDriverChecked(this).PIEInstance;
	}
#endif //WITH_EDITOR
	SpatialOutputDevice = MakeUnique<FSpatialOutputDevice>(Connection, TEXT("Unreal"), PIEIndex);
}

void USpatialNetDriver::CreateAndInitializeCoreClasses()
{
	InitializeSpatialOutputDevice();

	Dispatcher = NewObject<USpatialDispatcher>();
	Sender = NewObject<USpatialSender>();
	Receiver = NewObject<USpatialReceiver>();
	GlobalStateManager = NewObject<UGlobalStateManager>();
	PlayerSpawner = NewObject<USpatialPlayerSpawner>();
	StaticComponentView = NewObject<USpatialStaticComponentView>();
	SnapshotManager = NewObject<USnapshotManager>();
	SpatialMetrics = NewObject<USpatialMetrics>();

	PackageMap = Cast<USpatialPackageMapClient>(GetSpatialOSNetConnection()->PackageMap);
	PackageMap->Init(this);
	Dispatcher->Init(this);
	Sender->Init(this);
	Receiver->Init(this, &TimerManager);
	GlobalStateManager->Init(this, &TimerManager);
	SnapshotManager->Init(this);
	PlayerSpawner->Init(this, &TimerManager);
	SpatialMetrics->Init(this);

	// Entity Pools should never exist on clients
	if (IsServer())
	{
		EntityPool->Init(this, &TimerManager);

		// Create a entity representing the server worker.
		// This ensures that even if a server-worker isn't authoritative over anything, it will be able to
		// receive updates for the GSM.
		const FString WorkerId = FString(TEXT("workerId:")) + Connection->GetWorkerId();
		const WorkerRequirementSet WorkerIdPermission{ { WorkerId } };

		WriteAclMap ComponentWriteAcl;
		ComponentWriteAcl.Add(SpatialConstants::POSITION_COMPONENT_ID, WorkerIdPermission);
		ComponentWriteAcl.Add(SpatialConstants::METADATA_COMPONENT_ID, WorkerIdPermission);
		ComponentWriteAcl.Add(SpatialConstants::ENTITY_ACL_COMPONENT_ID, WorkerIdPermission);

		TArray<Worker_ComponentData> Components;
		Components.Add(improbable::Position().CreatePositionData());
		Components.Add(improbable::Metadata(WorkerId).CreateMetadataData());
		Components.Add(improbable::EntityAcl(WorkerIdPermission, ComponentWriteAcl).CreateEntityAclData());

		Connection->SendCreateEntityRequest(MoveTemp(Components), nullptr);
	}
}

void USpatialNetDriver::CreateServerSpatialOSNetConnection()
{
	check(!bConnectAsClient);

	EntityPool = NewObject<UEntityPool>();

	USpatialNetConnection* NetConnection = NewObject<USpatialNetConnection>(GetTransientPackage(), NetConnectionClass);
	check(NetConnection);

	ISocketSubsystem* SocketSubsystem = GetSocketSubsystem();
	TSharedRef<FInternetAddr> FromAddr = SocketSubsystem->CreateInternetAddr();

	// Each connection stores a URL with various optional settings (host, port, map, netspeed...)
	// We currently don't make use of any of these as some are meaningless in a SpatialOS world, and some are less of a priority.
	// So for now we just give the connection a dummy url, might change in the future.
	FURL DummyURL;

	NetConnection->InitRemoteConnection(this, nullptr, DummyURL, *FromAddr, USOCK_Open);
	Notify->NotifyAcceptedConnection(NetConnection);
	NetConnection->bReliableSpatialConnection = true;
	AddClientConnection(NetConnection);
	//Since this is not a "real" client connection, we immediately pretend that it is fully logged on.
	NetConnection->SetClientLoginState(EClientLoginState::Welcomed);

	// Bind the ProcessServerTravel delegate to the spatial variant. This ensures that if ServerTravel is called and Spatial networking is enabled, we can travel properly.
	GetWorld()->SpatialProcessServerTravelDelegate.BindStatic(SpatialProcessServerTravel);
}

void USpatialNetDriver::QueryGSMToLoadMap()
{
	check(bConnectAsClient);

	// Register our interest in spawning.
	bWaitingForAcceptingPlayersToSpawn = true;

	// Begin querying the state of the GSM so we know the state of AcceptingPlayers.
	GlobalStateManager->QueryGSM(true /*bRetryUntilAcceptingPlayers*/);
}

void USpatialNetDriver::HandleOngoingServerTravel()
{
	check(!bConnectAsClient);

	// Here if we are a server and this is server travel (there is a snapshot to load) we want to load the snapshot.
	if (!ServerConnection && !SnapshotToLoad.IsEmpty() && Cast<USpatialGameInstance>(GetWorld()->GetGameInstance())->bResponsibleForSnapshotLoading)
	{
		UE_LOG(LogSpatialOSNetDriver, Log, TEXT("Worker authoriative over the GSM is loading snapshot: %s"), *SnapshotToLoad);
		SnapshotManager->LoadSnapshot(SnapshotToLoad);

		// Once we've finished loading the snapshot we must update our bResponsibleForSnapshotLoading in-case we do not gain authority over the new GSM.
		Cast<USpatialGameInstance>(GetWorld()->GetGameInstance())->bResponsibleForSnapshotLoading = false;
	}
}

void USpatialNetDriver::OnMapLoaded(UWorld* LoadedWorld)
{
	if (LoadedWorld == nullptr)
	{
		return;
	}

	if (LoadedWorld->GetNetDriver() != this)
	{
		// In PIE, if we have more than 2 clients, then OnMapLoaded is going to be triggered once each client loads the world.
		// As the delegate is a global variable, it triggers all 3 USpatialNetDriver::OnMapLoaded callbacks. As a result, we should
		// make sure that the net driver of this world is in fact us.
		return;
	}

	// If we're the client, we can now ask the server to spawn our controller.
	if (!IsServer())
	{
		// If we know the GSM is already accepting players, simply spawn.
		if (GlobalStateManager->bAcceptingPlayers && GetWorld()->RemovePIEPrefix(GlobalStateManager->DeploymentMapURL) == GetWorld()->RemovePIEPrefix(GetWorld()->URL.Map))
		{
			PlayerSpawner->SendPlayerSpawnRequest();
			bWaitingForAcceptingPlayersToSpawn = false;
		}
		else
		{
			checkNoEntry();
		}
	}
}

void USpatialNetDriver::OnLevelAddedToWorld(ULevel* LoadedLevel, UWorld* OwningWorld)
{
	// Callback got called on a World that's not associated with this NetDriver.
	// Don't do anything.
	if (OwningWorld != World)
	{
		return;
	}

	// If we have authority over the GSM when loading a sublevel, make sure we have authority
	// over the actors in the sublevel.
	if (GlobalStateManager != nullptr)
	{
		if (GlobalStateManager->HasAuthority())
		{
			for (auto Actor : LoadedLevel->Actors)
			{
				if (Actor->GetIsReplicated())
				{
					Actor->Role = ROLE_Authority;
					Actor->RemoteRole = ROLE_SimulatedProxy;
				}
			}
		}
	}
}

void USpatialNetDriver::OnAcceptingPlayersChanged(bool bAcceptingPlayers)
{
	// If the deployment is now accepting players and we are waiting to spawn. Spawn.
	if (bWaitingForAcceptingPlayersToSpawn && bAcceptingPlayers)
	{
		// If we have the correct map loaded then ask to spawn.
		if (GetWorld() != nullptr && GetWorld()->RemovePIEPrefix(GlobalStateManager->DeploymentMapURL) == GetWorld()->RemovePIEPrefix(GetWorld()->URL.Map))
		{
			PlayerSpawner->SendPlayerSpawnRequest();

			// Unregister our interest in spawning on accepting players changing again.
			bWaitingForAcceptingPlayersToSpawn = false;
		}
		else
		{
			// Load the correct map based on the GSM URL
			UE_LOG(LogSpatialOSNetDriver, Log, TEXT("Welcomed by SpatialOS (Level: %s)"), *GlobalStateManager->DeploymentMapURL);

			// Extract map name and options
			FWorldContext& WorldContext = GEngine->GetWorldContextFromPendingNetGameNetDriverChecked(this);

			FURL RedirectURL = FURL(&WorldContext.LastURL, *GlobalStateManager->DeploymentMapURL, (ETravelType)WorldContext.TravelType);
			RedirectURL.Host = WorldContext.LastURL.Host;
			RedirectURL.Port = WorldContext.LastURL.Port;
			RedirectURL.Op.Append(WorldContext.LastURL.Op);
			RedirectURL.AddOption(*SpatialConstants::ClientsStayConnectedURLOption);

			WorldContext.PendingNetGame->bSuccessfullyConnected = true;
			WorldContext.PendingNetGame->bSentJoinRequest = false;
			WorldContext.PendingNetGame->URL = RedirectURL;
		}
	}
}

// NOTE: This method is a clone of the ProcessServerTravel located in GameModeBase with modifications to support Spatial.
// Will be called via a delegate that has been set in the UWorld instead of the original in GameModeBase.
void USpatialNetDriver::SpatialProcessServerTravel(const FString& URL, bool bAbsolute, AGameModeBase* GameMode)
{
#if WITH_SERVER_CODE

	UWorld* World = GameMode->GetWorld();
	USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(World->GetNetDriver());

	if (!NetDriver->StaticComponentView->HasAuthority(NetDriver->GlobalStateManager->GlobalStateManagerEntityId, SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID))
	{
		// TODO: UNR-678 Send a command to the GSM to initiate server travel on the correct server.
		UE_LOG(LogGameMode, Warning, TEXT("Trying to server travel on a server which is not authoritative over the GSM."));
		return;
	}

	// Register that this server will be responsible for loading the snapshot once it has finished wiping the world + loading the new map.
	Cast<USpatialGameInstance>(World->GetGameInstance())->bResponsibleForSnapshotLoading = true;

	GameMode->StartToLeaveMap();

	// Force an old style load screen if the server has been up for a long time so that TimeSeconds doesn't overflow and break everything
	bool bSeamless = (GameMode->bUseSeamlessTravel && World->TimeSeconds < 172800.0f); // 172800 seconds == 48 hours

	FString NextMap;
	if (URL.ToUpper().Contains(TEXT("?RESTART")))
	{
		NextMap = UWorld::RemovePIEPrefix(GameMode->GetOutermost()->GetName());
	}
	else
	{
		int32 OptionStart = URL.Find(TEXT("?"));
		if (OptionStart == INDEX_NONE)
		{
			NextMap = URL;
		}
		else
		{
			NextMap = URL.Left(OptionStart);
		}
	}

	FGuid NextMapGuid = UEngine::GetPackageGuid(FName(*NextMap), World->IsPlayInEditor());

	FString NewURL = URL;

	bool SnapshotOption = NewURL.Contains(TEXT("snapshot="));
	if (!SnapshotOption)
	{
		// In the case that we don't have a snapshot option, we assume the map name will be the snapshot name.
		// Remove any leading path before the map name.
		FString Path;
		FString MapName;
		NextMap.Split(TEXT("/"), &Path, &MapName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		NewURL.Append(FString::Printf(TEXT("?snapshot=%s"), *MapName));
	}

	// Notify clients we're switching level and give them time to receive.
	FString URLMod = NewURL;
	APlayerController* LocalPlayer = GameMode->ProcessClientTravel(URLMod, NextMapGuid, bSeamless, bAbsolute);

	// We can't have the NextURL set this early when using SpatialProcessServerTravel so empty the string here.
	// The reason for this is, on the next WorldTick the current World and NetDriver will be unloaded.
	// During the deployment wipe we are waiting for an entity query response of all entities in the deployment.
	// If the NetDriver has been unloaded in that time, the delegate to delete all these entities will be lost and server travel will fail.
	World->NextURL.Empty();

	ENetMode NetMode = GameMode->GetNetMode();

	// FinishServerTravel - Allows Unreal to finish it's normal server travel.
	USpatialNetDriver::PostWorldWipeDelegate FinishServerTravel;
	FinishServerTravel.BindLambda([World, NetDriver, NewURL, NetMode, bSeamless, bAbsolute]
	{
		UE_LOG(LogGameMode, Log, TEXT("SpatialServerTravel - Finishing Server Travel : %s"), *NewURL);
		check(World);
		World->NextURL = NewURL;

		if (bSeamless)
		{
			World->SeamlessTravel(World->NextURL, bAbsolute);
			World->NextURL = TEXT("");
		}
		// Switch immediately if not networking.
		else if (NetMode != NM_DedicatedServer && NetMode != NM_ListenServer)
		{
			World->NextSwitchCountdown = 0.0f;
		}
	});

	UE_LOG(LogGameMode, Log, TEXT("SpatialServerTravel - Wiping the world"), *NewURL);
	NetDriver->WipeWorld(FinishServerTravel);
#endif // WITH_SERVER_CODE
}

bool USpatialNetDriver::IsLevelInitializedForActor(const AActor* InActor, const UNetConnection* InConnection) const
{
	//In our case, the connection is not specific to a client. Thus, it's not relevant whether the level is initialized.
	return true;
}

void USpatialNetDriver::NotifyActorDestroyed(AActor* ThisActor, bool IsSeamlessTravel /*= false*/)
{
	// Intentionally does not call Super::NotifyActorDestroyed, but most of the functionality is copied here
	// The native UNetDriver would normally store destruction info here for "StartupActors" - replicated actors
	// placed in the level, but we handle this flow differently in the GDK

	// Remove the actor from the property tracker map
	RepChangedPropertyTrackerMap.Remove(ThisActor);

	const bool bIsServer = ServerConnection == nullptr;

	if (bIsServer)
	{
		for (int32 i = ClientConnections.Num() - 1; i >= 0; i--)
		{
			UNetConnection* ClientConnection = ClientConnections[i];
			if (ThisActor->bNetTemporary)
			{
				ClientConnection->SentTemporaries.Remove(ThisActor);
			}

			if (UActorChannel* Channel = ClientConnection->ActorChannelMap().FindRef(ThisActor))
			{
				check(Channel->OpenedLocally);
				Channel->bClearRecentActorRefs = false;
				// TODO: UNR-952 - Add code here for cleaning up actor channels from our maps.
				Channel->Close();
			}

			// Remove it from any dormancy lists
			ClientConnection->DormantReplicatorMap.Remove(ThisActor);
		}
	}

	// Remove this actor from the network object list
	GetNetworkObjectList().Remove(ThisActor);

	// Remove from renamed list if destroyed
	RenamedStartupActors.Remove(ThisActor->GetFName());
}

const FString USpatialNetDriver::GetUniqueIdentifier() const
{
	if (Connection != nullptr)
	{
		return Connection->GetWorkerId();
	}

	return Super::GetUniqueIdentifier();
}

void USpatialNetDriver::OnOwnerUpdated(AActor* Actor)
{
	// If PackageMap doesn't exist, we haven't connected yet, which means
	// we don't need to update the interest at this point
	if (PackageMap == nullptr)
	{
		return;
	}

	Worker_EntityId EntityId = PackageMap->GetEntityIdFromObject(Actor);
	if (EntityId == SpatialConstants::INVALID_ENTITY_ID)
	{
		return;
	}

	USpatialActorChannel* Channel = GetActorChannelByEntityId(EntityId);
	if (Channel == nullptr)
	{
		return;
	}

	Channel->MarkInterestDirty();
}

//SpatialGDK: Functions in the ifdef block below are modified versions of the UNetDriver:: implementations.
#if WITH_SERVER_CODE

// Returns true if this actor should replicate to *any* of the passed in connections
static FORCEINLINE_DEBUGGABLE bool IsActorRelevantToConnection(const AActor* Actor, const TArray<FNetViewer>& ConnectionViewers)
{
	// SpatialGDK: Currently we're just returning true as a worker replicates all the known actors in our design.
	// We might make some exceptions in the future, so keeping this function.
	// TODO: UNR-837 Start using IsNetRelevantFor again for relevancy checks rather than returning true.
	return true;
}

// Returns true if this actor is considered dormant (and all properties caught up) to the current connection
static FORCEINLINE_DEBUGGABLE bool IsActorDormant(FNetworkObjectInfo* ActorInfo, UNetConnection* Connection)
{
	// If actor is already dormant on this channel, then skip replication entirely
	return ActorInfo->DormantConnections.Contains(Connection);
}

// Returns true if this actor wants to go dormant for a particular connection
static FORCEINLINE_DEBUGGABLE bool ShouldActorGoDormant(AActor* Actor, const TArray<FNetViewer>& ConnectionViewers, UActorChannel* Channel, const float Time, const bool bLowNetBandwidth)
{
	if (Actor->NetDormancy <= DORM_Awake || !Channel || Channel->bPendingDormancy || Channel->Dormant)
	{
		// Either shouldn't go dormant, or is already dormant
		return false;
	}

	if (Actor->NetDormancy == DORM_DormantPartial)
	{
		for (int32 viewerIdx = 0; viewerIdx < ConnectionViewers.Num(); viewerIdx++)
		{
			if (!Actor->GetNetDormancy(ConnectionViewers[viewerIdx].ViewLocation, ConnectionViewers[viewerIdx].ViewDir, ConnectionViewers[viewerIdx].InViewer, ConnectionViewers[viewerIdx].ViewTarget, Channel, Time, bLowNetBandwidth))
			{
				return false;
			}
		}
	}

	return true;
}

int32 USpatialNetDriver::ServerReplicateActors_PrepConnections(const float DeltaSeconds)
{
	int32 NumClientsToTick = ClientConnections.Num();

	bool bFoundReadyConnection = false;

	for (int32 ConnIdx = 0; ConnIdx < ClientConnections.Num(); ConnIdx++)
	{
		USpatialNetConnection* SpatialConnection = Cast<USpatialNetConnection>(ClientConnections[ConnIdx]);
		check(SpatialConnection);
		check(SpatialConnection->State == USOCK_Pending || SpatialConnection->State == USOCK_Open || SpatialConnection->State == USOCK_Closed);
		checkSlow(SpatialConnection->GetUChildConnection() == NULL);

		// Handle not ready channels.
		//@note: we cannot add Connection->IsNetReady(0) here to check for saturation, as if that's the case we still want to figure out the list of relevant actors
		//			to reset their NetUpdateTime so that they will get sent as soon as the connection is no longer saturated
		AActor* OwningActor = SpatialConnection->OwningActor;

		//SpatialGDK: We allow a connection without an owner to process if it's meant to be the connection to the fake SpatialOS client.
		if ((SpatialConnection->bReliableSpatialConnection || OwningActor != NULL) && SpatialConnection->State == USOCK_Open && (SpatialConnection->Driver->Time - SpatialConnection->LastReceiveTime < 1.5f))
		{
			check(SpatialConnection->bReliableSpatialConnection || World == OwningActor->GetWorld());

			bFoundReadyConnection = true;

			// the view target is what the player controller is looking at OR the owning actor itself when using beacons
			SpatialConnection->ViewTarget = SpatialConnection->PlayerController ? SpatialConnection->PlayerController->GetViewTarget() : OwningActor;
		}
		else
		{
			SpatialConnection->ViewTarget = NULL;
		}

		if (SpatialConnection->Children.Num() > 0)
		{
			UE_LOG(LogSpatialOSNetDriver, Error, TEXT("Child connections present on Spatial connection %s! We don't support splitscreen yet, so this will not function correctly."), *SpatialConnection->GetName());
		}
	}

	return bFoundReadyConnection ? NumClientsToTick : 0;
}

int32 USpatialNetDriver::ServerReplicateActors_PrioritizeActors(UNetConnection* InConnection, const TArray<FNetViewer>& ConnectionViewers, const TArray<FNetworkObjectInfo*> ConsiderList, const bool bCPUSaturated, FActorPriority*& OutPriorityList, FActorPriority**& OutPriorityActors)
{
	// Get list of visible/relevant actors.

	NetTag++;
	InConnection->TickCount++;

	// Set up to skip all sent temporary actors
	for (int32 j = 0; j < InConnection->SentTemporaries.Num(); j++)
	{
		InConnection->SentTemporaries[j]->NetTag = NetTag;
	}

	int32 FinalSortedCount = 0;
	int32 DeletedCount = 0;

	const int32 MaxSortedActors = ConsiderList.Num() + DestroyedStartupOrDormantActors.Num();
	if (MaxSortedActors > 0)
	{
		OutPriorityList = new (FMemStack::Get(), MaxSortedActors) FActorPriority;
		OutPriorityActors = new (FMemStack::Get(), MaxSortedActors) FActorPriority*;

		AGameNetworkManager* const NetworkManager = World->NetworkManager;
		const bool bLowNetBandwidth = NetworkManager ? NetworkManager->IsInLowBandwidthMode() : false;

		for (FNetworkObjectInfo* ActorInfo : ConsiderList)
		{
			AActor* Actor = ActorInfo->Actor;

			UActorChannel* Channel = InConnection->ActorChannelMap().FindRef(Actor);

			UNetConnection* PriorityConnection = InConnection;

			// Skip Actor if dormant
			if (IsActorDormant(ActorInfo, InConnection))
			{
				continue;
			}

			// See of actor wants to try and go dormant
			if (ShouldActorGoDormant(Actor, ConnectionViewers, Channel, Time, bLowNetBandwidth))
			{
				// Channel is marked to go dormant now once all properties have been replicated (but is not dormant yet)
				Channel->StartBecomingDormant();
			}

			UE_LOG(LogSpatialOSNetDriver, Verbose, TEXT("Actor %s will be replicated on the catch-all connection"), *Actor->GetName());

			// SpatialGDK: Here, Unreal does initial relevancy checking and level load checking.
			// We have removed the level load check because it doesn't apply.
			// Relevancy checking is also mostly just a pass through, might be removed later.
			if (!IsActorRelevantToConnection(Actor, ConnectionViewers))
			{
				// If not relevant (and we don't have a channel), skip
				continue;
			}

			// Actor is relevant to this connection, add it to the list
			// NOTE - We use NetTag to make sure SentTemporaries didn't already mark this actor to be skipped
			if (Actor->NetTag != NetTag)
			{
				UE_LOG(LogNetTraffic, Log, TEXT("Consider %s alwaysrelevant %d frequency %f "), *Actor->GetName(), Actor->bAlwaysRelevant, Actor->NetUpdateFrequency);

				Actor->NetTag = NetTag;

				OutPriorityList[FinalSortedCount] = FActorPriority(PriorityConnection, Channel, ActorInfo, ConnectionViewers, bLowNetBandwidth);
				OutPriorityActors[FinalSortedCount] = OutPriorityList + FinalSortedCount;

				FinalSortedCount++;

				if (DebugRelevantActors)
				{
					LastPrioritizedActors.Add(Actor);
				}
			}
		}

		// Add in deleted actors
		for (auto It = InConnection->GetDestroyedStartupOrDormantActorGUIDs().CreateIterator(); It; ++It)
		{
			FActorDestructionInfo& DInfo = *DestroyedStartupOrDormantActors.FindChecked(*It);
			OutPriorityList[FinalSortedCount] = FActorPriority(InConnection, &DInfo, ConnectionViewers);
			OutPriorityActors[FinalSortedCount] = OutPriorityList + FinalSortedCount;
			FinalSortedCount++;
			DeletedCount++;
		}

		// Sort by priority
		Sort(OutPriorityActors, FinalSortedCount, FCompareFActorPriority());
	}

	UE_LOG(LogNetTraffic, Log, TEXT("ServerReplicateActors_PrioritizeActors: Potential %04i ConsiderList %03i FinalSortedCount %03i"), MaxSortedActors, ConsiderList.Num(), FinalSortedCount);

	return FinalSortedCount;
}

void USpatialNetDriver::ServerReplicateActors_ProcessPrioritizedActors(UNetConnection* InConnection, const TArray<FNetViewer>& ConnectionViewers, FActorPriority** PriorityActors, const int32 FinalSortedCount, int32& OutUpdated)
{
	// SpatialGDK - Here Unreal would check if the InConnection was saturated (!IsNetReady) and early out. Removed this as we do not currently use channel saturation.

	int32 ActorUpdatesThisConnection = 0;
	int32 ActorUpdatesThisConnectionSent = 0;

	// SpatialGDK - Entity creation rate limiting based on config value.
	uint32 EntityCreationRateLimit = GetDefault<USpatialGDKSettings>()->EntityCreationRateLimit;
	int32 MaxEntitiesToCreate = (EntityCreationRateLimit > 0) ? EntityCreationRateLimit : INT32_MAX;
	int32 FinalCreationCount = 0;

	// SpatialGDK - Actor replication rate limiting based on config value.
	uint32 ActorReplicationRateLimit = GetDefault<USpatialGDKSettings>()->ActorReplicationRateLimit;
	int32 MaxActorsToReplicate = (ActorReplicationRateLimit > 0) ? ActorReplicationRateLimit : INT32_MAX;
	int32 FinalReplicatedCount = 0;

	for (int32 j = 0; j < FinalSortedCount; j++)
	{
		// Deletion entry
		if (PriorityActors[j]->ActorInfo == NULL && PriorityActors[j]->DestructionInfo)
		{
			// Make sure client has streaming level loaded
			if (PriorityActors[j]->DestructionInfo->StreamingLevelName != NAME_None && !InConnection->ClientVisibleLevelNames.Contains(PriorityActors[j]->DestructionInfo->StreamingLevelName))
			{
				// This deletion entry is for an actor in a streaming level the connection doesn't have loaded, so skip it
				continue;
			}

			UActorChannel* Channel = (UActorChannel*)InConnection->CreateChannel(CHTYPE_Actor, 1);
			if (Channel)
			{
				UE_LOG(LogNetTraffic, Log, TEXT("Server replicate actor creating destroy channel for NetGUID <%s,%s> Priority: %d"), *PriorityActors[j]->DestructionInfo->NetGUID.ToString(), *PriorityActors[j]->DestructionInfo->PathName, PriorityActors[j]->Priority);

				InConnection->GetDestroyedStartupOrDormantActorGUIDs().Remove(PriorityActors[j]->DestructionInfo->NetGUID); // Remove from connections to-be-destroyed list (close bunch of reliable, so it will make it there)
			}
			continue;
		}

#if !( UE_BUILD_SHIPPING || UE_BUILD_TEST )
		static IConsoleVariable* DebugObjectCvar = IConsoleManager::Get().FindConsoleVariable(TEXT("net.PackageMap.DebugObject"));
		static IConsoleVariable* DebugAllObjectsCvar = IConsoleManager::Get().FindConsoleVariable(TEXT("net.PackageMap.DebugAll"));
		if (PriorityActors[j]->ActorInfo &&
			((DebugObjectCvar && !DebugObjectCvar->GetString().IsEmpty() && PriorityActors[j]->ActorInfo->Actor->GetName().Contains(DebugObjectCvar->GetString())) ||
			(DebugAllObjectsCvar && DebugAllObjectsCvar->GetInt() != 0)))
		{
			UE_LOG(LogNetPackageMap, Log, TEXT("Evaluating actor for replication %s"), *PriorityActors[j]->ActorInfo->Actor->GetName());
		}
#endif

		// Normal actor replication
		USpatialActorChannel* Channel = Cast<USpatialActorChannel>(PriorityActors[j]->Channel);
		UE_LOG(LogNetTraffic, Log, TEXT(" Maybe Replicate %s"), *PriorityActors[j]->ActorInfo->Actor->GetName());
		if (Channel == nullptr || Channel->Actor != nullptr) // Make sure didn't just close this channel.
		{
			AActor* Actor = PriorityActors[j]->ActorInfo->Actor;
			bool bIsRelevant = false;

			// SpatialGDK: Here, Unreal would check (again) whether an actor is relevant. Removed such checks.
			// only check visibility on already visible actors every 1.0 + 0.5R seconds
			// bTearOff actors should never be checked
			if (!Actor->GetTearOff() && (!Channel || Time - Channel->RelevantTime > 1.f))
			{
				if (DebugRelevantActors)
				{
					LastNonRelevantActors.Add(Actor);
				}
			}

			// SpatialGDK - Creation of new entities should always be handled and therefore is checked prior to actor throttling.
			// There is an EntityCreationRateLimit to prevent overloading Spatial with creation requests if the developer desires.
			// Creation of a new entity occurs when the channel is currently nullptr or if the channel does not have bCreatedEntity set to true.
			if (FinalCreationCount < MaxEntitiesToCreate && !Actor->GetTearOff() && (Channel == nullptr || Channel->bCreatingNewEntity))
			{
				bIsRelevant = true;
				FinalCreationCount++;
			}
			// SpatialGDK - We will only replicate the highest priority actors up the the rate limit and the final tick of TearOff actors.
			// Actors not replicated this frame will have their priority increased based on the time since the last replicated.
			// TearOff actors would normally replicate their final tick due to RecentlyRelevant, after which the channel is closed.
			// With throttling we no longer always replicate when RecentlyRelevant is true, thus we ensure to always replicate a TearOff actor while it still has a channel.
			else if ((FinalReplicatedCount < MaxActorsToReplicate && !Actor->GetTearOff()) || (Actor->GetTearOff() && Channel != nullptr))
			{
				bIsRelevant = true;
				FinalReplicatedCount++;
			}

			// If the actor is now relevant or was recently relevant.
			const bool bIsRecentlyRelevant = bIsRelevant || (Channel && Time - Channel->RelevantTime < RelevantTimeout);

			if (bIsRecentlyRelevant)
			{
				// Find or create the channel for this actor.
				// we can't create the channel if the client is in a different world than we are
				// or the package map doesn't support the actor's class/archetype (or the actor itself in the case of serializable actors)
				// or it's an editor placed actor and the client hasn't initialized the level it's in
				if (Channel == nullptr && GuidCache->SupportsObject(Actor->GetClass()) && GuidCache->SupportsObject(Actor->IsNetStartupActor() ? Actor : Actor->GetArchetype()))
				{
					if (Actor->GetClass()->HasAnySpatialClassFlags(SPATIALCLASS_NotSpatialType))
					{
						// Trying to replicate an actor that isn't supported by Spatial (e.g. marked NotSpatial)
						continue;
					}

					if (!Actor->HasAuthority())
					{
						// Trying to replicate Actor which we don't have authority over.
						// Remove after UNR-961
						continue;
					}

					// If we're a singleton, and don't have a channel, defer to GSM
					if (Actor->GetClass()->HasAnySpatialClassFlags(SPATIALCLASS_Singleton))
					{
						Channel = GlobalStateManager->AddSingleton(Actor);
					}
					else
					{
						// Create a new channel for this actor.
						Channel = (USpatialActorChannel*)InConnection->CreateChannel(CHTYPE_Actor, 1);
						if (Channel)
						{
							Channel->SetChannelActor(Actor);
						}
						else if (Actor->NetUpdateFrequency < 1.0f)
						{
							UE_LOG(LogNetTraffic, Log, TEXT("Unable to replicate %s"), *Actor->GetName());
							PriorityActors[j]->ActorInfo->NextUpdateTime = Actor->GetWorld()->TimeSeconds + 0.2f * FMath::FRand();
						}
					}
				}

				// SpatialGDK - Only replicate actors marked as relevant (rate limiting).
				if (Channel && bIsRelevant)
				{
					// If it is relevant then mark the channel as relevant for a short amount of time.
					Channel->RelevantTime = Time + 0.5f * FMath::SRand();

					// If the channel isn't saturated.
					if (Channel->IsNetReady(0))
					{
						// Replicate the actor.
						UE_LOG(LogNetTraffic, Log, TEXT("- Replicate %s. %d"), *Actor->GetName(), PriorityActors[j]->Priority);
						if (DebugRelevantActors)
						{
							LastRelevantActors.Add(Actor);
						}

						if (Channel->ReplicateActor())
						{
							ActorUpdatesThisConnectionSent++;
							if (DebugRelevantActors)
							{
								LastSentActors.Add(Actor);
							}

							// Calculate min delta (max rate actor will update), and max delta (slowest rate actor will update)
							const float MinOptimalDelta = 1.0f / Actor->NetUpdateFrequency;
							const float MaxOptimalDelta = FMath::Max(1.0f / Actor->MinNetUpdateFrequency, MinOptimalDelta);
							const float DeltaBetweenReplications = (World->TimeSeconds - PriorityActors[j]->ActorInfo->LastNetReplicateTime);

							// Choose an optimal time, we choose 70% of the actual rate to allow frequency to go up if needed
							PriorityActors[j]->ActorInfo->OptimalNetUpdateDelta = FMath::Clamp(DeltaBetweenReplications * 0.7f, MinOptimalDelta, MaxOptimalDelta);
							PriorityActors[j]->ActorInfo->LastNetReplicateTime = World->TimeSeconds;
						}

						ActorUpdatesThisConnection++;
						OutUpdated++;
					}

					// SpatialGDK - Here Unreal would do a second check for channel saturation and early out if needed. Removed such checks.
				}
			}

			// UNR-865 - Handle closing actor channels for non-relevant actors without deleting the entity.
			// If the actor wasn't recently relevant, or if it was torn off, close the actor channel if it exists for this connection
			if (Actor->GetTearOff() && Channel != NULL)
			{
				// Non startup (map) actors have their channels closed immediately, which destroys them.
				// Startup actors get to keep their channels open.
				if (!Actor->IsNetStartupActor())
				{
					UE_LOG(LogNetTraffic, Log, TEXT("- Closing channel for no longer relevant actor %s"), *Actor->GetName());
					// TODO: UNR-952 - Add code here for cleaning up actor channels from our maps.
					Channel->Close();
				}
			}
		}
	}

	// SpatialGDK - Here Unreal would return the position of the last replicated actor in PriorityActors before the channel became saturated.
	// In Spatial we use ActorReplicationRateLimit and EntityCreationRateLimit to limit replication so this return value is not relevant.
}
#endif

// SpatialGDK: This is a modified and simplified version of UNetDriver::ServerReplicateActors.
// In our implementation, connections on the server do not represent clients. They represent direct connections to SpatialOS.
// For this reason, things like ready checks, acks, throttling based on number of updated connections, interest management are irrelevant at this level.
int32 USpatialNetDriver::ServerReplicateActors(float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_SpatialServerReplicateActors);

#if WITH_SERVER_CODE
	if (ClientConnections.Num() == 0)
	{
		return 0;
	}

	check(World);

	int32 Updated = 0;

	// Bump the ReplicationFrame value to invalidate any properties marked as "unchanged" for this frame.
	ReplicationFrame++;

	const int32 NumClientsToTick = ServerReplicateActors_PrepConnections(DeltaSeconds);

	// SpatialGDK: This is a formality as there is at least one "perfect" Spatial connection in our design.
	if (NumClientsToTick == 0)
	{
		// No connections are ready this frame
		return 0;
	}

	AWorldSettings* WorldSettings = World->GetWorldSettings();

	bool bCPUSaturated = false;
	float ServerTickTime = GEngine->GetMaxTickRate(DeltaSeconds);
	if (ServerTickTime == 0.f)
	{
		ServerTickTime = DeltaSeconds;
	}
	else
	{
		ServerTickTime = 1.f / ServerTickTime;
		bCPUSaturated = DeltaSeconds > 1.2f * ServerTickTime;
	}

	SET_DWORD_STAT(STAT_SpatialConsiderList, 0);

	TArray<FNetworkObjectInfo*> ConsiderList;
	ConsiderList.Reserve(GetNetworkObjectList().GetActiveObjects().Num());

	// Build the consider list (actors that are ready to replicate)
	ServerReplicateActors_BuildConsiderList(ConsiderList, ServerTickTime);

	SET_DWORD_STAT(STAT_SpatialConsiderList, ConsiderList.Num());

	FMemMark Mark(FMemStack::Get());

	// Only process the fake spatial connection. It will be responsible for replicating all actors, regardless of whether they're owned by a client.
	USpatialNetConnection* SpatialConnection = Cast<USpatialNetConnection>(ClientConnections[0]);
	check(SpatialConnection && SpatialConnection->bReliableSpatialConnection);

	// Make a list of viewers this connection should consider
	TArray<FNetViewer>& ConnectionViewers = WorldSettings->ReplicationViewers;

	ConnectionViewers.Reset();

	// The fake spatial connection will borrow the player controllers from other connections.
	for (int i = 1; i < ClientConnections.Num(); i++)
	{
		USpatialNetConnection* ClientConnection = Cast<USpatialNetConnection>(ClientConnections[i]);
		check(ClientConnection);

		if (ClientConnection->ViewTarget != nullptr)
		{
			new(ConnectionViewers)FNetViewer(ClientConnection, DeltaSeconds);

			if (ClientConnection->Children.Num() > 0)
			{
				UE_LOG(LogSpatialOSNetDriver, Error, TEXT("Child connections present on Spatial client connection %s! We don't support splitscreen yet, so this will not function correctly."), *ClientConnection->GetName());
			}
		}
	}

	FMemMark RelevantActorMark(FMemStack::Get());

	FActorPriority* PriorityList = NULL;
	FActorPriority** PriorityActors = NULL;

	// Get a sorted list of actors for this connection
	const int32 FinalSortedCount = ServerReplicateActors_PrioritizeActors(SpatialConnection, ConnectionViewers, ConsiderList, bCPUSaturated, PriorityList, PriorityActors);

	// Process the sorted list of actors for this connection
	ServerReplicateActors_ProcessPrioritizedActors(SpatialConnection, ConnectionViewers, PriorityActors, FinalSortedCount, Updated);

	// SpatialGDK - Here Unreal would mark relevant actors that weren't processed this frame as bPendingNetUpdate. This is not used in the SpatialGDK and so has been removed.

	RelevantActorMark.Pop();
	ConnectionViewers.Reset();

	Mark.Pop();

	if (DebugRelevantActors)
	{
		PrintDebugRelevantActors();
		LastPrioritizedActors.Empty();
		LastSentActors.Empty();
		LastRelevantActors.Empty();
		LastNonRelevantActors.Empty();

		DebugRelevantActors = false;
	}

	return Updated;
#else
	return 0;
#endif // WITH_SERVER_CODE
}

void USpatialNetDriver::TickDispatch(float DeltaTime)
{
	// Not calling Super:: on purpose.
	UNetDriver::TickDispatch(DeltaTime);

	if (Connection != nullptr)
	{
		TArray<Worker_OpList*> OpLists = Connection->GetOpList();

		for (Worker_OpList* OpList : OpLists)
		{
			Dispatcher->ProcessOps(OpList);

			Worker_OpList_Destroy(OpList);
		}

		if (SpatialMetrics != nullptr && GetDefault<USpatialGDKSettings>()->bEnableMetrics)
		{
			SpatialMetrics->TickMetrics();
		}
	}
}

void USpatialNetDriver::ProcessRemoteFunction(
	AActor* Actor,
	UFunction* Function,
	void* Parameters,
	FOutParmRec* OutParms,
	FFrame* Stack,
	UObject* SubObject)
{
	if (Connection == nullptr)
	{
		UE_LOG(LogSpatialOSNetDriver, Error, TEXT("Attempted to call ProcessRemoteFunction before connection was established"));
		return;
	}

	USpatialNetConnection* NetConnection = ServerConnection ? Cast<USpatialNetConnection>(ServerConnection) : GetSpatialOSNetConnection();
	if (NetConnection == nullptr)
	{
		UE_LOG(LogSpatialOSNetDriver, Error, TEXT("Attempted to call ProcessRemoteFunction before connection was established"));
		return;
	}

	// This check mimics the way Unreal natively checks whether an AActor has ownership for sending server RPCs.
	// The function GetNetConnection() goes up the AActor ownership chain until it reaches an AActor that is possesed by an AController and
	// hence a UNetConnection. Server RPCs should only be sent by AActor instances that either are possessed by a UNetConnection or are owned by
	// other AActor instances possessed by a UNetConnection. For native Unreal reference see ProcessRemoteFunction() of IpNetDriver.cpp.
	// However if we are on the server, and the RPC is a CrossServer or NetMulticast RPC, this can be invoked without an owner.
	if (!Actor->GetNetConnection() && !(Function->FunctionFlags & (FUNC_NetCrossServer | FUNC_NetMulticast) && IsServer()))
	{
		UE_LOG(LogSpatialOSNetDriver, Warning, TEXT("No owning connection for actor %s. Function %s will not be processed."), *Actor->GetName(), *Function->GetName());
		return;
	}

	// Copied from UNetDriver::ProcessRemoteFunctionForChannel to copy pass-by-ref
	// parameters from OutParms into Parameters's memory.
	if (Stack == nullptr)
	{
		// Look for CPF_OutParm's, we'll need to copy these into the local parameter memory manually
		// The receiving side will pull these back out when needed
		for (TFieldIterator<UProperty> It(Function); It && (It->PropertyFlags & (CPF_Parm | CPF_ReturnParm)) == CPF_Parm; ++It)
		{
			if (It->HasAnyPropertyFlags(CPF_OutParm))
			{
				if (OutParms == nullptr)
				{
					continue;
				}

				FOutParmRec* Out = OutParms;

				while (Out->Property != *It)
				{
					Out = Out->NextOutParm;
				}

				void* Dest = It->ContainerPtrToValuePtr< void >(Parameters);

				const int32 CopySize = It->ElementSize * It->ArrayDim;

				check(((uint8*)Dest - (uint8*)Parameters) + CopySize <= Function->ParmsSize);

				It->CopyCompleteValue(Dest, Out->PropAddr);
			}
		}
	}

	// The RPC might have been called by an actor directly, or by a subobject on that actor
	UObject* CallingObject = SubObject ? SubObject : Actor;

	if (Function->FunctionFlags & FUNC_Net)
	{
		TSharedRef<FPendingRPCParams> RPCParams = MakeShared<FPendingRPCParams>(CallingObject, Function, Parameters, NextRPCIndex++);

#if !UE_BUILD_SHIPPING
		if (Function->HasAnyFunctionFlags(FUNC_NetReliable) && !Function->HasAnyFunctionFlags(FUNC_NetMulticast))
		{
			RPCParams->ReliableRPCIndex = GetNextReliableRPCId(Actor, FunctionFlagsToRPCSchemaType(Function->FunctionFlags), CallingObject);
		}
#endif // !UE_BUILD_SHIPPING

		Sender->SendRPC(RPCParams);
	}
}

void USpatialNetDriver::TickFlush(float DeltaTime)
{
	// Super::TickFlush() will not call ReplicateActors() because Spatial connections have InternalAck set to true.
	// In our case, our Spatial actor interop is triggered through ReplicateActors() so we want to call it regardless.

#if USE_SERVER_PERF_COUNTERS
	double ServerReplicateActorsTimeMs = 0.0f;
#endif // USE_SERVER_PERF_COUNTERS

	if (IsServer() && ClientConnections.Num() > 0 && EntityPool->IsReady())
	{
		// Update all clients.
#if WITH_SERVER_CODE

#if USE_SERVER_PERF_COUNTERS
		double ServerReplicateActorsTimeStart = FPlatformTime::Seconds();
#endif // USE_SERVER_PERF_COUNTERS

		int32 Updated = ServerReplicateActors(DeltaTime);

#if USE_SERVER_PERF_COUNTERS
		ServerReplicateActorsTimeMs = (FPlatformTime::Seconds() - ServerReplicateActorsTimeStart) * 1000.0;
#endif // USE_SERVER_PERF_COUNTERS

		static int32 LastUpdateCount = 0;
		// Only log the zero replicated actors once after replicating an actor
		if ((LastUpdateCount && !Updated) || Updated)
		{
			UE_LOG(LogNetTraffic, Verbose, TEXT("%s replicated %d actors"), *GetDescription(), Updated);
		}
		LastUpdateCount = Updated;
#endif // WITH_SERVER_CODE
	}

	// Tick the timer manager
	{
		TimerManager.Tick(DeltaTime);
	}

	Super::TickFlush(DeltaTime);
}

USpatialNetConnection * USpatialNetDriver::GetSpatialOSNetConnection() const
{
	if (ServerConnection)
	{
		return Cast<USpatialNetConnection>(ServerConnection);
	}
	else
	{
		return Cast<USpatialNetConnection>(ClientConnections[0]);
	}
}

USpatialNetConnection* USpatialNetDriver::AcceptNewPlayer(const FURL& InUrl, FUniqueNetIdRepl UniqueId, FName OnlinePlatformName, bool bExistingPlayer)
{
	bool bOk = true;

	USpatialNetConnection* SpatialConnection = NewObject<USpatialNetConnection>(GetTransientPackage(), NetConnectionClass);
	check(SpatialConnection);

	// We create a "dummy" connection that corresponds to this player. This connection won't transmit any data.
	// We may not need to keep it in the future, but for now it looks like path of least resistance is to have one UPlayer (UConnection) per player.
	ISocketSubsystem* SocketSubsystem = GetSocketSubsystem();
	TSharedRef<FInternetAddr> FromAddr = SocketSubsystem->CreateInternetAddr();

	SpatialConnection->InitRemoteConnection(this, nullptr, InUrl, *FromAddr, USOCK_Open);
	Notify->NotifyAcceptedConnection(SpatialConnection);
	AddClientConnection(SpatialConnection);

	// Set the unique net ID for this player. This and the code below is adapted from World.cpp:4499
	SpatialConnection->PlayerId = UniqueId;
	SpatialConnection->SetPlayerOnlinePlatformName(OnlinePlatformName);

	// Get the worker attribute.
	const TCHAR* WorkerAttributeOption = InUrl.GetOption(TEXT("workerAttribute"), nullptr);
	check(WorkerAttributeOption);
	SpatialConnection->WorkerAttribute = FString(WorkerAttributeOption).Mid(1); // Trim off the = at the beginning.

	// We will now ask GameMode/GameSession if it's ok for this user to join.
	// Note that in the initial implementation, we carry over no data about the user here (such as a unique player id, or the real IP)
	// In the future it would make sense to add metadata to the Spawn request and pass it here.
	// For example we can check whether a user is banned by checking against an OnlineSubsystem.

	// skip to the first option in the URL
	const FString UrlString = InUrl.ToString();
	const TCHAR* Tmp = *UrlString;
	for (; *Tmp && *Tmp != '?'; Tmp++);

	FString ErrorMsg;
	AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
	if (GameMode)
	{
		GameMode->PreLogin(Tmp, SpatialConnection->LowLevelGetRemoteAddress(), SpatialConnection->PlayerId, ErrorMsg);
	}

	if (!ErrorMsg.IsEmpty())
	{
		UE_LOG(LogSpatialOSNetDriver, Error, TEXT("PreLogin failure: %s"), *ErrorMsg);
		bOk = false;
	}

	if (bOk)
	{
		FString LevelName = GetWorld()->GetCurrentLevel()->GetOutermost()->GetName();
		SpatialConnection->SetClientWorldPackageName(GetWorld()->GetCurrentLevel()->GetOutermost()->GetFName());

		FString GameName;
		FString RedirectURL;
		if (GameMode)
		{
			GameName = GameMode->GetClass()->GetPathName();
			GameMode->GameWelcomePlayer(SpatialConnection, RedirectURL);
		}

		if (!bExistingPlayer)
		{
			SpatialConnection->PlayerController = World->SpawnPlayActor(SpatialConnection, ROLE_AutonomousProxy, InUrl, SpatialConnection->PlayerId, ErrorMsg);
		}
		else
		{
			// Most of this is taken from "World->SpawnPlayActor", excluding the logic to spawn a pawn which happens during
			// GameMode->PostLogin(...).
			APlayerController* NewPlayerController = GameMode->SpawnPlayerController(ROLE_AutonomousProxy, UrlString);

			// Destroy the player state (as we'll be replacing it anyway).
			NewPlayerController->CleanupPlayerState();

			// Possess the newly-spawned player.
			NewPlayerController->NetPlayerIndex = 0;
			NewPlayerController->Role = ROLE_Authority;
			NewPlayerController->SetReplicates(true);
			NewPlayerController->SetAutonomousProxy(true);
			NewPlayerController->SetPlayer(SpatialConnection);
			// We explicitly don't call GameMode->PostLogin(NewPlayerController) here, to avoid the engine restarting the player.
			// TODO: Should we call AGameSession::PostLogin? - UNR:583
			// TODO: Should we trigger to blueprints that a player has "joined" via GameMode->K2_PostLogin(Connection)? - UNR:583

			SpatialConnection->PlayerController = NewPlayerController;
		}

		if (SpatialConnection->PlayerController == NULL)
		{
			// Failed to connect.
			UE_LOG(LogSpatialOSNetDriver, Error, TEXT("Join failure: %s"), *ErrorMsg);
			SpatialConnection->FlushNet(true);
			bOk = false;
		}
	}

	if (!bOk)
	{
		// TODO: Destroy connection. UNR:584
	}

	return bOk ? SpatialConnection : nullptr;
}

bool USpatialNetDriver::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
#if !UE_BUILD_SHIPPING
	if (FParse::Command(&Cmd, TEXT("DUMPCROSSSERVERRPC")))
	{
		return HandleNetDumpCrossServerRPCCommand(Cmd, Ar);
	}
#endif // !UE_BUILD_SHIPPING
	return UNetDriver::Exec(InWorld, Cmd, Ar);
}

// This function is literally a copy paste of UNetDriver::HandleNetDumpServerRPCCommand. Didn't want to refactor to avoid divergence from engine.
#if !UE_BUILD_SHIPPING
bool USpatialNetDriver::HandleNetDumpCrossServerRPCCommand(const TCHAR* Cmd, FOutputDevice& Ar)
{
#if WITH_SERVER_CODE
	for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
	{
		bool bHasNetFields = false;

		ensureMsgf(!ClassIt->HasAnyFlags(RF_NeedLoad | RF_NeedPostLoad), TEXT("UNetDriver::HandleNetDumpCrossServerRPCCommand: %s has flag RF_NeedPostLoad. NetFields and ClassReps will be incorrect!"), *GetFullNameSafe(*ClassIt));

		for (int32 i = 0; i < ClassIt->NetFields.Num(); i++)
		{
			UFunction * Function = Cast<UFunction>(ClassIt->NetFields[i]);

			if (Function != NULL && Function->FunctionFlags & FUNC_NetCrossServer)
			{
				bHasNetFields = true;
				break;
			}
		}

		if (!bHasNetFields)
		{
			continue;
		}

		Ar.Logf(TEXT("Class: %s"), *ClassIt->GetName());

		for (int32 i = 0; i < ClassIt->NetFields.Num(); i++)
		{
			UFunction * Function = Cast<UFunction>(ClassIt->NetFields[i]);

			if (Function != NULL && Function->FunctionFlags & FUNC_NetCrossServer)
			{
				const FClassNetCache * ClassCache = NetCache->GetClassNetCache(*ClassIt);

				const FFieldNetCache * FieldCache = ClassCache->GetFromField(Function);

				TArray< UProperty * > Parms;

				for (TFieldIterator<UProperty> It(Function); It && (It->PropertyFlags & (CPF_Parm | CPF_ReturnParm)) == CPF_Parm; ++It)
				{
					Parms.Add(*It);
				}

				if (Parms.Num() == 0)
				{
					Ar.Logf(TEXT("    [0x%03x] %s();"), FieldCache->FieldNetIndex, *Function->GetName());
					continue;
				}

				FString ParmString;

				for (int32 j = 0; j < Parms.Num(); j++)
				{
					if (Cast<UStructProperty>(Parms[j]))
					{
						ParmString += Cast<UStructProperty>(Parms[j])->Struct->GetName();
					}
					else
					{
						ParmString += Parms[j]->GetClass()->GetName();
					}

					ParmString += TEXT(" ");

					ParmString += Parms[j]->GetName();

					if (j < Parms.Num() - 1)
					{
						ParmString += TEXT(", ");
					}
				}

				Ar.Logf(TEXT("    [0x%03x] %s( %s );"), FieldCache->FieldNetIndex, *Function->GetName(), *ParmString);
			}
		}
	}
#endif
	return true;
}
#endif // !UE_BUILD_SHIPPING

USpatialPendingNetGame::USpatialPendingNetGame(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void USpatialPendingNetGame::InitNetDriver()
{
	check(GIsClient);

	// This is a trimmed down version of UPendingNetGame::InitNetDriver(). We don't send any Unreal connection packets, just set up the net driver.
	if (!GDisallowNetworkTravel)
	{
		// Try to create network driver.
		if (GEngine->CreateNamedNetDriver(this, NAME_PendingNetDriver, NAME_GameNetDriver))
		{
			NetDriver = GEngine->FindNamedNetDriver(this, NAME_PendingNetDriver);
		}
		check(NetDriver);

		if (!NetDriver->InitConnect(this, URL, ConnectionError))
		{
			// error initializing the network stack...
			UE_LOG(LogNet, Warning, TEXT("error initializing the network stack"));
			GEngine->DestroyNamedNetDriver(this, NetDriver->NetDriverName);
			NetDriver = nullptr;

			// ConnectionError should be set by calling InitConnect...however, if we set NetDriver to NULL without setting a
			// value for ConnectionError, we'll trigger the assertion at the top of UPendingNetGame::Tick() so make sure it's set
			if (ConnectionError.Len() == 0)
			{
				ConnectionError = NSLOCTEXT("Engine", "NetworkInit", "Error initializing network layer.").ToString();
			}
		}
	}
	else
	{
		ConnectionError = NSLOCTEXT("Engine", "UsedCheatCommands", "Console commands were used which are disallowed in netplay.  You must restart the game to create a match.").ToString();
	}
}

void USpatialPendingNetGame::SendJoin()
{
	bSentJoinRequest = true;
}

void USpatialNetDriver::AddActorChannel(Worker_EntityId EntityId, USpatialActorChannel* Channel)
{
	EntityToActorChannel.Add(EntityId, Channel);
}

void USpatialNetDriver::RemoveActorChannel(Worker_EntityId EntityId)
{
	if (!EntityToActorChannel.Contains(EntityId))
	{
		UE_LOG(LogSpatialOSNetDriver, Verbose, TEXT("RemoveActorChannel: Failed to find entity/channel mapping for entity %lld."), EntityId);
		return;
	}

	EntityToActorChannel.FindAndRemoveChecked(EntityId);
}

TMap<Worker_EntityId_Key, USpatialActorChannel*>& USpatialNetDriver::GetEntityToActorChannelMap()
{
	return EntityToActorChannel;
}

USpatialActorChannel* USpatialNetDriver::GetActorChannelByEntityId(Worker_EntityId EntityId) const
{
	return EntityToActorChannel.FindRef(EntityId);
}

void USpatialNetDriver::WipeWorld(const USpatialNetDriver::PostWorldWipeDelegate& LoadSnapshotAfterWorldWipe)
{
	if (Cast<USpatialGameInstance>(GetWorld()->GetGameInstance())->bResponsibleForSnapshotLoading)
	{
		SnapshotManager->WorldWipe(LoadSnapshotAfterWorldWipe);
	}
}

#if !UE_BUILD_SHIPPING
uint32 USpatialNetDriver::GetNextReliableRPCId(AActor* Actor, ESchemaComponentType RPCType, UObject* TargetObject)
{
	if (!ReliableRPCIdMap.Contains(Actor))
	{
		ReliableRPCIdMap.Add(Actor);
	}
	FRPCTypeToReliableRPCIdMap& ReliableRPCIds = ReliableRPCIdMap[Actor];

	if (FReliableRPCId* RPCIdEntry = ReliableRPCIds.Find(RPCType))
	{
		if (!RPCIdEntry->WorkerId.IsEmpty())
		{
			// We previously used to receive RPCs of this type, now we're about to send one, so we reset the reliable RPC index.
			// This should only be possible for CrossServer RPCs.
			check(RPCType == SCHEMA_CrossServerRPC);
			UE_LOG(LogSpatialOSNetDriver, Verbose, TEXT("Actor %s, object %s: Used to receive reliable CrossServer RPCs from worker %s, now about to send one. The entity must have crossed boundary."),
				*Actor->GetName(), *TargetObject->GetName(), *RPCIdEntry->WorkerId);
			RPCIdEntry->WorkerId = FString();
			RPCIdEntry->RPCId = 0;
		}
	}
	else
	{
		// Add an entry for this RPC type with empty WorkerId and RPCId = 0
		ReliableRPCIds.Add(RPCType);
	}

	return ++ReliableRPCIds[RPCType].RPCId;
}

void USpatialNetDriver::OnReceivedReliableRPC(AActor* Actor, ESchemaComponentType RPCType, FString WorkerId, uint32 RPCId, UObject* TargetObject, UFunction* Function)
{
	check(!WorkerId.IsEmpty());

	if (!ReliableRPCIdMap.Contains(Actor))
	{
		ReliableRPCIdMap.Add(Actor);
	}
	FRPCTypeToReliableRPCIdMap& ReliableRPCIds = ReliableRPCIdMap[Actor];

	if (FReliableRPCId* RPCIdEntry = ReliableRPCIds.Find(RPCType))
	{
		if (WorkerId != RPCIdEntry->WorkerId)
		{
			if (RPCIdEntry->WorkerId.IsEmpty())
			{
				// We previously used to send RPCs of this type, now we received one. This should only be possible for CrossServer RPCs.
				check(RPCType == SCHEMA_CrossServerRPC);
				UE_LOG(LogSpatialOSNetDriver, Verbose, TEXT("Actor %s, object %s: Used to send reliable CrossServer RPCs, now received one from worker %s. The entity must have crossed boundary."),
					*Actor->GetName(), *TargetObject->GetName(), *WorkerId);
			}
			else
			{
				// We received an RPC from a different worker than the one we used to receive RPCs of this type from.
				UE_LOG(LogSpatialOSNetDriver, Verbose, TEXT("Actor %s, object %s: Received a reliable %s RPC from a different worker %s. Previously received from worker %s."),
					*Actor->GetName(), *TargetObject->GetName(), *RPCSchemaTypeToString(RPCType), *WorkerId, *RPCIdEntry->WorkerId);
			}
			RPCIdEntry->WorkerId = WorkerId;
		}
		else if (RPCId != RPCIdEntry->RPCId + 1)
		{
			if (RPCId < RPCIdEntry->RPCId)
			{
				UE_LOG(LogSpatialOSNetDriver, Verbose, TEXT("Actor %s: Reliable %s RPC received out of order! Previously received RPC: %s, target %s, index %d. Now received: %s, target %s, index %d. Sender: %s"),
					*Actor->GetName(), *RPCSchemaTypeToString(RPCType), *RPCIdEntry->LastRPCName, *RPCIdEntry->LastRPCTarget, RPCIdEntry->RPCId, *Function->GetName(), *TargetObject->GetName(), RPCId, *WorkerId);
			}
			else if (RPCId == RPCIdEntry->RPCId)
			{
				UE_LOG(LogSpatialOSNetDriver, Verbose, TEXT("Actor %s: Reliable %s RPC index duplicated! Previously received RPC: %s, target %s, index %d. Now received: %s, target %s, index %d. Sender: %s"),
					*Actor->GetName(), *RPCSchemaTypeToString(RPCType), *RPCIdEntry->LastRPCName, *RPCIdEntry->LastRPCTarget, RPCIdEntry->RPCId, *Function->GetName(), *TargetObject->GetName(), RPCId, *WorkerId);
			}
			else
			{
				UE_LOG(LogSpatialOSNetDriver, Verbose, TEXT("Actor %s: One or more reliable %s RPCs skipped! Previously received RPC: %s, target %s, index %d. Now received: %s, target %s, index %d. Sender: %s"),
					*Actor->GetName(), *RPCSchemaTypeToString(RPCType), *RPCIdEntry->LastRPCName, *RPCIdEntry->LastRPCTarget, RPCIdEntry->RPCId, *Function->GetName(), *TargetObject->GetName(), RPCId, *WorkerId);
			}
		}

		RPCIdEntry->RPCId = RPCId;
		RPCIdEntry->LastRPCTarget = TargetObject->GetName();
		RPCIdEntry->LastRPCName = Function->GetName();
	}
	else
	{
		ReliableRPCIds.Add(RPCType, FReliableRPCId(WorkerId, RPCId, TargetObject->GetName(), Function->GetName()));
	}
}

void USpatialNetDriver::OnRPCAuthorityGained(AActor* Actor, ESchemaComponentType RPCType)
{
	// When we gain authority on an RPC component of an actor that we previously received RPCs for, reset the reliable RPC counter.
	// This is to account for the case where the actor crosses to another worker, receives a couple of reliable RPCs, and comes back
	// to the original worker.
	if (FRPCTypeToReliableRPCIdMap* ReliableRPCIds = ReliableRPCIdMap.Find(Actor))
	{
		if (ReliableRPCIds->Contains(RPCType))
		{
			UE_LOG(LogSpatialOSNetDriver, Verbose, TEXT("Actor %s: Gained authority over %s RPC component. Resetting previous reliable RPC counter."),
				*Actor->GetName(), *RPCSchemaTypeToString(RPCType));
			ReliableRPCIds->Remove(RPCType);
		}
	}
}

#endif // !UE_BUILD_SHIPPING

void USpatialNetDriver::DelayedSendDeleteEntityRequest(Worker_EntityId EntityId, float Delay)
{
	FTimerHandle RetryTimer;
	TimerManager.SetTimer(RetryTimer, [this, EntityId]()
	{
		Sender->SendDeleteEntityRequest(EntityId);
	}, Delay, false);
}
