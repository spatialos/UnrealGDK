// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialPlayerController.h"

#include "Net/UnrealNetwork.h"

void ASpatialPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialPlayerController, WorkerId);
}
