// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKModule.h"

// clang-format off
#include "SpatialConstants.h"
#include "SpatialConstants.cxx"
// clang-format on

#define LOCTEXT_NAMESPACE "FSpatialGDKModule"

DEFINE_LOG_CATEGORY(LogSpatialGDKModule);

IMPLEMENT_MODULE(FSpatialGDKModule, SpatialGDK)

void FSpatialGDKModule::StartupModule()
{
	/*
	 * Adds support for multiple processes testing by force loading the 2 modules needed by the spatial functional tests.
	 * This is needed since both SpatialGDKServices and SpatialGDKFunctionalTests have an Editor Target type, and
	 * the Native flow of loading modules prevents loading a module with an Editor target type from a process that is not the Editor itself.
	 * (see FModuleDescriptor::IsLoadedInCurrentConfiguration() )
	 *
	 * Since  the SpatialGDK is a Runtime module, it will get loaded through Native's flow in all processes, then we can force
	 * load the 2 required Editor modules at this point, but only if we are currently executing a SpatialFunctionalTest.
	 */
	if (FString(FCommandLine::Get()).Contains(SpatialConstants::SpatialTestFlag))
	{
		FModuleManager::Get().LoadModule("SpatialGDKServices");
		FModuleManager::Get().LoadModule("SpatialGDKFunctionalTests");
	}
}

void FSpatialGDKModule::ShutdownModule() {}

#undef LOCTEXT_NAMESPACE
