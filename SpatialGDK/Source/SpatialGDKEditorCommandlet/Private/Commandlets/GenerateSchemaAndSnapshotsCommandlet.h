// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Commandlets/Commandlet.h"

#include "GenerateSchemaAndSnapshotsCommandlet.generated.h"

class FSpatialGDKEditor;

UCLASS()
class UGenerateSchemaAndSnapshotsCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UGenerateSchemaAndSnapshotsCommandlet();

public:
	virtual int32 Main(const FString& Params) override;

private:
	void GenerateSchemaAndSnapshotForMap(FSpatialGDKEditor& InSpatialGDKEditor, FString InMapName);

	void GenerateSchemaForLoadedMap(FSpatialGDKEditor& InSpatialGDKEditor);
	void GenerateSnapshotForLoadedMap(FSpatialGDKEditor& InSpatialGDKEditor, FString InMapName);
	void GenerateSchema(FSpatialGDKEditor& SpatialGDKEditor);
	void GenerateSnapshots(FSpatialGDKEditor& SpatialGDKEditor);
	TArray<FString> GetAllMapPaths(FString InMapsPath);
	void GenerateSnapshotForMap(FSpatialGDKEditor& SpatialGDKEditor, FString WorldPath);
};
