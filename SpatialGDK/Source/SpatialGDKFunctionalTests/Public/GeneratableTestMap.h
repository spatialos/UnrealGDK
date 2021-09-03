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
	virtual void GenerateMap()
	{
		check(false);
	}

	virtual bool SaveMap()
	{
		check(false);
		return false;
	}

	virtual bool GenerateCustomConfig()
	{
		check(false);
		return false;
	}

	virtual FString GetMapName()
	{
		check(false);
		return TEXT("Should not be calling member function on interface.");
	}

	virtual bool ShouldGenerateMap()
	{
		check(false);
		return false;
	}
};
