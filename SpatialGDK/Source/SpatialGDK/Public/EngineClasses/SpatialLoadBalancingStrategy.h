// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "SpatialLoadBalancingStrategy.generated.h"

class AActor;
class ASpatialVirtualWorkerTranslator;
class USpatialActorChannel;
class USpatialNetDriver;

UCLASS(Abstract, BlueprintInternalUseOnly)
class SPATIALGDK_API USpatialLoadBalancingStrategy: public UObject
{
	GENERATED_BODY()
public:

	virtual void Init(const ASpatialVirtualWorkerTranslator* Translator);
	virtual ~USpatialLoadBalancingStrategy();

	virtual bool ShouldChangeAuthority(const AActor& Actor) const { return false; }
	virtual FString GetAuthoritativeVirtualWorkerId(const AActor& Actor) const PURE_VIRTUAL(USpatialLoadBalancingStrategy::GetAuthoritativeVirtualWorkerId, return ""; )

protected:

	const ASpatialVirtualWorkerTranslator* Translator;
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class SPATIALGDK_API USingleWorkerLoadBalancingStrategy final : public USpatialLoadBalancingStrategy
{
	GENERATED_BODY()
public:

	virtual FString GetAuthoritativeVirtualWorkerId(const AActor& Actor) const override;
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class SPATIALGDK_API UGridBasedLoadBalancingStrategy final : public USpatialLoadBalancingStrategy
{
	GENERATED_BODY()
public:
	virtual void Init(const ASpatialVirtualWorkerTranslator* Translator) override;
	virtual bool ShouldChangeAuthority(const AActor& Actor) const override;
	virtual FString GetAuthoritativeVirtualWorkerId(const AActor& Actor) const override;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	int32 ColumnCount = 1;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	int32 RowCount = 1;
};

