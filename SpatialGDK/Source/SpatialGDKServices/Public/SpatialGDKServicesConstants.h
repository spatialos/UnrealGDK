// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialGDKServicesModule.h"

namespace SpatialGDKServicesConstants
{
#if PLATFORM_WINDOWS
// Assumes that spatial is installed and in the PATH
const FString SpatialPath = TEXT("");
const FString Extension = TEXT("exe");
#elif PLATFORM_MAC
// UNR-2518: This is currently hardcoded and we expect users to have spatial either installed or symlinked to this path.
// If they haven't, it is necessary to symlink it to /usr/local/bin. At some point we should expose this via
// the Unreal UI, however right now the SpatialGDKServices module is unable to see these.
const FString SpatialPath = TEXT("/usr/local/bin");
const FString Extension = TEXT("");
#endif

static inline const FString CreateExePath(FString Path, FString ExecutableName)
{
	FString ExecutableFile = FPaths::SetExtension(ExecutableName, Extension);
	return FPaths::Combine(Path, ExecutableFile);
}

const FString GDKProgramPath =
	FSpatialGDKServicesModule::GetSpatialGDKPluginDirectory(TEXT("SpatialGDK/Binaries/ThirdParty/Improbable/Programs"));
const FString SpatialExe = CreateExePath(SpatialPath, TEXT("spatial"));
const FString SpotExe = CreateExePath(GDKProgramPath, TEXT("spot"));
const FString SchemaCompilerExe = CreateExePath(GDKProgramPath, TEXT("schema_compiler"));
const FString SpatialOSDirectory = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), TEXT("/../spatial/")));
const FString SpatialOSConfigFileName = TEXT("spatialos.json");
const FString ChinaEnvironmentArgument = TEXT(" --environment=cn-production");

const FString SpatialOSRuntimePinnedStandardVersion = TEXT("0.4.3");
const FString SpatialOSRuntimePinnedCompatbilityModeVersion = TEXT("14.5.4");

const FString InspectorURL = TEXT("http://localhost:31000/inspector");
const FString InspectorV2URL = TEXT("http://localhost:31000/inspector-v2");

const FString PinnedStandardRuntimeTemplate = TEXT("n1standard4_std40_action1g1");
const FString PinnedCompatibilityModeRuntimeTemplate = TEXT("n1standard4_std40_r0500");
const FString PinnedChinaStandardRuntimeTemplate = TEXT("s5large16_std50_action1g1");
const FString PinnedChinaCompatibilityModeRuntimeTemplate = TEXT("s5large16_std50_r0500");

const FString DevLoginDeploymentTag = TEXT("dev_login");

const FString UseChinaServicesRegionFilename = TEXT("UseChinaServicesRegion");

const FString ProxyFileDirectory = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectIntermediateDir(), TEXT("Improbable")));
const FString ProxyInfoFilePath = FPaths::Combine(ProxyFileDirectory, TEXT("ServerReceptionistProxyInfo.json"));

#if PLATFORM_MAC
const FString LsofCmdFilePath = TEXT("/usr/sbin/");
const FString KillCmdFilePath = TEXT("/bin/");
#endif
} // namespace SpatialGDKServicesConstants
