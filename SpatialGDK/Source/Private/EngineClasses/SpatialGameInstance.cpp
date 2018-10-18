// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGameInstance.h"

#include "Engine/EngineTypes.h"
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

bool USpatialGameInstance::StartGameInstance_SpatialGDKClient(bool bIsPIE, FString& Error)
{
	FURL URL;
	if (bIsPIE)
	{
		// Do this so we load the same world that the editor had running.
		URL = WorldContext->LastURL;
	}
	else
	{
		URL = GetInitialGameURL();
	}
	// If the default is just a map, set the URL to localhost to make sure we attempt to make a connection.
	URL.Host = SpatialConstants::LOCAL_HOST;

	EBrowseReturnVal::Type BrowseResult = EBrowseReturnVal::Failure;

	if (URL.Valid)
	{
		BrowseResult = GEngine->Browse(*WorldContext, URL, Error);
	}

	return BrowseResult != EBrowseReturnVal::Failure;
}

FURL USpatialGameInstance::GetInitialGameURL() const
{
	// Load the default URL from config files.
	FURL DefaultURL;
	DefaultURL.LoadURLConfig(TEXT("DefaultPlayer"), GGameIni);

	const TCHAR* Cmd = FCommandLine::Get();

	// Load up the default game map settings.
	const UGameMapsSettings* GameMapsSettings = GetDefault<UGameMapsSettings>();
	const FString& DefaultMap = GameMapsSettings->GetGameDefaultMap();

	// Check to see if there's a command line URL override.
	FString URLToLoad;
	if (!FParse::Token(Cmd, URLToLoad, 0) || **URLToLoad == '-')
	{
		// If not, use the default map and options.
		URLToLoad = DefaultMap + GameMapsSettings->LocalMapOptions;
	}

	return FURL(&DefaultURL, *URLToLoad, TRAVEL_Partial);
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

	if (StartGameInstance_SpatialGDKClient(true, Error))
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
	if (!GIsClient || !HasSpatialNetDriver())
	{
		Super::StartGameInstance();
	}
	else
	{
		FString Error;

		if (!StartGameInstance_SpatialGDKClient(false, Error))
		{
			UE_LOG(LogSpatialGameInstance, Fatal, TEXT("Unable to browse to starting map: %s. Application will now exit."), *Error);
			FPlatformMisc::RequestExit(false);
		}
	}
}
