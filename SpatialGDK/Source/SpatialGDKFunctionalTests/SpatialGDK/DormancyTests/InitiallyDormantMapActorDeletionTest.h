// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "DormancyTest.h"
#include "InitiallyDormantMapActorDeletionTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AInitiallyDormantMapActorDeletionTest : public ADormancyTest
{
	GENERATED_BODY()

public:
	AInitiallyDormantMapActorDeletionTest();

	virtual void PrepareTest() override;
};
