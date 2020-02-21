// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialViewModule.h"

#include "SpatialViewPrivate.h"

#define LOCTEXT_NAMESPACE "FSpatialViewModule"

DEFINE_LOG_CATEGORY(LogSpatialView);

IMPLEMENT_MODULE(FSpatialViewModule, SpatialView);


void FSpatialViewModule::StartupModule()
{
}

void FSpatialViewModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
