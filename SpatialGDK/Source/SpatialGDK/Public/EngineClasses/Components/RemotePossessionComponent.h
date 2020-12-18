// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "SpatialCommonTypes.h"

#include "RemotePossessionComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRemotePossessionComponent, Log, All);

class UAbstractLBStrategy;
/*
 A generic feature for Cross-Server Possession
 This component should be attached to a player controller.
 */
UCLASS(ClassGroup = (SpatialGDK), Meta = (BlueprintSpawnableComponent))
class SPATIALGDK_API URemotePossessionComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()
public:
	virtual void OnAuthorityGained() override;

	virtual void Possess();

	virtual bool EvaluatePossess(AController* CurrentController, AController* WantController);

	virtual bool EvaluateMigration(UAbstractLBStrategy* LBStrategy, VirtualWorkerId& WorkerId);

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UPROPERTY(handover)
	APawn* Target;

private:
	bool PendingDestroy;
};
