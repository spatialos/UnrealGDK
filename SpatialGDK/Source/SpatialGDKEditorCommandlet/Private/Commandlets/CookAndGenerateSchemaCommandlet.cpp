// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "CookAndGenerateSchemaCommandlet.h"
#include "SpatialGDKEditorCommandletPrivate.h"
#include "SpatialGDKEditorSchemaGenerator.h"
#include "Async.h"

UCookAndGenerateSchemaCommandlet::UCookAndGenerateSchemaCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

bool UCookAndGenerateSchemaCommandlet::IsEditorOnly() const
{
	return true;
}

int32 UCookAndGenerateSchemaCommandlet::Main(const FString& CmdLineParams)
{
	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Cook and Generate Schema Started."));
	GEditor->OnObjectsReplaced().AddUObject(this, &UCookAndGenerateSchemaCommandlet::OnObjectsReplaced);

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

	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Loaded %d Classes."), ReferencedClasses.Num());
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

void UCookAndGenerateSchemaCommandlet::OnObjectsReplaced(const TMap<UObject*, UObject*>& ObjectsReplaced)
{
	TSet<UClass*> CandidateClasses;
	for (auto& Entry : ObjectsReplaced)
	{
		if (const UObject* NewObject = Entry.Value)
		{
			if (NewObject->HasAllFlags(RF_ArchetypeObject) && !NewObject->HasAnyFlags(RF_NeedPostLoad | RF_NeedPostLoadSubobjects))
			{
				bool bAlreadyLoaded = false;
				ReferencedClasses.Add(NewObject->GetClass()->GetPathName(), &bAlreadyLoaded);
				if (!bAlreadyLoaded)
				{
					CandidateClasses.Add(NewObject->GetClass());
				}
			}
		}
	}

	AsyncTask(ENamedThreads::GameThread, [CandidateClasses]()
	{
		SpatialGDKGenerateSchemaForClasses(CandidateClasses);
	});
}
