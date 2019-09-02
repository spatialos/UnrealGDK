// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "CookAndGenerateSchemaCommandlet.h"
#include "SpatialGDKEditorCommandletPrivate.h"
#include "SpatialGDKEditorSchemaGenerator.h"

struct FObjectListener : public FUObjectArray::FUObjectCreateListener
{
	FObjectListener()
	{
		GUObjectArray.AddUObjectCreateListener(this);
	}

	~FObjectListener()
	{
	}

	virtual void NotifyUObjectCreated(const class UObjectBase *Object, int32 Index) override
	{
		if (!VisitedClasses.Contains(Object->GetClass()) && IsSupportedClass(Object->GetClass()))
		{
			UE_LOG(LogTemp, Display, TEXT("[Object Created] Path: %s Class: %s"), *Object->GetFName().ToString(), *GetPathNameSafe(Object->GetClass()));
			VisitedClasses.Add(FSoftClassPath(Object->GetClass()));
		}
	}

public:
	TSet<FSoftClassPath> VisitedClasses;
};

UCookAndGenerateSchemaCommandlet::UCookAndGenerateSchemaCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;

	ObjectListener = new FObjectListener();
}

UCookAndGenerateSchemaCommandlet::~UCookAndGenerateSchemaCommandlet()
{
	delete ObjectListener;
	ObjectListener = nullptr;
}

bool UCookAndGenerateSchemaCommandlet::IsEditorOnly() const
{
	return true;
}

int32 UCookAndGenerateSchemaCommandlet::Main(const FString& CmdLineParams)
{
	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Cook and Generate Schema Started."));

	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Try Load Schema Database."));
	if (!TryLoadExistingSchemaDatabase())
	{
		return 1;
	}

	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Generate Schema for in-memory Classes."));
	if (!SpatialGDKGenerateSchema(false /* bSaveSchemaDatabase */, false /* bRunSchemaCompiler */))
	{
		return 2;
	}

	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Starting Cook Command."));
	int32 CookResult = Super::Main(CmdLineParams);
	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Cook Command Completed."));

	GUObjectArray.RemoveUObjectCreateListener(ObjectListener);

	ReferencedClasses.Append(ObjectListener->VisitedClasses);

	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Discovered %d Classes during cook."), ReferencedClasses.Num());

	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Schema Load/Gen Started at %s."), *FDateTime::Now().ToIso8601());
	TSet<UClass*> Classes;
	const int BatchSize = 100;
	for (FSoftClassPath SoftPath : ReferencedClasses)
	{
		if (!SoftPath.GetAssetPathString().StartsWith(TEXT("/Game"))) continue;
		if (UClass* LoadedClass = SoftPath.TryLoadClass<UObject>())
		{
			UE_LOG(LogTemp, Display, TEXT("Reloaded %s, add to batch"), *GetPathNameSafe(LoadedClass));
			Classes.Add(LoadedClass);
			if (Classes.Num() >= BatchSize)
			{
				SpatialGDKGenerateSchemaForClasses(Classes);
				Classes.Empty(100);
			}
		}
		else
		{
			UE_LOG(LogTemp, Display, TEXT("Failed to load %s for schema gen"), *SoftPath.ToString());
		}
	}
	SpatialGDKGenerateSchemaForClasses(Classes);

	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Schema Load/Gen Finished at %s."), *FDateTime::Now().ToIso8601());
	
	if (!SaveSchemaDatabase())
	{
		UE_LOG(LogSpatialGDKEditorCommandlet, Error, TEXT("Failed to save schema database."));
		return 3;
	}

	if (!RunSchemaCompiler())
	{
		UE_LOG(LogSpatialGDKEditorCommandlet, Warning, TEXT("Failed to run schema compiler."));
	}

	return CookResult;
}
