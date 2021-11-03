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
	if (FString(FCommandLine::Get()).Contains(TEXT("-SpatialTest")))
	{
		FModuleManager::Get().LoadModule("SpatialGDKServices");
		FModuleManager::Get().LoadModule("SpatialGDKFunctionalTests");
	}
}

void FSpatialGDKModule::ShutdownModule() {}

#undef LOCTEXT_NAMESPACE
