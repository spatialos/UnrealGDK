// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#ifdef SPATIALOS_WORKER_SDK_MOCK_ENABLED

#include "Mock/SpatialOSMockViewTypes.h"

#else

#include "improbable/view.h"

using SpatialOSView = worker::View;

#endif
