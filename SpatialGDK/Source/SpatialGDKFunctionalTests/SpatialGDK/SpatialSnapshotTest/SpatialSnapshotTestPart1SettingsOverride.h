// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialSnapshotTestPart1SettingsOverride.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialSnapshotTestPart1SettingsOverride : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialSnapshotTestPart1SettingsOverride();

	virtual void PrepareTest() override;
};
