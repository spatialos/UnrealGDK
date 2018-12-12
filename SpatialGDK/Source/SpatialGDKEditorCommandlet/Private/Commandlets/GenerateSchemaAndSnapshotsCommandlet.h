// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Commandlets/Commandlet.h"

#include "GenerateSchemaAndSnapshotsCommandlet.generated.h"

class USpatialGDKEditor;

UCLASS()
class UGenerateSchemaAndSnapshotsCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UGenerateSchemaAndSnapshotsCommandlet();

public:
	virtual int32 Main(const FString& Params) override;

private:
	void GenerateSchema(USpatialGDKEditor& SpatialGDKEditor);
	void GenerateSnapshots(USpatialGDKEditor& SpatialGDKEditor);
	TArray<FString> GetAllMapPaths(FString InMapsPath);
	void GenerateSnapshotForMap(USpatialGDKEditor& SpatialGDKEditor, FString WorldPath);
};
