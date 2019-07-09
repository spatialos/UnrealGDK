#pragma once
#include "AutomationCommon.h"
#include "Editor/UnrealEd/Public/Tests/AutomationEditorCommon.h"
#include "Kismet/GameplayStatics.h"
#include "CoreMinimal.h"
#include "SpatialAutomationCommon.h"

#if WITH_DEV_AUTOMATION_TESTS


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpatialLaunchLocalSpatialGameTest, "Spatial.Core.LaunchPIE", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter | EAutomationTestFlags::ServerContext)
DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FCheckPlayerSpawned, FSpatialLaunchLocalSpatialGameTest*, Test);
bool FCheckPlayerSpawned::Update()
{
	UWorld* World = SpatialAutomationCommon::GetActiveGameWorld();
	TArray<AActor*> Players;
	UGameplayStatics::GetAllActorsOfClass(SpatialAutomationCommon::GetActiveGameWorld(), APlayerController::StaticClass(), Players);
	
	Test->TestTrue("Player Controllers are spawned", Players.Num() >= 1);
	return true;
}

bool FSpatialLaunchLocalSpatialGameTest::RunTest(const FString& Parameters)
{
	FString WorkerConfigFile = TEXT("default_launch.json");
	SpatialAutomationCommon::SpatialProcessInfo ProcInfo;
	bool Started = SpatialAutomationCommon::StartSpatialAndPIE(this, ProcInfo, WorkerConfigFile);
	ADD_LATENT_AUTOMATION_COMMAND(FCheckPlayerSpawned(this));
	ADD_LATENT_AUTOMATION_COMMAND(FStopLocalSpatialGame(ProcInfo));
	return true;
}
#endif // WITH_DEV_AUTOMATION_TESTS
