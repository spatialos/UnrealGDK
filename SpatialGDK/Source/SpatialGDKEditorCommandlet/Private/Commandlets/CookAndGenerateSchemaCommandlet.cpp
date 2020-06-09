// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CookAndGenerateSchemaCommandlet.h"
#include "SpatialConstants.h"
#include "SpatialGDKEditorCommandletPrivate.h"
#include "SpatialGDKEditorSchemaGenerator.h"
#include "SpatialGDKEditorSettings.h"
#include "SpatialGDKServicesConstants.h"

#include "CookOnTheSide/CookOnTheFlyServer.h"
#include "Interfaces/ITargetPlatformManagerModule.h"
#include "Misc/RedirectCollector.h"
#include "Settings/ProjectPackagingSettings.h"

using namespace SpatialGDKEditor::Schema;

DEFINE_LOG_CATEGORY(LogCookAndGenerateSchemaCommandlet);

struct FObjectListener : public FUObjectArray::FUObjectCreateListener
{
public:
	void StartListening(TSet<FSoftClassPath>* ClassesFound)
	{
		VisitedClasses = ClassesFound;
		GUObjectArray.AddUObjectCreateListener(this);
	}

	void StopListening()
	{
		GUObjectArray.RemoveUObjectCreateListener(this);
	}

	virtual void NotifyUObjectCreated(const UObjectBase* Object, int32 Index) override
	{
		if (Object->GetClass() == UPackage::StaticClass())
		{
			NewlyLoadedPackages.Add(static_cast<const UPackage*>(Object));
		}

		FSoftClassPath SoftClass = FSoftClassPath(Object->GetClass());
		if (UnsupportedClasses.Contains(SoftClass))
		{
			return;
		}
		if (!VisitedClasses->Contains(SoftClass))
		{
			if (IsSupportedClass(Object->GetClass()))
			{
				UE_LOG(LogCookAndGenerateSchemaCommandlet, Verbose, TEXT("Object [%s] Created, Consider Class [%s] For Schema."),
					*Object->GetFName().ToString(), *GetPathNameSafe(Object->GetClass()));
				VisitedClasses->Add(SoftClass);
			}
			else
			{
				UnsupportedClasses.Add(SoftClass);
			}
		}
	}

	TSet<const UPackage*>& GetNewlyLoadedPackages()
	{
		return NewlyLoadedPackages;
	}

	virtual void OnUObjectArrayShutdown() override
	{
		GUObjectArray.RemoveUObjectCreateListener(this);
	}

private:
	TSet<FSoftClassPath>* VisitedClasses;
	TSet<FSoftClassPath> UnsupportedClasses;
	TSet<const UPackage*> NewlyLoadedPackages;
};

UCookAndGenerateSchemaCommandlet::UCookAndGenerateSchemaCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

int32 UCookAndGenerateSchemaCommandlet::Main(const FString& CmdLineParams)
{
	UE_LOG(LogCookAndGenerateSchemaCommandlet, Display, TEXT("Cook and Generate Schema Started."));

	TGuardValue<bool> UnattendedScriptGuard(GIsRunningUnattendedScript, GIsRunningUnattendedScript || IsRunningCommandlet());

	FObjectListener ObjectListener;
	TSet<FSoftClassPath> ReferencedClasses;
	ObjectListener.StartListening(&ReferencedClasses);

	UE_LOG(LogCookAndGenerateSchemaCommandlet, Display, TEXT("Try Load Schema Database."));
	if (IsAssetReadOnly(SpatialConstants::SCHEMA_DATABASE_FILE_PATH))
	{
		UE_LOG(LogCookAndGenerateSchemaCommandlet, Error, TEXT("Failed to load Schema Database."));
		return 0;
	}

	// UNR-1610 - This copy is a workaround to enable schema_compiler usage until FPL is ready. Without this prepare_for_run checks crash local launch and cloud upload.
	FString GDKSchemaCopyDir = FPaths::Combine(SpatialGDKServicesConstants::SpatialOSDirectory, TEXT("schema/unreal/gdk"));
	FString CoreSDKSchemaCopyDir = FPaths::Combine(SpatialGDKServicesConstants::SpatialOSDirectory, TEXT("build/dependencies/schema/standard_library"));
	SpatialGDKEditor::Schema::CopyWellKnownSchemaFiles(GDKSchemaCopyDir, CoreSDKSchemaCopyDir);
	SpatialGDKEditor::Schema::RefreshSchemaFiles(GetDefault<USpatialGDKEditorSettings>()->GetGeneratedSchemaOutputFolder());

	if (!LoadGeneratorStateFromSchemaDatabase(SpatialConstants::SCHEMA_DATABASE_FILE_PATH))
	{
		ResetSchemaGeneratorStateAndCleanupFolders();
	}

	UE_LOG(LogCookAndGenerateSchemaCommandlet, Display, TEXT("Finding supported C++ and in-memory Classes."));

	TArray<UObject*> AllClasses;
	GetObjectsOfClass(UClass::StaticClass(), AllClasses);
	for (const auto& SupportedClass : GetAllSupportedClasses(AllClasses))
	{
		ReferencedClasses.Add(FSoftClassPath(SupportedClass));
	}

	UE_LOG(LogCookAndGenerateSchemaCommandlet, Display, TEXT("Starting Cook Command."));
	int32 CookResult = MainCook(CmdLineParams, ObjectListener);
	UE_LOG(LogCookAndGenerateSchemaCommandlet, Display, TEXT("Cook Command Completed."));

	UE_LOG(LogCookAndGenerateSchemaCommandlet, Display, TEXT("Discovered %d Classes during cook."), ReferencedClasses.Num());

	ObjectListener.StopListening();

	// Sort classes here so that batching does not have an effect on ordering.
	ReferencedClasses.Sort([](const FSoftClassPath& A, const FSoftClassPath& B)
	{
		return FNameLexicalLess()(A.GetAssetPathName(), B.GetAssetPathName());
	});

	UE_LOG(LogCookAndGenerateSchemaCommandlet, Display, TEXT("Start Schema Generation for discovered assets."));
	FDateTime StartTime = FDateTime::Now();
	TSet<UClass*> Classes;
	const int BatchSize = 100;
	for (const FSoftClassPath& SoftPath : ReferencedClasses)
	{
		if (UClass* LoadedClass = SoftPath.TryLoadClass<UObject>())
		{
			UE_LOG(LogCookAndGenerateSchemaCommandlet, Verbose, TEXT("Reloaded %s, adding to batch"), *GetPathNameSafe(LoadedClass));
			Classes.Add(LoadedClass);
			if (Classes.Num() >= BatchSize)
			{
				SpatialGDKGenerateSchemaForClasses(Classes);
				Classes.Empty(BatchSize);
			}
		}
		else
		{
			UE_LOG(LogCookAndGenerateSchemaCommandlet, Warning, TEXT("Failed to reload %s"), *SoftPath.ToString());
		}
	}
	SpatialGDKGenerateSchemaForClasses(Classes);

	GenerateSchemaForSublevels();
	GenerateSchemaForRPCEndpoints();
	GenerateSchemaForNCDs();

	FTimespan Duration = FDateTime::Now() - StartTime;

	UE_LOG(LogCookAndGenerateSchemaCommandlet, Display, TEXT("Schema Generation Finished in %.2f seconds"), Duration.GetTotalSeconds());

	if (!RunSchemaCompiler())
	{
		UE_LOG(LogCookAndGenerateSchemaCommandlet, Error, TEXT("Failed to run schema compiler."));
		return 0;
	}

	if (!SaveSchemaDatabase(SpatialConstants::SCHEMA_DATABASE_ASSET_PATH))
	{
		UE_LOG(LogCookAndGenerateSchemaCommandlet, Error, TEXT("Failed to save schema database."));
		return 0;
	}

	return CookResult;
}

int32 UCookAndGenerateSchemaCommandlet::MainCook(const FString& CmdLineParams, FObjectListener& Listener)
{
	if (0)
	{
		return Super::Main(CmdLineParams);
	}
	else
	{
		// Copied from UCookCommandlet::CookByTheBook to fill cook parameters

		ITargetPlatformManagerModule& TPM = GetTargetPlatformManagerRef();
		const TArray<ITargetPlatform*>& Platforms = TPM.GetActiveTargetPlatforms();

		TArray<FString> CmdLineMapEntries;
		TArray<FString> CmdLineDirEntries;
		TArray<FString> CmdLineCultEntries;
		TArray<FString> CmdLineNeverCookDirEntries;
		for (int32 SwitchIdx = 0; SwitchIdx < Switches.Num(); SwitchIdx++)
		{
			const FString& Switch = Switches[SwitchIdx];

			auto GetSwitchValueElements = [&Switch](const FString SwitchKey) -> TArray<FString>
			{
				TArray<FString> ValueElements;
				if (Switch.StartsWith(SwitchKey + TEXT("=")) == true)
				{
					FString ValuesList = Switch.Right(Switch.Len() - (SwitchKey + TEXT("=")).Len());

					// Allow support for -KEY=Value1+Value2+Value3 as well as -KEY=Value1 -KEY=Value2
					for (int32 PlusIdx = ValuesList.Find(TEXT("+"), ESearchCase::CaseSensitive); PlusIdx != INDEX_NONE; PlusIdx = ValuesList.Find(TEXT("+"), ESearchCase::CaseSensitive))
					{
						const FString ValueElement = ValuesList.Left(PlusIdx);
						ValueElements.Add(ValueElement);

						ValuesList = ValuesList.Right(ValuesList.Len() - (PlusIdx + 1));
					}
					ValueElements.Add(ValuesList);
				}
				return ValueElements;
			};

			// Check for -MAP=<name of map> entries
			CmdLineMapEntries += GetSwitchValueElements(TEXT("MAP"));

			// Check for -COOKCULTURES=<culture name> entries
			CmdLineCultEntries += GetSwitchValueElements(TEXT("COOKCULTURES"));
		}

		// Also append any cookdirs from the project ini files; these dirs are relative to the game content directory or start with a / root
		{
			const FString AbsoluteGameContentDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());
			const UProjectPackagingSettings* const PackagingSettings = GetDefault<UProjectPackagingSettings>();
			for (const FDirectoryPath& DirToCook : PackagingSettings->DirectoriesToAlwaysCook)
			{
				if (DirToCook.Path.StartsWith(TEXT("/"), ESearchCase::CaseSensitive))
				{
					// If this starts with /, this includes a root like /engine
					FString RelativePath = FPackageName::LongPackageNameToFilename(DirToCook.Path / TEXT(""));
					CmdLineDirEntries.Add(FPaths::ConvertRelativePathToFull(RelativePath));
				}
				else
				{
					// This is relative to /game
					CmdLineDirEntries.Add(AbsoluteGameContentDir / DirToCook.Path);
				}
			}
		}

		TArray<FString> AlwaysCookMapList;
		TArray<FString> MapList;
		// Add any map specified on the command line.
		for (const auto& MapName : CmdLineMapEntries)
		{
			MapList.Add(MapName);
		}

		TArray<FString> MapIniSections;
		FString SectionStr;
		if (FParse::Value(*Params, TEXT("MAPINISECTION="), SectionStr))
		{
			if (SectionStr.Contains(TEXT("+")))
			{
				TArray<FString> Sections;
				SectionStr.ParseIntoArray(Sections, TEXT("+"), true);
				for (int32 Index = 0; Index < Sections.Num(); Index++)
				{
					MapIniSections.Add(Sections[Index]);
				}
			}
			else
			{
				MapIniSections.Add(SectionStr);
			}
		}

		// If we still don't have any mapsList check if the allmaps ini section is filled out
		// this is for backwards compatibility
		if (MapList.Num() == 0 && MapIniSections.Num() == 0)
		{
			MapIniSections.Add(FString(TEXT("AllMaps")));
		}

		if (!bCookSinglePackage)
		{
			// Put the always cook map list at the front of the map list
			AlwaysCookMapList.Append(MapList);
			Swap(MapList, AlwaysCookMapList);
		}

		// Set the list of cultures to cook as those on the commandline, if specified.
		// Otherwise, use the project packaging settings.
		TArray<FString> CookCultures;
		if (Switches.ContainsByPredicate([](const FString& Switch) -> bool
		{
			return Switch.StartsWith("COOKCULTURES=");
		}))
		{
			CookCultures = CmdLineCultEntries;
		}
		else
		{
			UProjectPackagingSettings* const PackagingSettings = Cast<UProjectPackagingSettings>(UProjectPackagingSettings::StaticClass()->GetDefaultObject());
			CookCultures = PackagingSettings->CulturesToStage;
		}

		//////////////////////////////////////////////////////////////////////////
		// start cook by the book 
		ECookByTheBookOptions CookOptions = ECookByTheBookOptions::None;

		CookOptions |= bCookAll ? ECookByTheBookOptions::CookAll : ECookByTheBookOptions::None;
		CookOptions |= Switches.Contains(TEXT("MAPSONLY")) ? ECookByTheBookOptions::MapsOnly : ECookByTheBookOptions::None;
		CookOptions |= Switches.Contains(TEXT("NODEV")) ? ECookByTheBookOptions::NoDevContent : ECookByTheBookOptions::None;
		CookOptions |= Switches.Contains(TEXT("FullLoadAndSave")) ? ECookByTheBookOptions::FullLoadAndSave : ECookByTheBookOptions::None;
		CookOptions |= Switches.Contains(TEXT("PackageStore")) ? ECookByTheBookOptions::PackageStore : ECookByTheBookOptions::None;

		const ECookByTheBookOptions SinglePackageFlags = ECookByTheBookOptions::NoAlwaysCookMaps | ECookByTheBookOptions::NoDefaultMaps | ECookByTheBookOptions::NoGameAlwaysCookPackages | ECookByTheBookOptions::NoInputPackages | ECookByTheBookOptions::NoSlatePackages | ECookByTheBookOptions::DisableUnsolicitedPackages | ECookByTheBookOptions::ForceDisableSaveGlobalShaders;
		CookOptions |= bCookSinglePackage ? SinglePackageFlags : ECookByTheBookOptions::None;

		UCookOnTheFlyServer::FCookByTheBookStartupOptions StartupOptions;

		StartupOptions.TargetPlatforms = Platforms;
		Swap(StartupOptions.CookMaps, MapList);
		Swap(StartupOptions.CookDirectories, CmdLineDirEntries);
		Swap(StartupOptions.NeverCookDirectories, CmdLineNeverCookDirEntries);
		Swap(StartupOptions.CookCultures, CookCultures);
		//Swap(StartupOptions.DLCName, DLCName);
		//Swap(StartupOptions.BasedOnReleaseVersion, BasedOnReleaseVersion);
		//Swap(StartupOptions.CreateReleaseVersion, CreateReleaseVersion);
		Swap(StartupOptions.IniMapSections, MapIniSections);
		StartupOptions.CookOptions = CookOptions;
		StartupOptions.bErrorOnEngineContentUse = bErrorOnEngineContentUse;
		StartupOptions.bGenerateDependenciesForMaps = Switches.Contains(TEXT("GenerateDependenciesForMaps"));
		StartupOptions.bGenerateStreamingInstallManifests = bGenerateStreamingInstallManifests;


		UCookOnTheFlyServer* CookServer = NewObject<UCookOnTheFlyServer>();

		CookServer->Initialize(ECookMode::CookByTheBook, ECookInitializationFlags::None);

		CookServer->StartCookByTheBook(StartupOptions);


		// Manually pump the package loading

		TSet<FName> LoadedPackages;
		LoadedPackages.Reserve(1 << 16);
		TArray<FName> PackagesToLoad = CookServer->GetPackagesToCook();

		while (PackagesToLoad.Num() > 0)
		{
			int32 CurLoaded = LoadedPackages.Num();
			while (PackagesToLoad.Num() > 0)
			{
				FName Asset = PackagesToLoad.Pop();
				if (!LoadedPackages.Contains(Asset))
				{
					if (UPackage* Package = CookServer->LoadPackageForCooking(Asset.ToString()))
					{
						LoadedPackages.Add(Asset);
					}
				}
			}
			PackagesToLoad = CookServer->GetPackagesToCook();
			CookServer->ProcessUnsolicitedPackages();

			TSet<const UPackage*>& NewlyLoadedPackages = Listener.GetNewlyLoadedPackages();
			for (auto Package : NewlyLoadedPackages)
			{
				TSet<FName> SoftObjectPackages;

				GRedirectCollector.ProcessSoftObjectPathPackageList(Package->GetFName(), false, SoftObjectPackages);

				for (FName SoftObjectPackage : SoftObjectPackages)
				{
					TMap<FName, FName> RedirectedPaths;

					// If this is a redirector, extract destination from asset registry
					if (CookServer->ContainsRedirector(SoftObjectPackage, RedirectedPaths))
					{
						for (TPair<FName, FName>& RedirectedPath : RedirectedPaths)
						{
							GRedirectCollector.AddAssetPathRedirection(RedirectedPath.Key, RedirectedPath.Value);
						}
					}

					//// Verify package actually exists
					FName StandardPackageName = CookServer->GetCachedStandardPackageFileFName(SoftObjectPackage);

					if (StandardPackageName != NAME_None)
					{
						PackagesToLoad.Add(StandardPackageName);
					}
				}
			}
			NewlyLoadedPackages.Empty();
			if (CurLoaded == LoadedPackages.Num())
			{
				break;
			}

			if (CurLoaded == LoadedPackages.Num())
			{
				break;
			}
		}

		return 0;
	}
	
}
