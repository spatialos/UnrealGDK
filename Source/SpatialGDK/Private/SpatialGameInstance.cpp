// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGameInstance.h"
#include "Engine/NetConnection.h"
#include "SpatialNetDriver.h"
#include "SpatialPendingNetGame.h"
#if WITH_EDITOR
#include "Editor/EditorEngine.h"
#include "Settings/LevelEditorPlaySettings.h"
#endif

DEFINE_LOG_CATEGORY(LogSpatialGDK);

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

	UEditorEngine* const EditorEngine = CastChecked<UEditorEngine>(GetEngine());

	// Ok for some reason the BaseURL does not have the host.
	// I can see that the attempt was to add the host. Let' 
	//FURL BaseURL = WorldContext->LastURL;
	//FURL URL(&BaseURL, TEXT("127.0.0.1"), ETravelType::TRAVEL_Absolute);

	// Josh - Correctly build the URL
	//FURL URL = FURL(&BaseURL, *EditorEngine->BuildPlayWorldURL(*PIEMapName), TRAVEL_Absolute);
	FURL URL = WorldContext->LastURL; // TOMORROW JOSH - THIS WORKS. LOOK AT MANAGED WORKERS.

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
FGameInstancePIEResult USpatialGameInstance::InitializeForPlayInEditor(int32 PIEInstanceIndex, const FGameInstancePIEParameters& Params)
{
	UEditorEngine* const EditorEngine = CastChecked<UEditorEngine>(GetEngine());

	// Look for an existing pie world context, may have been created before
	WorldContext = EditorEngine->GetWorldContextFromPIEInstance(PIEInstanceIndex);

	if (!WorldContext)
	{
		// If not, create a new one
		WorldContext = &EditorEngine->CreateNewWorldContext(EWorldType::PIE);
		WorldContext->PIEInstance = PIEInstanceIndex;
	}

	WorldContext->RunAsDedicated = Params.bRunAsDedicated;

	WorldContext->OwningGameInstance = this;

	// Here we can change the name of the world that we boot into.
	const FString WorldPackageName = EditorEngine->EditorWorld->GetOutermost()->GetName();

	// Establish World Context for PIE World
	WorldContext->LastURL.Map = WorldPackageName;
	WorldContext->PIEPrefix = WorldContext->PIEInstance != INDEX_NONE ? UWorld::BuildPIEPackagePrefix(WorldContext->PIEInstance) : FString();

	const ULevelEditorPlaySettings* PlayInSettings = GetDefault<ULevelEditorPlaySettings>();

	// We always need to create a new PIE world unless we're using the editor world for SIE
	UWorld* NewWorld = nullptr;

	bool bNeedsGarbageCollection = false;
	const EPlayNetMode PlayNetMode = [&PlayInSettings] { EPlayNetMode NetMode(PIE_Standalone); return (PlayInSettings->GetPlayNetMode(NetMode) ? NetMode : PIE_Standalone); }();
	const bool CanRunUnderOneProcess = [&PlayInSettings] { bool RunUnderOneProcess(false); return (PlayInSettings->GetRunUnderOneProcess(RunUnderOneProcess) && RunUnderOneProcess); }();
	if (PlayNetMode == PIE_Client)
	{
		// We are going to connect, so just load an empty world
		NewWorld = EditorEngine->CreatePIEWorldFromEntry(*WorldContext, EditorEngine->EditorWorld, PIEMapName);
	}
	else
	{
		// Standard PIE path: just duplicate the EditorWorld
		NewWorld = EditorEngine->CreatePIEWorldByDuplication(*WorldContext, EditorEngine->EditorWorld, PIEMapName);

		// Duplication can result in unreferenced objects, so indicate that we should do a GC pass after initializing the world context
		bNeedsGarbageCollection = true;
	}

	// failed to create the world!
	if (NewWorld == nullptr)
	{
		return FGameInstancePIEResult::Failure(NSLOCTEXT("UnrealEd", "Error_FailedCreateEditorPreviewWorld", "Failed to create editor preview world."));
	}

	NewWorld->SetGameInstance(this);
	WorldContext->SetCurrentWorld(NewWorld);
	WorldContext->AddRef(EditorEngine->PlayWorld);	// Tie this context to this UEngine::PlayWorld*		// @fixme, needed still?

													// make sure we can clean up this world!
	NewWorld->ClearFlags(RF_Standalone);
	NewWorld->bKismetScriptError = Params.bAnyBlueprintErrors;

	// Do a GC pass if necessary to remove any potentially unreferenced objects
	if (bNeedsGarbageCollection)
	{
		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
	}

	Init();

	// Give the deprecated method a chance to fail as well
	FGameInstancePIEResult InitResult = FGameInstancePIEResult::Success();

	if (InitResult.IsSuccess())
	{
		PRAGMA_DISABLE_DEPRECATION_WARNINGS
			InitResult = InitializePIE(Params.bAnyBlueprintErrors, PIEInstanceIndex, Params.bRunAsDedicated) ?
			FGameInstancePIEResult::Success() :
			FGameInstancePIEResult::Failure(NSLOCTEXT("UnrealEd", "Error_CouldntInitInstance", "The game instance failed to Play/Simulate In Editor"));
		PRAGMA_ENABLE_DEPRECATION_WARNINGS
	}

	return InitResult;
}

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
	if (!GIsClient || !HasSpatialNetDriver())
	{
		Super::StartGameInstance();
	}
	else
	{
		FString Error;

		if (!StartGameInstance_SpatialGDKClient(Error))
		{
			UE_LOG(LogSpatialGDK, Fatal, TEXT("Unable to browse to starting map: %s. Application will now exit."), *Error);
			FPlatformMisc::RequestExit(false);
		}
	}
}
