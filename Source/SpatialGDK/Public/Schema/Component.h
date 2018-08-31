#pragma once

struct Component
{
	virtual ~Component() {}

	bool bIsDynamic = false;
};
