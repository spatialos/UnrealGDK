// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

namespace improbable
{

struct Component
{
	virtual ~Component() {}

	bool bIsDynamic = false;
};

}
