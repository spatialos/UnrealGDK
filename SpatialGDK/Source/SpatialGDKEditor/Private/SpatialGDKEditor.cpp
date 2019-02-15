// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditor.h"

#include "Async/Async.h"
#include "SpatialGDKEditorSchemaGenerator.h"
#include "SpatialGDKEditorSnapshotGenerator.h"

#include "Editor.h"

#include "AssetRegistryModule.h"
#include "GeneralProjectSettings.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKEditor);

FSpatialGDKEditor::FSpatialGDKEditor()
	: bSchemaGeneratorRunning(false)
{
	InitClassPathToSchemaMap();
}

void FSpatialGDKEditor::GenerateSchema(FSimpleDelegate SuccessCallback, FSimpleDelegate FailureCallback, FSpatialGDKEditorErrorHandler ErrorCallback)
{
	if (bSchemaGeneratorRunning)
	{
		UE_LOG(LogSpatialGDKEditor, Warning, TEXT("Schema generation is already running"));
		return;
	}

	bSchemaGeneratorRunning = true;

	// Force spatial networking so schema layouts are correct
	UGeneralProjectSettings* GeneralProjectSettings = GetMutableDefault<UGeneralProjectSettings>();
	bool bCachedSpatialNetworking = GeneralProjectSettings->bSpatialNetworking;
	GeneralProjectSettings->bSpatialNetworking = true;

	PreProcessSchemaMap();

	// Compile all dirty blueprints
	TArray<UBlueprint*> ErroredBlueprints;
	bool PromptForCompilation = false;
	UEditorEngine::ResolveDirtyBlueprints(PromptForCompilation, ErroredBlueprints);

	if (ErroredBlueprints.Num() > 0)
	{
		UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("There were errors when compiling blueprints. Schema has not been generated."));
		return;
	}

	AsyncTask(ENamedThreads::GameThread, [this, bCachedSpatialNetworking, SuccessCallback, FailureCallback]() {
		auto SchemaGeneratorResult = SpatialGDKGenerateSchema();
		if (!SchemaGeneratorResult) {
			if (FailureCallback.IsBound())
			{
				FailureCallback.Execute();
			}
		}
		else
		{
			if (SuccessCallback.IsBound())
			{
				SuccessCallback.Execute();
			}
		}
		GetMutableDefault<UGeneralProjectSettings>()->bSpatialNetworking = bCachedSpatialNetworking;
		bSchemaGeneratorRunning = false;
	});
}

void FSpatialGDKEditor::GenerateSnapshot(UWorld* World, FString SnapshotFilename, FSimpleDelegate SuccessCallback, FSimpleDelegate FailureCallback, FSpatialGDKEditorErrorHandler ErrorCallback)
{
	const bool bSuccess = SpatialGDKGenerateSnapshot(World, SnapshotFilename);

	if (bSuccess)
	{
		if (SuccessCallback.IsBound())
		{
			SuccessCallback.Execute();
		}
	}
	else
	{
		if (FailureCallback.IsBound())
		{
			FailureCallback.Execute();
		}
	}
}
