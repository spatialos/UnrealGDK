// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "DerivedDataCache/Public/DerivedDataPluginInterface.h"

class FSchemaClassCache : public FDerivedDataPluginInterface
{
public:

	enum class OutputType : uint32
	{
		Empty,
		Actor,
		ActorAndSubobjects,
		Subobject,
	};

	FSchemaClassCache(UClass* Class);
	
	const TCHAR* GetPluginName() const override;
	
	const TCHAR* GetVersionString() const override;
	
	FString GetPluginSpecificCacheKeySuffix() const override;

	bool IsBuildThreadsafe() const override;

	bool IsDeterministic() const override;

	FString GetDebugContextString() const override;

	bool Build(TArray<uint8>& OutData) override;

	static OutputType ReadCachedData(const TArray<uint8>& CachedData, FString& OutObject, FString& OutSubobjects);

protected:
	UClass* ClassToInspect;
};
