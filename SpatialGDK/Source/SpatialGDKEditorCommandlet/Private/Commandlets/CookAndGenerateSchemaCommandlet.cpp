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
public:
	virtual void NotifyUObjectCreated(const class UObjectBase *Object, int32 Index) override
	{
		FSoftClassPath SoftClass = FSoftClassPath(Object->GetClass());
		if (UnsupportedClasses.Contains(SoftClass))
		{
			return;
		}
		if (!VisitedClasses.Contains(SoftClass))
		{
			if (IsSupportedClass(Object->GetClass()))
			{
				UE_LOG(LogTemp, Verbose, TEXT("[Object Created] Path: %s, Class: %s"), *Object->GetFName().ToString(), *GetPathNameSafe(Object->GetClass()));
				VisitedClasses.Add(SoftClass);
			}
			else
			{
				UnsupportedClasses.Add(SoftClass);
			}
		}
	}


	TSet<FSoftClassPath> VisitedClasses;
	TSet<FSoftClassPath> UnsupportedClasses;
};

UCookAndGenerateSchemaCommandlet::UCookAndGenerateSchemaCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
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
	ObjectListener = new FObjectListener();
	
	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Try Load Schema Database."));
	if (!TryLoadExistingSchemaDatabase())
	{
		return 1;
	}

	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("[%s] Generate Schema for C++ and in-memory Classes."), *FDateTime::Now().ToIso8601());
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

	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Start Schema Generation for discovered assets."));
	FDateTime StartTime = FDateTime::Now();
	TSet<UClass*> Classes;
	const int BatchSize = 100;
	for (FSoftClassPath SoftPath : ReferencedClasses)
	{
		if (!SoftPath.GetAssetPathString().StartsWith(TEXT("/Game"))) continue;
		if (UClass* LoadedClass = SoftPath.TryLoadClass<UObject>())
		{
			UE_LOG(LogTemp, Verbose, TEXT("Reloaded %s, add to batch"), *GetPathNameSafe(LoadedClass));
			Classes.Add(LoadedClass);
			if (Classes.Num() >= BatchSize)
			{
				SpatialGDKGenerateSchemaForClasses(Classes);
				Classes.Empty(100);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to load %s for schema gen"), *SoftPath.ToString());
		}
	}
	SpatialGDKGenerateSchemaForClasses(Classes);

	FTimespan Duration = FDateTime::Now() - StartTime;


	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Schema Load/Gen Finished in %.2f seconds"), Duration.GetTotalSeconds());
	
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
