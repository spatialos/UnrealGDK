// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SchemaCommandlet.h"
#include "SpatialGDKEditorCommandletPluginPrivate.h"
#include "SpatialGDKEditor.h"

USchemaCommandlet::USchemaCommandlet()
{
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
		FSimpleDelegate::CreateLambda([]() { UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Schema Generation Completed!")); }),
		FSimpleDelegate::CreateLambda([]() { UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Schema Generation Failed")); }),
		FSpatialGDKEditorErrorHandler::CreateLambda([](FString ErrorText) { UE_LOG(LogSpatialGDKEditorCommandlet, Error, TEXT("%s"), *ErrorText); }));
	while (SpatialGDKEditor.IsSchemaGeneratorRunning())
		FPlatformProcess::Sleep(0.1f);

	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Schema Generation Commandlet Complete"));

	return 0;
}
