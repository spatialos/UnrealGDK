// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GenerateSchemaCommandlet.h"
#include "SpatialConstants.h"
#include "SpatialGDKEditor.h"
#include "SpatialGDKEditorCommandletPrivate.h"
#include "SpatialGDKEditorSchemaGenerator.h"

using namespace SpatialGDKEditor::Schema;

UGenerateSchemaCommandlet::UGenerateSchemaCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

bool UGenerateSchemaCommandlet::HandleOptions(const TArray<FString>& Switches)
{
	if (Switches.Contains("delete-schema-db"))
	{
		if (DeleteSchemaDatabase(SpatialConstants::SCHEMA_DATABASE_FILE_PATH))
		{
			UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Deleted schema database"));
		}
		else
		{
			UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Failed to delete schema database"));
			return false;
		}
	}
	return true;
}

int32 UGenerateSchemaCommandlet::Main(const FString& Args)
{
	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Schema Generation Commandlet Started"));

	TGuardValue<bool> UnattendedScriptGuard(GIsRunningUnattendedScript, GIsRunningUnattendedScript || IsRunningCommandlet());

	TArray<FString> Tokens;
	TArray<FString> Switches;
	TMap<FString, FString> Params;
	ParseCommandLine(*Args, Tokens, Switches);

	if (!HandleOptions(Switches))
	{
		UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Schema generation aborted"));
		return 1;
	}

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
