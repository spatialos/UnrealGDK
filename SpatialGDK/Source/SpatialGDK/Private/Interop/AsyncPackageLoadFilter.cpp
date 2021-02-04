// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/AsyncPackageLoadFilter.h"

#include "UObject/UObjectGlobals.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/Connection/SpatialWorkerConnection.h"

DEFINE_LOG_CATEGORY(LogAsyncPackageLoadFilter);

namespace SpatialGDK
{
AsyncPackageLoadFilter::AsyncPackageLoadFilter(USpatialNetDriver* InNetDriver)
	: NetDriver(InNetDriver)
{
}

bool AsyncPackageLoadFilter::NeedToLoadClass(const FString& ClassPath)
{
	UObject* ClassObject = FindObject<UClass>(nullptr, *ClassPath, false);
	if (ClassObject == nullptr)
	{
		return true;
	}

	FString PackagePath = GetPackagePath(ClassPath);
	FName PackagePathName = *PackagePath;

	// UNR-3320 The following test checks if the package is currently being processed in the async loading thread.
	// Without it, we could be using an object loaded in memory, but not completely ready to be used.
	// Looking through PackageMapClient's code, which handles asset async loading in Native unreal, checking
	// UPackage::IsFullyLoaded, or UObject::HasAnyInternalFlag(EInternalObjectFlag::AsyncLoading) should tell us if it is the case.
	// In practice, these tests are not enough to prevent using objects too early (symptom is RF_NeedPostLoad being set, and crash when
	// using them later). GetAsyncLoadPercentage will actually look through the async loading thread's UAsyncPackage maps to see if there
	// are any entries.
	// TODO : UNR-3374 This looks like an expensive check, but it does the job. We should investigate further
	// what is the issue with the other flags and why they do not give us reliable information.

	float Percentage = GetAsyncLoadPercentage(PackagePathName);
	if (Percentage != -1.0f)
	{
		UE_LOG(LogAsyncPackageLoadFilter, Warning, TEXT("Class %s package is registered in async loading thread."), *ClassPath)
		return true;
	}
	return false;
}

FString AsyncPackageLoadFilter::GetPackagePath(const FString& ClassPath)
{
	return FSoftObjectPath(ClassPath).GetLongPackageName();
}

bool AsyncPackageLoadFilter::IsEntityWaitingForAsyncLoad(Worker_EntityId Entity)
{
	return EntitiesWaitingForAsyncLoad.Contains(Entity);
}

void AsyncPackageLoadFilter::StartAsyncLoadingClass(const FString& ClassPath, Worker_EntityId EntityId)
{
	FString PackagePath = GetPackagePath(ClassPath);
	FName PackagePathName = *PackagePath;

	bool bAlreadyLoading = AsyncLoadingPackages.Contains(PackagePathName);

	if (IsEntityWaitingForAsyncLoad(EntityId))
	{
		// This shouldn't happen because even if the entity goes out and comes back into view,
		// we would've received a RemoveEntity op that would remove the entry from the map.
		UE_LOG(LogAsyncPackageLoadFilter, Error,
			   TEXT("USpatialReceiver::ReceiveActor: Checked out entity but it's already waiting for async load! Entity: %lld"), EntityId);
	}

	EntitiesWaitingForAsyncLoad.Emplace(EntityId);
	AsyncLoadingPackages.FindOrAdd(PackagePathName).Add(EntityId);

	UE_LOG(LogAsyncPackageLoadFilter, Log, TEXT("Async loading package %s for entity %lld. Already loading: %s"), *PackagePath, EntityId,
		   bAlreadyLoading ? TEXT("true") : TEXT("false"));
	if (!bAlreadyLoading)
	{
		LoadPackageAsync(PackagePath, FLoadPackageAsyncDelegate::CreateRaw(this, &AsyncPackageLoadFilter::OnAsyncPackageLoaded));
	}
}

void AsyncPackageLoadFilter::OnAsyncPackageLoaded(const FName& PackageName, UPackage* Package, EAsyncLoadingResult::Type Result)
{
	if (Result != EAsyncLoadingResult::Succeeded)
	{
		UE_LOG(LogAsyncPackageLoadFilter, Error,
			   TEXT("USpatialReceiver::OnAsyncPackageLoaded: Package was not loaded successfully. Package: %s"), *PackageName.ToString());
		AsyncLoadingPackages.Remove(PackageName);
	}

	UE_LOG(LogAsyncPackageLoadFilter, Warning, TEXT("mcs: OnAsyncPackageLoaded: Package loaded successfully. Package: %s"),
		   *PackageName.ToString());
	LoadedPackages.Add(PackageName);
}

void AsyncPackageLoadFilter::ProcessActorsFromAsyncLoading()
{
	static_assert(TContainerTraits<decltype(LoadedPackages)>::MoveWillEmptyContainer, "Moving the set won't empty it");
	TSet<FName> PackagesToProcess = MoveTemp(LoadedPackages);

	for (const auto& PackageName : PackagesToProcess)
	{
		TArray<Worker_EntityId> Entities;
		if (!AsyncLoadingPackages.RemoveAndCopyValue(PackageName, Entities))
		{
			UE_LOG(LogAsyncPackageLoadFilter, Error,
				   TEXT("USpatialReceiver::OnAsyncPackageLoaded: Package loaded but no entry in AsyncLoadingPackages. Package: %s"),
				   *PackageName.ToString());
			continue;
		}

		for (Worker_EntityId Entity : Entities)
		{
			if (IsEntityWaitingForAsyncLoad(Entity))
			{
				UE_LOG(LogAsyncPackageLoadFilter, Log, TEXT("Finished async loading package %s for entity %lld."), *PackageName.ToString(),
					   Entity);

				check(EntitiesWaitingForAsyncLoad.Find(Entity) != nullptr);
				EntitiesWaitingForAsyncLoad.Remove(Entity);
				NetDriver->Connection->GetCoordinator().RefreshEntityCompleteness(Entity);
			}
		}
	}
}

} // namespace SpatialGDK
