// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GenerateSchemaCommandlet.h"
#include "SpatialConstants.h"
#include "SpatialGDKEditor.h"
#include "SpatialGDKEditorCommandletPrivate.h"
#include "SpatialGDKEditorSchemaGenerator.h"
#include "Modules/ModuleManager.h"
#include "Engine/World.h"
#include "AssetRegistryModule.h"

using namespace SpatialGDKEditor::Schema;

DEFINE_LOG_CATEGORY(LogGenerateSchemaCommandlet);

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
		FSoftClassPath SoftClass = FSoftClassPath(Object->GetClass());
		if (UnsupportedClasses.Contains(SoftClass))
		{
			return;
		}
		if (!VisitedClasses->Contains(SoftClass))
		{
			if (IsSupportedClass(Object->GetClass()))
			{
				UE_LOG(LogGenerateSchemaCommandlet, Verbose, TEXT("Object [%s] Created, Consider Class [%s] For Schema."),
					*Object->GetFName().ToString(), *GetPathNameSafe(Object->GetClass()));
				VisitedClasses->Add(SoftClass);
			}
			else
			{
				UnsupportedClasses.Add(SoftClass);
			}
		}
	}

#if ENGINE_MINOR_VERSION >= 23
	virtual void OnUObjectArrayShutdown() override
	{
		GUObjectArray.RemoveUObjectCreateListener(this);
	}
#endif

private:
	TSet<FSoftClassPath>* VisitedClasses;
	TSet<FSoftClassPath> UnsupportedClasses;
};

UGenerateSchemaCommandlet::UGenerateSchemaCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

void UGenerateSchemaCommandlet::CompileBlueprint(UBlueprint* Blueprint)
{
	// Don't actually compile anything
	return;
}

void UGenerateSchemaCommandlet::LoadLevelBlueprints()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetData;
	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.ClassNames.Add(UWorld::StaticClass()->GetFName());
	for (const FString MapPackagePath : MapPackagePaths)
	{
		Filter.PackagePaths.Add(FName(*MapPackagePath));
	}
	
	AssetRegistryModule.Get().GetAssets(Filter, AssetData);

	UE_LOG(LogGenerateSchemaCommandlet, Display, TEXT("Found Maps:"));
	for (FAssetData Asset : AssetData)
	{
		UE_LOG(LogGenerateSchemaCommandlet, Display, TEXT("- %s"), *Asset.ObjectPath.ToString());
		FString PackageName, ObjectName;

		if (Asset.ObjectPath.ToString().Split(TEXT("."), &PackageName, &ObjectName, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
		{
			UE_LOG(LogGenerateSchemaCommandlet, Display, TEXT("  Making Linker to look for %s_C in %s"), *ObjectName, *PackageName);
			FLinkerLoad* Linker = LoadPackageLinker(nullptr, *PackageName, LOAD_NoVerify);
			if (Linker)
			{
				UE_LOG(LogGenerateSchemaCommandlet, Display, TEXT("  Linker Valid, searching %d exports"), Linker->ExportMap.Num());
				FName ObjectFName = FName(*FString::Printf(TEXT("%s_C"), *ObjectName));
				for (FObjectExport Export : Linker->ExportMap)
				{
					if (Export.ObjectName == ObjectFName)
					{
						UE_LOG(LogGenerateSchemaCommandlet, Display, TEXT("  Found %s, loading"), *ObjectFName.ToString());
						UBlueprintGeneratedClass* LevelBP = LoadObject<UBlueprintGeneratedClass>(nullptr, *FString::Printf(TEXT("%s_C"), *Asset.ObjectPath.ToString()), nullptr,
							LOAD_NoWarn | LOAD_NoVerify | LOAD_Quiet);
						if (LevelBP)
						{
							UE_LOG(LogGenerateSchemaCommandlet, Display, TEXT("Loaded Level BP Class: %s"), *GetPathNameSafe(LevelBP));
						}
						break;
					}
				}
			}
		}
	}	
}

void UGenerateSchemaCommandlet::InitCmdLine(const FString& CmdLineParams)
{
	TArray<FString> Tokens;
	TArray<FString> Switches;
	TMap<FString, FString> SwitchParams;
	ParseCommandLine(*CmdLineParams, Tokens, Switches, SwitchParams);

	MapPackagePaths.Empty();
	if (SwitchParams.Contains(TEXT("MapPackagePaths")))
	{
		const FString& AllMapPackagePaths = SwitchParams[TEXT("MapPackagePaths")];
		TArray<FString> ParsedMapPackagePaths;
		AllMapPackagePaths.ParseIntoArray(ParsedMapPackagePaths, TEXT(","));

		for (const FString& MapPackagePath : ParsedMapPackagePaths)
		{
			MapPackagePaths.Add(MapPackagePath.TrimQuotes());
		}
	}
}

int32 UGenerateSchemaCommandlet::Main(const FString& CmdLineParams)
{
	UE_LOG(LogGenerateSchemaCommandlet, Display, TEXT("Generate Schema Started."));

	TGuardValue<bool> UnattendedScriptGuard(GIsRunningUnattendedScript, GIsRunningUnattendedScript || IsRunningCommandlet());

	InitCmdLine(CmdLineParams);

#if ENGINE_MINOR_VERSION <= 22
	// Force spatial networking
	GetMutableDefault<UGeneralProjectSettings>()->SetUsesSpatialNetworking(true);
#endif

	FObjectListener ObjectListener;
	TSet<FSoftClassPath> ReferencedClasses;
	ObjectListener.StartListening(&ReferencedClasses);

	UE_LOG(LogGenerateSchemaCommandlet, Display, TEXT("Try Load Schema Database."));
	if (IsAssetReadOnly(SpatialConstants::SCHEMA_DATABASE_FILE_PATH))
	{
		UE_LOG(LogGenerateSchemaCommandlet, Error, TEXT("Failed to load Schema Database."));
		return 0;
	}

	if (!LoadGeneratorStateFromSchemaDatabase(SpatialConstants::SCHEMA_DATABASE_FILE_PATH))
	{
		ResetSchemaGeneratorStateAndCleanupFolders();
	}

	UE_LOG(LogGenerateSchemaCommandlet, Display, TEXT("Finding supported C++ and in-memory Classes."));

	TArray<UObject*> AllClasses;
	GetObjectsOfClass(UClass::StaticClass(), AllClasses);
	for (const auto& SupportedClass : GetAllSupportedClasses(AllClasses))
	{
		ReferencedClasses.Add(FSoftClassPath(SupportedClass));
	}

	UE_LOG(LogGenerateSchemaCommandlet, Display, TEXT("Starting Recompile Command."));
	int32 ParentResult = Super::Main(CmdLineParams);
	UE_LOG(LogGenerateSchemaCommandlet, Display, TEXT("Recompile Command Completed."));

	int ClassesFoundDuringRecompile = ReferencedClasses.Num();
	UE_LOG(LogGenerateSchemaCommandlet, Display, TEXT("Discovered %d Classes during recompile."), ClassesFoundDuringRecompile);

	UE_LOG(LogGenerateSchemaCommandlet, Display, TEXT("Starting Level Blueprint Loads."));
	LoadLevelBlueprints();
	
	UE_LOG(LogGenerateSchemaCommandlet, Display, TEXT("Starting Level Blueprint Loads."));

	int32 ClassesFoundLoadingLevelBlueprints = ReferencedClasses.Num() - ClassesFoundDuringRecompile;

	UE_LOG(LogGenerateSchemaCommandlet, Display, TEXT("Discovered %d Classes loading level blueprints."), ClassesFoundLoadingLevelBlueprints);
	ObjectListener.StopListening();

	// Sort classes here so that batching does not have an effect on ordering.
	ReferencedClasses.Sort([](const FSoftClassPath& A, const FSoftClassPath& B)
	{
#if ENGINE_MINOR_VERSION <= 22
		return A.GetAssetPathName() < B.GetAssetPathName();
#else
		return FNameLexicalLess()(A.GetAssetPathName(), B.GetAssetPathName());
#endif
	});

	for (FSoftClassPath SoftClassPath : ReferencedClasses)
	{
		UE_LOG(LogGenerateSchemaCommandlet, Display, TEXT("- %s"), *SoftClassPath.ToString());
	}

	UE_LOG(LogGenerateSchemaCommandlet, Display, TEXT("Start Schema Generation for discovered assets."));
	FDateTime StartTime = FDateTime::Now();
	TSet<UClass*> Classes;
	const int BatchSize = 100;
	for (const FSoftClassPath& SoftPath : ReferencedClasses)
	{
		if (UClass* LoadedClass = SoftPath.TryLoadClass<UObject>())
		{
			UE_LOG(LogGenerateSchemaCommandlet, Verbose, TEXT("Reloaded %s, adding to batch"), *GetPathNameSafe(LoadedClass));
			Classes.Add(LoadedClass);
			if (Classes.Num() >= BatchSize)
			{
				SpatialGDKGenerateSchemaForClasses(Classes);
				Classes.Empty(BatchSize);
			}
		}
		else
		{
			UE_LOG(LogGenerateSchemaCommandlet, Warning, TEXT("Failed to reload %s"), *SoftPath.ToString());
		}
	}
	SpatialGDKGenerateSchemaForClasses(Classes);

	GenerateSchemaForSublevels();
	GenerateSchemaForRPCEndpoints();
	GenerateSchemaForNCDs();

	FTimespan Duration = FDateTime::Now() - StartTime;

	UE_LOG(LogGenerateSchemaCommandlet, Display, TEXT("Schema Generation Finished in %.2f seconds"), Duration.GetTotalSeconds());

	if (!RunSchemaCompiler())
	{
		UE_LOG(LogGenerateSchemaCommandlet, Error, TEXT("Failed to run schema compiler."));
		return 0;
	}

	if (!SaveSchemaDatabase(SpatialConstants::SCHEMA_DATABASE_ASSET_PATH))
	{
		UE_LOG(LogGenerateSchemaCommandlet, Error, TEXT("Failed to save schema database."));
		return 0;
	}

	// Maybe this should always pass?
	return 0;
}
