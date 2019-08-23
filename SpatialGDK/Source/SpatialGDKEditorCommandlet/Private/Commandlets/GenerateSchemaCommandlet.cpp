// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GenerateSchemaCommandlet.h"
#include "SpatialGDKEditor.h"
#include "SpatialGDKEditorCommandletPrivate.h"
#include "SpatialGDKEditorSchemaGenerator.h"

UGenerateSchemaCommandlet::UGenerateSchemaCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

void UGenerateSchemaCommandlet::HandleDeleteSchemaOption(const TArray<FString>& Switches) 
{
	if (Switches.Contains("delete-schema-db"))
	{
		if (DeleteSchemaDatabase())
		{
			UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Deleted schema database"));
		}
		else
		{
			UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Failed to delete schema database"));
		}
	}
}

int32 UGenerateSchemaCommandlet::Main(const FString& Args)
{
	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Schema Generation Commandlet Started"));

	TArray<FString> Tokens;
	TArray<FString> Switches;
	TMap<FString, FString> Params;
	ParseCommandLine(*Args, Tokens, Switches);

	HandleDeleteSchemaOption(Switches);

	//Generate Schema!
	bool bSchemaGenSuccess;
	FSpatialGDKEditor SpatialGDKEditor;
	if (SpatialGDKEditor.GenerateSchema(true))
	{
		UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Schema Generation Completed!"));
		bSchemaGenSuccess = true;
	}
	else
	{
		UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Schema Generation Failed"));
		bSchemaGenSuccess = false;
	}

	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Schema Generation Commandlet Complete"));

	return bSchemaGenSuccess ? 0 : 1;
}
