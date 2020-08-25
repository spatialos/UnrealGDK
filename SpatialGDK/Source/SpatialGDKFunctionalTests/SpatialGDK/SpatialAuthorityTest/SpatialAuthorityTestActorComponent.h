// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Components/SceneComponent.h"
#include "CoreMinimal.h"
#include "SpatialAuthorityTestActorComponent.generated.h"

class ASpatialFunctionalTest;
class ASpatialFunctionalTestFlowController;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API USpatialAuthorityTestActorComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	USpatialAuthorityTestActorComponent();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnAuthorityGained() override;

	virtual void OnAuthorityLost() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

	UPROPERTY(Replicated)
	int ReplicatedAuthWorkerIdOnBeginPlay = 0;

	int AuthWorkerIdOnBeginPlay = 0;

	int AuthWorkerIdOnTick = 0;

	int NumAuthorityGains = 0;

	int NumAuthorityLosses = 0;

protected:
	virtual void BeginPlay() override;
};
