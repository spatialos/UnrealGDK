#pragma once

#include "CoreMinimal.h"
#include "DerivedDataCache/Public/DerivedDataPluginInterface.h"

struct FAssetContentSummary
{
	FMD5Hash AssetFileHash;

	// Sorted. Array of hashes returned by FAssetContentSummary::ComputeHash.
	TArray<FMD5Hash> AssetDependencies;

	FMD5Hash ComputeHash();
};

struct FAssetContentIdentifierCache 
{
	TOptional<FMD5Hash> GetAssetIdentifier(FName PackageName);

	TMap<FName, TOptional<FMD5Hash>> AssetSignatures;
};

struct FSpatialAssetContentSummary
{
	TArray<FString> SpatialClasses;
	TSet<float> NetCullDistances;
};

class FAssetSpatialContentCache : public FDerivedDataPluginInterface
{
public:

	FAssetSpatialContentCache(const FName& Asset, FAssetContentIdentifierCache& InIDCache);

	const TCHAR* GetPluginName() const override;

	const TCHAR* GetVersionString() const override;

	FString GetPluginSpecificCacheKeySuffix() const override;

	bool IsBuildThreadsafe() const override;

	bool IsDeterministic() const override;

	FString GetDebugContextString() const override;

	bool Build(TArray<uint8>& OutData) override;

	static FSpatialAssetContentSummary ReadCachedData(const TArray<uint8>& CachedData);

protected:
	FName AssetToInspect;
	FAssetContentIdentifierCache& IDCache;
};
