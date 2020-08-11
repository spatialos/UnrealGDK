// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "RPCInInterfaceActor.h"
#include "SpatialFunctionalTest.h"
#include "RPCInInterfaceTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ARPCInInterfaceTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ARPCInInterfaceTest();

	virtual void BeginPlay() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	float StepTimer = 0.0f;

	UPROPERTY(Replicated)
	ARPCInInterfaceActor* TestActor = nullptr;
};
