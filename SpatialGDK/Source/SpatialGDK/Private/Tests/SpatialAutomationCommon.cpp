#pragma once
#include "SpatialAutomationCommon.h"
#include "SpatialGDKServicesModule.h"
#include "Engine.h"
#include "Editor/UnrealEd/Public/Tests/AutomationEditorCommon.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"


#if WITH_DEV_AUTOMATION_TESTS


DEFINE_LOG_CATEGORY(LogSpatialAutomation);

/**
 * Retrieves the active world for the game (i.e the world for the game currently being played).
 */
UWorld* SpatialAutomationCommon::GetActiveGameWorld()
{
	UWorld* TestWorld = nullptr;
	const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
	for (const FWorldContext& Context : WorldContexts)
	{
		if (((Context.WorldType == EWorldType::PIE) || (Context.WorldType == EWorldType::Game)) && (Context.World() != NULL))
		{
			TestWorld = Context.World();
			break;
		}
	}

	return TestWorld;
}

void SpatialAutomationCommon::StartPieWithDelay(float SpatialStartDelay, float PlayerJoinDelay)
{
	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(SpatialStartDelay)); // Hackery for now, as we have no good way to detect that the spatial process is ready to go
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(PlayerJoinDelay));
}

bool SpatialAutomationCommon::StartSpatialAndPIE(
	FAutomationTestBase* Test,
	SpatialProcessInfo& OutProcessInfo,
	const FString& WorkerConfigFile,
	float SpatialStartDelay,
	float PlayerJoinDelay)
{
	bool SpatialStarted = false;

	FString LaunchConfig = FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::ProjectIntermediateDir()), TEXT("Improbable/DefaultLaunchConfig.json"));

	//const FString LaunchFlags = SpatialGDKSettings->GetSpatialOSCommandLineLaunchFlags();
	const FString LaunchFlags {};
	if (FSpatialGDKServicesModule* GDKServices = FModuleManager::GetModulePtr<FSpatialGDKServicesModule>("SpatialGDKServices"))
	{
		FLocalDeploymentManager* LocalDeploymentManager = GDKServices->GetLocalDeploymentManager();
		if (LocalDeploymentManager->TryStartLocalDeployment(LaunchConfig, LaunchFlags))
		{
			SpatialStarted = true;
		}
	}

	StartPieWithDelay(SpatialStartDelay, PlayerJoinDelay);
	return SpatialStarted;
}

bool FTakeScreenShot::Update()
{
	// TODO(Alex): Put some path here?
	FString FileName = FString::Printf(TEXT(""), FEngineVersion::Current().GetChangelist(), *FileLabel);
	
	FScreenshotRequest::RequestScreenshot(FileName, false, true);

	return true;
}

bool FStopLocalSpatialGame::Update()
{
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());

	bool SpatialStopped = false;
	if (FSpatialGDKServicesModule* GDKServices = FModuleManager::GetModulePtr<FSpatialGDKServicesModule>("SpatialGDKServices"))
	{
		FLocalDeploymentManager* LocalDeploymentManager = GDKServices->GetLocalDeploymentManager();
		if (LocalDeploymentManager->TryStopLocalDeployment())
		{
			SpatialStopped = true;
		}
	}
	return SpatialStopped;
}

#endif // WITH_DEV_AUTOMATION_TESTS
