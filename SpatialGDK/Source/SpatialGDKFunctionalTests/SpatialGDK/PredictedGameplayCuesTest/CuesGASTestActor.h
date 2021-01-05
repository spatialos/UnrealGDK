// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/GASTestActorBase.h"
#include "CuesGASTestActor.generated.h"

/**
 * A replicated Actor with a Cube Mesh, used as a base for Actors used in spatial tests.
 */
UCLASS()
class ACuesGASTestActor : public AGASTestActorBase
{
	GENERATED_BODY()

public:
	ACuesGASTestActor();

	void IncrementedCue() { AddCounter++; }
	int GetAddCounter() { return AddCounter; }

	void ExecutedCue() { ExecuteCounter++; }
	int GetExecuteCounter() { return ExecuteCounter; }

	virtual void OnActorReady(bool bHasAuthority)
		override; // TODO remove this and make this actor a character to possess. Avoids any autonomous proxy role setting dance

protected:
	virtual void OnRep_Owner() override;

private:
	TArray<TSubclassOf<UGameplayAbility>> GetInitialGrantedAbilities() override;

	int ExecuteCounter;
	int AddCounter;
};
