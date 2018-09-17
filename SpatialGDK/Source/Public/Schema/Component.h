// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

struct SpatialComponent
{
	virtual ~SpatialComponent() {}

	bool bIsDynamic = false;
};
