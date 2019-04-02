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

	bool GenerateSchema(bool bFullRebuild);
	void GenerateSnapshot(UWorld* World, FString SnapshotFilename, FSimpleDelegate SuccessCallback, FSimpleDelegate FailureCallback, FSpatialGDKEditorErrorHandler ErrorCallback);

	bool IsSchemaGeneratorRunning() { return bSchemaGeneratorRunning; }

private:
	bool bSchemaGeneratorRunning;
	TFuture<bool> SchemaGeneratorResult;
};
