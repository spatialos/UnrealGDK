// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "TestMaps/GeneratedTestMap.h"

#include "SpatialTestNetOwnership.generated.h"

class ANetOwnershipCube;
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestNetOwnership : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialTestNetOwnership();

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
	USpatialTestNetOwnershipMap();

protected:
	virtual void CreateCustomContentForMap() override;
};
