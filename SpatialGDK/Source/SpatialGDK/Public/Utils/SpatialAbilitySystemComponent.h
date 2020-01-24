// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LoadBalancing/AbstractLockingPolicy.h"
#include "SpatialCommonTypes.h"

#include "GameplayAbilities/Public/AbilitySystemComponent.h"
#include "Components/ActorComponent.h"
#include "GameplayPrediction.h"

#include "SpatialAbilitySystemComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialAbilitySystemComponent, Log, All);

UCLASS(ClassGroup = AbilitySystem, hidecategories = (Object, LOD, Lighting, Transform, Sockets, TextureStreaming), editinlinenew, meta = (BlueprintSpawnableComponent))
class SPATIALGDK_API USpatialAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	// To identify unique ability activations we use the prediction key, which is the ID used by the server to cancel / rewind
	// CSP'd ability activations after the fact. It's a uint16 which wraps round, ability activations are so short-lived this
	// should never cause us an issue with collisions, or something else in Unreal would probably break if so anyway.
	using AbilityInstanceIdentifiers = FPredictionKey::KeyType;

	virtual void BeginPlay() override;
	virtual bool InternalTryActivateAbility(FGameplayAbilitySpecHandle AbilityToActivate, FPredictionKey InPredictionKey = FPredictionKey(), UGameplayAbility ** OutInstancedAbility = nullptr, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate = nullptr, const FGameplayEventData* TriggerEventData = nullptr) override;

private:
	bool bSpatialNetworkingEnabled;
	UAbstractLockingPolicy* LockingPolicy;
	TMap<AbilityInstanceIdentifiers, ActorLockToken> AbilityInstanceToLockTokens;

	void LockAbilityInstance(FPredictionKey InPredictionKey);
};
