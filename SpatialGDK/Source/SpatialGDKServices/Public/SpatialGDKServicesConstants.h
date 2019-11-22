// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialGDKServicesModule.h"

namespace SpatialGDKServicesConstants
{
    namespace
    {
#if PLATFORM_WINDOWS
        // Assumes that spatial is installed and in the PATH
        const FString SpatialPath = TEXT("");
        const FString Extension = TEXT("exe");
#elif PLATFORM_MAC
        // This is currently hardcoded and we expect users to have spatial either installed or symlinked to this path.
        // If they haven't, it is necessary to symlink it to /usr/local/bin. At some point we should expose this via
        // the Unreal UI, however right now the SpatialGDKServices module is unable to see these.
        const FString SpatialPath = TEXT("/usr/local/bin");
        const FString Extension = TEXT("");
#endif

        const FString CreateExePath(FString path, FString exeName)
        {
            FString exeFile = FPaths::SetExtension(exeName, Extension);
            return FPaths::Combine(path, exeFile);
        }

        const FString GDKProgramPath = FSpatialGDKServicesModule::GetSpatialGDKPluginDirectory(TEXT("SpatialGDK/Binaries/ThirdParty/Improbable/Programs"));
    }

    const FString SpatialExe = CreateExePath(SpatialPath, TEXT("spatial"));
    const FString SpotExe = CreateExePath(GDKProgramPath, TEXT("spot"));
    const FString SchemaCompilerExe = CreateExePath(GDKProgramPath, TEXT("schema_compiler"));
}
