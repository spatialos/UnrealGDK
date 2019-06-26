// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKServicesModule.h"
#include "SpatialGDKServicesPrivate.h"

#define LOCTEXT_NAMESPACE "FSpatialGDKServicesModule"

DEFINE_LOG_CATEGORY(LogSpatialGDKServices);

IMPLEMENT_MODULE(FSpatialGDKServicesModule, SpatialGDKServices);

void FSpatialGDKServicesModule::StartupModule()
{
	// Create an instance of the local deployment manager for tracking and controlling local deployment status.
	LocalDeploymentManager = FLocalDeploymentManager();
}

void FSpatialGDKServicesModule::ShutdownModule()
{
}

FLocalDeploymentManager* FSpatialGDKServicesModule::GetLocalDeploymentManager()
{
	return &LocalDeploymentManager;
}

#undef LOCTEXT_NAMESPACE
