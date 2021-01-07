// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GenerateTestMapsCommandlet.h"
#include "FileHelpers.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestPossession/SpatialTestPossession.h"
#include "Tests/AutomationEditorCommon.h"

#if WITH_EDITOR

DEFINE_LOG_CATEGORY(LogGenerateTestMapsCommandlet);

UGenerateTestMapsCommandlet::UGenerateTestMapsCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

int32 UGenerateTestMapsCommandlet::Main(const FString& CmdLineParams)
{
	UE_LOG(LogGenerateTestMapsCommandlet, Display, TEXT("Generate test maps commandlet started. Pray."));

	// What's this?
	TGuardValue<bool> UnattendedScriptGuard(GIsRunningUnattendedScript, GIsRunningUnattendedScript || IsRunningCommandlet());

	UWorld* CurrentWorld = FAutomationEditorCommonUtils::CreateNewMap();
	ULevel* CurrentLevel = CurrentWorld->GetCurrentLevel();
	GEditor->AddActor(CurrentLevel, ASpatialTestPossession::StaticClass(), FTransform::Identity);
	FString PathToSave =
		FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() + TEXT("Maps/FunctionalTests/GeneratedMaps/Lol.umap"));
	FEditorFileUtils::SaveLevel(CurrentLevel, PathToSave);

	// Success
	return 0;
}

#endif // WITH_EDITOR
