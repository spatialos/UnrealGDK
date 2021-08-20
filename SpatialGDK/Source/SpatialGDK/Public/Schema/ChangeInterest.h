// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "StandardLibrary.h"

DECLARE_LOG_CATEGORY_EXTERN(LogChangeInterest, Log, All);

namespace SpatialGDK
{
struct ChangeInterestQuery
{
	TArray<Worker_ComponentId> Components;
	TArray<Worker_ComponentSetId> ComponentSets;
	TArray<Worker_EntityId_Key> Entities;

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

	void CreateRequest(Worker_CommandRequest& OutCommandRequest) const;
};

} // namespace SpatialGDK
