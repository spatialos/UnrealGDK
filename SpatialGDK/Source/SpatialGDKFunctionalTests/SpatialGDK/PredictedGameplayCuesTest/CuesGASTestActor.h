// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "../TestActors/GASTestActorBase.h"
#include "CoreMinimal.h"
#include "CuesGASTestActor.generated.h"

/**
 * Test actor with an ASC, set up to be able to run predictive abilities,
 * and non-replicated counters to record gameplay cue events triggerd on UGC_SignalCueActivation, which the test asserts against.
 * To be able to run predictive abilities, this actor sets its own remote role to AutonomousProxy in OnActorReady,
 * and updates the ASC's actor info in OnRep_Owner. See their respective comments for more info.
 */
UCLASS()
class ACuesGASTestActor : public AGASTestActorBase
{
	GENERATED_BODY()

public:
	ACuesGASTestActor();

	void SignalOnActive() { OnActiveCounter++; }
	void SignalExecute() { ExecuteCounter++; }

	int GetOnActiveCounter() { return OnActiveCounter; }
	int GetExecuteCounter() { return ExecuteCounter; }

	virtual void OnActorReady(bool bHasAuthority) override;

protected:
	virtual void OnRep_Owner() override;

private:
	TArray<TSubclassOf<UGameplayAbility>> GetInitialGrantedAbilities() override;

	int OnActiveCounter;
	int ExecuteCounter;
};
