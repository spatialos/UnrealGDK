// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "SpatialView/CommandMessages.h"
#include "SpatialView/OpList/AbstractOpList.h"
#include "SpatialView/AuthorityRecord.h"
#include "Containers/Array.h"
#include "Templates/UniquePtr.h"
#include <improbable/c_worker.h>

namespace SpatialGDK
{

class ViewDelta
{
public:
	void AddCreateEntityResponse(CreateEntityResponse Response);

	void SetAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Worker_Authority Authority);

	const TArray<CreateEntityResponse>& GetCreateEntityResponses() const;

	// Returns an array of ops equivalent to the current state of the view delta.
	// It is expected that Clear should be called between calls to GenerateLegacyOpList.
	// todo Remove this once the view delta is not read via a legacy op list.
	TUniquePtr<AbstractOpList> GenerateLegacyOpList() const;

	void Clear();

private:
	// todo wrap world command responses in their own record?
	TArray<CreateEntityResponse> CreateEntityResponses;

	AuthorityRecord AuthorityChanges;
};

}  // namespace SpatialGDK
