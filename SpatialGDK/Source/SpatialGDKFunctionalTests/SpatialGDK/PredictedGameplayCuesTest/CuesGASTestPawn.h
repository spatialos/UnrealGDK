// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "../TestActors/GASTestPawnBase.h"
#include "CoreMinimal.h"
#include "CuesGASTestPawn.generated.h"

/**
 * Test actor with an ASC, set up to be able to run predictive abilities,
 * and non-replicated counters to record gameplay cue events triggerd on UGC_SignalCueActivation, which the test asserts against.
 * To be able to run predictive abilities, this actor sets its own remote role to AutonomousProxy in OnActorReady,
 * and updates the ASC's actor info in OnRep_Owner. See their respective comments for more info.
 */
UCLASS()
class ACuesGASTestPawn : public AGASTestPawnBase
{
	GENERATED_BODY()

public:
	ACuesGASTestPawn();

	void SignalOnActive() { OnActiveCounter++; }
	void SignalExecute() { ExecuteCounter++; }

	int GetOnActiveCounter() { return OnActiveCounter; }
	int GetExecuteCounter() { return ExecuteCounter; }

private:
	TArray<TSubclassOf<UGameplayAbility>> GetInitialGrantedAbilities() override;

	int OnActiveCounter;
	int ExecuteCounter;
};
