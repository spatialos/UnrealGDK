// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameEngine.h"
#include "SpatialGameEngine.generated.h"


UCLASS()
class SPATIALGDK_API USpatialGameEngine : public UGameEngine
{
	GENERATED_BODY()

public:
	virtual void TickWorldTravel(FWorldContext& WorldContext, float DeltaSeconds) override;

	virtual EBrowseReturnVal::Type Browse(FWorldContext& WorldContext, FURL URL, FString& Error) override;
};
