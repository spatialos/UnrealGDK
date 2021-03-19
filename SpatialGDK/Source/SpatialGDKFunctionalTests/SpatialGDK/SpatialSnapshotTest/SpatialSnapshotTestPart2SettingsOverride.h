// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialSnapshotTestPart2SettingsOverride.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialSnapshotTestPart2SettingsOverride : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialSnapshotTestPart2SettingsOverride();

	virtual void PrepareTest() override;
};
