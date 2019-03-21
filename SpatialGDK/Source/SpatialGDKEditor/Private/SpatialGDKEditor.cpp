// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditor.h"

#include "Async/Async.h"
#include "Engine/WorldComposition.h"
#include "UObject/StrongObjectPtr.h"

#include "SpatialGDKEditorSchemaGenerator.h"
#include "SpatialGDKEditorSnapshotGenerator.h"
#include "SpatialGDKEditorSettings.h"

#include "Editor.h"
#include "Engine/AssetManager.h"
#include "EditorLevelUtils.h"
#include "Engine/LevelStreamingKismet.h"

#include "AssetRegistryModule.h"
#include "GeneralProjectSettings.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKEditor);

FSpatialGDKEditor::FSpatialGDKEditor()
	: bSchemaGeneratorRunning(false)
{
	TryLoadExistingSchemaDatabase();
}

void FSpatialGDKEditor::GenerateSchema(FSimpleDelegate SuccessCallback, FSimpleDelegate FailureCallback, FSpatialGDKEditorErrorHandler ErrorCallback)
{
	if (bSchemaGeneratorRunning)
	{
		UE_LOG(LogSpatialGDKEditor, Warning, TEXT("Schema generation is already running"));
		return;
	}

	bSchemaGeneratorRunning = true;

	// Force spatial networking so schema layouts are correct
	UGeneralProjectSettings* GeneralProjectSettings = GetMutableDefault<UGeneralProjectSettings>();
	bool bCachedSpatialNetworking = GeneralProjectSettings->bSpatialNetworking;
	GeneralProjectSettings->bSpatialNetworking = true;

	TryLoadExistingSchemaDatabase();

	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();

	TArray<ULevelStreaming*> LoadedLevels;
	if (SpatialGDKSettings->bLoadStreamingLevelsWhenGeneratingSchema)
	{
		//LoadedLevels = LoadAllStreamingLevels(GWorld);
	}

	PreProcessSchemaMap();

	// Compile all dirty blueprints
	TArray<UBlueprint*> ErroredBlueprints;
	bool bPromptForCompilation = false;
	UEditorEngine::ResolveDirtyBlueprints(bPromptForCompilation, ErroredBlueprints);

	LoadDefaultGameModes();

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	SchemaGeneratorResult = Async<bool>(EAsyncExecution::Thread,
		[&, this, LoadedLevels]() {

		UE_LOG(LogSpatialGDKSchemaGenerator, Display, TEXT("Begin Schema Gen for %d Levels"), LoadedLevels.Num());

		bool bFoundAssetsToLoad = false;
		TArray<FAssetData> Assets;

		FFunctionGraphTask::CreateAndDispatchWhenReady([&]() {
			UE_LOG(LogSpatialGDKSchemaGenerator, Display, TEXT("Finding Assets To Load"));
			//FARFilter Filter;
			////Filter.ClassNames.Add(UWorld::StaticClass()->GetFName());
			//AssetRegistryModule.Get().GetAssets(Filter, Assets);

			AssetRegistryModule.Get().GetAllAssets(Assets, true);

			for (FAssetData Data : Assets)
			{
				FString IsLoaded = Data.IsAssetLoaded() ? TEXT("True") : TEXT("False");
				FString InGamePackage = Data.PackagePath.ToString().StartsWith("/Game") ? TEXT("True") : TEXT("False");
				FString HasGeneratedClass = Data.TagsAndValues.Contains("GeneratedClass") ? TEXT("True") : TEXT("False");

				UE_LOG(LogTemp, Log, TEXT("Found %s, Loaded: %s, GeneratedClass: %s, InGamePackage: %s (%s)"),
					*Data.GetFullName(), *IsLoaded, *HasGeneratedClass, *InGamePackage, *Data.PackagePath.ToString());
			}

			// Filter assets to blueprint classes that are not loaded.
			Assets = Assets.FilterByPredicate([](FAssetData Data) {
				return (!Data.IsAssetLoaded() && Data.TagsAndValues.Contains("GeneratedClass") && Data.PackagePath.ToString().StartsWith("/Game"));	
			});

			UE_LOG(LogTemp, Log, TEXT("---Filtered---"));


			for (FAssetData Data : Assets)
			{
				UE_LOG(LogTemp, Log, TEXT("Filtered %s"), *Data.GetFullName());
			}

			UE_LOG(LogSpatialGDKSchemaGenerator, Display, TEXT("Found %d assets to load."), Assets.Num());
			bFoundAssetsToLoad = true;
		}, TStatId(), NULL, ENamedThreads::BackgroundThreadPriority);

		while (!bFoundAssetsToLoad)
		{
			FPlatformProcess::Sleep(0.1f);
		}

		TArray<TStrongObjectPtr<UObject>> AssetPointers;

		for (FAssetData Data : Assets)
		{
			FFunctionGraphTask::CreateAndDispatchWhenReady([&, Data]() {
				AssetPointers.Add(TStrongObjectPtr<UObject>(Data.GetAsset()));
				UE_LOG(LogTemp, Log, TEXT("Loaded %s"), *Data.GetFullName());
			}, TStatId(), NULL, ENamedThreads::GameThread);
		}

		while (AssetPointers.Num() < Assets.Num())
		{
			FPlatformProcess::Sleep(0.1f);
		}

		bool bPreprocessed = false;

		FFunctionGraphTask::CreateAndDispatchWhenReady([&]() {
			UE_LOG(LogSpatialGDKSchemaGenerator, Display, TEXT("Preprocessing Map"));
			PreProcessSchemaMap();
			UE_LOG(LogSpatialGDKSchemaGenerator, Display, TEXT("Preprocessing Map: done."));
			bPreprocessed = true;
		}, TStatId(), NULL, ENamedThreads::GameThread);

		while (!bPreprocessed)
		{
			FPlatformProcess::Sleep(0.1f);
		}

		UE_LOG(LogSpatialGDKSchemaGenerator, Display, TEXT("Do Schema Gen"));
		bool bResult = SpatialGDKGenerateSchema();
		UE_LOG(LogSpatialGDKSchemaGenerator, Display, TEXT("Do Schema Gen: done"));

		bool bGarbageCollectionFinished = false;
		AssetPointers.Empty();

		FFunctionGraphTask::CreateAndDispatchWhenReady([&]() {
			UE_LOG(LogSpatialGDKSchemaGenerator, Display, TEXT("Triggering GC"));
			CollectGarbage(RF_NoFlags);
			UE_LOG(LogSpatialGDKSchemaGenerator, Display, TEXT("Triggering GC: done."));
			bGarbageCollectionFinished = true;
		}, TStatId(), NULL, ENamedThreads::GameThread);

		while (!bGarbageCollectionFinished)
		{
			FPlatformProcess::Sleep(0.1f);
		}

		return bResult;
	},
		[this, bCachedSpatialNetworking, ErroredBlueprints, SuccessCallback, FailureCallback]()
	{
		// We delay printing this error until after the schema spam to make it have a higher chance of being noticed.
		if (ErroredBlueprints.Num() > 0)
		{
			UE_LOG(LogSpatialGDKEditor, Error, TEXT("Errors compiling blueprints during schema generation! The following blueprints did not have schema generated for them:"));
			for (const auto& Blueprint : ErroredBlueprints)
			{
				UE_LOG(LogSpatialGDKEditor, Error, TEXT("%s"), *GetPathNameSafe(Blueprint));
			}
		}
		
		if (!SchemaGeneratorResult.IsReady() || SchemaGeneratorResult.Get() != true)
		{
			FailureCallback.ExecuteIfBound();
		}
		else
		{
			SuccessCallback.ExecuteIfBound();
		}
		GetMutableDefault<UGeneralProjectSettings>()->bSpatialNetworking = bCachedSpatialNetworking;
		bSchemaGeneratorRunning = false;
	});                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 
}

void FSpatialGDKEditor::GenerateSnapshot(UWorld* World, FString SnapshotFilename, FSimpleDelegate SuccessCallback, FSimpleDelegate FailureCallback, FSpatialGDKEditorErrorHandler ErrorCallback)
{
	const bool bSuccess = SpatialGDKGenerateSnapshot(World, SnapshotFilename);

	if (bSuccess)
	{
		SuccessCallback.ExecuteIfBound();
	}
	else
	{
		FailureCallback.ExecuteIfBound();
	}
}

void FSpatialGDKEditor::UnloadLevels(TArray<ULevelStreaming*> LoadedLevels)
{
	for (ULevelStreaming* Level : LoadedLevels)
	{
		if (Level->HasLoadedLevel())
		{
			bool Success = EditorLevelUtils::RemoveLevelFromWorld(Level->GetLoadedLevel());
			UE_LOG(LogSpatialGDKEditor, Display, TEXT("Unloading %s : %s"), *GetPathNameSafe(Level), Success ? TEXT("Success") : TEXT("Failure"));
		}
		else
		{
			UE_LOG(LogSpatialGDKEditor, Display, TEXT("%s has no loaded level, skipping"), *GetPathNameSafe(Level));
		}
	}
}

TArray<ULevelStreaming*> FSpatialGDKEditor::LoadAllStreamingLevels(UWorld* World)
{
	/*const TArray<ULevelStreaming*> StreamingLevels = World->GetStreamingLevels();
	UE_LOG(LogSpatialGDKEditor, Display, TEXT("Loading %d Streaming SubLevels"), StreamingLevels.Num());
	for (ULevelStreaming* StreamingLevel : StreamingLevels)
	{
		StreamingLevel->SetShouldBeVisible(true);
		StreamingLevel->SetShouldBeVisibleInEditor(false);
		StreamingLevel->bShouldBlockOnLoad = true;
		World->AddStreamingLevel(StreamingLevel);
	}*/

	{
		UE_LOG(LogSpatialGDKEditor, Display, TEXT("Loading All Assets"));
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

		TArray<ULevelStreaming*> StreamingTiles = World->WorldComposition->TilesStreaming;
		TArray<FName> StreamingDependencies;

		for (ULevelStreaming* Tile : StreamingTiles)
		{
			UE_LOG(LogSpatialGDKEditor, Display, TEXT("Get Deps for %s"), *GetPathNameSafe(Tile));
			AssetRegistryModule.Get().GetDependencies(Tile->GetWorldAssetPackageFName(), StreamingDependencies);
		}

		TArray<FAssetData> AssetData;

		for (FName Package : StreamingDependencies)
		{
			AssetRegistryModule.Get().GetAssetsByPackageName(Package, AssetData, true);
		}

		UE_LOG(LogSpatialGDKEditor, Display, TEXT("Got %d Assets from maps:"), AssetData.Num());

		for (const FAssetData& Data : AssetData)
		{
			UE_LOG(LogSpatialGDKEditor, Display, TEXT("%s:"), *Data.AssetName.ToString());

			if (Data.TagsAndValues.Contains("GeneratedClass") && !Data.IsAssetLoaded())
			{
				UE_LOG(LogSpatialGDKEditor, Display, TEXT("Loading Blueprint Asset %s before schema gen"), *Data.AssetName.ToString());
				UObject* Asset = Data.GetAsset();
				UE_LOG(LogSpatialGDKEditor, Display, TEXT("Loaded Asset %s"), *GetNameSafe(Asset));
			}

			for (TPair<FName, FString> KV : Data.TagsAndValues.GetMap())
			{
				if (KV.Key != "FiBData")
				{
					UE_LOG(LogSpatialGDKEditor, Display, TEXT("-  %s = %s"), *KV.Key.ToString(), *KV.Value);
				}
			}
		}
	}


	TArray<ULevelStreaming*> LoadedLevels;

	return LoadedLevels;

	// Ensure all world composition tiles are also loaded
	if (World->WorldComposition != nullptr)
	{
		TArray<ULevelStreaming*> StreamingTiles = World->WorldComposition->TilesStreaming;

		UE_LOG(LogSpatialGDKEditor, Display, TEXT("Loading %d World Composition Tiles"), StreamingTiles.Num());
		for (ULevelStreaming* StreamingLevel : StreamingTiles)
		{
			TSharedPtr<FStreamableHandle> RequestHandle;

			TSoftObjectPtr<UWorld> WorldPtr = StreamingLevel->GetWorldAsset();
			if (WorldPtr.IsValid())
			{
				UE_LOG(LogSpatialGDKEditor, Display, TEXT("Level %s Already loaded, skipping"), *StreamingLevel->GetWorldAssetPackageName());
				continue;
			}
			else
			{
				LoadedLevels.Add(StreamingLevel);
				//LoadedLevels.Add(EditorLevelUtils::AddLevelToWorld(World, *StreamingLevel->GetWorldAssetPackageName(), ULevelStreamingKismet::StaticClass()));
				/*UObject* LoadedAsset = UAssetManager::GetStreamableManager().LoadSynchronous(StreamingLevel->GetWorldAsset(), false, &RequestHandle);
				UE_LOG(LogSpatialGDKEditor, Display, TEXT("Loading Level Tile %s [%s]"), *StreamingLevel->GetWorldAssetPackageName(), *GetNameSafe(LoadedAsset));*/
				//LoadedAssets.Add(StreamingLevel->GetWorldAsset().ToSoftObjectPath());
			}

			/*
			StreamingLevel->SetShouldBeVisible(true);
			StreamingLevel->SetShouldBeVisibleInEditor(false);
			StreamingLevel->bShouldBlockOnLoad = true;
			World->AddStreamingLevel(StreamingLevel);*/
		}
	}

	/*World->FlushLevelStreaming(EFlushLevelStreamingType::Full);*/
	return LoadedLevels;
}
