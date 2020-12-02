// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/GASTestActorBase.h"
#include "SpyValueGASTestActor.generated.h"

/**
 * A replicated Actor with a Cube Mesh, used as a base for Actors used in spatial tests.
 */
UCLASS()
class ASpyValueGASTestActor : public AGASTestActorBase
{
	GENERATED_BODY()

public:
	ASpyValueGASTestActor();

	void OnAuthorityGained() override;

private:
	TArray<TSubclassOf<UGameplayAbility>> GetInitialGrantedAbilities() override;
};
