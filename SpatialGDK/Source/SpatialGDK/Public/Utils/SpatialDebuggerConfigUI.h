// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "SpatialDebuggerConfigUI.generated.h"

class ASpatialDebugger;

/**
 * UI to change visualization settings of the spatial debugger in-game.
 */
UCLASS(Abstract)
class SPATIALGDK_API USpatialDebuggerConfigUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "Debugger")
	void SetSpatialDebugger(ASpatialDebugger* InDebugger);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Debugger")
	ASpatialDebugger* SpatialDebugger;
};
