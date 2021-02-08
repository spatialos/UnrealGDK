// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "SpatialConstants.h"

class USpatialNetDriver;

DECLARE_LOG_CATEGORY_EXTERN(LogAsyncPackageLoadFilter, Log, All);

namespace SpatialGDK
{
class AsyncPackageLoadFilter
{
public:
	AsyncPackageLoadFilter(USpatialNetDriver* InNetDriver);

	// Returns if asset package required by entity-actor is loaded
	bool IsAssetLoadedOrTriggerAsyncLoad(Worker_EntityId EntityId, const FString& ClassPath);
	void ProcessActorsFromAsyncLoading();

private:
	bool NeedToLoadClass(const FString& ClassPath);
	bool IsEntityWaitingForAsyncLoad(Worker_EntityId Entity);
	void StartAsyncLoadingClass(Worker_EntityId EntityId, const FString& ClassPath);

	FString GetPackagePath(const FString& ClassPath);
	void OnAsyncPackageLoaded(const FName& PackageName, UPackage* Package, EAsyncLoadingResult::Type Result);

	USpatialNetDriver* NetDriver;
	TSet<Worker_EntityId_Key> EntitiesWaitingForAsyncLoad;
	TMap<FName, TArray<Worker_EntityId_Key>> AsyncLoadingPackages;
	TSet<FName> LoadedPackages;
};

} // namespace SpatialGDK
