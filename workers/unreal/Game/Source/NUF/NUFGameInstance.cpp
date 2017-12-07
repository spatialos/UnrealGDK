// Fill out your copyright notice in the Description page of Project Settings.

#include "NUFGameInstance.h"
#include "Engine/NetConnection.h"
#include "Engine/NetDriver.h"
#include "SpatialPendingNetGame.h"
#if WITH_EDITOR
#include "Settings/LevelEditorPlaySettings.h"
#include "Editor/EditorEngine.h"
#endif


#if WITH_EDITOR
FGameInstancePIEResult UNUFGameInstance::StartPlayInEditorGameInstance(ULocalPlayer* LocalPlayer, const FGameInstancePIEParameters& Params)
{
	// This is sadly hacky to avoid a larger engine change. It borrows code from UGameInstance::StartPlayInEditorGameInstance() and 
	//  UEngine::Browse().

	check(WorldContext);

	UEditorEngine* const EditorEngine = CastChecked<UEditorEngine>(GetEngine());
	ULevelEditorPlaySettings const* PlayInSettings = GetDefault<ULevelEditorPlaySettings>();
	
	const EPlayNetMode PlayNetMode = [&PlayInSettings] { EPlayNetMode NetMode(PIE_Standalone); return (PlayInSettings->GetPlayNetMode(NetMode) ? NetMode : PIE_Standalone); }();

	// for clients, just connect to the server
	bool bOk = true;

	if (PlayNetMode != PIE_Client)
	{
		return Super::StartPlayInEditorGameInstance(LocalPlayer, Params);
	}

	// Network URL.
	if (WorldContext->PendingNetGame)
	{
		if (WorldContext->PendingNetGame && WorldContext->PendingNetGame->NetDriver && WorldContext->PendingNetGame->NetDriver->ServerConnection)
		{
			WorldContext->PendingNetGame->NetDriver->ServerConnection->Close();
			EditorEngine->DestroyNamedNetDriver(WorldContext->PendingNetGame, WorldContext->PendingNetGame->NetDriver->NetDriverName);
			WorldContext->PendingNetGame->NetDriver = NULL;
		}

		WorldContext->PendingNetGame = NULL;
	}

	// Clean up the netdriver/socket so that the pending level succeeds
	if (GetWorldContext()->World())
	{
		EditorEngine->ShutdownWorldNetDriver(GetWorldContext()->World());
	}
			
	FString Error;
	FURL BaseURL = WorldContext->LastURL;
	FURL URL(&BaseURL, TEXT("127.0.0.1"), ETravelType::TRAVEL_Absolute);

	WorldContext->PendingNetGame = NewObject<USpatialPendingNetGame>();
	WorldContext->PendingNetGame->Initialize(URL);
	WorldContext->PendingNetGame->InitNetDriver();
	if (!WorldContext->PendingNetGame->NetDriver)
	{
		// UPendingNetGame will set the appropriate error code and connection lost type, so
		// we just have to propagate that message to the game.
		EditorEngine->BroadcastTravelFailure(WorldContext->World(), ETravelFailure::PendingNetGameCreateFailure, WorldContext->PendingNetGame->ConnectionError);
		WorldContext->PendingNetGame = NULL;
		bOk = false;
	}
		
	if (bOk)
	{
		EditorEngine->TransitionType = TT_WaitingToConnect;
		return FGameInstancePIEResult::Success();
	}
	else
	{
		return FGameInstancePIEResult::Failure(FText::Format(NSLOCTEXT("UnrealEd", "Error_CouldntLaunchPIEClient", "Couldn't Launch PIE Client: {0}"), FText::FromString(Error)));
	}
}

#endif
