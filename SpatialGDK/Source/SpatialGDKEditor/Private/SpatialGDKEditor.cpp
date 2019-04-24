// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditor.h"

#include "Async/Async.h"
#include "SpatialGDKEditorCloudLauncher.h"
#include "SpatialGDKEditorSchemaGenerator.h"
#include "SpatialGDKEditorSnapshotGenerator.h"

#include "Editor.h"
#include "FileHelpers.h"

#include "AssetRegistryModule.h"
#include "GeneralProjectSettings.h"
#include "SpatialGDKEditorSettings.h"
#include "Misc/ScopedSlowTask.h"
#include "UObject/StrongObjectPtr.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKEditor);

#define LOCTEXT_NAMESPACE "FSpatialGDKEditor"

bool FSpatialGDKEditor::GenerateSchema(bool bFullScan)
{
	if (bSchemaGeneratorRunning)
	{
		UE_LOG(LogSpatialGDKEditor, Warning, TEXT("Schema generation is already running"));
		return false;
	}

	// Prompt the user to save packages/maps.
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

	bSchemaGeneratorRunning = true;

	// 80/10/10 load assets / gen schema / garbage collection.
	FScopedSlowTask Progress(100.f, LOCTEXT("GeneratingSchema", "Generating Schema..."));
	Progress.MakeDialog(true);

	// Force spatial networking so schema layouts are correct
	UGeneralProjectSettings* GeneralProjectSettings = GetMutableDefault<UGeneralProjectSettings>();
	bool bCachedSpatialNetworking = GeneralProjectSettings->bSpatialNetworking;
	GeneralProjectSettings->bSpatialNetworking = true;

	RemoveEditorAssetLoadedCallback();

	TArray<TStrongObjectPtr<UObject>> LoadedAssets;
	if (bFullScan)
	{
		Progress.EnterProgressFrame(80.f);
		if (!LoadPotentialAssets(LoadedAssets))
		{
			bSchemaGeneratorRunning = false;
			LoadedAssets.Empty();
			CollectGarbage(RF_NoFlags, true);
			return false;
		}
	}

	// Compile all dirty blueprints
	TArray<UBlueprint*> ErroredBlueprints;
	bool bPromptForCompilation = false;
	UEditorEngine::ResolveDirtyBlueprints(bPromptForCompilation, ErroredBlueprints);

	TryLoadExistingSchemaDatabase();

	Progress.EnterProgressFrame(bFullScan ? 10.f : 100.f);
	bool bResult = SpatialGDKGenerateSchema();

	if (bFullScan)
	{
		Progress.EnterProgressFrame(10.f);
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

	return bResult;
}

//bool FSpatialGDKEditor::SaveAllAssets()
//{
	// Set up the save package dialog
	//FPackagesDialogModule& PackagesDialogModule = FModuleManager::LoadModuleChecked<FPackagesDialogModule>(TEXT("PackagesDialog"));
	//PackagesDialogModule.CreatePackagesDialog(NSLOCTEXT("PackagesDialogModule", "PackagesDialogTitle", "Save Content"), NSLOCTEXT("PackagesDialogModule", "PackagesDialogMessage", "Select content to save."));
	//PackagesDialogModule.AddButton(DRT_Save, NSLOCTEXT("PackagesDialogModule", "SaveSelectedButton", "Save Selected"), NSLOCTEXT("PackagesDialogModule", "SaveSelectedButtonTip", "Attempt to save the selected content"));
	//PackagesDialogModule.AddButton(DRT_DontSave, NSLOCTEXT("PackagesDialogModule", "DontSaveSelectedButton", "Don't Save"), NSLOCTEXT("PackagesDialogModule", "DontSaveSelectedButtonTip", "Do not save any content"));
	//PackagesDialogModule.AddButton(DRT_Cancel, NSLOCTEXT("PackagesDialogModule", "CancelButton", "Cancel"), NSLOCTEXT("PackagesDialogModule", "CancelButtonTip", "Do not save any content and cancel the current operation"));

	//TArray<UPackage*> AddPackageItemsChecked;
	//TArray<UPackage*> AddPackageItemsUnchecked;
	//for (TArray<UPackage*>::TConstIterator PkgIter(InPackages); PkgIter; ++PkgIter)
	//{
	//	UPackage* CurPackage = *PkgIter;
	//	check(CurPackage);

	//	// If the caller set bCheckDirty to true, only consider dirty packages
	//	if (!bCheckDirty || (bCheckDirty && CurPackage->IsDirty()))
	//	{
	//		// Never save the transient package
	//		if (CurPackage != GetTransientPackage())
	//		{
	//			// Never save compiled in packages
	//			if (CurPackage->HasAnyPackageFlags(PKG_CompiledIn) == false)
	//			{
	//				if (UncheckedPackages.Contains(MakeWeakObjectPtr(CurPackage)))
	//				{
	//					AddPackageItemsUnchecked.Add(CurPackage);
	//				}
	//				else
	//				{
	//					AddPackageItemsChecked.Add(CurPackage);
	//				}
	//			}
	//			else
	//			{
	//				UE_LOG(LogFileHelpers, Warning, TEXT("PromptForCheckoutAndSave attempted to open the save dialog with a compiled in package: %s"), *CurPackage->GetName());
	//			}
	//		}
	//		else
	//		{
	//			UE_LOG(LogFileHelpers, Warning, TEXT("PromptForCheckoutAndSave attempted to open the save dialog with the transient package"));
	//		}
	//	}
	//}

	//if (AddPackageItemsUnchecked.Num() > 0 || AddPackageItemsChecked.Num() > 0)
	//{
	//	for (auto Iter = AddPackageItemsChecked.CreateIterator(); Iter; ++Iter)
	//	{
	//		PackagesDialogModule.AddPackageItem(*Iter, (*Iter)->GetName(), ECheckBoxState::Checked);
	//	}
	//	for (auto Iter = AddPackageItemsUnchecked.CreateIterator(); Iter; ++Iter)
	//	{
	//		PackagesDialogModule.AddPackageItem(*Iter, (*Iter)->GetName(), ECheckBoxState::Unchecked);
	//	}

	//	// If valid packages were added to the dialog, display it to the user
	//	const EDialogReturnType UserResponse = PackagesDialogModule.ShowPackagesDialog(PackagesNotSavedDuringSaveAll);

	//	// If the user has responded yes, they want to save the packages they have checked
	//	if (UserResponse == DRT_Save)
	//	{
	//		PackagesDialogModule.GetResults(FilteredPackages, ECheckBoxState::Checked);

	//		TArray<UPackage*> UncheckedPackagesRaw;
	//		PackagesDialogModule.GetResults(UncheckedPackagesRaw, ECheckBoxState::Unchecked);
	//		UncheckedPackages.Empty();
	//		for (UPackage* Package : UncheckedPackagesRaw)
	//		{
	//			UncheckedPackages.Add(MakeWeakObjectPtr(Package));
	//		}
	//	}
	//	// If the user has responded they don't wish to save, set the response type accordingly
	//	else if (UserResponse == DRT_DontSave)
	//	{
	//		ReturnResponse = PR_Declined;
	//	}
	//	// If the user has cancelled from the dialog, set the response type accordingly
	//	else
	//	{
	//		ReturnResponse = PR_Cancelled;
	//	}
	//}
//}

bool FSpatialGDKEditor::LoadPotentialAssets(TArray<TStrongObjectPtr<UObject>>& OutAssets)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	// Search project for all assets. This is required as the Commandlet will not have any paths cached.
	AssetRegistryModule.Get().SearchAllAssets(true);

	TArray<FAssetData> FoundAssets;
	AssetRegistryModule.Get().GetAllAssets(FoundAssets, true);

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
		Progress.EnterProgressFrame(1, FText::FromString(FString::Printf(TEXT("Loading %s"), *Data.AssetName.ToString())));
		if (auto GeneratedClassPathPtr = Data.TagsAndValues.Find("GeneratedClass"))
		{
			const FString ClassObjectPath = FPackageName::ExportTextPathToObjectPath(*GeneratedClassPathPtr);
			const FString ClassName = FPackageName::ObjectPathToObjectName(ClassObjectPath);
			FSoftObjectPath SoftPath = FSoftObjectPath(ClassObjectPath);
			OutAssets.Add(TStrongObjectPtr<UObject>(SoftPath.TryLoad()));
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

void FSpatialGDKEditor::LaunchCloudDeployment(FSimpleDelegate SuccessCallback, FSimpleDelegate FailureCallback)
{
	LaunchCloudResult = Async<bool>(EAsyncExecution::Thread, SpatialGDKCloudLaunch,
		[this, SuccessCallback, FailureCallback]
		{
			if (!LaunchCloudResult.IsReady() || LaunchCloudResult.Get() != true)
			{
				FailureCallback.ExecuteIfBound();
			}
			else
			{
				SuccessCallback.ExecuteIfBound();
			}
		});
}

void FSpatialGDKEditor::StopCloudDeployment(FSimpleDelegate SuccessCallback, FSimpleDelegate FailureCallback)
{
	StopCloudResult = Async<bool>(EAsyncExecution::Thread, SpatialGDKCloudStop,
		[this, SuccessCallback, FailureCallback]
		{
			if (!StopCloudResult.IsReady() || StopCloudResult.Get() != true)
			{
				FailureCallback.ExecuteIfBound();
			}
			else
			{
				SuccessCallback.ExecuteIfBound();
			}
		});
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
