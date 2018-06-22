// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#ifdef SPATIALOS_WORKER_SDK_MOCK_ENABLED

#include "SpatialOSMockViewTypes.h"

using SpatialOSView = MockView;

#else

#include "improbable/view.h"

using SpatialOSView = worker::View;

#endif
