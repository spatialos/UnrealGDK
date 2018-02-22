// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "NUFGameStateBase.generated.h"

UCLASS()
class NUF_API ANUFGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	void FakeServerHasBegunPlay();
};
