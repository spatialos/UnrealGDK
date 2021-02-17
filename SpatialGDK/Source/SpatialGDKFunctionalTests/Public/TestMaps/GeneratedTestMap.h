// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GenerateTestMapsCommandlet.h"
#include "GeneratedTestMap.generated.h"

/**
 * The base class for automatically generating test maps.
 * Descend from this and your class should automatically be picked up by the GenerateTestMapsCommandlet for generation.
 */
UCLASS(Abstract)
class SPATIALGDKFUNCTIONALTESTS_API UGeneratedTestMap : public UObject
{
	GENERATED_BODY()

public:
	UGeneratedTestMap();
	void GenerateMap();
	virtual bool ShouldGenerateMap() { return true; } // To control whether to generate a map from this class
	FString GetMapName() { return MapName; }
	static FString GetGeneratedMapFolder();

protected:
	enum class EMapCategory
	{
		CI_PREMERGE,
		CI_PREMERGE_SPATIAL_ONLY,
		CI_NIGHTLY,
		CI_NIGHTLY_SPATIAL_ONLY,
		NO_CI
	};

	// This constructor with arguments is the one that the descended classes should be using within their no-argument constructor.
	UGeneratedTestMap(EMapCategory MapCategory, FString MapName);

	// This is what test maps descending from this class should normally override to add their own content to the map
	virtual void CreateCustomContentForMap() {}

	// You may be asking: why do we have these two simple functions here (one private AddActorToLevel and one public)?
	// The answer is linkers & compilers. I wanted an easy templated function that will automatically cast the result of adding an actor to
	// a level to the appropriate class, since we basically know it should be our desired class and if it isn't, we can check and crash.
	// However, I have to introduce a layer of indirection, using the non-templated AddActorToLevel, so that the usage of GEditor is
	// localized to the cpp file, so that the include does not leak into the header file. Thus modules relying on SpatialGDKFunctionalTests
	// can define TestMaps without needing UnrealEd (most of the time).
	template <class T>
	T* AddActorToLevel(ULevel* Level, const FTransform& Transform)
	{
		return CastChecked<T>(AddActorToLevel(Level, T::StaticClass(), Transform));
	}

	UPROPERTY()
	UWorld* World;
	UPROPERTY()
	UStaticMesh* PlaneStaticMesh;
	UPROPERTY()
	UMaterial* BasicShapeMaterial;

private:
	AActor* AddActorToLevel(ULevel* Level, UClass* Class, const FTransform& Transform);
	void GenerateBaseMap();
	void SaveMap();
	FString GetPathToSaveTheMap();

	bool bIsValidForGeneration;
	EMapCategory MapCategory;
	FString MapName;
};
