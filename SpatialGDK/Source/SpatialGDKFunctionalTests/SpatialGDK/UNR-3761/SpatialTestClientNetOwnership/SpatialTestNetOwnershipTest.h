// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "TestMaps/GeneratedTestMap.h"
#include "SpatialTestNetOwnershipTest.generated.h"

class ANetOwnershipCube;
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestNetOwnershipTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialTestNetOwnershipTest();

	virtual void PrepareTest() override;

	// Reference to the NetOwnershipCube, used to avoid using GetAllActorsOfClass() in every step to get a reference to the
	// NetOwnershipCube.
	ANetOwnershipCube* NetOwnershipCube;
};

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API USpatialTestNetOwnershipMap : public UGeneratedTestMap
{
	GENERATED_BODY()

public:
	USpatialTestNetOwnershipMap()
		: UGeneratedTestMap(EMapCategory::CI_PREMERGE, TEXT("SpatialTestNetOwnershipMap"))
	{
	}

protected:
	virtual void CreateCustomContentForMap() override;
};
