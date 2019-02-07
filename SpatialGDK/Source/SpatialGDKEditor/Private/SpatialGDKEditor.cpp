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

	// Commandlet needs to run in the Game Thread to be able to save the SchemaDatabase.uasset
	if (IsRunningCommandlet())
	{
		const bool bSuccess = SpatialGDKGenerateSchema();
		if (bSuccess)
		{
			SuccessCallback.ExecuteIfBound();
		}
		else
		{
			FailureCallback.ExecuteIfBound();
		}
		GetMutableDefault<UGeneralProjectSettings>()->bSpatialNetworking = bCachedSpatialNetworking;
		bSchemaGeneratorRunning = false;
	}
	else
	{
		SchemaGeneratorResult = Async<bool>(EAsyncExecution::Thread, SpatialGDKGenerateSchema,
			[this, bCachedSpatialNetworking, SuccessCallback, FailureCallback]()
		{
			const bool bSuccess = (SchemaGeneratorResult.IsReady() && SchemaGeneratorResult.Get() == true);
			if (bSuccess)
			{
				SuccessCallback.ExecuteIfBound();
			}
			else
			{
				FailureCallback.ExecuteIfBound();
			}
			GetMutableDefault<UGeneralProjectSettings>()->bSpatialNetworking = bCachedSpatialNetworking;
			bSchemaGeneratorRunning = false;
		});
	}
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
