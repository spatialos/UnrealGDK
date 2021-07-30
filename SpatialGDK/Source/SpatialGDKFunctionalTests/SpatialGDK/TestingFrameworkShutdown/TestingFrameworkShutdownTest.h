// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "TestingFrameworkShutdownTest.generated.h"

UCLASS()
class ATestingFrameworkShutdownTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ATestingFrameworkShutdownTest();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PrepareTest() override;
};
