// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GenerateSchemaCommandlet.h"
#include "SpatialGDKEditorCommandletPrivate.h"
#include "SpatialGDKEditor.h"

UGenerateSchemaCommandlet::UGenerateSchemaCommandlet()
{
	IsClient = false;
	IsEditor = false;
	IsServer = false;
	LogToConsole = true;
}

int32 UGenerateSchemaCommandlet::Main(const FString& Args)
{
	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Schema Generation Commandlet Started"));

	//NOTE: For future use, if schema generation configuration at the command line is desired
	//TArray<FString> Tokens;
	//TArray<FString> Switches;
	//TMap<FString, FString> Params;
	//ParseCommandLine(*Args, Tokens, Switches, Params);

	//Generate Schema!
	bool bSchemaGenSuccess = false;
	FSpatialGDKEditor SpatialGDKEditor;
	SpatialGDKEditor.GenerateSchema(
		FSimpleDelegate::CreateLambda([&bSchemaGenSuccess]()
		{
			UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Schema Generation Completed!"));
			bSchemaGenSuccess = true;
		}),
		FSimpleDelegate::CreateLambda([]() { UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Schema Generation Failed")); }),
		FSpatialGDKEditorErrorHandler::CreateLambda([](FString ErrorText) { UE_LOG(LogSpatialGDKEditorCommandlet, Error, TEXT("%s"), *ErrorText); }));
	while (SpatialGDKEditor.IsSchemaGeneratorRunning())
		FPlatformProcess::Sleep(0.1f);

	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Schema Generation Commandlet Complete"));

	return bSchemaGenSuccess ? 0 : 1;
}
