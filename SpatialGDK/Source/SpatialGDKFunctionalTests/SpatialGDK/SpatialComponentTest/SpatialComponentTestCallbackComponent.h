// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Components/SceneComponent.h"
#include "SpatialComponentTestCallbackComponent.generated.h"

class ASpatialFunctionalTest;
class ASpatialFunctionalTestFlowController;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API USpatialComponentTestCallbackComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	USpatialComponentTestCallbackComponent();

	virtual void OnAuthorityGained() override;

	virtual void OnAuthorityLost() override;

	virtual void OnActorReady(bool bHasAuthority) override;

	virtual void OnClientOwnershipGained() override;

	virtual void OnClientOwnershipLost() override;

	// Adds two components and then removes one so the net effect is one added component
	void AddAndRemoveComponents();
};
