// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialNetDriver.h"
#include "SpatialNetConnection.h"
#include "EntityRegistry.h"
#include "EntityPipeline.h"
#include "SocketSubsystem.h"
#include "SpatialConstants.h"
#include "SpatialInteropBlock.h"
#include "SpatialOS.h"
#include "SpatialOSComponentUpdater.h"
#include "Engine/ActorChannel.h"
#include "EngineStats.h"
#include "Engine/NetworkObjectList.h"
#include "Net/RepLayout.h"
#include "Net/DataReplication.h"
#include "SpatialPackageMapClient.h"
#include "SpatialPendingNetGame.h"
#include "SpatialActorChannel.h"
#include "improbable/spawner/spawner.h"

//#include "Generated/SpatialInteropCharacter.h"

#define ENTITY_BLUEPRINTS_FOLDER "/Game/EntityBlueprints"



bool USpatialNetDriver::InitBase(bool bInitAsClient, FNetworkNotify* InNotify, const FURL& URL, bool bReuseAddressAndPort, FString& Error)
{
	if (!Super::InitBase(bInitAsClient, InNotify, URL, bReuseAddressAndPort, Error))
	{
		return false;
	}

	// make absolutely sure that the actor channel that we are using is our Spatial actor channel
	UChannel::ChannelClasses[CHTYPE_Actor] = USpatialActorChannel::StaticClass();

	SpatialOSInstance = NewObject<USpatialOS>(this);

	SpatialOSInstance->OnConnectedDelegate.AddDynamic(this,
		&USpatialNetDriver::OnSpatialOSConnected);
	SpatialOSInstance->OnConnectionFailedDelegate.AddDynamic(
		this, &USpatialNetDriver::OnSpatialOSConnectFailed);
	SpatialOSInstance->OnDisconnectedDelegate.AddDynamic(
		this, &USpatialNetDriver::OnSpatialOSDisconnected);

	auto workerConfig = FSOSWorkerConfigurationData();

	workerConfig.Networking.UseExternalIp = false;
	workerConfig.SpatialOSApplication.WorkerPlatform =
		bInitAsClient ? TEXT("UnrealClient") : TEXT("UnrealWorker");

	SpatialOSInstance->ApplyConfiguration(workerConfig);
	SpatialOSInstance->Connect();

	SpatialOSComponentUpdater = NewObject<USpatialOSComponentUpdater>(this);

	EntityRegistry = NewObject<UEntityRegistry>(this);

	return true;
}

bool USpatialNetDriver::InitConnect(FNetworkNotify* InNotify, const FURL& ConnectURL, FString& Error)
{
	if (!Super::InitConnect(InNotify, ConnectURL, Error))
	{
		return false;
	}
		
	return true;
}

bool USpatialNetDriver::InitListen(FNetworkNotify* InNotify, FURL& LocalURL, bool bReuseAddressAndPort, FString& Error)
{
	if (!Super::InitListen(InNotify, LocalURL, bReuseAddressAndPort, Error))
	{
		return false;
	}

	return true;
}

void USpatialNetDriver::PostInitProperties()
{
	Super::PostInitProperties();

	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		// GuidCache will be allocated as an FNetGUIDCache above. To avoid an engine code change, we re-do it with the Spatial equivalent.
		GuidCache = TSharedPtr< FNetGUIDCache >(new FSpatialNetGUIDCache(this));
	}	
}

void USpatialNetDriver::OnSpatialOSConnected()
{
	UE_LOG(LogTemp, Warning, TEXT("Connected to SpatialOS."));

	SpatialInteropBlock = NewObject<USpatialInteropBlock>();
	SpatialInteropBlock->Init(EntityRegistry);
	SpatialOSInstance->GetEntityPipeline()->AddBlock(SpatialInteropBlock);

	ShadowActorPipelineBlock = NewObject<USpatialShadowActorPipelineBlock>();
	ShadowActorPipelineBlock->Init(EntityRegistry);
	SpatialOSInstance->GetEntityPipeline()->AddBlock(ShadowActorPipelineBlock);

	TArray<FString> BlueprintPaths;
	BlueprintPaths.Add(TEXT(ENTITY_BLUEPRINTS_FOLDER));

	EntityRegistry->RegisterEntityBlueprints(BlueprintPaths);

	// If we're the client, we can now ask the server to spawn our controller.
	if (ServerConnection)
	{
		auto LockedConnection = SpatialOSInstance->GetConnection().Pin();

		if (LockedConnection.IsValid())
		{
			LockedConnection->SendCommandRequest<improbable::spawner::Spawner::Commands::SpawnPlayer>(SpatialConstants::SPAWNER_ENTITY_ID,
				improbable::spawner::SpawnPlayerRequest(TCHAR_TO_UTF8(*ServerConnection->URL.ToString())),
				worker::Option<std::uint32_t>(0),
				worker::CommandParameters());
		}

		// 
		FWorldContext* WorldContext = GEngine->GetWorldContextFromPendingNetGameNetDriver(this);

		// Here we need to fake a few things to start ticking the level travel on client.
		if (WorldContext && WorldContext->PendingNetGame)
		{
			WorldContext->PendingNetGame->bSuccessfullyConnected = true;
			WorldContext->PendingNetGame->bSentJoinRequest = false;
		}		
	}
}

void USpatialNetDriver::OnSpatialOSDisconnected()
{
	UE_LOG(LogTemp, Warning, TEXT("Disconnected from SpatialOS."));
}

void USpatialNetDriver::OnSpatialOSConnectFailed()
{
	UE_LOG(LogTemp, Warning, TEXT("Could not connect to SpatialOS."));
}

//NUF: This is a modified and simplified version of UNetDriver::ServerReplicateActors.
// In our implementation, connections on the server do not represent clients. They represent direct connections to SpatialOS.
// For this reason, things like ready checks, acks, throttling based on number of updated connections, interest management are irrelevant at this level.
int32 USpatialNetDriver::ServerReplicateActors(float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_NetServerRepActorsTime);

#if WITH_SERVER_CODE
	if (ClientConnections.Num() == 0)
	{
		return 0;
	}

	check(World);

	int32 Updated = 0;

	// Bump the ReplicationFrame value to invalidate any properties marked as "unchanged" for this frame.
	ReplicationFrame++;

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

	TArray<FNetworkObjectInfo*> ConsiderList;
	ConsiderList.Reserve(GetNetworkObjectList().GetActiveObjects().Num());

	// Build the consider list (actors that are ready to replicate)
	ServerReplicateActors_BuildConsiderList(ConsiderList, ServerTickTime);

	FMemMark Mark(FMemStack::Get());

	for (int32 i = 0; i < ClientConnections.Num(); i++)
	{
		UNetConnection* Connection = ClientConnections[i];
		check(Connection);

		if (Connection->ViewTarget)
		{
			// Make a list of viewers this connection should consider (this connection and children of this connection)
			TArray<FNetViewer>& ConnectionViewers = WorldSettings->ReplicationViewers;

			ConnectionViewers.Reset();
			new(ConnectionViewers)FNetViewer(Connection, DeltaSeconds);
			for (int32 ViewerIndex = 0; ViewerIndex < Connection->Children.Num(); ViewerIndex++)
			{
				if (Connection->Children[ViewerIndex]->ViewTarget != NULL)
				{
					new(ConnectionViewers)FNetViewer(Connection->Children[ViewerIndex], DeltaSeconds);
				}
			}
			
			//todo-giray: This requires additional attention as we don't have player specific connections.
			if (Connection->PlayerController)
			{
				Connection->PlayerController->SendClientAdjustment();
			}

			for (int32 ChildIdx = 0; ChildIdx < Connection->Children.Num(); ChildIdx++)
			{
				if (Connection->Children[ChildIdx]->PlayerController != NULL)
				{
					Connection->Children[ChildIdx]->PlayerController->SendClientAdjustment();
				}
			}			
			
			//NUF: Here, Unreal sorts and prioritizes actors. Actor prioritization is heavily related to player position and we don't send player specific updates. So it's moot.
			//Maybe we can do it at the Spatial level.
			
			for (int ActorIdx = 0; ActorIdx <= ConsiderList.Num(); ++ActorIdx)
			{
				AActor* Actor = ConsiderList[ActorIdx]->Actor;
				check(Actor);
				UActorChannel* Channel = Connection->ActorChannels.FindRef(Actor);
				if (!Channel)
				{
					// Find or create the channel for this actor.
					// we can't create the channel if the client is in a different world than we are
					// or the package map doesn't support the actor's class/archetype (or the actor itself in the case of serializable actors)
					// or it's an editor placed actor and the client hasn't initialized the level it's in
					if (GuidCache->SupportsObject(Actor->GetClass()) && GuidCache->SupportsObject(Actor->IsNetStartupActor() ? Actor : Actor->GetArchetype()))
					{
						// Create a new channel for this actor.
						Channel = (UActorChannel*)Connection->CreateChannel(CHTYPE_Actor, 1);
						if (Channel)
						{
							Channel->SetChannelActor(Actor);
						}
					}
				}				
			}
		}
	}
	
	Mark.Pop();

	return ClientConnections.Num(); //return Updated;
#else
	return 0;
#endif // WITH_SERVER_CODE
}

void USpatialNetDriver::TickDispatch(float DeltaTime)
{
	if (GetNetMode() == NM_Client)
	{
		// On client I want to disable all Unreal socket based communication.
		UNetDriver::TickDispatch(DeltaTime);
	}
	else
	{
		Super::TickDispatch(DeltaTime);
	}

	if (SpatialOSInstance != nullptr && SpatialOSInstance->GetEntityPipeline() != nullptr)
	{
		SpatialOSInstance->ProcessOps();
		SpatialOSInstance->GetEntityPipeline()->ProcessOps(SpatialOSInstance->GetView(), SpatialOSInstance->GetConnection(), GetWorld());
		SpatialOSComponentUpdater->UpdateComponents(EntityRegistry, DeltaTime);
		if (ShadowActorPipelineBlock)
		{
			ShadowActorPipelineBlock->ReplicateShadowActorChanges(DeltaTime);
		}
	}
}

bool USpatialNetDriver::AcceptNewPlayer(const FURL& InUrl)
{
	check(GetNetMode() != NM_Client);

	bool bOk = true;
	USpatialNetConnection* Connection = NewObject<USpatialNetConnection>(GetTransientPackage(), NetConnectionClass);
	check(Connection);
	
	// We create a "dummy" connection that corresponds to this player. This connection won't transmit any data.
	// We may not need to keep it in the future, but for now it looks like path of least resistance is to have one UPlayer (UConnection) per player.
	ISocketSubsystem* SocketSubsystem = GetSocketSubsystem();
	TSharedRef<FInternetAddr> FromAddr = SocketSubsystem->CreateInternetAddr();

	Connection->InitRemoteConnection(this, nullptr, InUrl, *FromAddr, USOCK_Open);
	Notify->NotifyAcceptedConnection(Connection);
	AddClientConnection(Connection);

	// We will now ask GameMode/GameSession if it's ok for this user to join.
	// Note that in the initial implementation, we carry over no data about the user here (such as a unique player id, or the real IP)
	// In the future it would make sense to add metadata to the Spawn request and pass it here.
	// For example we can check whether a user is banned by checking against an OnlineSubsystem.

	// skip to the first option in the URL
	const TCHAR* Tmp = *InUrl.ToString();
	for (; *Tmp && *Tmp != '?'; Tmp++);

	FString ErrorMsg;
	AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
	if (GameMode)
	{
		GameMode->PreLogin(Tmp, Connection->LowLevelGetRemoteAddress(), Connection->PlayerId, ErrorMsg);
	}
	
	if (!ErrorMsg.IsEmpty())
	{
		UE_LOG(LogNet, Log, TEXT("PreLogin failure: %s"), *ErrorMsg);
		bOk = false;		
	}
	else
	{
		FString LevelName = GetWorld()->GetCurrentLevel()->GetOutermost()->GetName();
		Connection->ClientWorldPackageName = GetWorld()->GetCurrentLevel()->GetOutermost()->GetFName();

		FString GameName;
		FString RedirectURL;
		if (GameMode)
		{
			GameName = GameMode->GetClass()->GetPathName();
			GameMode->GameWelcomePlayer(Connection, RedirectURL);
		}
	}

	// Go all the way to creating the player controller here.

	return bOk;
}

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

		if (NetDriver->InitConnect(this, URL, ConnectionError))
		{
		
		}
		else 
		{
			// error initializing the network stack...
			UE_LOG(LogNet, Warning, TEXT("error initializing the network stack"));
			GEngine->DestroyNamedNetDriver(this, NetDriver->NetDriverName);
			NetDriver = NULL;

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


#if WITH_SERVER_CODE
int32 UNetDriver::ServerReplicateActors_PrepConnections(const float DeltaSeconds)
{
	int32 NumClientsToTick = ClientConnections.Num();

	// by default only throttle update for listen servers unless specified on the commandline
	static bool bForceClientTickingThrottle = FParse::Param(FCommandLine::Get(), TEXT("limitclientticks"));
	if (bForceClientTickingThrottle || GetNetMode() == NM_ListenServer)
	{
		// determine how many clients to tick this frame based on GEngine->NetTickRate (always tick at least one client), double for lan play
		// FIXME: DeltaTimeOverflow is a static, and will conflict with other running net drivers, we investigate storing it on the driver itself!
		static float DeltaTimeOverflow = 0.f;
		// updates are doubled for lan play
		static bool LanPlay = FParse::Param(FCommandLine::Get(), TEXT("lanplay"));
		//@todo - ideally we wouldn't want to tick more clients with a higher deltatime as that's not going to be good for performance and probably saturate bandwidth in hitchy situations, maybe 
		// come up with a solution that is greedier with higher framerates, but still won't risk saturating server upstream bandwidth
		float ClientUpdatesThisFrame = GEngine->NetClientTicksPerSecond * (DeltaSeconds + DeltaTimeOverflow) * (LanPlay ? 2.f : 1.f);
		NumClientsToTick = FMath::Min<int32>(NumClientsToTick, FMath::TruncToInt(ClientUpdatesThisFrame));
		//UE_LOG(LogNet, Log, TEXT("%2.3f: Ticking %d clients this frame, %2.3f/%2.4f"),GetWorld()->GetTimeSeconds(),NumClientsToTick,DeltaSeconds,ClientUpdatesThisFrame);
		if (NumClientsToTick == 0)
		{
			// if no clients are ticked this frame accumulate the time elapsed for the next frame
			DeltaTimeOverflow += DeltaSeconds;
			return 0;
		}
		DeltaTimeOverflow = 0.f;
	}

	bool bFoundReadyConnection = false;

	for (int32 ConnIdx = 0; ConnIdx < ClientConnections.Num(); ConnIdx++)
	{
		UNetConnection* Connection = ClientConnections[ConnIdx];
		check(Connection);
		check(Connection->State == USOCK_Pending || Connection->State == USOCK_Open || Connection->State == USOCK_Closed);
		checkSlow(Connection->GetUChildConnection() == NULL);

		// Handle not ready channels.
		//@note: we cannot add Connection->IsNetReady(0) here to check for saturation, as if that's the case we still want to figure out the list of relevant actors
		//			to reset their NetUpdateTime so that they will get sent as soon as the connection is no longer saturated
		AActor* OwningActor = Connection->OwningActor;
		if (OwningActor != NULL && Connection->State == USOCK_Open && (Connection->Driver->Time - Connection->LastReceiveTime < 1.5f))
		{
			check(World == OwningActor->GetWorld());

			bFoundReadyConnection = true;

			// the view target is what the player controller is looking at OR the owning actor itself when using beacons
			Connection->ViewTarget = Connection->PlayerController ? Connection->PlayerController->GetViewTarget() : OwningActor;

			for (int32 ChildIdx = 0; ChildIdx < Connection->Children.Num(); ChildIdx++)
			{
				UNetConnection *Child = Connection->Children[ChildIdx];
				APlayerController* ChildPlayerController = Child->PlayerController;
				if (ChildPlayerController != NULL)
				{
					Child->ViewTarget = ChildPlayerController->GetViewTarget();
				}
				else
				{
					Child->ViewTarget = NULL;
				}
			}
		}
		else
		{
			Connection->ViewTarget = NULL;
			for (int32 ChildIdx = 0; ChildIdx < Connection->Children.Num(); ChildIdx++)
			{
				Connection->Children[ChildIdx]->ViewTarget = NULL;
			}
		}
	}

	return bFoundReadyConnection ? NumClientsToTick : 0;
}

void UNetDriver::ServerReplicateActors_BuildConsiderList(TArray<FNetworkObjectInfo*>& OutConsiderList, const float ServerTickTime)
{
	SCOPE_CYCLE_COUNTER(STAT_NetConsiderActorsTime);

	UE_LOG(LogNetTraffic, Log, TEXT("ServerReplicateActors_BuildConsiderList, Building ConsiderList %4.2f"), World->GetTimeSeconds());

	int32 NumInitiallyDormant = 0;

	const bool bUseAdapativeNetFrequency = IsAdaptiveNetUpdateFrequencyEnabled();

	TArray<AActor*> ActorsToRemove;

	for (const TSharedPtr<FNetworkObjectInfo>& ObjectInfo : GetNetworkObjectList().GetActiveObjects())
	{
		FNetworkObjectInfo* ActorInfo = ObjectInfo.Get();

		if (!ActorInfo->bPendingNetUpdate && World->TimeSeconds <= ActorInfo->NextUpdateTime)
		{
			continue;		// It's not time for this actor to perform an update, skip it
		}

		AActor* Actor = ActorInfo->Actor;

		if (Actor->IsPendingKill())
		{
			ActorsToRemove.Add(Actor);
			continue;
		}

		if (Actor->GetRemoteRole() == ROLE_None)
		{
			ActorsToRemove.Add(Actor);
			continue;
		}

		// This actor may belong to a different net driver, make sure this is the correct one
		// (this can happen when using beacon net drivers for example)
		if (Actor->GetNetDriverName() != NetDriverName)
		{
			UE_LOG(LogNetTraffic, Error, TEXT("Actor %s in wrong network actors list!"), *Actor->GetName());
			continue;
		}

		// Verify the actor is actually initialized (it might have been intentionally spawn deferred until a later frame)
		if (!Actor->IsActorInitialized())
		{
			continue;
		}

		// Don't send actors that may still be streaming in or out
		ULevel* Level = Actor->GetLevel();
		if (Level->HasVisibilityChangeRequestPending() || Level->bIsAssociatingLevel)
		{
			continue;
		}

		if (Actor->NetDormancy == DORM_Initial && Actor->IsNetStartupActor())
		{
			// This stat isn't that useful in its current form when using NetworkActors list
			// We'll want to track initially dormant actors some other way to track them with stats
			SCOPE_CYCLE_COUNTER(STAT_NetInitialDormantCheckTime);
			NumInitiallyDormant++;
			ActorsToRemove.Add(Actor);
			//UE_LOG(LogNetTraffic, Log, TEXT("Skipping Actor %s - its initially dormant!"), *Actor->GetName() );
			continue;
		}

		checkSlow(Actor->NeedsLoadForClient()); // We have no business sending this unless the client can load
		checkSlow(World == Actor->GetWorld());

		// Set defaults if this actor is replicating for first time
		if (ActorInfo->LastNetReplicateTime == 0)
		{
			ActorInfo->LastNetReplicateTime = World->TimeSeconds;
			ActorInfo->OptimalNetUpdateDelta = 1.0f / Actor->NetUpdateFrequency;
		}

		const float ScaleDownStartTime = 2.0f;
		const float ScaleDownTimeRange = 5.0f;

		const float LastReplicateDelta = World->TimeSeconds - ActorInfo->LastNetReplicateTime;

		if (LastReplicateDelta > ScaleDownStartTime)
		{
			if (Actor->MinNetUpdateFrequency == 0.0f)
			{
				Actor->MinNetUpdateFrequency = 2.0f;
			}

			// Calculate min delta (max rate actor will update), and max delta (slowest rate actor will update)
			const float MinOptimalDelta = 1.0f / Actor->NetUpdateFrequency;									  // Don't go faster than NetUpdateFrequency
			const float MaxOptimalDelta = FMath::Max(1.0f / Actor->MinNetUpdateFrequency, MinOptimalDelta); // Don't go slower than MinNetUpdateFrequency (or NetUpdateFrequency if it's slower)

																											// Interpolate between MinOptimalDelta/MaxOptimalDelta based on how long it's been since this actor actually sent anything
			const float Alpha = FMath::Clamp((LastReplicateDelta - ScaleDownStartTime) / ScaleDownTimeRange, 0.0f, 1.0f);
			ActorInfo->OptimalNetUpdateDelta = FMath::Lerp(MinOptimalDelta, MaxOptimalDelta, Alpha);
		}

		// Setup ActorInfo->NextUpdateTime, which will be the next time this actor will replicate properties to connections
		// NOTE - We don't do this if bPendingNetUpdate is true, since this means we're forcing an update due to at least one connection
		//	that wasn't to replicate previously (due to saturation, etc)
		// NOTE - This also means all other connections will force an update (even if they just updated, we should look into this)
		if (!ActorInfo->bPendingNetUpdate)
		{
			UE_LOG(LogNetTraffic, Log, TEXT("actor %s requesting new net update, time: %2.3f"), *Actor->GetName(), World->TimeSeconds);

			const float NextUpdateDelta = bUseAdapativeNetFrequency ? ActorInfo->OptimalNetUpdateDelta : 1.0f / Actor->NetUpdateFrequency;

			// then set the next update time
			ActorInfo->NextUpdateTime = World->TimeSeconds + FMath::SRand() * ServerTickTime + NextUpdateDelta;

			// and mark when the actor first requested an update
			//@note: using Time because it's compared against UActorChannel.LastUpdateTime which also uses that value
			ActorInfo->LastNetUpdateTime = Time;
		}

		// and clear the pending update flag assuming all clients will be able to consider it
		ActorInfo->bPendingNetUpdate = false;

		// add it to the list to consider below
		// For performance reasons, make sure we don't resize the array. It should already be appropriately sized above!
		ensure(OutConsiderList.Num() < OutConsiderList.Max());
		OutConsiderList.Add(ActorInfo);

		// Call PreReplication on all actors that will be considered
		Actor->CallPreReplication(this);
	}

	for (AActor* Actor : ActorsToRemove)
	{
		GetNetworkObjectList().Remove(Actor);
	}

	// Update stats
	SET_DWORD_STAT(STAT_NumInitiallyDormantActors, NumInitiallyDormant);
	SET_DWORD_STAT(STAT_NumConsideredActors, OutConsiderList.Num());
}

// Returns true if this actor should replicate to *any* of the passed in connections
static FORCEINLINE_DEBUGGABLE bool IsActorRelevantToConnection(const AActor* Actor, const TArray<FNetViewer>& ConnectionViewers)
{
	for (int32 viewerIdx = 0; viewerIdx < ConnectionViewers.Num(); viewerIdx++)
	{
		if (Actor->IsNetRelevantFor(ConnectionViewers[viewerIdx].InViewer, ConnectionViewers[viewerIdx].ViewTarget, ConnectionViewers[viewerIdx].ViewLocation))
		{
			return true;
		}
	}

	return false;
}

// Returns true if this actor is owned by, and should replicate to *any* of the passed in connections
static FORCEINLINE_DEBUGGABLE UNetConnection* IsActorOwnedByAndRelevantToConnection(const AActor* Actor, const TArray<FNetViewer>& ConnectionViewers, bool& bOutHasNullViewTarget)
{
	const AActor* ActorOwner = Actor->GetNetOwner();

	bOutHasNullViewTarget = false;

	for (int i = 0; i < ConnectionViewers.Num(); i++)
	{
		UNetConnection* ViewerConnection = ConnectionViewers[i].Connection;

		if (ViewerConnection->ViewTarget == nullptr)
		{
			bOutHasNullViewTarget = true;
		}

		if (ActorOwner == ViewerConnection->PlayerController ||
			(ViewerConnection->PlayerController && ActorOwner == ViewerConnection->PlayerController->GetPawn()) ||
			(ViewerConnection->ViewTarget && ViewerConnection->ViewTarget->IsRelevancyOwnerFor(Actor, ActorOwner, ViewerConnection->OwningActor)))
		{
			return ViewerConnection;
		}
	}

	return nullptr;
}

// Returns true if this actor is considered dormant (and all properties caught up) to the current connection
static FORCEINLINE_DEBUGGABLE bool IsActorDormant(FNetworkObjectInfo* ActorInfo, const UNetConnection* Connection)
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

int32 UNetDriver::ServerReplicateActors_PrioritizeActors(UNetConnection* Connection, const TArray<FNetViewer>& ConnectionViewers, const TArray<FNetworkObjectInfo*> ConsiderList, const bool bCPUSaturated, FActorPriority*& OutPriorityList, FActorPriority**& OutPriorityActors)
{
	SCOPE_CYCLE_COUNTER(STAT_NetPrioritizeActorsTime);

	// Get list of visible/relevant actors.

	NetTag++;
	Connection->TickCount++;

	// Set up to skip all sent temporary actors
	for (int32 j = 0; j < Connection->SentTemporaries.Num(); j++)
	{
		Connection->SentTemporaries[j]->NetTag = NetTag;
	}

	// Make list of all actors to consider.
	check(World == Connection->OwningActor->GetWorld());

	int32 FinalSortedCount = 0;
	int32 DeletedCount = 0;

	const int32 MaxSortedActors = ConsiderList.Num() + DestroyedStartupOrDormantActors.Num();
	if (MaxSortedActors > 0)
	{
		OutPriorityList = new (FMemStack::Get(), MaxSortedActors) FActorPriority;
		OutPriorityActors = new (FMemStack::Get(), MaxSortedActors) FActorPriority*;

		check(World == Connection->ViewTarget->GetWorld());

		AGameNetworkManager* const NetworkManager = World->NetworkManager;
		const bool bLowNetBandwidth = NetworkManager ? NetworkManager->IsInLowBandwidthMode() : false;

		for (FNetworkObjectInfo* ActorInfo : ConsiderList)
		{
			AActor* Actor = ActorInfo->Actor;

			UActorChannel* Channel = Connection->ActorChannels.FindRef(Actor);

			UNetConnection* PriorityConnection = Connection;

			if (Actor->bOnlyRelevantToOwner)
			{
				// This actor should be owned by a particular connection, see if that connection is the one passed in
				bool bHasNullViewTarget = false;

				PriorityConnection = IsActorOwnedByAndRelevantToConnection(Actor, ConnectionViewers, bHasNullViewTarget);

				if (PriorityConnection == nullptr)
				{
					// Not owned by this connection, if we have a channel, close it, and continue
					// NOTE - We won't close the channel if any connection has a NULL view target.
					//	This is to give all connections a chance to own it
					if (!bHasNullViewTarget && Channel != NULL && Time - Channel->RelevantTime >= RelevantTimeout)
					{
						Channel->Close();
					}

					// This connection doesn't own this actor
					continue;
				}
			}
			else if (CVarSetNetDormancyEnabled.GetValueOnGameThread() != 0)
			{
				// Skip Actor if dormant
				if (IsActorDormant(ActorInfo, Connection))
				{
					continue;
				}

				// See of actor wants to try and go dormant
				if (ShouldActorGoDormant(Actor, ConnectionViewers, Channel, Time, bLowNetBandwidth))
				{
					// Channel is marked to go dormant now once all properties have been replicated (but is not dormant yet)
					Channel->StartBecomingDormant();
				}
			}

			// Skip actor if not relevant and theres no channel already.
			// Historically Relevancy checks were deferred until after prioritization because they were expensive (line traces).
			// Relevancy is now cheap and we are dealing with larger lists of considered actors, so we want to keep the list of
			// prioritized actors low.
			if (!Channel)
			{
				if (!IsLevelInitializedForActor(Actor, Connection))
				{
					// If the level this actor belongs to isn't loaded on client, don't bother sending
					continue;
				}

				if (!IsActorRelevantToConnection(Actor, ConnectionViewers))
				{
					// If not relevant (and we don't have a channel), skip
					continue;
				}
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
		for (auto It = Connection->DestroyedStartupOrDormantActors.CreateIterator(); It; ++It)
		{
			FActorDestructionInfo& DInfo = DestroyedStartupOrDormantActors.FindChecked(*It);
			OutPriorityList[FinalSortedCount] = FActorPriority(Connection, &DInfo, ConnectionViewers);
			OutPriorityActors[FinalSortedCount] = OutPriorityList + FinalSortedCount;
			FinalSortedCount++;
			DeletedCount++;
		}

		// Sort by priority
		Sort(OutPriorityActors, FinalSortedCount, FCompareFActorPriority());
	}

	UE_LOG(LogNetTraffic, Log, TEXT("ServerReplicateActors_PrioritizeActors: Potential %04i ConsiderList %03i FinalSortedCount %03i"), MaxSortedActors, ConsiderList.Num(), FinalSortedCount);

	// Setup stats
	SET_DWORD_STAT(STAT_PrioritizedActors, FinalSortedCount);
	SET_DWORD_STAT(STAT_NumRelevantDeletedActors, DeletedCount);

	return FinalSortedCount;
}

int32 UNetDriver::ServerReplicateActors_ProcessPrioritizedActors(UNetConnection* Connection, const TArray<FNetViewer>& ConnectionViewers, FActorPriority** PriorityActors, const int32 FinalSortedCount, int32& OutUpdated)
{
	if (!Connection->IsNetReady(0))
	{
		// Connection saturated, don't process any actors
		return 0;
	}

	int32 ActorUpdatesThisConnection = 0;
	int32 ActorUpdatesThisConnectionSent = 0;
	int32 FinalRelevantCount = 0;

	for (int32 j = 0; j < FinalSortedCount; j++)
	{
		// Deletion entry
		if (PriorityActors[j]->ActorInfo == NULL && PriorityActors[j]->DestructionInfo)
		{
			// Make sure client has streaming level loaded
			if (PriorityActors[j]->DestructionInfo->StreamingLevelName != NAME_None && !Connection->ClientVisibleLevelNames.Contains(PriorityActors[j]->DestructionInfo->StreamingLevelName))
			{
				// This deletion entry is for an actor in a streaming level the connection doesn't have loaded, so skip it
				continue;
			}

			UActorChannel* Channel = (UActorChannel*)Connection->CreateChannel(CHTYPE_Actor, 1);
			if (Channel)
			{
				FinalRelevantCount++;
				UE_LOG(LogNetTraffic, Log, TEXT("Server replicate actor creating destroy channel for NetGUID <%s,%s> Priority: %d"), *PriorityActors[j]->DestructionInfo->NetGUID.ToString(), *PriorityActors[j]->DestructionInfo->PathName, PriorityActors[j]->Priority);

				Channel->SetChannelActorForDestroy(PriorityActors[j]->DestructionInfo);						   // Send a close bunch on the new channel
				Connection->DestroyedStartupOrDormantActors.Remove(PriorityActors[j]->DestructionInfo->NetGUID); // Remove from connections to-be-destroyed list (close bunch of reliable, so it will make it there)
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
		UActorChannel* Channel = PriorityActors[j]->Channel;
		UE_LOG(LogNetTraffic, Log, TEXT(" Maybe Replicate %s"), *PriorityActors[j]->ActorInfo->Actor->GetName());
		if (!Channel || Channel->Actor) //make sure didn't just close this channel
		{
			AActor* Actor = PriorityActors[j]->ActorInfo->Actor;
			bool bIsRelevant = false;

			const bool bLevelInitializedForActor = IsLevelInitializedForActor(Actor, Connection);

			// only check visibility on already visible actors every 1.0 + 0.5R seconds
			// bTearOff actors should never be checked
			if (bLevelInitializedForActor)
			{
				if (!Actor->bTearOff && (!Channel || Time - Channel->RelevantTime > 1.f))
				{
					if (IsActorRelevantToConnection(Actor, ConnectionViewers))
					{
						bIsRelevant = true;
					}
					else if (DebugRelevantActors)
					{
						LastNonRelevantActors.Add(Actor);
					}
				}
			}
			else
			{
				// Actor is no longer relevant because the world it is/was in is not loaded by client
				// exception: player controllers should never show up here
				UE_LOG(LogNetTraffic, Log, TEXT("- Level not initialized for actor %s"), *Actor->GetName());
			}

			// if the actor is now relevant or was recently relevant
			const bool bIsRecentlyRelevant = bIsRelevant || (Channel && Time - Channel->RelevantTime < RelevantTimeout);

			if (bIsRecentlyRelevant)
			{
				FinalRelevantCount++;

				// Find or create the channel for this actor.
				// we can't create the channel if the client is in a different world than we are
				// or the package map doesn't support the actor's class/archetype (or the actor itself in the case of serializable actors)
				// or it's an editor placed actor and the client hasn't initialized the level it's in
				if (Channel == NULL && GuidCache->SupportsObject(Actor->GetClass()) && GuidCache->SupportsObject(Actor->IsNetStartupActor() ? Actor : Actor->GetArchetype()))
				{
					if (bLevelInitializedForActor)
					{
						// Create a new channel for this actor.
						Channel = (UActorChannel*)Connection->CreateChannel(CHTYPE_Actor, 1);
						if (Channel)
						{
							Channel->SetChannelActor(Actor);
						}
					}
					// if we couldn't replicate it for a reason that should be temporary, and this Actor is updated very infrequently, make sure we update it again soon
					else if (Actor->NetUpdateFrequency < 1.0f)
					{
						UE_LOG(LogNetTraffic, Log, TEXT("Unable to replicate %s"), *Actor->GetName());
						PriorityActors[j]->ActorInfo->NextUpdateTime = Actor->GetWorld()->TimeSeconds + 0.2f * FMath::FRand();
					}
				}

				if (Channel)
				{
					// if it is relevant then mark the channel as relevant for a short amount of time
					if (bIsRelevant)
					{
						Channel->RelevantTime = Time + 0.5f * FMath::SRand();
					}
					// if the channel isn't saturated
					if (Channel->IsNetReady(0))
					{
						// replicate the actor
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

							// Calculate min delta (max rate actor will upate), and max delta (slowest rate actor will update)
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
					else
					{
						UE_LOG(LogNetTraffic, Log, TEXT("- Channel saturated, forcing pending update for %s"), *Actor->GetName());
						// otherwise force this actor to be considered in the next tick again
						Actor->ForceNetUpdate();
					}
					// second check for channel saturation
					if (!Connection->IsNetReady(0))
					{
						// We can bail out now since this connection is saturated, we'll return how far we got though
						SET_DWORD_STAT(STAT_NumReplicatedActorAttempts, ActorUpdatesThisConnection);
						SET_DWORD_STAT(STAT_NumReplicatedActors, ActorUpdatesThisConnectionSent);
						SET_DWORD_STAT(STAT_NumRelevantActors, FinalRelevantCount);
						return j;
					}
				}
			}

			// If the actor wasn't recently relevant, or if it was torn off, close the actor channel if it exists for this connection
			if ((!bIsRecentlyRelevant || Actor->bTearOff) && Channel != NULL)
			{
				// Non startup (map) actors have their channels closed immediately, which destroys them.
				// Startup actors get to keep their channels open.

				// Fixme: this should be a setting
				if (!bLevelInitializedForActor || !Actor->IsNetStartupActor())
				{
					UE_LOG(LogNetTraffic, Log, TEXT("- Closing channel for no longer relevant actor %s"), *Actor->GetName());
					Channel->Close();
				}
			}
		}
	}

	SET_DWORD_STAT(STAT_NumReplicatedActorAttempts, ActorUpdatesThisConnection);
	SET_DWORD_STAT(STAT_NumReplicatedActors, ActorUpdatesThisConnectionSent);
	SET_DWORD_STAT(STAT_NumRelevantActors, FinalRelevantCount);

	return FinalSortedCount;
}
#endif

