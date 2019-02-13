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
	const FString MapPathsParamName = TEXT("MapPaths");	// Commandline Argument Name used to declare the paths to generate schema/snapshots against
	const FString AssetPathGameDirName = TEXT("/Game");	// Root asset path directory name that maps will ultimately be found in

	TArray<FString> GeneratedMapPaths;

private:
	void GenerateSchemaAndSnapshotForPath(FSpatialGDKEditor& InSpatialGDKEditor, const FString& InPath);
	void GenerateSchemaAndSnapshotForMap(FSpatialGDKEditor& InSpatialGDKEditor, const FString& InMapName);

	void GenerateSchemaForLoadedMap(FSpatialGDKEditor& InSpatialGDKEditor);
	void GenerateSnapshotForLoadedMap(FSpatialGDKEditor& InSpatialGDKEditor, const FString& InMapName);
};
