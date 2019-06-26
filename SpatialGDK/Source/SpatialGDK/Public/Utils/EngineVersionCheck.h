// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Launch/Resources/SpatialVersion.h"

// GDK Version to be updated with SPATIAL_ENGINE_VERSION
// when breaking changes are made to the engine that requires
// changes to the GDK to remain compatible
#define SPATIAL_GDK_VERSION 1

// Check if GDK is compatible with the current version of Unreal Engine
// SPATIAL_ENGINE_VERSION is incremented in engine when breaking changes
// are made that make previous versions of the GDK incompatible
static_assert(SPATIAL_ENGINE_VERSION == SPATIAL_GDK_VERSION, "GDK Version is incompatible with the Engine Version. Check both the GDK and Engine are up to date");
