// Fill out your copyright notice in the Description page of Project Settings.

#include "Editor/SpatialEditorEngine.h"

#include "GameMapsSettings.h"
#include "SpatialNetDriver.h"
#include "SpatialPendingNetGame.h"


void USpatialEditorEngine::TickWorldTravel(FWorldContext& WorldContext, float DeltaSeconds)
{
	Super::TickWorldTravel(WorldContext, DeltaSeconds);
}

EBrowseReturnVal::Type USpatialEditorEngine::Browse(FWorldContext& WorldContext, FURL URL, FString& Error)
{
	Error = TEXT("");

	WorldContext.TravelURL = TEXT("");

	// Convert .unreal link files.
	const TCHAR* LinkStr = TEXT(".unreal");//!!
	if (FCString::Strstr(*URL.Map, LinkStr) - *URL.Map == FCString::Strlen(*URL.Map) - FCString::Strlen(LinkStr))
	{
		UE_LOG(LogNet, Log, TEXT("Link: %s"), *URL.Map);
		FString NewUrlString;
		if (GConfig->GetString(TEXT("Link")/*!!*/, TEXT("Server"), NewUrlString, *URL.Map))
		{
			// Go to link.
			URL = FURL(NULL, *NewUrlString, TRAVEL_Absolute);//!!
		} else
		{
			// Invalid link.
			Error = FText::Format(NSLOCTEXT("Engine", "InvalidLink", "Invalid Link: {0}"), FText::FromString(URL.Map)).ToString();
			return EBrowseReturnVal::Failure;
		}
	}

	// Crack the URL.
	UE_LOG(LogNet, Log, TEXT("Browse: %s"), *URL.ToString());

	// Handle it.
	if (!URL.Valid)
	{
		// Unknown URL.
		Error = FText::Format(NSLOCTEXT("Engine", "InvalidUrl", "Invalid URL: {0}"), FText::FromString(URL.ToString())).ToString();
		BroadcastTravelFailure(WorldContext.World(), ETravelFailure::InvalidURL, Error);
		return EBrowseReturnVal::Failure;
	} else if (URL.HasOption(TEXT("failed")) || URL.HasOption(TEXT("closed")))
	{
		// Browsing after a failure, load default map

		if (WorldContext.PendingNetGame)
		{
			CancelPending(WorldContext);
		}
		// Handle failure URL.
		UE_LOG(LogNet, Log, TEXT("%s"), TEXT("Failed; returning to Entry"));
		if (WorldContext.World() != NULL)
		{
			ResetLoaders(WorldContext.World()->GetOuter());
		}

		const UGameMapsSettings* GameMapsSettings = GetDefault<UGameMapsSettings>();
		const FString TextURL = GameMapsSettings->GetGameDefaultMap() + GameMapsSettings->LocalMapOptions;
		if (!LoadMap(WorldContext, FURL(&URL, *TextURL, TRAVEL_Partial), NULL, Error))
		{
			HandleBrowseToDefaultMapFailure(WorldContext, TextURL, Error);
			return EBrowseReturnVal::Failure;
		}

		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);

		// now remove "failed" and "closed" options from LastURL so it doesn't get copied on to future URLs
		WorldContext.LastURL.RemoveOption(TEXT("failed"));
		WorldContext.LastURL.RemoveOption(TEXT("closed"));
		return EBrowseReturnVal::Success;
	} else if (URL.HasOption(TEXT("restart")))
	{
		// Handle restarting.
		URL = WorldContext.LastURL;
	}

	// Handle normal URL's.
	if (GDisallowNetworkTravel && URL.HasOption(TEXT("listen")))
	{
		Error = NSLOCTEXT("Engine", "UsedCheatCommands", "Console commands were used which are disallowed in netplay.  You must restart the game to create a match.").ToString();
		BroadcastTravelFailure(WorldContext.World(), ETravelFailure::CheatCommands, Error);
		return EBrowseReturnVal::Failure;
	}
// 	if (URL.IsLocalInternal())
// 	{
// 		// Local map file.
// 		return LoadMap(WorldContext, URL, NULL, Error) ? EBrowseReturnVal::Success : EBrowseReturnVal::Failure;
// 	} else
	if (URL.IsInternal() && GIsClient)
	{
		// Network URL.
		if (WorldContext.PendingNetGame)
		{
			CancelPending(WorldContext);
		}

		// Clean up the netdriver/socket so that the pending level succeeds
		if (WorldContext.World() && ShouldShutdownWorldNetDriver())
		{
			ShutdownWorldNetDriver(WorldContext.World());
		}

		// IMPROBABLE-BEGIN
		WorldContext.PendingNetGame = NewObject<UPendingNetGame>();
		WorldContext.PendingNetGame->Initialize(URL);

		bool bIsSpatial = false;
		UNetDriver* TempNetDriver = nullptr;
		if (GEngine->CreateNamedNetDriver(WorldContext.PendingNetGame, NAME_PendingNetDriver, NAME_GameNetDriver))
		{
			TempNetDriver = GEngine->FindNamedNetDriver(WorldContext.PendingNetGame, NAME_PendingNetDriver);
			// Setting the PendingNetGame's NetDriver is necessary here because the call to CreateNamedNetDriver above will interfere
			// with the internals of InitNetDriver and cause the NetDriver not to be initialized, and fail a check().
			WorldContext.PendingNetGame->NetDriver = TempNetDriver;
			bIsSpatial = TempNetDriver->IsA(USpatialNetDriver::StaticClass());
		}

		// Create the proper PendingNetGame depending on what NetDriver we have loaded.
		// This is required so that we don't break vanilla Unreal networking with SpatialOS switched off.
		if (bIsSpatial)
		{
			WorldContext.PendingNetGame = NewObject<USpatialPendingNetGame>();
			WorldContext.PendingNetGame->Initialize(URL);
			// See above comment about setting NetDriver manually here.
			WorldContext.PendingNetGame->NetDriver = TempNetDriver;
		}
		// IMPROBABLE-END

		WorldContext.PendingNetGame->InitNetDriver();
		if (!WorldContext.PendingNetGame->NetDriver)
		{
			// UPendingNetGame will set the appropriate error code and connection lost type, so
			// we just have to propagate that message to the game.
			BroadcastTravelFailure(WorldContext.World(), ETravelFailure::PendingNetGameCreateFailure, WorldContext.PendingNetGame->ConnectionError);
			WorldContext.PendingNetGame = NULL;
			return EBrowseReturnVal::Failure;
		}
		return EBrowseReturnVal::Pending;
	} else if (URL.IsInternal())
	{
		// Invalid.
		Error = NSLOCTEXT("Engine", "ServerOpen", "Servers can't open network URLs").ToString();
		return EBrowseReturnVal::Failure;
	}
	{
		// External URL - disabled by default.
		// Client->Viewports(0)->Exec(TEXT("ENDFULLSCREEN"));
		// FPlatformProcess::LaunchURL( *URL.ToString(), TEXT(""), &Error );
		return EBrowseReturnVal::Failure;
	}
}
