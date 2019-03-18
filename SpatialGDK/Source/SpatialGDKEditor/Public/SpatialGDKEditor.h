// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Async/Future.h"
#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKEditor, Log, All);

DECLARE_DELEGATE_OneParam(FSpatialGDKEditorErrorHandler, FString);

class SPATIALGDKEDITOR_API FSpatialGDKEditor
{
public:
	FSpatialGDKEditor();

	void GenerateSchema(FSimpleDelegate SuccessCallback, FSimpleDelegate FailureCallback, FSpatialGDKEditorErrorHandler ErrorCallback);
	void GenerateSnapshot(UWorld* World, FString SnapshotFilename, FSimpleDelegate SuccessCallback, FSimpleDelegate FailureCallback, FSpatialGDKEditorErrorHandler ErrorCallback);

	bool IsSchemaGeneratorRunning() { return bSchemaGeneratorRunning; }

	TArray<ULevelStreaming*> LoadAllStreamingLevels(UWorld* World);
	void UnloadLevels(TArray<ULevelStreaming*> LoadedLevels);

private:
	bool bSchemaGeneratorRunning;
	TFuture<bool> SchemaGeneratorResult;
};
