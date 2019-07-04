// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKServicesModule.h"

#include "SpatialGDKServicesPrivate.h"

#define LOCTEXT_NAMESPACE "FSpatialGDKServicesModule"

DEFINE_LOG_CATEGORY(LogSpatialGDKServices);

IMPLEMENT_MODULE(FSpatialGDKServicesModule, SpatialGDKServices);

void FSpatialGDKServicesModule::StartupModule()
{
}

void FSpatialGDKServicesModule::ShutdownModule()
{
}

FLocalDeploymentManager* FSpatialGDKServicesModule::GetLocalDeploymentManager()
{
	return &LocalDeploymentManager;
}

FString FSpatialGDKServicesModule::GetSpatialOSDirectory()
{
	return FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), TEXT("/../spatial/")));
}

#undef LOCTEXT_NAMESPACE
