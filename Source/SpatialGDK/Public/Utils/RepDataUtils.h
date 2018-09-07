// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/Array.h"
#include "HAL/Platform.h"
#include "Net/RepLayout.h"

// Storage for a changelist created by the replication system when replicating from the server.
struct FPropertyChangeState
{
	const TArray<uint16> RepChanged; // changed replicated properties
	FRepLayout& RepLayout;
};
