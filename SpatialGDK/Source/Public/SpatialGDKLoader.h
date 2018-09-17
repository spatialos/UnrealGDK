// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Paths.h"
#include "PlatformProcess.h"

#include "IPluginManager.h"

/**
 * This class ensures that the C API worker library is loaded before it is needed by code.
 */
class FSpatialGDKLoader
{
public:
	FSpatialGDKLoader()
	{
#if PLATFORM_WINDOWS
		FString Path = IPluginManager::Get().FindPlugin(TEXT("SpatialGDK"))->GetBaseDir() / TEXT("Binaries/ThirdParty/Improbable");

#if PLATFORM_64BITS
		Path = Path / TEXT("Win64");
#else
		Path = Path / TEXT("Win32");
#endif
		Path = Path / TEXT("worker.dll");
		WorkerLibraryHandle = FPlatformProcess::GetDllHandle(*Path);
#elif PLATFORM_PS4
		WorkerLibraryHandle = FPlatformProcess::GetDllHandle(TEXT("libworker.prx"));
#endif
	}

	~FSpatialGDKLoader()
	{
		if (WorkerLibraryHandle != nullptr)
		{
			FPlatformProcess::FreeDllHandle(WorkerLibraryHandle);
			WorkerLibraryHandle = nullptr;
		}
	}

	FSpatialGDKLoader(const FSpatialGDKLoader& rhs) = delete;
	FSpatialGDKLoader& operator=(const FSpatialGDKLoader& rhs) = delete;

private:
	void* WorkerLibraryHandle = nullptr;
};
