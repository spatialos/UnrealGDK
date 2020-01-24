// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SpatialAbilitySystemComponent.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "SpatialConstants.h"
#include "GeneralProjectSettings.h"
#include "GameplayAbilitySpec.h"

DEFINE_LOG_CATEGORY(LogSpatialAbilitySystemComponent);

void USpatialAbilitySystemComponent::BeginPlay()
{

	bSpatialNetworkingEnabled = GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking();
	LockingPolicy = bSpatialNetworkingEnabled ? Cast<USpatialNetDriver>(GetWorld()->NetDriver)->LockingPolicy : nullptr;
}

bool USpatialAbilitySystemComponent::InternalTryActivateAbility(FGameplayAbilitySpecHandle AbilityToActivate, FPredictionKey InPredictionKey, UGameplayAbility ** OutInstancedAbility, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData)
{
	ABILITYLIST_SCOPE_LOCK(); // make sure our spec doesn't get removed during activation in the super call

	bool bAbilitySuccessfullyActivated = Super::InternalTryActivateAbility(AbilityToActivate, InPredictionKey, OutInstancedAbility, OnGameplayAbilityEndedDelegate, TriggerEventData);

	// If we failed the ability or we're using Unreal Networking we can just return the result early and not care about locking.
	if (!bAbilitySuccessfullyActivated || !bSpatialNetworkingEnabled)
	{
		return bAbilitySuccessfullyActivated;
	}

	//todo: may want to get the activation info from the UGameplayAbility instance when running an instanced ability, might be more reliable.
	// though really prediction keys may not be the best mapping to use so this will likely change
	FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(AbilityToActivate);
	FPredictionKey ActivationPredictionKey = Spec->ActivationInfo.GetActivationPredictionKey();

	//todo: check whether the ability activated and ended immediately. In that case, don't lock at all

	UE_LOG(LogSpatialAbilitySystemComponent, Warning, TEXT("AbilitySystemComponent::InternalTryActivateAbility returned true"));
	bool LockSucceeded = LockAbilityInstance(ActivationPredictionKey);
	if (LockSucceeded)
	{
		BindUnlockToAbilityEnd(ActivationPredictionKey, OnGameplayAbilityEndedDelegate);
	}

	return true; // for now we'll just let the ability continued regardless of lock success, we print an error in the method below
}

bool USpatialAbilitySystemComponent::LockAbilityInstance(FPredictionKey InPredictionKey)
{
	check(LockingPolicy != nullptr);
	ActorLockToken LockToken = LockingPolicy->AcquireLock(GetOwner(), FString::Printf(TEXT("Ability instance lock. Prediction key Id: %d"), InPredictionKey.Current));
	if (LockToken == SpatialConstants::INVALID_ACTOR_LOCK_TOKEN)
	{
		// Could be worth trying to pass in the ability name here for debugging this error.
		UE_LOG(LogSpatialAbilitySystemComponent, Error, TEXT("Failed to lock Actor for ability instance. Prediction key Id: %d"), InPredictionKey.Current);
		return false;
	}
	check(!AbilityInstanceToLockTokens.Contains(LockToken));
	AbilityInstanceToLockTokens.Add(InPredictionKey.Current, LockToken);
	return true;
}

void USpatialAbilitySystemComponent::BindUnlockToAbilityEnd(FPredictionKey InPredictionKey, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate)
{
	check(OnGameplayAbilityEndedDelegate != nullptr);
	OnGameplayAbilityEndedDelegate->BindLambda([this, InPredictionKey](UGameplayAbility* GameplayAbility)
	{
		this->UnlockAfterAbilityEnded(InPredictionKey);
	});
}

bool USpatialAbilitySystemComponent::UnlockAfterAbilityEnded(FPredictionKey InPredictionKey)
{
	uint16 KeyId = InPredictionKey.Current;
	if (!AbilityInstanceToLockTokens.Contains(KeyId))
	{
		UE_LOG(LogSpatialAbilitySystemComponent, Error, TEXT("Tried to unlock after an ability ended but no lock token was found for the prediction key. Prediction key Id: %d"), InPredictionKey.Current);
		return false;
	}

	check(LockingPolicy != nullptr);
	ActorLockToken LockToken = AbilityInstanceToLockTokens.FindAndRemoveChecked(KeyId);
	LockingPolicy->ReleaseLock(LockToken);
	return true;
}
