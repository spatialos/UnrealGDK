// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorToolbarCommands.h"

#define LOCTEXT_NAMESPACE "FSpatialGDKEditorToolbarModule"

void FSpatialGDKEditorToolbarCommands::RegisterCommands()
{
	UI_COMMAND(CreateSpatialGDKSchema, "Schema", "Creates SpatialOS Unreal GDK schema for assets in memory.", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(CreateSpatialGDKSchemaFull, "Schema (Full Scan)", "Creates SpatialOS Unreal GDK schema for all assets.", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(DeleteSchemaDatabase, "Delete schema database", "Deletes the scheme database file", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(CreateSpatialGDKSnapshot, "Snapshot", "Creates SpatialOS Unreal GDK snapshot.", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(StartNoAutomaticConnection, "Start", "Not Automatic Connect to service", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(StartLocalSpatialDeployment, "Local", "Connect to local deployment service", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(StartCloudSpatialDeployment, "Cloud", "Connect to cloud SpatialOS service", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(StopSpatialDeployment, "Stop", "Stops SpatialOS.", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(LaunchInspectorWebPageAction, "Inspector", "Launches default web browser to SpatialOS Inspector.", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(OpenSimulatedPlayerConfigurationWindowAction, "Deployment Configuration", "Opens a configuration menu for cloud deployments.", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(OpenLaunchConfigurationEditorAction, "Create Launch Configuration", "Opens an editor to create SpatialOS Launch configurations", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(QuickDeployAction, "Deploy", "Launch Deployment.", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(EnableBuildClientWorker, "Build Client Worker", "If checked, ClientWorker will be built and uploaded when launching the cloud deployment.", EUserInterfaceActionType::ToggleButton, FInputChord());
	UI_COMMAND(EnableBuildSimulatedPlayer, "Build Simulated Player", "If checked, SimulatedPlayer client will be built and uploaded when launching the cloud deployment.", EUserInterfaceActionType::ToggleButton, FInputChord());
	UI_COMMAND(StartSpatialService, "Start Service", "Starts the Spatial service daemon.", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(StopSpatialService, "Stop Service", "Stops the Spatial service daemon.", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(EnableSpatialNetworking, "Spatial Networking", "If checked, the SpatialOS networking is used. Otherwise, native Unreal networking is used.", EUserInterfaceActionType::ToggleButton, FInputChord());
	UI_COMMAND(GDKEditorSettings, "Editor Settings", "Open the SpatialOS GDK Editor Settings", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(NoAutomaticConnection, "Do not connect", "Don't connect automatically using SpatialOS", EUserInterfaceActionType::RadioButton, FInputChord());
	UI_COMMAND(LocalDeployment, "Connect to local deployment", "Automatically connect to a local deployment", EUserInterfaceActionType::RadioButton, FInputChord());
	UI_COMMAND(CloudDeployment, "Connect to cloud deployment", "Automatically connect to a cloud deployment", EUserInterfaceActionType::RadioButton, FInputChord());
}

#undef LOCTEXT_NAMESPACE
