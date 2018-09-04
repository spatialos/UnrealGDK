// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

struct Component
{
	virtual ~Component() {}

	bool bIsDynamic = false;
};
