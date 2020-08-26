// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/Array.h"
#include "SpatialView/OpList/OpList.h"
#include "SpatialView/ViewDelta.h"

namespace SpatialGDK
{
class SpatialEventTracer;

struct ViewDeltaLegacyOpListData : OpListData
{
	ViewDeltaLegacyOpListData(SpatialEventTracer* EventTracer)
		: Delta(EventTracer)
	{
	}

	TArray<Worker_Op> Ops;
	// Used to store UTF8 disconnect string.
	ViewDelta Delta;
	TUniquePtr<char[]> DisconnectReason;
};

/** Creates an OpList from a ViewDelta. */
OpList GetOpListFromViewDelta(ViewDelta Delta);

} // namespace SpatialGDK
