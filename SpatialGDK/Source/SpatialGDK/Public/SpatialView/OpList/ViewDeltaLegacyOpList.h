// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/OpList/OpList.h"
#include "SpatialView/ViewDelta.h"
#include "Containers/Array.h"

namespace SpatialGDK
{

struct ViewDeltaLegacyOpListData: OpListData
{
	TArray<Worker_Op> Ops;
	// Used to store UTF8 disconnect string.
	ViewDelta Delta;
	TUniquePtr<char[]> DisconnectReason;
};

/** Creates an OpList from a ViewDelta. */
OpList GetOpListFromViewDelta(ViewDelta Delta);

}  // namespace SpatialGDK
