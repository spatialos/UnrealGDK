#pragma once
#include "SpatialAutomationCommon.h"
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

bool SpatialAutomationCommon::StartLocalSpatialDeploy(FAutomationTestBase* Test, SpatialProcessInfo& OutProcessInfo, const FString& WorkerConfigFile)
{

	const FString ExecuteAbsolutePath{ FPaths::Combine(FPaths::RootDir(), TEXT("nwx/spatial/")) };
	const FString CmdExecutable = TEXT("cmd.exe");

	const FString SpatialCmdArgument = FString::Printf(TEXT("/c cmd.exe /c spatial.exe worker build build-config ^& spatial.exe local launch %s ^& pause"), *WorkerConfigFile);

	UE_LOG(LogSpatialAutomation, Log, TEXT("Starting cmd.exe with `%s` arguments."), *SpatialCmdArgument);
	// Temporary workaround: To get spatial.exe to properly show a window we have to call cmd.exe to
	// execute it. We currently can't use pipes to capture output as it doesn't work properly with current
	// spatial.exe.
	FProcHandle SpatialOSStackProcHandle;
	bool ProcessStartedSuccefully = false;
	if (FPlatformProcess::CreatePipe(OutProcessInfo.ReadPipe, OutProcessInfo.WritePipe))
	{
		SpatialOSStackProcHandle = FPlatformProcess::CreateProc(
			*(CmdExecutable), *SpatialCmdArgument, true, false, false, &OutProcessInfo.ProcessID, 0,
			*ExecuteAbsolutePath, nullptr, OutProcessInfo.WritePipe);
		if (!SpatialOSStackProcHandle.IsValid())
		{
			UE_LOG(LogSpatialAutomation, Error,
				TEXT("Failed to start SpatialOS, please refer to log file for more information."));
		}
		else
		{
			ProcessStartedSuccefully = true;
			OutProcessInfo.ProcessHandle = SpatialOSStackProcHandle;
		}
	}
	else
	{

		UE_LOG(LogSpatialAutomation, Error,
			TEXT("Failed to Create pipe, please refer to log file for more information."));
	}
	Test->TestTrue(TEXT("Spatial Started Succesfully"), ProcessStartedSuccefully);
	return ProcessStartedSuccefully;
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
	bool SpatialStarted = StartLocalSpatialDeploy(Test,OutProcessInfo,WorkerConfigFile);
	StartPieWithDelay(SpatialStartDelay, PlayerJoinDelay);
	return SpatialStarted;
}

bool FTakeScreenShot::Update()
{
	FString FileName = FString::Printf(TEXT("../../../nwx/Game/Saved/Automation/Comparisons/%d/%s"), FEngineVersion::Current().GetChangelist(), *FileLabel);
	
	FScreenshotRequest::RequestScreenshot(FileName, false, true);

	return true;
}

bool FStopLocalSpatialGame::Update()
{
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	if (ProcessInfo.ProcessHandle.IsValid())
	{
		FString LogMsg = FString::Printf(TEXT("Stopping spatial process '%u'."), ProcessInfo.ProcessID);
		UE_LOG(LogSpatialAutomation, Log, TEXT("%s"), *LogMsg);

		FPlatformProcess::TerminateProc(ProcessInfo.ProcessHandle, true);
	}
	else
	{
		UE_LOG(LogSpatialAutomation, Error, TEXT("Unable to stop process, Handle is invalid."));
	}

	FPlatformProcess::ClosePipe(ProcessInfo.ReadPipe, ProcessInfo.WritePipe);
	ProcessInfo.ReadPipe = nullptr;
	ProcessInfo.WritePipe = nullptr;
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
