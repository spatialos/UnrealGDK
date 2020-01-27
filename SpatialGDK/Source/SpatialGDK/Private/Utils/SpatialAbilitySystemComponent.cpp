// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SpatialAbilitySystemComponent.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "LoadBalancing/AbstractLockingPolicy.h"
#include "SpatialConstants.h"
#include "SpatialGDKSettings.h"

#include "GameplayAbilities/Public/Abilities/GameplayAbility.h"
#include "GameplayAbilities/Public/GameplayAbilitySpec.h"
#include "GameplayAbilities/Public/Abilities/GameplayAbilityTypes.h"
#include "GeneralProjectSettings.h"

DEFINE_LOG_CATEGORY(LogSpatialAbilitySystemComponent);

void USpatialAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();

	bool bSpatialNetworkingEnabled = GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking();
	bool bEnableUnrealLoadBalancer = GetDefault<USpatialGDKSettings>()->bEnableUnrealLoadBalancer;
	USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(GetWorld()->NetDriver);

	if (!bSpatialNetworkingEnabled || !bEnableUnrealLoadBalancer || !NetDriver->IsServer())
	{
		return;
	}

	LockingPolicy = NetDriver->LockingPolicy;
	AbilityActivatedCallbacks.AddUObject(this, &USpatialAbilitySystemComponent::OnAbilityActivateCallback);
	AbilityEndedCallbacks.AddUObject(this, &USpatialAbilitySystemComponent::OnAbilityEndedCallback);
}

void USpatialAbilitySystemComponent::OnAbilityActivateCallback(UGameplayAbility* ActivatedAbility)
{
	if (ActivatedAbility->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::NonInstanced)
	{
		UE_LOG(LogSpatialAbilitySystemComponent, Error, TEXT("Failed to lock Actor for non-instanced ability"));
		return;
	}

	const FGameplayAbilityActivationInfo& ActivationInfo = ActivatedAbility->GetCurrentActivationInfoRef();
	FPredictionKey ActivationPredictionKey = ActivationInfo.GetActivationPredictionKey();

	bool LockSucceeded = LockAbilityInstance(ActivationPredictionKey.Current);
	if (!LockSucceeded)
	{
		UE_LOG(LogSpatialAbilitySystemComponent, Error, TEXT("Failed to lock Actor for ability instance"));
	}
}

void USpatialAbilitySystemComponent::OnAbilityEndedCallback(UGameplayAbility* AbilityThatEnded)
{
	if (AbilityThatEnded->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::NonInstanced)
	{
		UE_LOG(LogSpatialAbilitySystemComponent, Error, TEXT("Failed to unlock Actor for non-instanced ability"));
		return;
	}

	const FGameplayAbilityActivationInfo& ActivationInfo = AbilityThatEnded->GetCurrentActivationInfoRef();
	FPredictionKey ActivationPredictionKey = ActivationInfo.GetActivationPredictionKey();

	UnlockAfterAbilityEnded(ActivationPredictionKey.Current);
}

bool USpatialAbilitySystemComponent::LockAbilityInstance(AbilityInstanceIdentifier AbilityInstanceId)
{
	check(LockingPolicy != nullptr);
	ActorLockToken LockToken = LockingPolicy->AcquireLock(GetOwner(), FString::Printf(TEXT("Ability instance lock. Ability Id: %d"), AbilityInstanceId));
	if (LockToken == SpatialConstants::INVALID_ACTOR_LOCK_TOKEN)
	{
		// Could be worth trying to pass in the ability name here for debugging this error.
		UE_LOG(LogSpatialAbilitySystemComponent, Error, TEXT("Failed to lock Actor for ability instance. Ability Id: %d"), AbilityInstanceId);
		return false;
	}
	check(!AbilityInstanceToLockTokens.Contains(LockToken));
	AbilityInstanceToLockTokens.Add(AbilityInstanceId, LockToken);
	return true;
}

bool USpatialAbilitySystemComponent::UnlockAfterAbilityEnded(AbilityInstanceIdentifier AbilityInstanceId)
{
	if (!AbilityInstanceToLockTokens.Contains(AbilityInstanceId))
	{
		UE_LOG(LogSpatialAbilitySystemComponent, Error, TEXT("Tried to unlock after an ability ended but no lock token was found for the ability Id. Ability Id: %d"), AbilityInstanceId);
		return false;
	}

	ActorLockToken LockToken = AbilityInstanceToLockTokens.FindAndRemoveChecked(AbilityInstanceId);
	check(LockingPolicy != nullptr);
	LockingPolicy->ReleaseLock(LockToken);
	return true;
}
