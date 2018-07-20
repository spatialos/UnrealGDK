// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorToolbarCommands.h"

#define LOCTEXT_NAMESPACE "FSpatialGDKEditorToolbarModule"

void FSpatialGDKEditorToolbarCommands::RegisterCommands()
{
	UI_COMMAND(CreateSpatialGDKSnapshot, "SpatialGDK Snapshot", "Creates snapshot.", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(GenerateInteropCode, "Interop Codegen", "Generates interop code.", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(StartSpatialOSStackAction, "Launch", "Starts SpatialOS locally.", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(StopSpatialOSStackAction, "Stop", "Stops SpatialOS.", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(LaunchInspectorWebPageAction, "Inspector", "Launches default web browser to SpatialOS Inspector.", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
