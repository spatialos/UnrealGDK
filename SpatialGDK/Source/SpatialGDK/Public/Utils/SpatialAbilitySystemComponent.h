// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialCommonTypes.h"

#include "Components/ActorComponent.h"
#include "AbilitySystemComponent.h"
#include "GameplayPrediction.h"

#include "SpatialAbilitySystemComponent.generated.h"

class UAbstractLockingPolicy;
class UGameplayAbility;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialAbilitySystemComponent, Log, All);

UCLASS(ClassGroup = AbilitySystem, hidecategories = (Object, LOD, Lighting, Transform, Sockets, TextureStreaming), editinlinenew, meta = (BlueprintSpawnableComponent))
class SPATIALGDK_API USpatialAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	// To identify unique ability activations we use the prediction key, which is the ID used by the server to cancel / rewind
	// CSP'd ability activations after the fact. It's a uint16 which wraps round, ability activations are so short-lived this
	// should never cause us an issue with collisions, or something else in Unreal would probably break if so anyway.
	using AbilityInstanceIdentifier = FPredictionKey::KeyType;

	virtual void BeginPlay() override;
	void OnAbilityActivateCallback(UGameplayAbility* ActivatedAbility);
	void OnAbilityEndedCallback(UGameplayAbility* AbilityThatEnded);

private:
	UAbstractLockingPolicy* LockingPolicy;
	TMap<AbilityInstanceIdentifier, ActorLockToken> AbilityInstanceToLockTokens;

	bool LockAbilityInstance(AbilityInstanceIdentifier AbilityInstanceId);
	bool UnlockAfterAbilityEnded(AbilityInstanceIdentifier AbilityInstanceId);
};
