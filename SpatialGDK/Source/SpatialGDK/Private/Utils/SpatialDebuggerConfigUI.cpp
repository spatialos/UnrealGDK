// Fill out your copyright notice in the Description page of Project Settings.

#include "Utils/SpatialDebuggerConfigUI.h"
#include "EngineClasses/SpatialNetDriver.h"

void USpatialDebuggerConfigUI::NativeOnInitialized()
{
	USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver());
	if (NetDriver)
	{
		SpatialDebugger = NetDriver->SpatialDebugger;
	}

	Super::NativeOnInitialized();
}
