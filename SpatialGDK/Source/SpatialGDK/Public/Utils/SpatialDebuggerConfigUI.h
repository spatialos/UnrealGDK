// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "SpatialDebuggerConfigUI.generated.h"

class ASpatialDebugger;

/**
 *
 */
UCLASS(Abstract)
class SPATIALGDK_API USpatialDebuggerConfigUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Debugger")
	ASpatialDebugger* SpatialDebugger;

	virtual void NativeOnInitialized() override;
};
