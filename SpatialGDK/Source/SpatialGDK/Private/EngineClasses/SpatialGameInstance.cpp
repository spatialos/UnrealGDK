// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialGameInstance.h"

#include "Engine/NetConnection.h"
#include "GeneralProjectSettings.h"
#include "Misc/Guid.h"

#if WITH_EDITOR
#include "Editor/EditorEngine.h"
#include "Settings/LevelEditorPlaySettings.h"
#endif

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPendingNetGame.h"
#include "Interop/Connection/SpatialConnectionManager.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/GlobalStateManager.h"
#include "Interop/SpatialStaticComponentView.h"
#include "Utils/SpatialDebugger.h"
#include "Utils/SpatialLatencyTracer.h"
#include "Utils/SpatialMetrics.h"
#include "Utils/SpatialMetricsDisplay.h"
#include "Utils/SpatialStatics.h"

DEFINE_LOG_CATEGORY(LogSpatialGameInstance);

USpatialGameInstance::USpatialGameInstance()
	: Super()
	, bIsSpatialNetDriverReady(false)
{
}

bool USpatialGameInstance::HasSpatialNetDriver() const
{
	bool bHasSpatialNetDriver = false;

	if (WorldContext != nullptr)
	{
		UWorld* World = GetWorld();
		UNetDriver* NetDriver = GEngine->FindNamedNetDriver(World, NAME_PendingNetDriver);
		bool bShouldDestroyNetDriver = false;

		if (NetDriver == nullptr)
		{
			// If Spatial networking is enabled, override the GameNetDriver with the SpatialNetDriver
			if (GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking())
			{
				if (FNetDriverDefinition* DriverDefinition =
						GEngine->NetDriverDefinitions.FindByPredicate([](const FNetDriverDefinition& CurDef) {
							return CurDef.DefName == NAME_GameNetDriver;
						}))
				{
					DriverDefinition->DriverClassName = DriverDefinition->DriverClassNameFallback =
						TEXT("/Script/SpatialGDK.SpatialNetDriver");
				}
			}

			bShouldDestroyNetDriver = GEngine->CreateNamedNetDriver(World, NAME_PendingNetDriver, NAME_GameNetDriver);
			NetDriver = GEngine->FindNamedNetDriver(World, NAME_PendingNetDriver);
		}

		if (NetDriver != nullptr)
		{
			bHasSpatialNetDriver = NetDriver->IsA<USpatialNetDriver>();

			if (bShouldDestroyNetDriver)
			{
				GEngine->DestroyNamedNetDriver(World, NAME_PendingNetDriver);
			}
		}
	}

	if (GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking() && !bHasSpatialNetDriver)
	{
		UE_LOG(LogSpatialGameInstance, Error,
			   TEXT("Could not find SpatialNetDriver even though Spatial networking is switched on! "
					"Please make sure you set up the net driver definitions as specified in the porting "
					"guide and that you don't override the main net driver."));
	}

	return bHasSpatialNetDriver;
}

void USpatialGameInstance::CreateNewSpatialConnectionManager()
{
	SpatialConnectionManager = NewObject<USpatialConnectionManager>(this);

	GlobalStateManager = NewObject<UGlobalStateManager>();
	StaticComponentView = NewObject<USpatialStaticComponentView>();
}

void USpatialGameInstance::DestroySpatialConnectionManager()
{
	if (SpatialConnectionManager != nullptr)
	{
		SpatialConnectionManager->DestroyConnection();
		SpatialConnectionManager = nullptr;
	}

	if (GlobalStateManager != nullptr)
	{
		GlobalStateManager->ConditionalBeginDestroy();
		GlobalStateManager = nullptr;
	}

	if (StaticComponentView != nullptr)
	{
		StaticComponentView->ConditionalBeginDestroy();
		StaticComponentView = nullptr;
	}
}

#if WITH_EDITOR
FGameInstancePIEResult USpatialGameInstance::StartPlayInEditorGameInstance(ULocalPlayer* LocalPlayer,
																		   const FGameInstancePIEParameters& Params)
{
	SpatialWorkerType = Params.SpatialWorkerType;
	bIsSimulatedPlayer = Params.bIsSimulatedPlayer;

	StartSpatialConnection();
	return Super::StartPlayInEditorGameInstance(LocalPlayer, Params);
}
#endif

void USpatialGameInstance::StartSpatialConnection()
{
	if (HasSpatialNetDriver())
	{
		// If we are using spatial networking then prepare a spatial connection.
		TryInjectSpatialLocatorIntoCommandLine();
		CreateNewSpatialConnectionManager();
	}
#if TRACE_LIB_ACTIVE
	else
	{
		// In native, setup worker name here as we don't get a HandleOnConnected() callback
		FString WorkerName =
			FString::Printf(TEXT("%s:%s"), *SpatialWorkerType.ToString(), *FGuid::NewGuid().ToString(EGuidFormats::Digits));
		SpatialLatencyTracer->SetWorkerId(WorkerName);
	}
#endif
}

void USpatialGameInstance::TryInjectSpatialLocatorIntoCommandLine()
{
	if (!HasPreviouslyConnectedToSpatial())
	{
		SetHasPreviouslyConnectedToSpatial();
		// Native Unreal creates a NetDriver and attempts to automatically connect if a Host is specified as the first commandline argument.
		// Since the SpatialOS Launcher does not specify this, we need to check for a locator loginToken to allow automatic connection to
		// provide parity with native.

		// Initialize a locator configuration which will parse command line arguments.
		FLocatorConfig LocatorConfig;
		if (LocatorConfig.TryLoadCommandLineArgs())
		{
			// Modify the commandline args to have a Host IP to force a NetDriver to be used.
			const TCHAR* CommandLineArgs = FCommandLine::Get();

			FString NewCommandLineArgs = LocatorConfig.LocatorHost + TEXT(" ");
			NewCommandLineArgs.Append(FString(CommandLineArgs));

			FCommandLine::Set(*NewCommandLineArgs);
		}
	}
}

void USpatialGameInstance::StartGameInstance()
{
	if (GetDefault<USpatialGDKSettings>()->GetPreventClientCloudDeploymentAutoConnect())
	{
		DisableShouldConnectUsingCommandLineArgs();
	}
	else
	{
		StartSpatialConnection();
	}

	Super::StartGameInstance();
}

bool USpatialGameInstance::ProcessConsoleExec(const TCHAR* Cmd, FOutputDevice& Ar, UObject* Executor)
{
	if (Super::ProcessConsoleExec(Cmd, Ar, Executor))
	{
		return true;
	}

	if (const UWorld* World = GetWorld())
	{
		if (const USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(World->GetNetDriver()))
		{
			if (NetDriver->SpatialMetrics && NetDriver->SpatialMetrics->ProcessConsoleExec(Cmd, Ar, Executor))
			{
				return true;
			}

			if (NetDriver->SpatialMetricsDisplay && NetDriver->SpatialMetricsDisplay->ProcessConsoleExec(Cmd, Ar, Executor))
			{
				return true;
			}

			if (NetDriver->SpatialDebugger && NetDriver->SpatialDebugger->ProcessConsoleExec(Cmd, Ar, Executor))
			{
				return true;
			}
		}
	}

	return false;
}

void USpatialGameInstance::Init()
{
	Super::Init();

	SpatialLatencyTracer = NewObject<USpatialLatencyTracer>(this);

	if (HasSpatialNetDriver())
	{
		FWorldDelegates::LevelInitializedNetworkActors.AddUObject(this, &USpatialGameInstance::OnLevelInitializedNetworkActors);
	}
}

void USpatialGameInstance::HandleOnConnected()
{
	UE_LOG(LogSpatialGameInstance, Log, TEXT("Successfully connected to SpatialOS"));
	SpatialWorkerId = SpatialConnectionManager->GetWorkerConnection()->GetWorkerId();
#if TRACE_LIB_ACTIVE
	SpatialLatencyTracer->SetWorkerId(SpatialWorkerId);

	USpatialWorkerConnection* WorkerConnection = SpatialConnectionManager->GetWorkerConnection();
	WorkerConnection->OnEnqueueMessage.AddUObject(SpatialLatencyTracer, &USpatialLatencyTracer::OnEnqueueMessage);
	WorkerConnection->OnDequeueMessage.AddUObject(SpatialLatencyTracer, &USpatialLatencyTracer::OnDequeueMessage);
#endif

	OnSpatialConnected.Broadcast();
}

void USpatialGameInstance::CleanupCachedLevelsAfterConnection()
{
	// Cleanup any actors which were created during level load.
	UWorld* World = GetWorld();
	check(World != nullptr);
	for (ULevel* Level : CachedLevelsForNetworkIntialize)
	{
		if (World->ContainsLevel(Level))
		{
			CleanupLevelInitializedNetworkActors(Level);
		}
	}
	CachedLevelsForNetworkIntialize.Empty();
}

void USpatialGameInstance::HandleOnConnectionFailed(const FString& Reason)
{
	UE_LOG(LogSpatialGameInstance, Error, TEXT("Could not connect to SpatialOS. Reason: %s"), *Reason);
#if TRACE_LIB_ACTIVE
	SpatialLatencyTracer->ResetWorkerId();
#endif
	OnSpatialConnectionFailed.Broadcast(Reason);
}

void USpatialGameInstance::HandleOnPlayerSpawnFailed(const FString& Reason)
{
	UE_LOG(LogSpatialGameInstance, Error, TEXT("Could not spawn the local player on SpatialOS. Reason: %s"), *Reason);
	OnSpatialPlayerSpawnFailed.Broadcast(Reason);
}

void USpatialGameInstance::OnLevelInitializedNetworkActors(ULevel* LoadedLevel, UWorld* OwningWorld)
{
	if (OwningWorld != GetWorld() || !OwningWorld->IsServer() || !GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking()
		|| (OwningWorld->WorldType != EWorldType::PIE && OwningWorld->WorldType != EWorldType::Game
			&& OwningWorld->WorldType != EWorldType::GamePreview))
	{
		// We only want to do something if this is the correct process and we are on a spatial server, and we are in-game
		return;
	}

	if (bIsSpatialNetDriverReady)
	{
		CleanupLevelInitializedNetworkActors(LoadedLevel);
	}
	else
	{
		CachedLevelsForNetworkIntialize.Add(LoadedLevel);
	}
}

void USpatialGameInstance::CleanupLevelInitializedNetworkActors(ULevel* LoadedLevel)
{
	bIsSpatialNetDriverReady = true;
	for (int32 ActorIndex = 0; ActorIndex < LoadedLevel->Actors.Num(); ActorIndex++)
	{
		AActor* Actor = LoadedLevel->Actors[ActorIndex];
		if (Actor == nullptr)
		{
			continue;
		}

		if (USpatialStatics::IsSpatialOffloadingEnabled(GetWorld()))
		{
			if (!USpatialStatics::IsActorGroupOwnerForActor(Actor))
			{
				if (!Actor->bNetLoadOnNonAuthServer)
				{
					Actor->Destroy(true);
				}
				else
				{
					UE_LOG(LogSpatialGameInstance, Verbose, TEXT("This worker %s is not the owner of startup actor %s, exchanging Roles"),
						   *GetPathNameSafe(Actor));
					ENetRole Temp = Actor->Role;
					Actor->Role = Actor->RemoteRole;
					Actor->RemoteRole = Temp;
				}
			}
		}
		else
		{
			if (Actor->GetIsReplicated())
			{
				// Always wait for authority to be delegated down from SpatialOS, if not using offloading
				Actor->Role = ROLE_SimulatedProxy;
				Actor->RemoteRole = ROLE_Authority;
			}
		}
	}
}
