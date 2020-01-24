// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialAbilitySystemComponent.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "SpatialConstants.h"

DEFINE_LOG_CATEGORY(LogSpatialAbilitySystemComponent);

void USpatialAbilitySystemComponent::BeginPlay()
{

	bSpatialNetworkingEnabled = GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking();
	LockingPolicy = bSpatialNetworkingEnabled ? Cast<USpatialNetDriver>(GetWorld()->NetDriver)->LockingPolicy : nullptr;
}

bool USpatialAbilitySystemComponent::InternalTryActivateAbility(FGameplayAbilitySpecHandle AbilityToActivate, FPredictionKey InPredictionKey = FPredictionKey(), UGameplayAbility ** OutInstancedAbility = nullptr, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate = nullptr, const FGameplayEventData* TriggerEventData = nullptr)
{
	bool bAbilitySuccessfullyActivated = Super::InternalTryActivateAbility(AbilityToActivate, InPredictionKey, OutInstancedAbility, OnGameplayAbilityEndedDelegate, TriggerEventData);

	// If we failed the ability or we're using Unreal Networking we can just return the result early and not care about locking.
	if (!bAbilitySuccessfullyActivated || !bSpatialNetworkingEnabled)
	{
		return bAbilitySuccessfullyActivated;
	}

	UE_LOG(LogSpatialAbilitySystemComponent, Warning, TEXT("AbilitySystemComponent::InternalTryActivateAbility returned true"));
	bool LockSucceeded = LockAbilityInstance(InPredictionKey);
	if (LockSucceeded)
	{
		BindUnlockToAbilityEnd(InPredictionKey, OnGameplayAbilityEndedDelegate);
	}
}

bool USpatialAbilitySystemComponent::LockAbilityInstance(FPredictionKey InPredictionKey)
{
	check(LockingPolicy != nullptr);
	ActorLockToken LockToken = LockingPolicy->AcquireLock(GetOwner(), "Ability instance lock. Prediction key Id: %d", InPredictionKey.Current);
	if (LockToken == SpatialConstants::INVALID_ACTOR_LOCK_TOKEN_LOCK)
	{
		// Could be worth trying to pass in the ability name here for debugging this error.
		UE_LOG(LogSpatialAbilitySystemComponent, Error, TEXT("Failed to lock Actor for ability instance. Prediction key Id: %d", InPredictionKey.Current));
		return false;
	}
	check(!AbilityInstanceToLockTokens.Contains(LockToken));
	AbilityInstanceToLockTokens.Add(InPredictionKey.Current, LockToken);
	return true;
}

bool USpatialAbilitySystemComponent::BindUnlockToAbilityEnd(FPredictionKey InPredictionKey, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate)
{
	check(OnGameplayAbilityEndedDelegate != nullptr);
	OnGameplayAbilityEndedDelegate->BindLambda([this, InPredictionKey](UGameplayAbility* GameplayAbility)
	{
		this->UnlockAfterAbilityEnded(InPredictionKey);
	});
}

void USpatialAbilitySystemComponent::UnlockAfterAbilityEnded(FPredictionKey InPredictionKey)
{
	uint16 KeyId = InPredictionKey.Current;
	if (!AbilityInstanceToLockTokens.Contains(KeyId))
	{
		UE_LOG(LogSpatialAbilitySystemComponent, Error, TEXT("Tried to unlock after an ability ended but no lock token was found for the prediction key. Prediction key Id: %d", InPredictionKey.Current));
		return false;
	}

	check(LockingPolicy != nullptr);
	ActorLockToken LockToken = AbilityInstanceToLockTokens.FindAndRemoveChecked(KeyId);
	LockingPolicy->ReleaseLock(LockToken);
}
