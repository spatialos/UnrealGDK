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

	bool NeedToLoadClass(const FString& ClassPath);
	bool IsEntityWaitingForAsyncLoad(Worker_EntityId Entity);
	void StartAsyncLoadingClass(const FString& ClassPath, Worker_EntityId EntityId);
	void ProcessActorsFromAsyncLoading();

private:
	FString GetPackagePath(const FString& ClassPath);
	void OnAsyncPackageLoaded(const FName& PackageName, UPackage* Package, EAsyncLoadingResult::Type Result);

	USpatialNetDriver* NetDriver;
	TSet<Worker_EntityId_Key> EntitiesWaitingForAsyncLoad;
	TMap<FName, TArray<Worker_EntityId_Key>> AsyncLoadingPackages;
	TSet<FName> LoadedPackages;
};

} // namespace SpatialGDK
