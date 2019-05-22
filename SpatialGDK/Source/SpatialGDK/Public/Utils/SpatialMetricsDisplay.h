// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "SpatialMetricsDisplay.generated.h"

USTRUCT()
struct FWorkerStats
{
	GENERATED_BODY()

	UPROPERTY()
	FString WorkerName;
	UPROPERTY()
	float AverageFPS;
	UPROPERTY()
	float WorkerLoad;

	bool operator==(const FWorkerStats& other) const
	{
		return (WorkerName.Equals(other.WorkerName));
	}
};

UCLASS()
class SPATIALGDK_API ASpatialMetricsDisplay :
	public AInfo
{
	GENERATED_UCLASS_BODY()

public:

	virtual void Tick(float DeltaSeconds) override;

	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	UFUNCTION(BlueprintCallable)
	void ToggleStatDisplay();

private:

	FDelegateHandle DrawDebugDelegateHandle;

	UPROPERTY(Replicated)
	TArray<FWorkerStats> WorkerStats;

	TMap<FString, float> WorkerStatsLastUpdateTime;

	const uint32 PreallocatedWorkerCount = 8;
	const uint32 WorkerNameMaxLength = 18;

	const uint32 DropStatsIfNoUpdateForTime = 10; // seconds

	UFUNCTION(CrossServer, Unreliable, WithValidation)
	virtual void ServerUpdateWorkerStats(const float Time, const FWorkerStats& OneWorkerStats);

	void DrawDebug(class UCanvas* Canvas, APlayerController* Controller);
};
