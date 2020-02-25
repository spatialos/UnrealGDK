// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialGameInstance.h"

#include "Engine/Engine.h"
#include "Engine/NetConnection.h"
#include "GeneralProjectSettings.h"
#if WITH_EDITOR
#include "Editor/EditorEngine.h"
#include "Settings/LevelEditorPlaySettings.h"
#endif

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPendingNetGame.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/GlobalStateManager.h"
#include "Interop/SpatialStaticComponentView.h"
#include "Utils/SpatialDebugger.h"
#include "Utils/SpatialLatencyTracer.h"
#include "Utils/SpatialMetrics.h"
#include "Utils/SpatialMetricsDisplay.h"
#include "Utils/SpatialStatics.h"

DEFINE_LOG_CATEGORY(LogSpatialGameInstance);

bool USpatialGameInstance::HasSpatialNetDriver() const
{
	bool bHasSpatialNetDriver = false;

	if (WorldContext != nullptr)
	{
		UWorld* World = GetWorld();
		UNetDriver * NetDriver = GEngine->FindNamedNetDriver(World, NAME_PendingNetDriver);
		bool bShouldDestroyNetDriver = false;

		if (NetDriver == nullptr)
		{
			// If Spatial networking is enabled, override the GameNetDriver with the SpatialNetDriver
			if (GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking())
			{
				if (FNetDriverDefinition* DriverDefinition = GEngine->NetDriverDefinitions.FindByPredicate([](const FNetDriverDefinition& CurDef)
				{
					return CurDef.DefName == NAME_GameNetDriver;
				}))
				{
					DriverDefinition->DriverClassName = DriverDefinition->DriverClassNameFallback = TEXT("/Script/SpatialGDK.SpatialNetDriver");
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
		UE_LOG(LogSpatialGameInstance, Error, TEXT("Could not find SpatialNetDriver even though Spatial networking is switched on! "
										  "Please make sure you set up the net driver definitions as specified in the porting "
										  "guide and that you don't override the main net driver."));
	}

	return bHasSpatialNetDriver;
}

void USpatialGameInstance::CreateNewSpatialWorkerConnection()
{
	SpatialConnection = NewObject<USpatialWorkerConnection>(this);

#if TRACE_LIB_ACTIVE
	SpatialConnection->OnEnqueueMessage.AddUObject(SpatialLatencyTracer, &USpatialLatencyTracer::OnEnqueueMessage);
	SpatialConnection->OnDequeueMessage.AddUObject(SpatialLatencyTracer, &USpatialLatencyTracer::OnDequeueMessage);
#endif
}

void USpatialGameInstance::DestroySpatialWorkerConnection()
{
	if (SpatialConnection != nullptr)
	{
		SpatialConnection->DestroyConnection();
		SpatialConnection = nullptr;
	}
}

#if WITH_EDITOR
FGameInstancePIEResult USpatialGameInstance::StartPlayInEditorGameInstance(ULocalPlayer* LocalPlayer, const FGameInstancePIEParameters& Params)
{
	if (HasSpatialNetDriver())
	{
		// If we are using spatial networking then prepare a spatial connection.
		CreateNewSpatialWorkerConnection();
	}

	return Super::StartPlayInEditorGameInstance(LocalPlayer, Params);
}
#endif

void USpatialGameInstance::TryConnectToSpatial()
{
	if (HasSpatialNetDriver())
	{
		// If we are using spatial networking then prepare a spatial connection.
		CreateNewSpatialWorkerConnection();

		// Native Unreal creates a NetDriver and attempts to automatically connect if a Host is specified as the first commandline argument.
		// Since the SpatialOS Launcher does not specify this, we need to check for a locator loginToken to allow automatic connection to provide parity with native.
		// If a developer wants to use the Launcher and NOT automatically connect they will have to set the `PreventAutoConnectWithLocator` flag to true.
		if (!bPreventAutoConnectWithLocator)
		{
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
}

void USpatialGameInstance::StartGameInstance()
{
	TryConnectToSpatial();

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
	GlobalStateManager = NewObject<UGlobalStateManager>();
	StaticComponentView = NewObject<USpatialStaticComponentView>();

	FWorldDelegates::LevelInitializedNetworkActors.AddUObject(this, &USpatialGameInstance::OnLevelInitializedNetworkActors);

	ActorGroupManager = MakeUnique<SpatialActorGroupManager>();
	ActorGroupManager->Init();
}

void USpatialGameInstance::HandleOnConnected()
{
	UE_LOG(LogSpatialGameInstance, Log, TEXT("Successfully connected to SpatialOS"));
	SpatialWorkerId = SpatialConnection->GetWorkerId();
#if TRACE_LIB_ACTIVE
	SpatialLatencyTracer->SetWorkerId(SpatialWorkerId);
#endif
	OnConnected.Broadcast();
}

void USpatialGameInstance::HandleOnConnectionFailed(const FString& Reason)
{
	UE_LOG(LogSpatialGameInstance, Error, TEXT("Could not connect to SpatialOS. Reason: %s"), *Reason);
#if TRACE_LIB_ACTIVE
	SpatialLatencyTracer->ResetWorkerId();
#endif
	OnConnectionFailed.Broadcast(Reason);
}

void USpatialGameInstance::OnLevelInitializedNetworkActors(ULevel* LoadedLevel, UWorld* OwningWorld)
{
	const FString WorkerType = GetSpatialWorkerType().ToString();

	if (OwningWorld != GetWorld())
	{
		// Not current world. Another server running in the same process.
		return;
	}

	if (!OwningWorld->IsServer())
	{
		return;
	}

	if (USpatialStatics::IsSpatialOffloadingEnabled())
	{
		if (OwningWorld->WorldType != EWorldType::PIE
			&& OwningWorld->WorldType != EWorldType::Game
			&& OwningWorld->WorldType != EWorldType::GamePreview)
		{
			return;
		}

		for (int32 ActorIndex = 0; ActorIndex < LoadedLevel->Actors.Num(); ActorIndex++)
		{
			AActor* Actor = LoadedLevel->Actors[ActorIndex];
			if (Actor == nullptr)
			{
				continue;
			}

			if (!USpatialStatics::IsActorGroupOwnerForActor(Actor))
			{
				if (!Actor->bNetLoadOnNonAuthServer)
				{
					Actor->Destroy();
				}
				else
				{
					UE_LOG(LogSpatialGameInstance, Verbose, TEXT("WorkerType %s Not actor group owner of startup actor %s, Exchanging Roles"), *WorkerType, *GetPathNameSafe(Actor));
					ENetRole Temp = Actor->Role;
					Actor->Role = Actor->RemoteRole;
					Actor->RemoteRole = Temp;
				}
			}
		}
	}
	else if (GetDefault<USpatialGDKSettings>()->bEnableUnrealLoadBalancer)
	{
		// Necessary for levels loaded before connecting to Spatial
		if (GlobalStateManager == nullptr)
		{
			return;
		}

		// If load balancing is enabled and lb strategy says we should have authority over a
		// loaded level Actor then also set Role_Authority on Actors in the sublevel.
		const bool bHaveGSMAuthority = StaticComponentView->HasAuthority(SpatialConstants::INITIAL_GLOBAL_STATE_MANAGER_ENTITY_ID, SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID);

		for (auto Actor : LoadedLevel->Actors)
		{
			if (Actor->GetIsReplicated() && !((USpatialNetDriver*)OwningWorld->GetNetDriver())->LoadBalanceStrategy->ShouldHaveAuthority(*Actor))
			{
				Actor->Role = ROLE_SimulatedProxy;
				Actor->RemoteRole = ROLE_Authority;
			}
		}
	}
}
