// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditor.h"

#include "Async/Async.h"
#include "SpatialGDKEditorSchemaGenerator.h"
#include "SpatialGDKEditorSnapshotGenerator.h"

#include "Editor.h"

#include "AssetRegistryModule.h"
#include "GeneralProjectSettings.h"
#include "Misc/ScopedSlowTask.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKEditor);

#define LOCTEXT_NAMESPACE "FSpatialGDKEditor"

FSpatialGDKEditor::FSpatialGDKEditor()
	: bSchemaGeneratorRunning(false)
{
	TryLoadExistingSchemaDatabase();
}

bool FSpatialGDKEditor::GenerateSchema(bool bFullRebuild)
{
	if (bSchemaGeneratorRunning)
	{
		UE_LOG(LogSpatialGDKEditor, Warning, TEXT("Schema generation is already running"));
		return false;
	}

	bSchemaGeneratorRunning = true;

	FScopedSlowTask Progress(5.f, LOCTEXT("GeneratingSchema", "Generating Schema..."));
	Progress.MakeDialog();

	// Force spatial networking so schema layouts are correct
	UGeneralProjectSettings* GeneralProjectSettings = GetMutableDefault<UGeneralProjectSettings>();
	bool bCachedSpatialNetworking = GeneralProjectSettings->bSpatialNetworking;
	GeneralProjectSettings->bSpatialNetworking = true;

	if (bFullRebuild)
	{
		ClearGeneratedSchema();
	}
	else
	{
		TryLoadExistingSchemaDatabase();
	}
	Progress.EnterProgressFrame(1.f);

	PreProcessSchemaMap();
	Progress.EnterProgressFrame(1.f);

	// Compile all dirty blueprints
	TArray<UBlueprint*> ErroredBlueprints;
	bool bPromptForCompilation = false;
	UEditorEngine::ResolveDirtyBlueprints(bPromptForCompilation, ErroredBlueprints);
	Progress.EnterProgressFrame(1.f);

	LoadDefaultGameModes();
	Progress.EnterProgressFrame(1.f);

	bool bResult = SpatialGDKGenerateSchema();
	Progress.EnterProgressFrame(1.f);

	// We delay printing this error until after the schema spam to make it have a higher chance of being noticed.
	if (ErroredBlueprints.Num() > 0)
	{
		UE_LOG(LogSpatialGDKEditor, Error, TEXT("Errors compiling blueprints during schema generation! The following blueprints did not have schema generated for them:"));
		for (const auto& Blueprint : ErroredBlueprints)
		{
			UE_LOG(LogSpatialGDKEditor, Error, TEXT("%s"), *GetPathNameSafe(Blueprint));
		}
	}

	GetMutableDefault<UGeneralProjectSettings>()->bSpatialNetworking = bCachedSpatialNetworking;
	bSchemaGeneratorRunning = false;

	return bResult;
}

void FSpatialGDKEditor::GenerateSnapshot(UWorld* World, FString SnapshotFilename, FSimpleDelegate SuccessCallback, FSimpleDelegate FailureCallback, FSpatialGDKEditorErrorHandler ErrorCallback)
{
	const bool bSuccess = SpatialGDKGenerateSnapshot(World, SnapshotFilename);

	if (bSuccess)
	{
		SuccessCallback.ExecuteIfBound();
	}
	else
	{
		FailureCallback.ExecuteIfBound();
	}
}

#undef LOCTEXT_NAMESPACE
