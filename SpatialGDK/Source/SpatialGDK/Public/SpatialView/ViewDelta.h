// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "CommandMessages.h"
#include "Containers/Array.h"
#include "OpList/AbstractOpList.h"
#include "Templates/UniquePtr.h"
#include <improbable/c_worker.h>

namespace SpatialGDK
{

class ViewDelta
{
public:
	void AddCreateEntityResponse(CreateEntityResponse Response);

	const TArray<CreateEntityResponse>& GetCreateEntityResponses() const;

	// Returns an array of ops equivalent to the current state of the view delta.
	// It is expected that Clear should be called between calls to GenerateLegacyOpList.
	// todo Remove this once the view delta is not read via a legacy op list.
	TUniquePtr<AbstractOpList> GenerateLegacyOpList() const;

	void Clear();

private:
	TArray<CreateEntityResponse> CreateEntityResponses;
};

}  // namespace SpatialGDK
