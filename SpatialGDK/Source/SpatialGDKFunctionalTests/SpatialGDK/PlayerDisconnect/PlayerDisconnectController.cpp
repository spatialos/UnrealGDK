// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "PlayerDisconnectController.h"
#include "InputCoreTypes.h"

void APlayerDisconnectController::SetupInputComponent()
{
	Super::SetupInputComponent();
	EnableInput(this);
	check(InputComponent);
	InputComponent->BindKey(EKeys::M, IE_Pressed, this, &APlayerDisconnectController::MPressed);
}

void APlayerDisconnectController::MPressed()
{
	// Test a client disconnecting by returning to the main menu, in this case will travel to the default map
	GetGameInstance()->ReturnToMainMenu();
}
