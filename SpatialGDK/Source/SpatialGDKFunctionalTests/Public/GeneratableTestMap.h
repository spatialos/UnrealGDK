// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "GeneratableTestMap.generated.h"

UINTERFACE(MinimalAPI)
class UGeneratableTestMap : public UInterface
{
	GENERATED_BODY()
};

class IGeneratableTestMap
{
	GENERATED_BODY()

public:
	virtual void GenerateMap() = 0;

	virtual bool SaveMap() = 0;

	virtual bool GenerateCustomConfig() = 0;

	virtual FString GetMapName() = 0;

	virtual bool ShouldGenerateMap() = 0;
};
