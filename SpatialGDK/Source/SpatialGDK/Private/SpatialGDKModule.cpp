// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKModule.h"
#include "SpatialGDKSettings.h"

#define LOCTEXT_NAMESPACE "FSpatialGDKModule"

DEFINE_LOG_CATEGORY(LogSpatialGDKModule);

IMPLEMENT_MODULE(FSpatialGDKModule, SpatialGDK)

void FSpatialGDKModule::StartupModule()
{
}

void FSpatialGDKModule::ShutdownModule()
{
}

bool FSpatialGDKModule::UsesSpatialNetworking() const
{
	return GetDefault<USpatialGDKSettings>()->bSpatialNetworking;
}

void FSpatialGDKModule::SetUsesSpatialNetworking(bool bEnabled)
{
	GetMutableDefault<USpatialGDKSettings>()->bSpatialNetworking = bEnabled;
}

#undef LOCTEXT_NAMESPACE
