// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "StandardLibrary.h"

DECLARE_LOG_CATEGORY_EXTERN(LogChangeInterest, Log, All);

namespace SpatialGDK
{
class ISpatialOSWorker;

struct ChangeInterestQuery
{
	TArray<Worker_ComponentId> Components;
	TArray<Worker_ComponentSetId> ComponentSets;
	TArray<Worker_EntityId_Key> Entities;
	bool TrueConstraint = false;

	void DebugOutput(const FString& DiffType) const;
};

struct ChangeInterestRequest
{
	Worker_EntityId SystemEntityId = SpatialConstants::INVALID_ENTITY_ID;
	TArray<ChangeInterestQuery> QueriesToAdd;
	TArray<ChangeInterestQuery> QueriesToRemove;
	bool bOverwrite = false;

	void Clear();

	bool IsEmpty() const { return QueriesToAdd.Num() == 0 && QueriesToRemove.Num() == 0; }

	void DebugOutput() const;

	Worker_CommandRequest CreateRequest() const;

	Worker_RequestId SendRequest(ISpatialOSWorker& Connection);
};

} // namespace SpatialGDK
