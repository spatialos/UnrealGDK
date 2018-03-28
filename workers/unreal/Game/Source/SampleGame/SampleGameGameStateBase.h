// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SampleGameGameStateBase.generated.h"

UCLASS()
class SAMPLEGAME_API ASampleGameGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	void FakeServerHasBegunPlay();
};
