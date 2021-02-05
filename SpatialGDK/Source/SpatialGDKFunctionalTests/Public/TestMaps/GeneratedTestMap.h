// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GenerateTestMapsCommandlet.h"
#include "GeneratedTestMap.generated.h"

/**
 * The base class for automatically generating test maps.
 * Descend from this and your class should automatically be picked up by the GenerateTestMapsCommandlet for generation.
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UGeneratedTestMap : public UObject
{
	GENERATED_BODY()

public:
	UGeneratedTestMap();

	virtual void GenerateMap();
	virtual bool ShouldGenerateMap() { return true; } // To control whether to generate a map from this class
	virtual FString GetMapName() { return MapName; }
	static FString GetGeneratedMapFolder();

protected:
	virtual void GenerateBaseMap();
	virtual void SaveMap();
	// This is what test maps descending from this class should normally override to add their own content to the map
	virtual void CreateCustomContentForMap() {}

	virtual FString GetPathToSaveTheMap();

	enum EMapCategory
	{
		CI_FAST,
		CI_FAST_SPATIAL_ONLY,
		CI_SLOW,
		CI_SLOW_SPATIAL_ONLY,
		NO_CI
	};

	EMapCategory MapCategory;
	FString MapName;
	UWorld* World;
	UStaticMesh* PlaneStaticMesh;
	UMaterial* BasicShapeMaterial;
};
