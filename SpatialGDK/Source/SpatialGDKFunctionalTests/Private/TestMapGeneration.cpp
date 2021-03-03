// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMapGeneration.h"
#include "SpatialGDKEditor/Public/SpatialTestSettings.h"
#include "TestMaps/GeneratedTestMap.h"

DEFINE_LOG_CATEGORY(LogTestMapGeneration);

namespace SpatialGDK
{
namespace TestMapGeneration
{
bool GenerateTestMaps()
{
	bool bSuccess = true;
	UE_LOG(LogTestMapGeneration, Display, TEXT("Deleting the generated test map folder %s."), *UGeneratedTestMap::GetGeneratedMapFolder());
	if (FPaths::DirectoryExists(UGeneratedTestMap::GetGeneratedMapFolder()))
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		if (!PlatformFile.DeleteDirectoryRecursively(*UGeneratedTestMap::GetGeneratedMapFolder()))
		{
			bSuccess = false;
			UE_LOG(LogTestMapGeneration, Error, TEXT("Failed to delete the generated test map folder %s."),
				   *UGeneratedTestMap::GetGeneratedMapFolder());
		}
	}

	UE_LOG(LogTestMapGeneration, Display, TEXT("Deleting the generated test config folder %s."),
		   *FSpatialTestSettings::GeneratedOverrideSettingsDirectory);
	if (FPaths::DirectoryExists(FSpatialTestSettings::GeneratedOverrideSettingsDirectory))
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		if (!PlatformFile.DeleteDirectoryRecursively(*FSpatialTestSettings::GeneratedOverrideSettingsDirectory))
		{
			bSuccess = false;
			UE_LOG(LogTestMapGeneration, Error, TEXT("Failed to delete the generated test config folder %s."),
				   *FSpatialTestSettings::GeneratedOverrideSettingsDirectory);
		}
	}

	// Have to gather the classes first and then iterate over the copy, because creating a map triggers a GC which can modify the object
	// array this is iterating through
	TArray<UClass*> TestMapClasses;
	for (TObjectIterator<UClass> Iter; Iter; ++Iter)
	{
		if (Iter->IsChildOf(UGeneratedTestMap::StaticClass()) && *Iter != UGeneratedTestMap::StaticClass())
		{
			TestMapClasses.Add(*Iter);
		}
	}

	for (UClass* TestMapClass : TestMapClasses)
	{
		UGeneratedTestMap* TestMap = NewObject<UGeneratedTestMap>(GetTransientPackage(), TestMapClass);
		TestMap->AddToRoot(); // Okay, must admit, not completely sure what's going on here, seems like even though the commandlet is
							  // the outer of the newly generated object, the object still gets GCed when creating a new map, so have to add
							  // to root here to prevent GC
		if (TestMap->ShouldGenerateMap())
		{
			UE_LOG(LogTestMapGeneration, Display, TEXT("Creating the %s."), *TestMap->GetMapName());
			if (!TestMap->GenerateMap())
			{
				bSuccess = false;
				UE_LOG(LogTestMapGeneration, Error, TEXT("Failed to create the map for %s."), *TestMap->GetMapName());
			}
			if (!TestMap->GenerateCustomConfig())
			{
				bSuccess = false;
				UE_LOG(LogTestMapGeneration, Error, TEXT("Failed to create the custom config for %s."), *TestMap->GetMapName());
			}
		}
		TestMap->RemoveFromRoot();
	}

	// Success
	return bSuccess;
}

} // namespace TestMapGeneration

} // namespace SpatialGDK
