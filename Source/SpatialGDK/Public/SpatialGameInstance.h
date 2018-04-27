// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SpatialGameInstance.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDK, Log, All);

/**
*
*/
UCLASS()
class SPATIALGDK_API USpatialGameInstance : public UGameInstance
{
	GENERATED_BODY()

	bool StartGameInstance_SpatialGDKClient(FString& Error);

#if WITH_EDITOR
	virtual FGameInstancePIEResult StartPlayInEditorGameInstance(ULocalPlayer* LocalPlayer,
																 const FGameInstancePIEParameters& Params) override;
#endif
	virtual void StartGameInstance() override;
};
