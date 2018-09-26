// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorToolbarCommands.h"

#define LOCTEXT_NAMESPACE "FSpatialGDKEditorToolbarModule"

void FSpatialGDKEditorToolbarCommands::RegisterCommands()
{
	UI_COMMAND(CreateSpatialGDKSnapshot, "Snapshot", "Creates SpatialOS Unreal GDK snapshot.", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(GenerateSchema, "Schema", "Generates schema.", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(StartSpatialOSStackAction, "Launch", "Starts a local instance of SpatialOS.", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(StopSpatialOSStackAction, "Stop", "Stops SpatialOS.", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(LaunchInspectorWebPageAction, "Inspector", "Launches default web browser to SpatialOS Inspector.", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
