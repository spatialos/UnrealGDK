// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Delegates/Delegate.h"

#include "SpatialLoadBalancingStrategy.generated.h"

class AActor;
class ASpatialVirtualWorkerTranslator;
class USpatialActorChannel;
class USpatialNetDriver;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialLoadBalancer, Log, All);

UCLASS(Abstract, BlueprintInternalUseOnly)
class SPATIALGDK_API USpatialLoadBalancingStrategy: public UObject
{
	GENERATED_BODY()

public:
	virtual void Init(const USpatialNetDriver* InNetDriver, const ASpatialVirtualWorkerTranslator* Translator);
	virtual ~USpatialLoadBalancingStrategy();

	// TODO - timgibson - consider ShouldHaveAuthority instead?
	virtual bool ShouldChangeAuthority(const AActor& Actor) const { return false; }
	virtual FString GetAuthoritativeVirtualWorkerId(const AActor& Actor) const PURE_VIRTUAL(USpatialLoadBalancingStrategy::GetAuthoritativeVirtualWorkerId, return ""; )

protected:
	int32 GetLocalWorkerIndex() const { return LocalWorkerIndex; };

	const ASpatialVirtualWorkerTranslator* Translator = nullptr;
	const USpatialNetDriver* NetDriver = nullptr;

private:
	const FString GetWorkerId() const;
	void OnWorkerAssignmentChanged(const TArray<FString>& NewAssignements);

	int32 LocalWorkerIndex = INDEX_NONE;
	FDelegateHandle OnWorkerAssignmentChangedDelegateHandle = FDelegateHandle();
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class SPATIALGDK_API UGridBasedLoadBalancingStrategy final : public USpatialLoadBalancingStrategy
{
	GENERATED_BODY()

public:
	virtual void Init(const USpatialNetDriver* InNetDriver, const ASpatialVirtualWorkerTranslator* Translator) override;
	virtual bool ShouldChangeAuthority(const AActor& Actor) const override;
	virtual FString GetAuthoritativeVirtualWorkerId(const AActor& Actor) const override;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	int32 ColumnCount = 1;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	int32 RowCount = 1;

private:
	bool IsActorInCell(const AActor& Actor, const FBox2D& Cell) const;

	TArray<FBox2D> WorkerCells;
};

