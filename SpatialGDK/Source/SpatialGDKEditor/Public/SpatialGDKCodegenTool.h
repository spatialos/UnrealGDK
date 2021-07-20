// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Utils/TransientUObjectEditor.h"

#include "SpatialGDKCodegenTool.generated.h"

UCLASS()
class SPATIALGDKEDITOR_API USpatialGDKCodegenTool : public UTransientUObjectEditor
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Meta = (FilePathFilter = "schema", RelativeToGameDir), Category = "Spatial")
	FFilePath CustomSchemaPath;

	UFUNCTION(Exec)
	void GenerateSource();
};
