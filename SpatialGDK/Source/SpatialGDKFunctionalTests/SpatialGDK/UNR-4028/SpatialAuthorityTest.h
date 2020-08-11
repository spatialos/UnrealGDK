// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialAuthorityTest.generated.h"

class ASpatialAuthorityTestActor;
class ASpatialAuthorityTestReplicatedActor;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialAuthorityTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialAuthorityTest();

	virtual void BeginPlay() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, Category = "Default")
	ASpatialAuthorityTestActor* LevelActor;

	UPROPERTY(EditAnywhere, Category = "Default")
	ASpatialAuthorityTestReplicatedActor* LevelReplicatedActor;

	UPROPERTY(Replicated)

};
