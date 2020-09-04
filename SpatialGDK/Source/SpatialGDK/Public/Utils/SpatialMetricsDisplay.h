// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/Queue.h"
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
	float ServerMovementCorrections; // per second
	UPROPERTY()
	int32 ServerConsiderListSize;
	UPROPERTY()
	uint32 ServerReplicationLimit;

	bool operator==(const FWorkerStats& other) const { return (WorkerName.Equals(other.WorkerName)); }
};

UCLASS(SpatialType)
class SPATIALGDK_API ASpatialMetricsDisplay : public AInfo
{
	GENERATED_UCLASS_BODY()

public:
	virtual void Tick(float DeltaSeconds) override;

	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	UFUNCTION(Exec, Category = "SpatialGDK", BlueprintCallable)
	void SpatialToggleStatDisplay();

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

	bool ShouldRemoveStats(const float CurrentTime, const FWorkerStats& OneWorkerStats) const;
	void DrawDebug(class UCanvas* Canvas, APlayerController* Controller);

	struct MovementCorrectionRecord
	{
		int32 MovementCorrections;
		float Time;
	};
	TQueue<MovementCorrectionRecord> MovementCorrectionRecords;
};
