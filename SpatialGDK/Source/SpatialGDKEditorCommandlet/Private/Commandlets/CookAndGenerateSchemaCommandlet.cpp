// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "CookAndGenerateSchemaCommandlet.h"
#include "SpatialGDKEditorCommandletPrivate.h"
#include "SpatialGDKEditorSchemaGenerator.h"

DEFINE_LOG_CATEGORY(LogCookAndGenerateSchemaCommandlet);

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
				UE_LOG(LogCookAndGenerateSchemaCommandlet, Verbose, TEXT("Object [%s] Created, Consider Class [%s] For Schema."),
					*Object->GetFName().ToString(), *GetPathNameSafe(Object->GetClass()));
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

int32 UCookAndGenerateSchemaCommandlet::Main(const FString& CmdLineParams)
{
	UE_LOG(LogCookAndGenerateSchemaCommandlet, Display, TEXT("Cook and Generate Schema Started."));
	ObjectListener = new FObjectListener();
	TSet<FSoftClassPath> ReferencedClasses;
	
	UE_LOG(LogCookAndGenerateSchemaCommandlet, Display, TEXT("Try Load Schema Database."));
	if (!TryLoadExistingSchemaDatabase())
	{
		return 1;
	}

	UE_LOG(LogCookAndGenerateSchemaCommandlet, Display, TEXT("Finding supported C++ and in-memory Classes."));
	for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
	{
		UClass* SupportedClass = *ClassIt;

		if (IsSupportedClass(SupportedClass))
		{
			ReferencedClasses.Add(FSoftClassPath(SupportedClass));
		}
	}

	UE_LOG(LogCookAndGenerateSchemaCommandlet, Display, TEXT("Starting Cook Command."));
	int32 CookResult = Super::Main(CmdLineParams);
	UE_LOG(LogCookAndGenerateSchemaCommandlet, Display, TEXT("Cook Command Completed."));

	GUObjectArray.RemoveUObjectCreateListener(ObjectListener);

	ReferencedClasses.Append(ObjectListener->VisitedClasses);
	UE_LOG(LogCookAndGenerateSchemaCommandlet, Display, TEXT("Discovered %d Classes during cook."), ReferencedClasses.Num());

	UE_LOG(LogCookAndGenerateSchemaCommandlet, Display, TEXT("Start Schema Generation for discovered assets."));
	FDateTime StartTime = FDateTime::Now();
	TSet<UClass*> Classes;
	const int BatchSize = 100;
	for (FSoftClassPath SoftPath : ReferencedClasses)
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

	FTimespan Duration = FDateTime::Now() - StartTime;

	UE_LOG(LogCookAndGenerateSchemaCommandlet, Display, TEXT("Schema Generation Finished in %.2f seconds"), Duration.GetTotalSeconds());
	
	if (!SaveSchemaDatabase())
	{
		UE_LOG(LogCookAndGenerateSchemaCommandlet, Error, TEXT("Failed to save schema database."));
		return 3;
	}

	if (!RunSchemaCompiler())
	{
		UE_LOG(LogCookAndGenerateSchemaCommandlet, Warning, TEXT("Failed to run schema compiler."));
	}

	return CookResult;
}
