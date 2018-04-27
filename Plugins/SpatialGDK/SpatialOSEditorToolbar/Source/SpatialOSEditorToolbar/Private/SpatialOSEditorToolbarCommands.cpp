// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialOSEditorToolbarCommands.h"

#define LOCTEXT_NAMESPACE "FSpatialOSEditorToolbarModule"

void FSpatialOSEditorToolbarCommands::RegisterCommands()
{
	UI_COMMAND(StartSpatialOSStackAction, "Launch", "Starts SpatialOS locally.", EUserInterfaceActionType::Button,
			   FInputGesture());
	UI_COMMAND(StopSpatialOSStackAction, "Stop", "Stops SpatialOS.", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(LaunchInspectorWebPageAction, "Inspector", "Launches default web browser to SpatialOS Inspector.",
			   EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
