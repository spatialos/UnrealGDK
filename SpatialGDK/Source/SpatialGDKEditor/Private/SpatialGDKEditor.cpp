// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditor.h"

#include "Async/Async.h"
#include "SpatialGDKEditorSchemaGenerator.h"
#include "SpatialGDKEditorSnapshotGenerator.h"

#include "Editor.h"
#include "FileHelpers.h"

#include "AssetRegistryModule.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/WorldSettings.h"
#include "GeneralProjectSettings.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/ScopedSlowTask.h"
#include "UObject/StrongObjectPtr.h"
#include "Engine/WorldComposition.h"



DEFINE_LOG_CATEGORY(LogSpatialGDKEditor);

#define LOCTEXT_NAMESPACE "FSpatialGDKEditor"

bool FSpatialGDKEditor::GenerateSchema(bool bFullScan)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	AssetRegistry.SearchAllAssets(true);

	TSet<FAssetData> AssetsToLoad;

	if (bFullScan)
	{
		TArray<FAssetData> AllAssetDatas;
		AssetRegistry.GetAllAssets(AllAssetDatas, true);
		AssetsToLoad = TSet<FAssetData>(AllAssetDatas);

		DeleteGeneratedSchemaFiles();
	}

	UE_LOG(LogTemp, Log, TEXT("Generating Schema, loading %d potential assets"), AssetsToLoad.Num());

	return GenerateSchema(AssetsToLoad);
}

bool FSpatialGDKEditor::GenerateSchema(TSet<FAssetData> AssetsToLoad)
{
	if (bSchemaGeneratorRunning)
	{
		UE_LOG(LogSpatialGDKEditor, Warning, TEXT("Schema generation is already running"));
		return false;
	}

	// If this has been run from an open editor then prompt the user to save dirty packages and maps.
	if (!IsRunningCommandlet())
	{
		const bool bPromptUserToSave = true;
		const bool bSaveMapPackages = true;
		const bool bSaveContentPackages = true;
		const bool bFastSave = false;
		const bool bNotifyNoPackagesSaved = false;
		const bool bCanBeDeclined = true;
		if (!FEditorFileUtils::SaveDirtyPackages(bPromptUserToSave, bSaveMapPackages, bSaveContentPackages, bFastSave, bNotifyNoPackagesSaved, bCanBeDeclined))
		{
			// User hit cancel don't generate schema.
			return false;
		}
	}

	bSchemaGeneratorRunning = true;

	// Total Progress = Loading Assets (Assets) + Generating Schema (10) + Garbage Collecting (0.5 * Assets)
	FScopedSlowTask Progress(AssetsToLoad.Num() * 1.5f + 10.f, LOCTEXT("GeneratingSchema", "Generating Schema..."));
	Progress.MakeDialog(true);

	// Force spatial networking so schema layouts are correct
	UGeneralProjectSettings* GeneralProjectSettings = GetMutableDefault<UGeneralProjectSettings>();
	bool bCachedSpatialNetworking = GeneralProjectSettings->bSpatialNetworking;
	GeneralProjectSettings->bSpatialNetworking = true;

	RemoveEditorAssetLoadedCallback();

	TArray<TStrongObjectPtr<UObject>> LoadedAssets;
	if (AssetsToLoad.Num() > 0)
	{
		Progress.EnterProgressFrame(static_cast<float>(AssetsToLoad.Num()));
		{
			bool bSuccess = LoadPotentialAssets(LoadedAssets, AssetsToLoad);
			if (!bSuccess)
			{
				bSchemaGeneratorRunning = false;
				LoadedAssets.Empty();
				CollectGarbage(RF_NoFlags, true);
				return false;
			}
		}
	}

	// If running from an open editor then compile all dirty blueprints
	TArray<UBlueprint*> ErroredBlueprints;
	if (!IsRunningCommandlet())
	{
		const bool bPromptForCompilation = false;
		UEditorEngine::ResolveDirtyBlueprints(bPromptForCompilation, ErroredBlueprints);
	}

	TryLoadExistingSchemaDatabase();

	Progress.EnterProgressFrame(10.f);
	bool bResult = SpatialGDKGenerateSchema();

	if (LoadedAssets.Num() > 0)
	{
		Progress.EnterProgressFrame(AssetsToLoad.Num() * 0.5f);
		LoadedAssets.Empty();
		CollectGarbage(RF_NoFlags, true);
	}
	
	// We delay printing this error until after the schema spam to make it have a higher chance of being noticed.
	if (ErroredBlueprints.Num() > 0)
	{
		UE_LOG(LogSpatialGDKEditor, Error, TEXT("Errors compiling blueprints during schema generation! The following blueprints did not have schema generated for them:"));
		for (const auto& Blueprint : ErroredBlueprints)
		{
			UE_LOG(LogSpatialGDKEditor, Error, TEXT("%s"), *GetPathNameSafe(Blueprint));
		}
	}

	GetMutableDefault<UGeneralProjectSettings>()->bSpatialNetworking = bCachedSpatialNetworking;
	bSchemaGeneratorRunning = false;

	if (bResult)
	{
		UE_LOG(LogSpatialGDKEditor, Display, TEXT("Schema Generation succeeded!"));
	}
	else
	{
		UE_LOG(LogSpatialGDKEditor, Error, TEXT("Schema Generation failed. View earlier log messages for errors."));
	}

	return bResult;
}

void FSpatialGDKEditor::GetWorldDependencies(UWorld* World, TSet<FAssetData>& OutAssets)
{
	TQueue<FName> DepsToSearch;
	DepsToSearch.Enqueue(*World->GetOutermost()->GetPathName());

	if (World->GetWorldSettings()->DefaultGameMode != nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("Adding GameModeOverride: %s"), *World->GetWorldSettings()->DefaultGameMode->GetOutermost()->GetPathName())
			DepsToSearch.Enqueue(*World->GetWorldSettings()->DefaultGameMode->GetOutermost()->GetPathName());
	}
	else
	{
		// Add default game mode
		for (FString GameMode : TArray<FString>{ TEXT("GlobalDefaultGameMode"), TEXT("GlobalDefaultServerGameMode") })
		{
			FString GameModePath;
			GConfig->GetString(
				TEXT("/Script/EngineSettings.GameMapsSettings"),
				*GameMode,
				GameModePath,
				GEngineIni
			);

			if (!GameModePath.IsEmpty())
			{
				UE_LOG(LogTemp, Log, TEXT("Adding %s %s"), *GameMode, *FSoftObjectPath(GameModePath).GetLongPackageName());
				DepsToSearch.Enqueue(*FSoftObjectPath(GameModePath).GetLongPackageName());
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Loading all deps for %s"), *World->GetOutermost()->GetPathName());

	if (World->WorldComposition != nullptr)
	{
		for (auto& Tile : World->WorldComposition->GetTilesList())
		{
			UE_LOG(LogTemp, Log, TEXT("Adding World Comp Tile %s to Deps"), *Tile.PackageName.ToString());
			DepsToSearch.Enqueue(Tile.PackageName);
		}
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	while(!DepsToSearch.IsEmpty())
	{
		FName NextDep;
		DepsToSearch.Dequeue(NextDep);
		FAssetData NextDepAsset = AssetRegistry.GetAssetByObjectPath(NextDep);

		if (OutAssets.Contains(NextDepAsset))
		{
			UE_LOG(LogTemp, Log, TEXT("Dep %s already contained in outdeps, skipping"), *NextDep.ToString());
			continue;
		}
		else
		{
			OutAssets.Add(NextDepAsset);
		}

		TArray<FName> FoundDeps;
		AssetRegistry.GetDependencies(NextDep, FoundDeps, EAssetRegistryDependencyType::All);
		UE_LOG(LogTemp, Log, TEXT("Found %d Deps of %s"), FoundDeps.Num(), *NextDep.ToString());

		for (FName FoundDep : FoundDeps)
		{
			DepsToSearch.Enqueue(FoundDep);
		}
	}
}

bool FSpatialGDKEditor::LoadPotentialAssets(TArray<TStrongObjectPtr<UObject>>& OutAssetsLoaded, TSet<FAssetData> AssetsToLoad)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Search project for all assets. This is required as the Commandlet will not have any paths cached.
	AssetRegistry.SearchAllAssets(true);

	TArray<FAssetData> FoundAssets = AssetsToLoad.Array();

	// Filter assets to game blueprint classes that are not loaded.
	FoundAssets = FoundAssets.FilterByPredicate([](FAssetData Data) {
		return (!Data.IsAssetLoaded() && Data.TagsAndValues.Contains("GeneratedClass") && Data.PackagePath.ToString().StartsWith("/Game"));
	});

	FScopedSlowTask Progress(static_cast<float>(FoundAssets.Num()), FText::FromString(FString::Printf(TEXT("Loading %d Assets before generating schema"), FoundAssets.Num())));

	for (const FAssetData& Data : FoundAssets)
	{
		if (Progress.ShouldCancel())
		{
			return false;
		}
		if (auto GeneratedClassPathPtr = Data.TagsAndValues.Find("GeneratedClass"))
		{
			const FString ClassObjectPath = FPackageName::ExportTextPathToObjectPath(*GeneratedClassPathPtr);
			const FString ClassName = FPackageName::ObjectPathToObjectName(ClassObjectPath);
			FSoftObjectPath SoftPath = FSoftObjectPath(ClassObjectPath);
			Progress.EnterProgressFrame(1.f, FText::FromString(FString::Printf(TEXT("Loading %s"), *Data.AssetName.ToString())));
			OutAssetsLoaded.Add(TStrongObjectPtr<UObject>(SoftPath.TryLoad()));
		}
	}

	return true;
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

void FSpatialGDKEditor::RemoveEditorAssetLoadedCallback()
{
	if (OnAssetLoadedHandle.IsValid())
	{
		return;
	}

	if (GEditor != nullptr)
	{
		UE_LOG(LogSpatialGDKEditor, Verbose, TEXT("Removing UEditorEngine::OnAssetLoaded."));
		FCoreUObjectDelegates::OnAssetLoaded.RemoveAll(GEditor);
		UE_LOG(LogSpatialGDKEditor, Verbose, TEXT("Replacing UEditorEngine::OnAssetLoaded with spatial version that won't run during schema gen."));
		OnAssetLoadedHandle = FCoreUObjectDelegates::OnAssetLoaded.AddLambda([this](UObject* Asset) {
			OnAssetLoaded(Asset);
		});
	}

}

// This callback is copied from UEditorEngine::OnAssetLoaded so that we can turn it off during schema gen in editor.
void FSpatialGDKEditor::OnAssetLoaded(UObject* Asset)
{
	// do not init worlds when running schema gen.
	if (bSchemaGeneratorRunning)
	{
		return;
	}

	if (UWorld* World = Cast<UWorld>(Asset))
	{
		// Init inactive worlds here instead of UWorld::PostLoad because it is illegal to call UpdateWorldComponents while IsRoutingPostLoad
		if (!World->bIsWorldInitialized && World->WorldType == EWorldType::Inactive)
		{
			// Create the world without a physics scene because creating too many physics scenes causes deadlock issues in PhysX. The scene will be created when it is opened in the level editor.
			// Also, don't create an FXSystem because it consumes too much video memory. This is also created when the level editor opens this world.
			World->InitWorld(UWorld::InitializationValues()
				.ShouldSimulatePhysics(false)
				.EnableTraceCollision(true)
				.CreatePhysicsScene(false)
				.CreateFXSystem(false)
			);

			// Update components so the scene is populated
			World->UpdateWorldComponents(true, true);
		}
	}
}

#undef LOCTEXT_NAMESPACE
