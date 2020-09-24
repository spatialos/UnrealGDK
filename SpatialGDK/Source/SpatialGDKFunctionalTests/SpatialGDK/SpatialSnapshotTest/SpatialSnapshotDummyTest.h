// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialFunctionalTest.h"
#include "SpatialSnapshotDummyTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialSnapshotDummyTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()
public:
	ASpatialSnapshotDummyTest();

	virtual void BeginPlay() override;
};
