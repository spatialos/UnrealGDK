// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGameInstance.h"

#include "Engine/NetConnection.h"
#include "GeneralProjectSettings.h"
#if WITH_EDITOR
#include "Editor/EditorEngine.h"
#include "Settings/LevelEditorPlaySettings.h"
#endif

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPendingNetGame.h"

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

	if (GetDefault<UGeneralProjectSettings>()->bSpatialNetworking && !bHasSpatialNetDriver)
	{
		UE_LOG(LogSpatialGameInstance, Error, TEXT("Could not find SpatialNetDriver even though Spatial networking is switched on! "
										  "Please make sure you set up the net driver definitions as specified in the porting "
										  "guide and that you don't override the main net driver."));
	}

	return bHasSpatialNetDriver;
}

bool USpatialGameInstance::StartGameInstance_SpatialGDKClient(FString& Error)
{
	if (WorldContext->PendingNetGame)
	{
		if (WorldContext->PendingNetGame->NetDriver && WorldContext->PendingNetGame->NetDriver->ServerConnection)
		{
			WorldContext->PendingNetGame->NetDriver->ServerConnection->Close();
			GetEngine()->DestroyNamedNetDriver(WorldContext->PendingNetGame, WorldContext->PendingNetGame->NetDriver->NetDriverName);
			WorldContext->PendingNetGame->NetDriver = nullptr;
		}

		WorldContext->PendingNetGame = nullptr;
	}

	// Clean up the netdriver/socket so that the pending level succeeds
	if (GetWorldContext()->World())
	{
		GetEngine()->ShutdownWorldNetDriver(GetWorldContext()->World());
	}

	FURL URL = WorldContext->LastURL;
	URL.Host = SpatialConstants::LOCAL_HOST;

	WorldContext->PendingNetGame = NewObject<USpatialPendingNetGame>();
	WorldContext->PendingNetGame->Initialize(URL);
	WorldContext->PendingNetGame->InitNetDriver();

	bool bOk = true;

	if (!WorldContext->PendingNetGame->NetDriver)
	{
		// UPendingNetGame will set the appropriate error code and connection lost type, so
		// we just have to propagate that message to the game.
		GetEngine()->BroadcastTravelFailure(WorldContext->World(), ETravelFailure::PendingNetGameCreateFailure, WorldContext->PendingNetGame->ConnectionError);
		Error = WorldContext->PendingNetGame->ConnectionError;
		WorldContext->PendingNetGame = NULL;
		bOk = false;
	}

	return bOk;
}

#if WITH_EDITOR
FGameInstancePIEResult USpatialGameInstance::StartPlayInEditorGameInstance(ULocalPlayer* LocalPlayer, const FGameInstancePIEParameters& Params)
{
	if (!HasSpatialNetDriver())
	{
		// If we are not using USpatialNetDriver, revert to the regular Unreal codepath.
		// Allows you to switch between SpatialOS and Unreal networking at game launch.
		// e.g. to enable Unreal networking, add to your cmdline: -NetDriverOverrides=/Script/OnlineSubsystemUtils.IpNetDriver
		return Super::StartPlayInEditorGameInstance(LocalPlayer, Params);
	}

	// This is sadly hacky to avoid a larger engine change. It borrows code from UGameInstance::StartPlayInEditorGameInstance() and 
	//  UEngine::Browse().
	check(WorldContext);

	ULevelEditorPlaySettings const* PlayInSettings = GetDefault<ULevelEditorPlaySettings>();
	const EPlayNetMode PlayNetMode = [&PlayInSettings] { EPlayNetMode NetMode(PIE_Standalone); return (PlayInSettings->GetPlayNetMode(NetMode) ? NetMode : PIE_Standalone); }();

	// for clients, just connect to the server
	bool bOk = true;

	if (PlayNetMode != PIE_Client)
	{
		return Super::StartPlayInEditorGameInstance(LocalPlayer, Params);
	}

	FString Error;

	if (StartGameInstance_SpatialGDKClient(Error))
	{
		GetEngine()->TransitionType = TT_WaitingToConnect;
		return FGameInstancePIEResult::Success();
	}
	else
	{
		return FGameInstancePIEResult::Failure(FText::Format(NSLOCTEXT("UnrealEd", "Error_CouldntLaunchPIEClient", "Couldn't Launch PIE Client: {0}"), FText::FromString(Error)));
	}
}
#endif

void USpatialGameInstance::StartGameInstance()
{
	if (!GIsClient)
	{
		if (!HasSpatialNetDriver()){
			Super::StartGameInstance();
		}
		else
		{
			// Spatial server workers require a persistent spatial connection and so it is initalised and stored here in the GameInstance.
			SpatialConnection = NewObject<USpatialWorkerConnection>();
			Super::StartGameInstance();
		}
	}
	else
	{
		FString Error;

		if (!StartGameInstance_SpatialGDKClient(Error))
		{
			UE_LOG(LogSpatialGameInstance, Fatal, TEXT("Unable to browse to starting map: %s. Application will now exit."), *Error);
			FPlatformMisc::RequestExit(false);
		}
	}
}
