// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKFunctionalTestsModule.h"

#include "LogSpatialFunctionalTest.h"
#include "SpatialGDKFunctionalTestsPrivate.h"

#define LOCTEXT_NAMESPACE "FSpatialGDKFunctionalTestsModule"

DEFINE_LOG_CATEGORY(LogSpatialGDKFunctionalTests);
DEFINE_LOG_CATEGORY(LogSpatialFunctionalTest);

IMPLEMENT_MODULE(FSpatialGDKFunctionalTestsModule, SpatialGDKFunctionalTests);

void FSpatialGDKFunctionalTestsModule::StartupModule() {}

void FSpatialGDKFunctionalTestsModule::ShutdownModule() {}

#undef LOCTEXT_NAMESPACE
