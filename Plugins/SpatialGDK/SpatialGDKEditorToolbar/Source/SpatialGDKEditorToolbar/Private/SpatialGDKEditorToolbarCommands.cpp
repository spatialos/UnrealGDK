// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorToolbarCommands.h"

#define LOCTEXT_NAMESPACE "FSpatialGDKEditorToolbarModule"

void FSpatialGDKEditorToolbarCommands::RegisterCommands()
{
	UI_COMMAND(CreateSpatialGDKSnapshot, "SpatialGDK Snapshot", "Creates SpatialGDK snapshot.", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
