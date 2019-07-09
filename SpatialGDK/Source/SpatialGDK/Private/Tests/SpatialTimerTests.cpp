#pragma once
#include "AutomationCommon.h"
#include "Editor/UnrealEd/Public/Tests/AutomationEditorCommon.h"
#include "Kismet/GameplayStatics.h"
#include "CoreMinimal.h"
#include "SpatialWorldTimeComponent.h"
#include "SpatialAutomationCommon.h"

#if WITH_DEV_AUTOMATION_TESTS


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSetTimerByFunctionTestValidHandle, "Spatial.SpatialWorldTime.SpatialWorldTimeComponent.SetTimerByFunctionTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter | EAutomationTestFlags::ServerContext)
DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FCheckTimerIsValid, FSetTimerByFunctionTestValidHandle*, Test);

bool FCheckTimerIsValid::Update()
{
	UWorld* World = SpatialAutomationCommon::GetActiveGameWorld();
	if (World == nullptr)
		return false;

	AActor* Actor{ NewObject<AActor>(World->PersistentLevel) };
	USpatialWorldTimeComponent* WTComp{ NewObject<USpatialWorldTimeComponent>(Actor, USpatialWorldTimeComponent::StaticClass()) };
	Actor->AddOwnedComponent(WTComp);

	FString CallbackName{ "ForceNetUpdate" };
	FTimerHandle Handle{ WTComp->SetTimerByFunctionName(Actor, CallbackName, 1.0f, false) };
	const bool IsTimerValid{ Handle.IsValid() };

	Test->TestTrue("IsTimerValid", IsTimerValid);
	return true;
}

bool FSetTimerByFunctionTestValidHandle::RunTest(const FString& Parameters)
{
	FString WorkerConfigFile = TEXT("default_launch.json");
	SpatialAutomationCommon::SpatialProcessInfo ProcInfo;
	bool Started = SpatialAutomationCommon::StartSpatialAndPIE(this, ProcInfo, WorkerConfigFile);
	ADD_LATENT_AUTOMATION_COMMAND(FCheckTimerIsValid(this));
	ADD_LATENT_AUTOMATION_COMMAND(FStopLocalSpatialGame(ProcInfo));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
