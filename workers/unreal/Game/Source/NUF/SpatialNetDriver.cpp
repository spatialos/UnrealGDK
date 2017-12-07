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

int32 USpatialNetDriver::ServerReplicateActors(float DeltaSeconds)
{
	int32 RetVal = Super::ServerReplicateActors(DeltaSeconds);

	if (!ShadowActorPipelineBlock)
	{
		return RetVal;
	}

#if WITH_SERVER_CODE
	/*
	for (int32 ClientId = 0; ClientId < ClientConnections.Num(); ClientId++)
	{
		UNetConnection* NetConnection = ClientConnections[ClientId];
		for (int32 ChannelId = 0; ChannelId < NetConnection->OpenChannels.Num(); ChannelId++)
		{
			UActorChannel* ActorChannel = Cast<UActorChannel>(NetConnection->OpenChannels[ChannelId]);
			if (!ActorChannel)
			{
				continue;
			}

			// TODO: Remove this once we are using our entity pipeline.
			if (!ActorChannel->Actor->IsA<ACharacter>())
			{
				continue;
			}

			// Get FRepState.
			auto RepData = ActorChannel->ActorReplicator;
			FRepState* RepState = RepData->RepState;
			if (!RepState)
			{
				continue;
			}

			// Get entity ID.
			// TODO: Replace with EntityId from registry corresponding to this replicated actor.
			FEntityId EntityId = 2;//EntityRegistry->GetEntityIdFromActor(ActorChannel->Actor);
			if (EntityId == FEntityId())
			{
				continue;
			}

			// Get shadow actor.
			ASpatialShadowActor* ShadowActor = ShadowActorPipelineBlock->GetShadowActor(EntityId);
			if (!ShadowActor)
			{
				UE_LOG(LogTemp, Warning, TEXT("Actor channel has no corresponding shadow actor. That means the entity hasn't been checked out yet."));
				continue;
			}

			// Write changed properties to SpatialOS.
			auto RepLayout = RepState->RepLayout;
			for (int k = RepState->HistoryStart; k <= RepState->HistoryEnd; k++)
			{
				auto Changed = RepState->ChangeHistory[k].Changed;
				if (Changed.Num() > 0)
				{
					for (int idx = 0; idx < Changed.Num(); idx++)
					{
						if (Changed[idx] == 0)
						{
							continue;
						}
						int CmdIndex = Changed[idx] - 1;

						// Ignore dynamic arrays.
						auto Type = RepLayout->Cmds[CmdIndex].Type;
						if (Type == REPCMD_Return || Type == REPCMD_DynamicArray)
						{
							continue;
						}

						UProperty* Property = RepLayout->Cmds[CmdIndex].Property;
						UProperty* ParentProperty = RepLayout->Parents[RepLayout->Cmds[CmdIndex].ParentIndex].Property;
						//ApplyUpdateToSpatial_Character(ActorChannel->Actor, CmdIndex, ParentProperty, Property, ShadowActor->ReplicatedData);

						FString ChangedProp = Property->GetNameCPP();
						UE_LOG(LogTemp, Warning, TEXT("Actor: %s, cmd %s"), *GetNameSafe(ActorChannel->Actor), *ChangedProp);
					}
				}
			}
		}
	}
	*/
#endif
	return RetVal;
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
