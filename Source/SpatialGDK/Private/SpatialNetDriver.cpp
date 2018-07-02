// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialNetDriver.h"

#include "Engine/ActorChannel.h"
#include "Engine/ChildConnection.h"
#include "Engine/NetworkObjectList.h"
#include "EngineGlobals.h"
#include "EntityPipeline.h"
#include "EntityRegistry.h"
#include "GameFramework/GameNetworkManager.h"
#include "Net/DataReplication.h"
#include "Net/RepLayout.h"
#include "SocketSubsystem.h"
#include "SpatialActorChannel.h"
#include "SpatialConstants.h"
#include "SpatialInteropPipelineBlock.h"
#include "SpatialNetConnection.h"
#include "SpatialOS.h"
#include "SpatialPackageMapClient.h"
#include "SpatialPendingNetGame.h"

#define ENTITY_BLUEPRINTS_FOLDER "/Game/EntityBlueprints"

DEFINE_LOG_CATEGORY(LogSpatialOSNetDriver);

bool USpatialNetDriver::InitBase(bool bInitAsClient, FNetworkNotify* InNotify, const FURL& URL, bool bReuseAddressAndPort, FString& Error)
{
	if (!Super::InitBase(bInitAsClient, InNotify, URL, bReuseAddressAndPort, Error))
	{
		return false;
	}

	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &USpatialNetDriver::OnMapLoaded);

	// Make absolutely sure that the actor channel that we are using is our Spatial actor channel
	ChannelClasses[CHTYPE_Actor] = USpatialActorChannel::StaticClass();

	// Create SpatialOS instance and setup callbacks.
	SpatialOSInstance = NewObject<USpatialOS>(this);
	SpatialOSInstance->OnConnectedDelegate.AddDynamic(this, &USpatialNetDriver::OnSpatialOSConnected);
	SpatialOSInstance->OnConnectionFailedDelegate.AddDynamic(this, &USpatialNetDriver::OnSpatialOSConnectFailed);
	SpatialOSInstance->OnDisconnectedDelegate.AddDynamic(this, &USpatialNetDriver::OnSpatialOSDisconnected);
	SpatialOutputDevice = MakeUnique<FSpatialOutputDevice>(SpatialOSInstance, TEXT("Unreal"));

	// Set up the worker config.
	// todo-giray: Give this the correct value
	WorkerConfig.Networking.UseExternalIp = false;
	WorkerConfig.SpatialGDKApplication.WorkerPlatform = bInitAsClient ? TEXT("UnrealClient") : TEXT("UnrealWorker");

	// We do this here straight away to trigger LoadMap.
	if (bInitAsClient)
	{
		FWorldContext* WorldContext = GEngine->GetWorldContextFromPendingNetGameNetDriver(this);
		check(WorldContext);

		// Here we need to fake a few things to start ticking the level travel on client.
		if (WorldContext->PendingNetGame)
		{
			WorldContext->PendingNetGame->bSuccessfullyConnected = true;
			WorldContext->PendingNetGame->bSentJoinRequest = false;
		}
	}
	else
	{
		// The server should already have a world.
		OnMapLoaded(GetWorld());
	}

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

void USpatialNetDriver::OnMapLoaded(UWorld* LoadedWorld)
{
	if (LoadedWorld->GetNetDriver() != this)
	{
		// In PIE, if we have more than 2 clients, then OnMapLoaded is going to be triggered once each client loads the world.
		// As the delegate is a global variable, it triggers all 3 USpatialNetDriver::OnMapLoaded callbacks. As a result, we should
		// make sure that the net driver of this world is in fact us.
		return;
	}

	UE_LOG(LogSpatialOSNetDriver, Log, TEXT("Loaded Map %s. Connecting to SpatialOS."), *LoadedWorld->GetName());

	checkf(!SpatialOSInstance->IsConnected(), TEXT("SpatialOS should not be connected already. This is probably because we attempted to travel to a different level, which current isn't supported."));

	// Set the timer manager.
	TimerManager = &LoadedWorld->GetTimerManager();

	// Connect to SpatialOS.
	SpatialOSInstance->ApplyConfiguration(WorkerConfig);
	SpatialOSInstance->Connect();

	// Set up manager objects.
	EntityRegistry = NewObject<UEntityRegistry>(this);
	Interop = NewObject<USpatialInterop>(this);
}

void USpatialNetDriver::OnSpatialOSConnected()
{
	UE_LOG(LogSpatialOSNetDriver, Log, TEXT("Connected to SpatialOS."));

	InteropPipelineBlock = NewObject<USpatialInteropPipelineBlock>();
	InteropPipelineBlock->Init(EntityRegistry, this, GetWorld());
	SpatialOSInstance->GetEntityPipeline()->AddBlock(InteropPipelineBlock);

	TArray<FString> BlueprintPaths;
	BlueprintPaths.Add(TEXT(ENTITY_BLUEPRINTS_FOLDER));

	EntityRegistry->RegisterEntityBlueprints(BlueprintPaths);

	// Each connection stores a URL with various optional settings (host, port, map, netspeed...)
	// We currently don't make use of any of these as some are meaningless in a SpatialOS world, and some are less of a priority.
	// So for now we just give the connection a dummy url, might change in the future.
	FURL DummyURL;

	// If we're the client, we can now ask the server to spawn our controller.

	// If we're the server, we will spawn the special Spatial connection that will route all updates to SpatialOS.
	// There may be more than one of these connections in the future for different replication conditions.
	if (ServerConnection)
	{
		// Send the player spawn commands with retries
		PlayerSpawner.RequestPlayer(SpatialOSInstance, TimerManager, DummyURL);
	}
	else
	{
		USpatialNetConnection* Connection = NewObject<USpatialNetConnection>(GetTransientPackage(), NetConnectionClass);
		check(Connection);

		ISocketSubsystem* SocketSubsystem = GetSocketSubsystem();
		TSharedRef<FInternetAddr> FromAddr = SocketSubsystem->CreateInternetAddr();

		Connection->InitRemoteConnection(this, nullptr, DummyURL, *FromAddr, USOCK_Open);
		Notify->NotifyAcceptedConnection(Connection);
		Connection->bReliableSpatialConnection = true;
		AddClientConnection(Connection);
		//Since this is not a "real" client connection, we immediately pretend that it is fully logged on.
		Connection->SetClientLoginState(EClientLoginState::Welcomed);
	}

	Interop->Init(SpatialOSInstance, this, TimerManager);
}

void USpatialNetDriver::OnSpatialOSDisconnected(const FString& Reason)
{
	UE_LOG(LogSpatialOSNetDriver, Warning, TEXT("Disconnected from SpatialOS. Reason: %s"), *Reason);
}

void USpatialNetDriver::OnSpatialOSConnectFailed(const FString& Reason)
{
	UE_LOG(LogSpatialOSNetDriver, Error, TEXT("Could not connect to SpatialOS. Reason: %s"), *Reason);
}

bool USpatialNetDriver::IsLevelInitializedForActor(const AActor* InActor, const UNetConnection* InConnection) const
{
	//In our case, the connection is not specific to a client. Thus, it's not relevant whether the level is initialized.
	return true;
}

//SpatialGDK: Functions in the ifdef block below are modified versions of the UNetDriver:: implementations.
#if WITH_SERVER_CODE

// Returns true if this actor should replicate to *any* of the passed in connections
static FORCEINLINE_DEBUGGABLE bool IsActorRelevantToConnection(const AActor* Actor, const TArray<FNetViewer>& ConnectionViewers)
{
	//SpatialGDK: Currently we're just returning true as a worker replicates all the known actors in our design.
	// We might make some exceptions in the future, so keeping this function.
	return true;
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

int32 USpatialNetDriver::ServerReplicateActors_PrepConnections(const float DeltaSeconds)
{
	int32 NumClientsToTick = ClientConnections.Num();

	bool bFoundReadyConnection = false;

	for (int32 ConnIdx = 0; ConnIdx < ClientConnections.Num(); ConnIdx++)
	{
		USpatialNetConnection* Connection = Cast<USpatialNetConnection>(ClientConnections[ConnIdx]);
		check(Connection);
		check(Connection->State == USOCK_Pending || Connection->State == USOCK_Open || Connection->State == USOCK_Closed);
		checkSlow(Connection->GetUChildConnection() == NULL);

		// Handle not ready channels.
		//@note: we cannot add Connection->IsNetReady(0) here to check for saturation, as if that's the case we still want to figure out the list of relevant actors
		//			to reset their NetUpdateTime so that they will get sent as soon as the connection is no longer saturated
		AActor* OwningActor = Connection->OwningActor;

		//SpatialGDK: We allow a connection without an owner to process if it's meant to be the connection to the fake SpatialOS client.
		if ((Connection->bReliableSpatialConnection || OwningActor != NULL) && Connection->State == USOCK_Open && (Connection->Driver->Time - Connection->LastReceiveTime < 1.5f))
		{
			check(Connection->bReliableSpatialConnection || World == OwningActor->GetWorld());

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

int32 USpatialNetDriver::ServerReplicateActors_PrioritizeActors(UNetConnection* Connection, const TArray<FNetViewer>& ConnectionViewers, const TArray<FNetworkObjectInfo*> ConsiderList, const bool bCPUSaturated, FActorPriority*& OutPriorityList, FActorPriority**& OutPriorityActors)
{
	// Get list of visible/relevant actors.

	NetTag++;
	Connection->TickCount++;

	// Set up to skip all sent temporary actors
	for (int32 j = 0; j < Connection->SentTemporaries.Num(); j++)
	{
		Connection->SentTemporaries[j]->NetTag = NetTag;
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

			UActorChannel* Channel = Connection->ActorChannels.FindRef(Actor);

			UNetConnection* PriorityConnection = Connection;

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

			// SpatialGDK: This actor should only be replicated if GetNetworkConnection() matches this connection. However, if this actor doesn't have a connection
			// (which implies that it's owned by the server rather than a client), then it should fall back to the "catch all" SpatialOS connection which is
			// ClientConnections[0]. The below condition means that each actor should only be replicated once, unless "ClientConnections" contain duplicates,
			// which should never happen.
			UNetConnection* ActorConnection = Actor->GetNetConnection();
			if (ActorConnection != Connection)
			{
				if (ActorConnection == nullptr && Connection == ClientConnections[0])
				{
					UE_LOG(LogSpatialOSNetDriver, Verbose, TEXT("Actor %s will be replicated on the catch-all connection"), *Actor->GetName());
				}
				else
				{
					continue;
				}
			}
			else
			{
				UE_LOG(LogSpatialOSNetDriver, Verbose, TEXT("Actor %s will be replicated on the connection %s"), *Actor->GetName(), *Connection->GetName());
			}

			//SpatialGDK: Here, Unreal does initial relevancy checking and level load checking.
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

	return FinalSortedCount;
}

int32 USpatialNetDriver::ServerReplicateActors_ProcessPrioritizedActors(UNetConnection* Connection, const TArray<FNetViewer>& ConnectionViewers, FActorPriority** PriorityActors, const int32 FinalSortedCount, int32& OutUpdated)
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
		USpatialActorChannel* Channel = Cast<USpatialActorChannel>(PriorityActors[j]->Channel);
		UE_LOG(LogNetTraffic, Log, TEXT(" Maybe Replicate %s"), *PriorityActors[j]->ActorInfo->Actor->GetName());
		if (!Channel || Channel->Actor) //make sure didn't just close this channel
		{
			AActor* Actor = PriorityActors[j]->ActorInfo->Actor;
			bool bIsRelevant = false;

			//SpatialGDK: Here, Unreal would check (again) whether an actor is relevant. Removed such checks.
			// only check visibility on already visible actors every 1.0 + 0.5R seconds
			// bTearOff actors should never be checked
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
					// Create a new channel for this actor.
					Channel = (USpatialActorChannel*)Connection->CreateChannel(CHTYPE_Actor, 1);
					if (Channel)
					{
						const USpatialTypeBinding* TypeBinding = Interop->GetTypeBindingByClass(Actor->GetClass());
						if (TypeBinding == nullptr)
						{
							Channel->bCoreActor = false;
						}

						Channel->SetChannelActor(Actor);
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

					// second check for channel saturation
					if (!Connection->IsNetReady(0))
					{
						// We can bail out now since this connection is saturated, we'll return how far we got though
						return j;
					}
				}
			}

			// If the actor wasn't recently relevant, or if it was torn off, close the actor channel if it exists for this connection
			if ((!bIsRecentlyRelevant || Actor->bTearOff) && Channel != NULL)
			{
				// Non startup (map) actors have their channels closed immediately, which destroys them.
				// Startup actors get to keep their channels open.

				if (!Actor->IsNetStartupActor())
				{
					UE_LOG(LogNetTraffic, Log, TEXT("- Closing channel for no longer relevant actor %s"), *Actor->GetName());
					Channel->Close();
				}
			}
		}
	}

	return FinalSortedCount;
}
#endif

//SpatialGDK: This is a modified and simplified version of UNetDriver::ServerReplicateActors.
// In our implementation, connections on the server do not represent clients. They represent direct connections to SpatialOS.
// For this reason, things like ready checks, acks, throttling based on number of updated connections, interest management are irrelevant at this level.
int32 USpatialNetDriver::ServerReplicateActors(float DeltaSeconds)
{
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

	//SpatialGDK: This is a formality as there is at least one "perfect" Spatial connection in our design.
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

	TArray<FNetworkObjectInfo*> ConsiderList;
	ConsiderList.Reserve(GetNetworkObjectList().GetActiveObjects().Num());

	// Build the consider list (actors that are ready to replicate)
	ServerReplicateActors_BuildConsiderList(ConsiderList, ServerTickTime);
	FMemMark Mark(FMemStack::Get());

	for (int32 i = 0; i < ClientConnections.Num(); i++)
	{
		USpatialNetConnection* Connection = Cast<USpatialNetConnection>(ClientConnections[i]);
		check(Connection);

		// if this client shouldn't be ticked this frame
		if (i >= NumClientsToTick)
		{
			//SpatialGDK: This should not really happen in our case because we only replicate to SpatialOS and not to individual clients. Leaving the code here just in case.

			//UE_LOG(LogNet, Log, TEXT("skipping update to %s"),*Connection->GetName());
			// then mark each considered actor as bPendingNetUpdate so that they will be considered again the next frame when the connection is actually ticked
			for (int32 ConsiderIdx = 0; ConsiderIdx < ConsiderList.Num(); ConsiderIdx++)
			{
				AActor *Actor = ConsiderList[ConsiderIdx]->Actor;
				// if the actor hasn't already been flagged by another connection,
				if (Actor != NULL && !ConsiderList[ConsiderIdx]->bPendingNetUpdate)
				{
					// find the channel
					UActorChannel *Channel = Connection->ActorChannels.FindRef(Actor);
					// and if the channel last update time doesn't match the last net update time for the actor
					if (Channel != NULL && Channel->LastUpdateTime < ConsiderList[ConsiderIdx]->LastNetUpdateTime)
					{
						//UE_LOG(LogNet, Log, TEXT("flagging %s for a future update"),*Actor->GetName());
						// flag it for a pending update
						ConsiderList[ConsiderIdx]->bPendingNetUpdate = true;
					}
				}
			}
			// clear the time sensitive flag to avoid sending an extra packet to this connection
			Connection->TimeSensitive = false;
		}
		else if (Connection->bReliableSpatialConnection || Connection->ViewTarget)
		{
			// Make a list of viewers this connection should consider (this connection and children of this connection)
			TArray<FNetViewer>& ConnectionViewers = WorldSettings->ReplicationViewers;

			if (Connection->ViewTarget)
			{
				ConnectionViewers.Reset();
				new(ConnectionViewers)FNetViewer(Connection, DeltaSeconds);
				for (int32 ViewerIndex = 0; ViewerIndex < Connection->Children.Num(); ViewerIndex++)
				{
					if (Connection->Children[ViewerIndex]->ViewTarget != NULL)
					{
						new(ConnectionViewers)FNetViewer(Connection->Children[ViewerIndex], DeltaSeconds);
					}
				}
			}

			FMemMark RelevantActorMark(FMemStack::Get());

			FActorPriority* PriorityList = NULL;
			FActorPriority** PriorityActors = NULL;

			// Get a sorted list of actors for this connection
			const int32 FinalSortedCount = ServerReplicateActors_PrioritizeActors(Connection, ConnectionViewers, ConsiderList, bCPUSaturated, PriorityList, PriorityActors);

			// Process the sorted list of actors for this connection
			const int32 LastProcessedActor = ServerReplicateActors_ProcessPrioritizedActors(Connection, ConnectionViewers, PriorityActors, FinalSortedCount, Updated);

			// relevant actors that could not be processed this frame are marked to be considered for next frame
			for (int32 k = LastProcessedActor; k < FinalSortedCount; k++)
			{
				if (!PriorityActors[k]->ActorInfo)
				{
					// A deletion entry, skip it because we dont have anywhere to store a 'better give higher priority next time'
					continue;
				}

				AActor* Actor = PriorityActors[k]->ActorInfo->Actor;

				UActorChannel* Channel = PriorityActors[k]->Channel;

				UE_LOG(LogNetTraffic, Verbose, TEXT("Saturated. %s"), *Actor->GetName());
				if (Channel != NULL && Time - Channel->RelevantTime <= 1.f)
				{
					UE_LOG(LogNetTraffic, Log, TEXT(" Saturated. Mark %s NetUpdateTime to be checked for next tick"), *Actor->GetName());
					PriorityActors[k]->ActorInfo->bPendingNetUpdate = true;
				}
				else if (IsActorRelevantToConnection(Actor, ConnectionViewers))
				{
					// If this actor was relevant but didn't get processed, force another update for next frame
					UE_LOG(LogNetTraffic, Log, TEXT(" Saturated. Mark %s NetUpdateTime to be checked for next tick"), *Actor->GetName());
					PriorityActors[k]->ActorInfo->bPendingNetUpdate = true;
					if (Channel != NULL)
					{
						Channel->RelevantTime = Time + 0.5f * FMath::SRand();
					}
				}
			}
			RelevantActorMark.Pop();
			ConnectionViewers.Reset();
		}
	}

	// shuffle the list of connections if not all connections were ticked
	if (NumClientsToTick < ClientConnections.Num())
	{
		int32 NumConnectionsToMove = NumClientsToTick;
		while (NumConnectionsToMove > 0)
		{
			// move all the ticked connections to the end of the list so that the other connections are considered first for the next frame
			UNetConnection *Connection = ClientConnections[0];
			ClientConnections.RemoveAt(0, 1);
			ClientConnections.Add(Connection);
			NumConnectionsToMove--;
		}
	}
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

	if (SpatialOSInstance != nullptr && SpatialOSInstance->GetEntityPipeline() != nullptr)
	{
		SpatialOSInstance->ProcessOps();
		SpatialOSInstance->GetEntityPipeline()->ProcessOps(SpatialOSInstance->GetView(), SpatialOSInstance->GetConnection(), GetWorld());
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
	USpatialNetConnection* Connection = ServerConnection ? Cast<USpatialNetConnection>(ServerConnection) : GetSpatialOSNetConnection();
	if (!Connection)
	{
		UE_LOG(LogSpatialOSNetDriver, Error, TEXT("Attempted to call ProcessRemoteFunction before connection was establised"))
		return;
	}

	// The RPC might have been called by an actor directly, or by a subobject on that actor (e.g. UCharacterMovementComponent).
	UObject* CallingObject = SubObject ? SubObject : Actor;

	if (Function->FunctionFlags & FUNC_Net)
	{
		Interop->InvokeRPC(CallingObject, Function, Parameters);
	}

	// Shouldn't need to call Super here as we've replaced pretty much all the functionality in UIpNetDriver
	//UIpNetDriver::ProcessRemoteFunction(Actor, Function, Parameters, OutParms, Stack, SubObject);
}

void USpatialNetDriver::TickFlush(float DeltaTime)
{
	// Super::TickFlush() will not call ReplicateActors() because Spatial connections have InternalAck set to true.
	// In our case, our Spatial actor interop is triggered through ReplicateActors() so we want to call it regardless.

#if USE_SERVER_PERF_COUNTERS
	double ServerReplicateActorsTimeMs = 0.0f;
#endif // USE_SERVER_PERF_COUNTERS
	if (IsServer() && ClientConnections.Num() > 0)
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

USpatialNetConnection* USpatialNetDriver::AcceptNewPlayer(const FURL& InUrl, bool bExistingPlayer)
{
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

	// Set up the net ID for this player.
	const TCHAR* ClientWorkerIdOption = InUrl.GetOption(TEXT("workerId"), nullptr);
	check(ClientWorkerIdOption);
	FString ClientWorkerId(ClientWorkerIdOption);
	ClientWorkerId = ClientWorkerId.Mid(1); // Trim off the = at the beginning.
	FUniqueNetIdRepl WorkerId(TSharedPtr<FSpatialWorkerUniqueNetId>(new FSpatialWorkerUniqueNetId(ClientWorkerId)));

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
		GameMode->PreLogin(Tmp, Connection->LowLevelGetRemoteAddress(), WorkerId, ErrorMsg);
	}

	if (!ErrorMsg.IsEmpty())
	{
		UE_LOG(LogNet, Log, TEXT("PreLogin failure: %s"), *ErrorMsg);
		bOk = false;
	}

	if (bOk)
	{
		FString LevelName = GetWorld()->GetCurrentLevel()->GetOutermost()->GetName();
		Connection->SetClientWorldPackageName(GetWorld()->GetCurrentLevel()->GetOutermost()->GetFName());

		FString GameName;
		FString RedirectURL;
		if (GameMode)
		{
			GameName = GameMode->GetClass()->GetPathName();
			GameMode->GameWelcomePlayer(Connection, RedirectURL);
		}

		if (!bExistingPlayer)
		{
			Connection->PlayerController = World->SpawnPlayActor(Connection, ROLE_AutonomousProxy, InUrl, WorkerId, ErrorMsg);
		}
		else
		{
			// Most of this is taken from "World->SpawnPlayActor", excluding the logic to spawn a pawn which happens during
			// GameMode->PostLogin(...).
			APlayerController* NewPlayerController = GameMode->SpawnPlayerController(ROLE_AutonomousProxy, FVector::ZeroVector, FRotator::ZeroRotator);

			// Destroy the player state (as we'll be replacing it anyway).
			NewPlayerController->CleanupPlayerState();

			// Possess the newly-spawned player.
			NewPlayerController->NetPlayerIndex = 0;
			NewPlayerController->Role = ROLE_Authority;
			NewPlayerController->SetReplicates(true);
			NewPlayerController->SetAutonomousProxy(true);
			NewPlayerController->SetPlayer(Connection);
			// We explicitly don't call GameMode->PostLogin(NewPlayerController) here, to avoid the engine restarting the player.
			// TODO: Should we call AGameSession::PostLogin?
			// TODO: Should we trigger to blueprints that a player has "joined" via GameMode->K2_PostLogin(Connection)?

			Connection->PlayerController = NewPlayerController;
		}

		if (Connection->PlayerController == NULL)
		{
			// Failed to connect.
			UE_LOG(LogNet, Log, TEXT("Join failure: %s"), *ErrorMsg);
			Connection->FlushNet(true);
			bOk = false;
		}
		else
		{
			//todo-giray: Client travel needs to be handled here.
		}
	}

	if (!bOk)
	{
		// TODO(David): Destroy connection.
	}

	return bOk ? Connection : nullptr;
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
