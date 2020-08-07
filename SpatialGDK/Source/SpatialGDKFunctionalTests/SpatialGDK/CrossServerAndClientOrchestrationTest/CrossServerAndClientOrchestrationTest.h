// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "CrossServerAndClientOrchestrationTest.generated.h"

/**
 *
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ACrossServerAndClientOrchestrationTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ACrossServerAndClientOrchestrationTest();

	virtual void BeginPlay() override;

	UPROPERTY(Replicated)
	TArray<bool> ServerWorkerSetValues;
	UPROPERTY(Replicated)
	TArray<bool> ClientWorkerSetValues;

	UFUNCTION(CrossServer, Reliable)
	void CrossServerSetTestValue(ESpatialFunctionalTestWorkerType ControllerType, uint8 ChangedInstance);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

private:
	void Assert_ServerStepIsRunningInExpectedEnvironment(int InstanceToRunIn, ASpatialFunctionalTestFlowController* FlowController);
	void Assert_ClientStepIsRunningInExpectedEnvironment(int InstanceToRunIn, ASpatialFunctionalTestFlowController* FlowController);
	bool CheckAllValuesHaveBeenSetAndCanBeLocallyRead();
};
