// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SchemaCommandlet.h"
#include "SpatialGDKEditorCommandletPluginPrivate.h"
#include "SpatialGDKEditor.h"

USchemaCommandlet::USchemaCommandlet()
{
	//TODO: HelpDescription = TEXT("Generate Schema and/or Snapshots");
	//TODO: HelpParamNames
	//TODO: HelpUsage
	IsClient = false;
	IsEditor = false;
	IsServer = false;
	LogToConsole = true;
}

int32 USchemaCommandlet::Main(const FString& Args)
{
	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Schema Generation Commandlet Started"));

	//NOTE: For future use, if schema generation configuration at the command line is desired
	//TArray<FString> Tokens;
	//TArray<FString> Switches;
	//TMap<FString, FString> Params;
	//ParseCommandLine(*Args, Tokens, Switches, Params);

	//Generate Schema!
	USpatialGDKEditor SpatialGDKEditor;
	SpatialGDKEditor.GenerateSchema(
		FExecuteAction::CreateLambda([]() {UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Schema Generation Completed!")); }),
		FExecuteAction::CreateLambda([]() { UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Schema Generation Failed")); }));
	while (SpatialGDKEditor.IsSchemaGeneratorRunning())
		FPlatformProcess::Sleep(0.1f);

	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Schema Generation Commandlet Complete"));

	return 0;
}
