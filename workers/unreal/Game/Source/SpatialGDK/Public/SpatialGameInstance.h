// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SpatialGameInstance.generated.h"

/**
*
*/
UCLASS()
class SPATIALGDK_API USpatialGameInstance : public UGameInstance
{
GENERATED_BODY()

	bool StartGameInstance_NUFClient(FString& Error);

#if WITH_EDITOR
	virtual FGameInstancePIEResult StartPlayInEditorGameInstance(ULocalPlayer* LocalPlayer, const FGameInstancePIEParameters& Params) override;
#endif
	virtual void StartGameInstance() override;
};
