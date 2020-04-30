// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include <improbable/c_worker.h>

namespace SpatialGDK
{

class AbstractOpList
{
public:
	virtual ~AbstractOpList() = default;

	virtual uint32 GetCount() const = 0;
	virtual Worker_Op& operator[](uint32 Index) = 0;
	virtual const Worker_Op& operator[](uint32 Index) const = 0;
};

}  // namespace SpatialGDK
