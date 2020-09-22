// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestPrepareShutdownListener.h"
#include "EngineClasses/SpatialGameInstance.h"

ATestPrepareShutdownListener::ATestPrepareShutdownListener()
	: NativePrepareShutdownEventCount(0)
	, BlueprintPrepareShutdownEventCount(0)
{
}

bool ATestPrepareShutdownListener::RegisterCallback_Implementation()
{
	USpatialGameInstance* GameInstance = GetGameInstance<USpatialGameInstance>();
	if (GameInstance == nullptr)
	{
		return false;
	}

	GameInstance->OnPrepareShutdown.AddDynamic(this, &ATestPrepareShutdownListener::OnPrepareShutdownNative);
	return true;
}

void ATestPrepareShutdownListener::OnPrepareShutdownNative()
{
	NativePrepareShutdownEventCount++;
}
