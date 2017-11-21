// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlacedEditorUtilityBase.h"
#include "PackageMapperUtility.generated.h"

/**
 * 
 */
UCLASS()
class NUF_API APackageMapperUtility : public APlacedEditorUtilityBase
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "PackageMapperUtil")
	void GeneratePackageMap(UObject* WorldContextObject);

private:

	void MapActorPaths(TMap<uint32, FString>& OutMap, UObject* WorldContextObject);
};
