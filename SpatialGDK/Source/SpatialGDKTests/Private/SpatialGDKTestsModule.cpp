// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKTestsModule.h"

// clang-format off
#include "SpatialConstants.h"
#include "SpatialConstants.cxx"
// clang-format on

#include "SpatialGDKTestsPrivate.h"

#define LOCTEXT_NAMESPACE "FSpatialGDKTestsModule"

DEFINE_LOG_CATEGORY(LogSpatialGDKTests);

IMPLEMENT_MODULE(FSpatialGDKTestsModule, SpatialGDKTests);

void InitializeSpatialFlagEarlyValues();

void FSpatialGDKTestsModule::StartupModule()
{
	InitializeSpatialFlagEarlyValues();
}

void FSpatialGDKTestsModule::ShutdownModule() {}

#undef LOCTEXT_NAMESPACE
