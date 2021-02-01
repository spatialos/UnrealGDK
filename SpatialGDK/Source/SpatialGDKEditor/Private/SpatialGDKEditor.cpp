// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditor.h"

#include "AssetDataTagMap.h"
#include "AssetRegistryModule.h"
#include "Async/Async.h"
#include "Editor.h"
#include "FileHelpers.h"
#include "GeneralProjectSettings.h"
#include "IUATHelperModule.h"
#include "Internationalization/Regex.h"
#include "Misc/MessageDialog.h"
#include "Misc/ScopedSlowTask.h"
#include "PackageTools.h"
#include "Settings/ProjectPackagingSettings.h"
#include "UObject/StrongObjectPtr.h"
#include "UnrealEdMisc.h"

#include "SpatialGDKDevAuthTokenGenerator.h"
#include "SpatialGDKEditorCloudLauncher.h"
#include "SpatialGDKEditorPackageAssembly.h"
#include "SpatialGDKEditorSchemaGenerator.h"
#include "SpatialGDKEditorSettings.h"
#include "SpatialGDKEditorSnapshotGenerator.h"
#include "SpatialGDKServicesConstants.h"

using namespace SpatialGDKEditor;

DEFINE_LOG_CATEGORY(LogSpatialGDKEditor);

#define LOCTEXT_NAMESPACE "FSpatialGDKEditor"

namespace
{
bool CheckAutomationToolsUpToDate()
{
#if PLATFORM_WINDOWS
	FString RunUATScriptName = TEXT("RunUAT.bat");
	FString CmdExe = TEXT("cmd.exe");
#elif PLATFORM_LINUX
	FString RunUATScriptName = TEXT("RunUAT.sh");
	FString CmdExe = TEXT("/bin/bash");
#else
	FString RunUATScriptName = TEXT("RunUAT.command");
	FString CmdExe = TEXT("/bin/sh");
#endif

	FString UatPath = FPaths::ConvertRelativePathToFull(FPaths::EngineDir() / TEXT("Build/BatchFiles") / RunUATScriptName);

	if (!FPaths::FileExists(UatPath))
	{
		FFormatNamedArguments Arguments;
		Arguments.Add(TEXT("File"), FText::FromString(UatPath));
		FMessageDialog::Open(
			EAppMsgType::Ok,
			FText::Format(LOCTEXT("RequiredFileNotFoundMessage", "A required file could not be found:\n{File}"), Arguments));

		return false;
	}

#if PLATFORM_WINDOWS
	FString FullCommandLine = FString::Printf(TEXT("/c \"\"%s\" -list\""), *UatPath);
#else
	FString FullCommandLine = FString::Printf(TEXT("\"%s\" -list"), *UatPath);
#endif

	FString ListCommandResult;
	int32 ResultCode = -1;
	FSpatialGDKServicesModule::ExecuteAndReadOutput(CmdExe, FullCommandLine, FPaths::EngineDir(), ListCommandResult, ResultCode);

	if (ResultCode != 0)
	{
		UE_LOG(LogSpatialGDKEditor, Error, TEXT("Automation tool execution error : %i"), ResultCode);
		return false;
	}

	if (ListCommandResult.Find(TEXT("CookAndGenerateSchema")) >= 0)
	{
		return true;
	}

	FMessageDialog::Open(EAppMsgType::Ok,
						 LOCTEXT("GenerateSchemaUATOutOfDate",
								 "Could not generate Schema because the AutomationTool is out of date.\n"
								 "Please rebuild the AutomationTool project which can be found alongside the UE4 project files"));

	return false;
}

} // namespace

FSpatialGDKEditor::FSpatialGDKEditor()
	: bSchemaGeneratorRunning(false)
	, SpatialGDKDevAuthTokenGeneratorInstance(MakeShared<FSpatialGDKDevAuthTokenGenerator>())
	, SpatialGDKPackageAssemblyInstance(MakeShared<FSpatialGDKPackageAssembly>())
{
}

void FSpatialGDKEditor::GenerateSchema(ESchemaGenerationMethod Method, TFunction<void(bool)> ResultCallback)
{
	if (bSchemaGeneratorRunning)
	{
		UE_LOG(LogSpatialGDKEditor, Warning, TEXT("Schema generation is already running"));
		ResultCallback(false);
		return;
	}

	if (!FPaths::IsProjectFilePathSet())
	{
		UE_LOG(LogSpatialGDKEditor, Error, TEXT("Schema generation called when no project was opened"));
		ResultCallback(false);
		return;
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
		if (!FEditorFileUtils::SaveDirtyPackages(bPromptUserToSave, bSaveMapPackages, bSaveContentPackages, bFastSave,
												 bNotifyNoPackagesSaved, bCanBeDeclined))
		{
			// User hit cancel don't generate schema.
			ResultCallback(false);
			return;
		}
	}

	if (Schema::IsAssetReadOnly(SpatialConstants::SCHEMA_DATABASE_FILE_PATH))
	{
		ResultCallback(false);
		return;
	}

	if (Method == FullAssetScan)
	{
		if (!CheckAutomationToolsUpToDate())
		{
			ResultCallback(false);
			return;
		}

		// Make sure SchemaDatabase is not loaded.
		if (UPackage* LoadedDatabase = FindPackage(nullptr, *FPaths::Combine(TEXT("/Game/"), *SpatialConstants::SCHEMA_DATABASE_FILE_PATH)))
		{
			TArray<UPackage*> ToUnload;
			ToUnload.Add(LoadedDatabase);
			UPackageTools::UnloadPackages(ToUnload);
		}

		const USpatialGDKEditorSettings* EditorSettings = GetDefault<USpatialGDKEditorSettings>();

		const FString& PlatformName = EditorSettings->GetCookAndGenerateSchemaTargetPlatform();

		if (PlatformName.IsEmpty())
		{
			UE_LOG(LogSpatialGDKEditor, Error, TEXT("Empty platform passed to CookAndGenerateSchema"));
			ResultCallback(false);
			return;
		}

		FString OptionalParams = EditorSettings->GetCookAndGenerateSchemaAdditionalArgs();
		OptionalParams += FString::Printf(TEXT(" -targetplatform=%s"), *PlatformName);

		FString ProjectPath = FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath());
		FString UATCommandLine = FString::Printf(TEXT("-ScriptsForProject=\"%s\" CookAndGenerateSchema -nocompile -nocompileeditor -server "
													  "-noclient %s -nop4 -project=\"%s\" -cook -skipstage -ue4exe=\"%s\" %s -utf8output"),
												 *ProjectPath, FApp::IsEngineInstalled() ? TEXT(" -installed") : TEXT(""), *ProjectPath,
												 *FUnrealEdMisc::Get().GetExecutableForCommandlets(), *OptionalParams);

		bSchemaGeneratorRunning = true;
		TFunction<void(FString, double)> Callback = [this, ResultCallback = MoveTemp(ResultCallback)](const FString& UATResult, double) {
			ResultCallback(UATResult == FString(TEXT("Completed")));
			bSchemaGeneratorRunning = false;
		};
		IUATHelperModule::Get().CreateUatTask(UATCommandLine, FText::FromString(PlatformName),
											  LOCTEXT("CookAndGenerateSchemaTaskName", "Cook and generate project schema"),
											  LOCTEXT("CookAndGenerateSchemaTaskShortName", "Generating schema"),
											  FEditorStyle::GetBrush(TEXT("MainFrame.PackageProject")), MoveTemp(Callback));

		return;
	}
	else
	{
		bSchemaGeneratorRunning = true;

		FScopedSlowTask Progress(100.f, LOCTEXT("GeneratingSchema", "Generating schema..."));
		Progress.MakeDialog(true);

		RemoveEditorAssetLoadedCallback();

		if (!Schema::LoadGeneratorStateFromSchemaDatabase(SpatialConstants::SCHEMA_DATABASE_FILE_PATH))
		{
			Schema::ResetSchemaGeneratorStateAndCleanupFolders();
		}

		// If running from an open editor then compile all dirty blueprints
		TArray<UBlueprint*> ErroredBlueprints;
		if (!IsRunningCommandlet())
		{
			const bool bPromptForCompilation = false;
			UEditorEngine::ResolveDirtyBlueprints(bPromptForCompilation, ErroredBlueprints);
		}

		Progress.EnterProgressFrame(100.f);

		bool bResult = Schema::SpatialGDKGenerateSchema();

		// We delay printing this error until after the schema spam to make it have a higher chance of being noticed.
		if (ErroredBlueprints.Num() > 0)
		{
			UE_LOG(LogSpatialGDKEditor, Error,
				   TEXT("Errors compiling blueprints during schema generation! The following blueprints did not have schema generated for "
						"them:"));
			for (const auto& Blueprint : ErroredBlueprints)
			{
				UE_LOG(LogSpatialGDKEditor, Error, TEXT("%s"), *GetPathNameSafe(Blueprint));
			}
		}

		bSchemaGeneratorRunning = false;

		if (bResult)
		{
			UE_LOG(LogSpatialGDKEditor, Display, TEXT("Schema generation succeeded!"));
		}
		else
		{
			UE_LOG(LogSpatialGDKEditor, Error, TEXT("Schema generation failed. View earlier log messages for errors."));
		}

		ResultCallback(bResult);
		return;
	}
}

FSpatialGDKEditor::ESchemaDatabaseValidationResult FSpatialGDKEditor::ValidateSchemaDatabase()
{
	FString GdkFolderPath = FPaths::Combine(SpatialGDKServicesConstants::SpatialOSDirectory, TEXT("schema/unreal/gdk"));
	if (!FPaths::DirectoryExists(GdkFolderPath))
	{
		return ESchemaDatabaseValidationResult::NotFound;
	}

	return SpatialGDKEditor::Schema::ValidateSchemaDatabase();
}

bool FSpatialGDKEditor::LoadPotentialAssets(TArray<TStrongObjectPtr<UObject>>& OutAssets)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	// Search project for all assets. This is required as the Commandlet will not have any paths cached.
	AssetRegistryModule.Get().SearchAllAssets(true);

	TArray<FAssetData> FoundAssets;
	AssetRegistryModule.Get().GetAllAssets(FoundAssets, true);

	const TArray<FDirectoryPath>& DirectoriesToNeverCook = GetDefault<UProjectPackagingSettings>()->DirectoriesToNeverCook;

	// Filter assets to game blueprint classes that are not loaded and not inside DirectoriesToNeverCook.
	FoundAssets = FoundAssets.FilterByPredicate([&DirectoriesToNeverCook](const FAssetData& Data) {
		if (Data.IsAssetLoaded())
		{
			return false;
		}
		if (!Data.TagsAndValues.Contains("GeneratedClass"))
		{
			return false;
		}
		const FString PackagePath = Data.PackagePath.ToString();

		for (const auto& Directory : DirectoriesToNeverCook)
		{
			if (PackagePath.StartsWith(Directory.Path))
			{
				return false;
			}
		}
		return true;
	});

	FScopedSlowTask Progress(
		static_cast<float>(FoundAssets.Num()),
		FText::Format(LOCTEXT("LoadingAssets_Text", "Loading {0} assets before generating schema"), FoundAssets.Num()));

	for (const FAssetData& Data : FoundAssets)
	{
		if (Progress.ShouldCancel())
		{
			return false;
		}
		Progress.EnterProgressFrame(1, FText::Format(LOCTEXT("LoadingSingleAsset_Text", "Loading {0}"), FText::FromName(Data.AssetName)));

		const FString* GeneratedClassPathPtr = nullptr;

		FAssetDataTagMapSharedView::FFindTagResult GeneratedClassFindTagResult = Data.TagsAndValues.FindTag("GeneratedClass");
		if (GeneratedClassFindTagResult.IsSet())
		{
			GeneratedClassPathPtr = &GeneratedClassFindTagResult.GetValue();
		}

		if (GeneratedClassPathPtr != nullptr)
		{
			const FString ClassObjectPath = FPackageName::ExportTextPathToObjectPath(*GeneratedClassPathPtr);
			const FString ClassName = FPackageName::ObjectPathToObjectName(ClassObjectPath);
			FSoftObjectPath SoftPath = FSoftObjectPath(ClassObjectPath);
			OutAssets.Add(TStrongObjectPtr<UObject>(SoftPath.TryLoad()));
		}
	}

	return true;
}

void FSpatialGDKEditor::GenerateSnapshot(UWorld* World, FString SnapshotFilename, FSimpleDelegate SuccessCallback,
										 FSimpleDelegate FailureCallback, FSpatialGDKEditorErrorHandler ErrorCallback)
{
	const USpatialGDKEditorSettings* Settings = GetDefault<USpatialGDKEditorSettings>();
	FString SavePath = FPaths::Combine(SpatialGDKServicesConstants::SpatialOSSnapshotFolderPath, SnapshotFilename);
	const bool bSuccess = SpatialGDKGenerateSnapshot(World, SavePath);

	if (bSuccess)
	{
		SuccessCallback.ExecuteIfBound();
	}
	else
	{
		FailureCallback.ExecuteIfBound();
	}
}

void FSpatialGDKEditor::StartCloudDeployment(const FCloudDeploymentConfiguration& Configuration, FSimpleDelegate SuccessCallback,
											 FSimpleDelegate FailureCallback)
{
	LaunchCloudResult = Async(
		EAsyncExecution::Thread,
		[&Configuration]() {
			return SpatialGDKCloudLaunch(Configuration);
		},
		[this, SuccessCallback, FailureCallback] {
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
	StopCloudResult = Async(EAsyncExecution::Thread, SpatialGDKCloudStop, [this, SuccessCallback, FailureCallback] {
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

bool FSpatialGDKEditor::FullScanRequired()
{
	return !Schema::GeneratedSchemaFolderExists() || (Schema::ValidateSchemaDatabase() != FSpatialGDKEditor::Ok);
}

void FSpatialGDKEditor::SetProjectName(const FString& InProjectName)
{
	if (!FSpatialGDKServicesModule::GetProjectName().Equals(InProjectName))
	{
		FSpatialGDKServicesModule::SetProjectName(InProjectName);
		SpatialGDKDevAuthTokenGeneratorInstance->AsyncGenerateDevAuthToken();
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
		UE_LOG(LogSpatialGDKEditor, Verbose,
			   TEXT("Replacing UEditorEngine::OnAssetLoaded with spatial version that won't run during schema gen."));
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
			// Create the world without a physics scene because creating too many physics scenes causes deadlock issues in PhysX. The scene
			// will be created when it is opened in the level editor. Also, don't create an FXSystem because it consumes too much video
			// memory. This is also created when the level editor opens this world.
			World->InitWorld(UWorld::InitializationValues()
								 .ShouldSimulatePhysics(false)
								 .EnableTraceCollision(true)
								 .CreatePhysicsScene(false)
								 .CreateFXSystem(false));

			// Update components so the scene is populated
			World->UpdateWorldComponents(true, true);
		}
	}
}

TSharedRef<FSpatialGDKDevAuthTokenGenerator> FSpatialGDKEditor::GetDevAuthTokenGeneratorRef()
{
	return SpatialGDKDevAuthTokenGeneratorInstance;
}

TSharedRef<FSpatialGDKPackageAssembly> FSpatialGDKEditor::GetPackageAssemblyRef()
{
	return SpatialGDKPackageAssemblyInstance;
}

#undef LOCTEXT_NAMESPACE
