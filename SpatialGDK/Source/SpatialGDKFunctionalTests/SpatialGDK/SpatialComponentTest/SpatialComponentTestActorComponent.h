// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Components/SceneComponent.h"
#include "CoreMinimal.h"
#include "SpatialComponentTestActorComponent.generated.h"

class ASpatialFunctionalTest;
class ASpatialFunctionalTestFlowController;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API USpatialComponentTestActorComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	USpatialComponentTestActorComponent();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnAuthorityGained() override;

	virtual void OnAuthorityLost() override;

	virtual void OnActorReady(bool bHasAuthority) override;

	// TODO : Add OnClientOwnershipGained and OnClientOwnershipLost

	// Adds two components and then removes one so the net effect is one added component
	void AddAndRemoveComponents();
};
