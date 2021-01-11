// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Components/ActorComponent.h"
#include "SpatialComponentTestDummyComponent.generated.h"

/*
 * Empty component to be added and removed from components so that the component callbacks can be tested
 */
UCLASS(NotBlueprintable, ClassGroup = SpatialFunctionalTest)
class SPATIALGDKFUNCTIONALTESTS_API USpatialComponentTestDummyComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	USpatialComponentTestDummyComponent();

	virtual void OnAuthorityGained() override;

	virtual void OnAuthorityLost() override;

	virtual void OnActorReady(bool bHasAuthority) override;

	virtual void OnClientOwnershipGained() override;

	virtual void OnClientOwnershipLost() override;

	int NumAuthorityGains = 0;

	int NumAuthorityLosses = 0;

	int NumActorReadyAuth = 0;

	int NumActorReadyNonAuth = 0;

	int NumClientOwnershipGains = 0;

	int NumClientOwnershipLosses = 0;
};
