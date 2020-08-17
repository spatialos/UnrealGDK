// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/Array.h"
#include "SpatialView/OpList/OpList.h"
#include "SpatialView/ViewDelta.h"

namespace SpatialGDK
{
struct ViewDeltaLegacyOpListData : OpListData
{
	TArray<Worker_Op> Ops;
	// Used to store UTF8 disconnect string.
	TUniquePtr<char[]> DisconnectReason;
};

/** Creates an OpList from a ViewDelta. */
OpList GetOpListFromViewDelta(const ViewDelta& Delta);

} // namespace SpatialGDK
