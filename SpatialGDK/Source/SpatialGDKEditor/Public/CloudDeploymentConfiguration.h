// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/UnrealString.h"

/**
 * This struct is used to save all the fields needed to build, upload, and launch a cloud deployment.
 * This lets the user continue to modify the settings without affecting the deployment that is being prepared.
 */
struct SPATIALGDKEDITOR_API FCloudDeploymentConfiguration
{
	void InitFromSettings();

	FString AssemblyName;
	FString RuntimeVersion;
	FString PrimaryDeploymentName;
	FString PrimaryLaunchConfigPath;
	FString SnapshotPath;
	FString PrimaryRegionCode;
	FString MainDeploymentCluster;
	FString DeploymentTags;

	bool bSimulatedPlayersEnabled = false;
	FString SimulatedPlayerDeploymentName;
	FString SimulatedPlayerLaunchConfigPath;
	FString SimulatedPlayerRegionCode;
	FString SimulatedPlayerCluster;
	uint32 NumberOfSimulatedPlayers = 0;

	bool bGenerateSchema = false;
	bool bGenerateSnapshot = false;
	FString BuildConfiguration;
	bool bBuildClientWorker = false;
	bool bForceAssemblyOverwrite = false;

	FString BuildServerExtraArgs;
	FString BuildClientExtraArgs;
	FString BuildSimulatedPlayerExtraArgs;
};
