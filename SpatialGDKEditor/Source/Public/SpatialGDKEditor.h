// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

DECLARE_DELEGATE_OneParam(FSpatialGDKEditorErrorHandler, FString);

class SPATIALGDKEDITOR_API USpatialGDKEditor
{
public:
	USpatialGDKEditor();

	void GenerateSchema(FSimpleDelegate SuccessCallback, FSimpleDelegate FailureCallback, FSpatialGDKEditorErrorHandler ErrorCallback);

	void CacheSpatialObjects(uint32 SpatialFlags, FSpatialGDKEditorErrorHandler ErrorCallback);

	bool IsSchemaGeneratorRunning() { return bSchemaGeneratorRunning; }

private:
	bool bSchemaGeneratorRunning;
	TFuture<bool> SchemaGeneratorResult;
};
