// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestPrepareShutdownListener.h"
#include "EngineClasses/SpatialGameInstance.h"

ATestPrepareShutdownListener::ATestPrepareShutdownListener()
	: NativePrepareShutdownEventCount(0)
	, BlueprintPrepareShutdownEventCount(0)
{
}

bool ATestPrepareShutdownListener::RegisterCallback()
{
	USpatialGameInstance* GameInstance = GetGameInstance<USpatialGameInstance>();
	if(!GameInstance)
	{
		return false;
	}

	GameInstance->OnPrepareShutdown.AddDynamic(this, &ATestPrepareShutdownListener::OnPrepareShutdown);
	return true;
}

void ATestPrepareShutdownListener::OnPrepareShutdown_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("Prepare for shutdown!"));
	NativePrepareShutdownEventCount++;
}
