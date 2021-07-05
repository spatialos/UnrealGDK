// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once


#include "CoreMinimal.h"
#include "DormancyTest.h"
#include "DynamicActorDormantAllChangeProperty.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ADynamicActorDormantAllChangeProperty : public ADormancyTest
{
	GENERATED_BODY()

public:
	ADynamicActorDormantAllChangeProperty();

	virtual void PrepareTest() override;
};
