// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/AsyncPackageLoadFilter.h"

#include "UObject/UObjectGlobals.h"
#include "SpatialGDKLLM.h"

DEFINE_LOG_CATEGORY(LogAsyncPackageLoadFilter);

void UAsyncPackageLoadFilter::Init(const FOnPackageLoadedForEntity& InOnPackageLoadedForEntity)
{
	check(InOnPackageLoadedForEntity.IsBound());
	OnPackageLoadedForEntity = InOnPackageLoadedForEntity;
}

bool UAsyncPackageLoadFilter::IsAssetLoadedOrTriggerAsyncLoad(Worker_EntityId EntityId, const FString& ClassPath)
{
	if (IsEntityWaitingForAsyncLoad(EntityId))
	{
		return false;
	}

	if (NeedToLoadClass(ClassPath))
	{
		StartAsyncLoadingClass(EntityId, ClassPath);
		return false;
	}

	return true;
}

bool UAsyncPackageLoadFilter::NeedToLoadClass(const FString& ClassPath)
{
	UObject* ClassObject = FindObject<UClass>(nullptr, *ClassPath, false);
	if (ClassObject == nullptr)
	{
		return true;
	}

	FString PackagePath = GetPackagePath(ClassPath);
	FName PackagePathName = *PackagePath;

	// The following test checks if the package is currently being processed in the async loading thread.
	// Without it, we could be using an object loaded in memory, but not completely ready to be used.
	// Looking through PackageMapClient's code, which handles asset async loading in Native unreal, checking
	// UPackage::IsFullyLoaded, or UObject::HasAnyInternalFlag(EInternalObjectFlag::AsyncLoading) should tell us if it is the case.
	// In practice, these tests are not enough to prevent using objects too early (symptom is RF_NeedPostLoad being set, and crash when
	// using them later). GetAsyncLoadPercentage will actually look through the async loading thread's UAsyncPackage maps to see if there
	// are any entries. See UNR-3320 for more context.
	// TODO : UNR-3374 This looks like an expensive check, but it does the job. We should investigate further
	// what is the issue with the other flags and why they do not give us reliable information.

	float Percentage = GetAsyncLoadPercentage(PackagePathName);
	if (Percentage != -1.0f)
	{
		UE_LOG(LogAsyncPackageLoadFilter, Warning, TEXT("Class package is registered in async loading thread. Class path: %s"), *ClassPath)
		return true;
	}
	return false;
}

FString UAsyncPackageLoadFilter::GetPackagePath(const FString& ClassPath)
{
	return FSoftObjectPath(ClassPath).GetLongPackageName();
}

bool UAsyncPackageLoadFilter::IsEntityWaitingForAsyncLoad(Worker_EntityId Entity)
{
	return EntitiesWaitingForAsyncLoad.Contains(Entity);
}

void UAsyncPackageLoadFilter::StartAsyncLoadingClass(Worker_EntityId EntityId, const FString& ClassPath)
{
	FString PackagePath = GetPackagePath(ClassPath);
	FName PackagePathName = *PackagePath;

	bool bAlreadyLoading = AsyncLoadingPackages.Contains(PackagePathName);

	EntitiesWaitingForAsyncLoad.Emplace(EntityId);
	AsyncLoadingPackages.FindOrAdd(PackagePathName).Add(EntityId);

	UE_LOG(LogAsyncPackageLoadFilter, Log, TEXT("Async loading package for entity. Package: %s, entity: %lld, already loading: %s"),
		   *PackagePath, EntityId, bAlreadyLoading ? TEXT("true") : TEXT("false"));
	if (!bAlreadyLoading)
	{
		LoadPackageAsync(PackagePath, FLoadPackageAsyncDelegate::CreateUObject(this, &UAsyncPackageLoadFilter::OnAsyncPackageLoaded));
	}
}

void UAsyncPackageLoadFilter::OnAsyncPackageLoaded(const FName& PackageName, UPackage* Package, EAsyncLoadingResult::Type Result)
{
	if (Result != EAsyncLoadingResult::Succeeded)
	{
		UE_LOG(LogAsyncPackageLoadFilter, Error, TEXT("Package was not loaded successfully. Package: %s"), *PackageName.ToString());
		AsyncLoadingPackages.Remove(PackageName);
		return;
	}

	LoadedPackages.Add(PackageName);
}

void UAsyncPackageLoadFilter::ProcessActorsFromAsyncLoading()
{
	LLM_PLATFORM_SCOPE_SPATIAL(ELLMTagSpatialGDK::AsyncPackageLoadFilter);
	static_assert(TContainerTraits<decltype(LoadedPackages)>::MoveWillEmptyContainer, "Moving the set won't empty it");
	TSet<FName> PackagesToProcess = MoveTemp(LoadedPackages);

	for (const auto& PackageName : PackagesToProcess)
	{
		TArray<Worker_EntityId_Key>* Entities = AsyncLoadingPackages.Find(PackageName);
		if (Entities == nullptr)
		{
			UE_LOG(LogAsyncPackageLoadFilter, Error,
				   TEXT("USpatialReceiver::OnAsyncPackageLoaded: Package loaded but no entry in AsyncLoadingPackages. Package: %s"),
				   *PackageName.ToString());
			continue;
		}

		for (Worker_EntityId_Key Entity : *Entities)
		{
			if (IsEntityWaitingForAsyncLoad(Entity))
			{
				UE_LOG(LogAsyncPackageLoadFilter, Log, TEXT("Finished async loading package for entity. Package: %s, entity: %lld."),
					   *PackageName.ToString(), Entity);

				check(EntitiesWaitingForAsyncLoad.Find(Entity) != nullptr);
				EntitiesWaitingForAsyncLoad.Remove(Entity);
				OnPackageLoadedForEntity.ExecuteIfBound(Entity);
			}
		}

		AsyncLoadingPackages.Remove(PackageName);
	}
}
