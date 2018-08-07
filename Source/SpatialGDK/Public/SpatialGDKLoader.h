// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Paths.h"
#include "PlatformProcess.h"

/**
 * This class ensures that the CoreSdkDll is loaded before it is needed by code.
 */
class FSpatialGDKLoader
{
public:
	FSpatialGDKLoader()
	{
#if PLATFORM_WINDOWS
		// Disable FPaths::GameDir deprecation warning until <= 4.17 is unsupported.
		PRAGMA_DISABLE_DEPRECATION_WARNINGS
		FString Path = FPaths::GameDir() / TEXT("Binaries/ThirdParty/Improbable");
		PRAGMA_ENABLE_DEPRECATION_WARNINGS

#if PLATFORM_64BITS
		Path = Path / TEXT("Win64");
#else
		Path = Path / TEXT("Win32");
#endif
		Path = Path / TEXT("worker.dll");
		CoreSdkHandle = FPlatformProcess::GetDllHandle(*Path);
#elif PLATFORM_PS4
		CoreSdkHandle = FPlatformProcess::GetDllHandle(TEXT("libCoreSdkDll.prx"));
#endif
	}

	~FSpatialGDKLoader()
	{
		if (CoreSdkHandle != nullptr)
		{
			FPlatformProcess::FreeDllHandle(CoreSdkHandle);
			CoreSdkHandle = nullptr;
		}
	}

	FSpatialGDKLoader(const FSpatialGDKLoader& rhs) = delete;
	FSpatialGDKLoader& operator=(const FSpatialGDKLoader& rhs) = delete;

private:
	void* CoreSdkHandle = nullptr;
};
