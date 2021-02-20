// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKModule.h"
#include "SpatialGDKLLM.h"

#define LOCTEXT_NAMESPACE "FSpatialGDKModule"

DEFINE_LOG_CATEGORY(LogSpatialGDKModule);

IMPLEMENT_MODULE(FSpatialGDKModule, SpatialGDK)

void FSpatialGDKModule::StartupModule()
{
	SpatialGDKLLM::Initialise();
}

void FSpatialGDKModule::ShutdownModule() {}

#undef LOCTEXT_NAMESPACE
