// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/Array.h"
#include "HAL/Platform.h"
#include "Net/RepLayout.h"

// Storage for a changelist created by the replication system when replicating from the server.
struct FRepChangeState
{
	const TArray<uint16> RepChanged; // changed replicated properties
	FRepLayout& RepLayout;
};

using FHandoverChangeState = TArray<uint16>; // changed handover properties
using FInterestChangeState = TArray<uint16>; // changed interest properties
