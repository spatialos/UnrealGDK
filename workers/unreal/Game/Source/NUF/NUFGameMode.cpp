// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "NUFGameMode.h"
#include "UObject/ConstructorHelpers.h"
#include "VehicleCppPawn.h"

ANUFGameMode::ANUFGameMode()
{
	//set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/EntityBlueprints/NUFCharacter_BP"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
