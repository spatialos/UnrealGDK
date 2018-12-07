// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKEditor, Log, All);

DECLARE_DELEGATE_OneParam(FSpatialGDKEditorErrorHandler, FString);

class SPATIALGDKEDITOR_API USpatialGDKEditor
{
public:
	USpatialGDKEditor();

	void GenerateSchema(FSimpleDelegate SuccessCallback, FSimpleDelegate FailureCallback, FSpatialGDKEditorErrorHandler ErrorCallback);
	void GenerateSnapshot(UWorld* World, FString SnapshotFilename, FSimpleDelegate SuccessCallback, FSimpleDelegate FailureCallback, FSpatialGDKEditorErrorHandler ErrorCallback);

	void CacheSpatialObjects(uint32 SpatialFlags, FSpatialGDKEditorErrorHandler ErrorCallback);

	bool IsSchemaGeneratorRunning() { return bSchemaGeneratorRunning; }

private:
	bool bSchemaGeneratorRunning;
	TFuture<bool> SchemaGeneratorResult;
};
