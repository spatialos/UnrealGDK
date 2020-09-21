// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialFunctionalTest.h"
#include "SpatialSnapshotTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialSnapshotTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()
public:
	ASpatialSnapshotTest();

	virtual void PrepareTest() override;
};
