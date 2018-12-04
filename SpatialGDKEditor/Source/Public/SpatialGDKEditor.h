// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

class SPATIALGDKEDITOR_API USpatialGDKEditor
{
public:
	USpatialGDKEditor();

	void GenerateSchema();
	void CacheSpatialObjects(uint32 SpatialFlags);

	bool IsSchemaGeneratorRunning() { return bSchemaGeneratorRunning; }

private:
	bool bSchemaGeneratorRunning;
	TFuture<bool> SchemaGeneratorResult;
};
