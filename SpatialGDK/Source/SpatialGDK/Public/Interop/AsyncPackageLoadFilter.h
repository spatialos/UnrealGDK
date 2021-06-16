// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "SpatialConstants.h"

#include "AsyncPackageLoadFilter.generated.h"

DECLARE_DELEGATE_OneParam(FOnPackageLoadedForEntity, FSpatialEntityId /*EntityId*/);

DECLARE_LOG_CATEGORY_EXTERN(LogAsyncPackageLoadFilter, Log, All);

UCLASS()
class SPATIALGDK_API UAsyncPackageLoadFilter : public UObject
{
	GENERATED_BODY()

public:
	void Init(const FOnPackageLoadedForEntity& OnPackageLoadedForEntityDelegate);

	// Returns if asset package required by entity-actor is loaded
	bool IsAssetLoadedOrTriggerAsyncLoad(FSpatialEntityId EntityId, const FString& ClassPath);
	void ProcessActorsFromAsyncLoading();

private:
	bool NeedToLoadClass(const FString& ClassPath);
	bool IsEntityWaitingForAsyncLoad(FSpatialEntityId Entity);
	void StartAsyncLoadingClass(FSpatialEntityId EntityId, const FString& ClassPath);

	FString GetPackagePath(const FString& ClassPath);
	void OnAsyncPackageLoaded(const FName& PackageName, UPackage* Package, EAsyncLoadingResult::Type Result);

	FOnPackageLoadedForEntity OnPackageLoadedForEntity;

	TSet<Worker_EntityId_Key> EntitiesWaitingForAsyncLoad;
	TMap<FName, TArray<Worker_EntityId_Key>> AsyncLoadingPackages;
	TSet<FName> LoadedPackages;
};
