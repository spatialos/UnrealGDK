// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "NUFEditorToolbarCommands.h"

#define LOCTEXT_NAMESPACE "FNUFEditorToolbarModule"

void FNUFEditorToolbarCommands::RegisterCommands()
{
	UI_COMMAND(CreateNUFSnapshot, "NUF Snapshot", "Creates NUF snapshot.", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
