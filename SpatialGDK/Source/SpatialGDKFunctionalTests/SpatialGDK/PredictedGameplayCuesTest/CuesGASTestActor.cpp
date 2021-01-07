// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CuesGASTestActor.h"
#include "GA_ApplyCueEffect.h"

ACuesGASTestActor::ACuesGASTestActor()
	: OnActiveCounter(0)
	, ExecuteCounter(0)
{
}

void ACuesGASTestActor::OnRep_Owner()
{
	// Make sure that any cached state in the ASC is up to date about our ownership chain.
	// This is relevant for ability prediction - one of the conditions for allowing ability prediction is
	// that the ASC's avatar actor is owned by the client's player controller.
	// (See FGameplayAbilityActorInfo::IsLocallyControlled, which is checked before allowing ability prediction)
	GetAbilitySystemComponent()->RefreshAbilityActorInfo();
}

void ACuesGASTestActor::OnActorReady(bool bHasAuthority)
{
	if (bHasAuthority)
	{
		// We need this actor to be an autonomous proxy on the owning client, since that is one of the conditions
		// for being allowed to predict abilities on the client.
		// We can't set the remote role to autonomous proxy when we spawn this actor since an actor's remote role
		// gets reset to SimulatedProxy by the GDK when the spawning server receives authority over the entity back from SpatialOS.
		// (Except for player-controlled controllers, pawns and player states. See USpatialReceiver::HandleActorAuthority)
		// OnActorReady is called after that has occurred, so here we can set ourselves to be an autonomous proxy
		// without the remote role getting stomped again.
		SetAutonomousProxy(true);
	}
}

TArray<TSubclassOf<UGameplayAbility>> ACuesGASTestActor::GetInitialGrantedAbilities()
{
	return { UGA_ApplyCueEffect::StaticClass() };
}
