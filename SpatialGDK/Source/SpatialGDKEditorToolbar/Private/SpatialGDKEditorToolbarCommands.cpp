// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorToolbarCommands.h"

#define LOCTEXT_NAMESPACE "FSpatialGDKEditorToolbarModule"

void FSpatialGDKEditorToolbarCommands::RegisterCommands()
{
	UI_COMMAND(CreateSpatialGDKSchema, "Schema", "Creates SpatialOS Unreal GDK schema for assets in memory.", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(CreateSpatialGDKSchemaFull, "Schema (Full Scan)", "Creates SpatialOS Unreal GDK schema for all assets.", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(DeleteSchemaDatabase, "Delete schema database", "Deletes the scheme database file", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(CreateSpatialGDKSnapshot, "Snapshot", "Creates SpatialOS Unreal GDK snapshot.", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(StartSpatialDeployment, "Start", "Starts a local instance of SpatialOS.", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(StopSpatialDeployment, "Stop", "Stops SpatialOS.", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(LaunchInspectorWebPageAction, "Inspector", "Launches default web browser to SpatialOS Inspector.", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(OpenSimulatedPlayerConfigurationWindowAction, "Deploy", "Opens a configuration menu for cloud deployments.", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(OpenLaunchConfigurationEditorAction, "Create Launch Configuration", "Opens an editor to create SpatialOS Launch configurations", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(BuildServerWorkerAction, "Build Server Worker", "Build Server Worker for Assembly", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(BuildClientWorkerAction, "Build Client Worker", "Build Client Worker for Assembly", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(BuildSimulatedPlayerWorkerAction, "Build Simulated Player Worker", "Build Simulated Player Worker for Assembly", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(BuildAllAction, "Build Full Assembly", "Build all workers for Assembly", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(UploadAssemblyAction, "Upload Assembly", "Upload the Assembly", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(BuildAndUploadAction, "Build and Upload Assembly", "Build all workers and upload assembly", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(StartSpatialService, "Start Service", "Starts the Spatial service daemon.", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(StopSpatialService, "Stop Service", "Stops the Spatial service daemon.", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
