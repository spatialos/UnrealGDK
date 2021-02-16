// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMapGeneration.h"
#include "TestMaps/GeneratedTestMap.h"

DEFINE_LOG_CATEGORY(LogTestMapGeneration);

namespace SpatialGDK
{
namespace TestMapGeneration
{
bool GenerateTestMaps()
{
	UE_LOG(LogTestMapGeneration, Display, TEXT("Deleting the %s folder."), *UGeneratedTestMap::GetGeneratedMapFolder());
	if (FPaths::DirectoryExists(UGeneratedTestMap::GetGeneratedMapFolder()))
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		const bool bSuccess = PlatformFile.DeleteDirectoryRecursively(*UGeneratedTestMap::GetGeneratedMapFolder());
		UE_CLOG(!bSuccess, LogTestMapGeneration, Error, TEXT("Failed to delete the generated test map folder %s."),
				*UGeneratedTestMap::GetGeneratedMapFolder());
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
			TestMap->GenerateMap();
		}
		TestMap->RemoveFromRoot();
	}

	// Success
	return true;
}

} // namespace TestMapGeneration

} // namespace SpatialGDK
