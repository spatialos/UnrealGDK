// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// Note that this file has been generated automatically

#include "SpatialTypeBindingList.h"

#include "SpatialTypeBinding_PlayerController.h"
#include "SpatialTypeBinding_PlayerState.h"
#include "SpatialTypeBinding_NUFCharacter.h"
#include "SpatialTypeBinding_WheeledVehicle.h"

TArray<UClass*> GetGeneratedTypeBindings()
{
	return {
		USpatialTypeBinding_PlayerController::StaticClass(),
		USpatialTypeBinding_PlayerState::StaticClass(),
		USpatialTypeBinding_NUFCharacter::StaticClass(),
		USpatialTypeBinding_WheeledVehicle::StaticClass()
	};
}
