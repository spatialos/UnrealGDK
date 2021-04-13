// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "TestMaps/GeneratedTestMap.h"

#include "PartiallyStablePathTest.generated.h"

class APartiallyStablePathPawn;
class UStaticMeshComponent;

UCLASS()
class APartiallyStablePathTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	APartiallyStablePathTest();

	virtual void PrepareTest() override;

private:
	APartiallyStablePathPawn* Pawn;
};

UCLASS()
class UPartiallyStablePathTestMap : public UGeneratedTestMap
{
	GENERATED_BODY()

public:
	UPartiallyStablePathTestMap();

protected:
	virtual void CreateCustomContentForMap() override;
};
