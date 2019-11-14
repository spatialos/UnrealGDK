// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CookAndGenerateSchemaCommandlet.h"
#include "SpatialConstants.h"
#include "SpatialGDKEditorCommandletPrivate.h"
#include "SpatialGDKEditorSchemaGenerator.h"

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

UCookAndGenerateSchemaCommandlet::UCookAndGenerateSchemaCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

int32 UCookAndGenerateSchemaCommandlet::Main(const FString& CmdLineParams)
{
#if ENGINE_MINOR_VERSION <= 22
	// Force spatial networking
	GetMutableDefault<UGeneralProjectSettings>()->SetUsesSpatialNetworking(true);
#endif

	UE_LOG(LogCookAndGenerateSchemaCommandlet, Display, TEXT("Cook and Generate Schema Started."));
	FObjectListener ObjectListener;
	TSet<FSoftClassPath> ReferencedClasses;
	ObjectListener.StartListening(&ReferencedClasses);
	
	UE_LOG(LogCookAndGenerateSchemaCommandlet, Display, TEXT("Try Load Schema Database."));
	if (IsAssetReadOnly(SpatialConstants::SCHEMA_DATABASE_FILE_PATH))
	{
		UE_LOG(LogCookAndGenerateSchemaCommandlet, Error, TEXT("Failed to load Schema Database."));
		return 0;
	}

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
	int32 CookResult = Super::Main(CmdLineParams);
	UE_LOG(LogCookAndGenerateSchemaCommandlet, Display, TEXT("Cook Command Completed."));

	UE_LOG(LogCookAndGenerateSchemaCommandlet, Display, TEXT("Discovered %d Classes during cook."), ReferencedClasses.Num());

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

	FTimespan Duration = FDateTime::Now() - StartTime;

	UE_LOG(LogCookAndGenerateSchemaCommandlet, Display, TEXT("Schema Generation Finished in %.2f seconds"), Duration.GetTotalSeconds());
	
	if (!SaveSchemaDatabase(SpatialConstants::SCHEMA_DATABASE_ASSET_PATH))
	{
		UE_LOG(LogCookAndGenerateSchemaCommandlet, Error, TEXT("Failed to save schema database."));
		return 0;
	}

	if (!RunSchemaCompiler())
	{
		UE_LOG(LogCookAndGenerateSchemaCommandlet, Error, TEXT("Failed to run schema compiler."));
		return 0;
	}

	return CookResult;
}
