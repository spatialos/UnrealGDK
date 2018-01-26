// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "NUFGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class NUF_API UNUFGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
#if WITH_EDITOR
	virtual FGameInstancePIEResult StartPlayInEditorGameInstance(ULocalPlayer* LocalPlayer, const FGameInstancePIEParameters& Params) override;
#endif
	
	void RequestPlayerSpawn();
};
