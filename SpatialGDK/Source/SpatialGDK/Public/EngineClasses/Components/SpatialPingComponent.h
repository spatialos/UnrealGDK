// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "SpatialPingComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialPingComponent, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRecordPing, float, Ping);

USTRUCT(BlueprintType)
struct FSpatialPingAverageData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = SpatialPing)
	float LastMeasurementsWindowAvg;
	UPROPERTY(BlueprintReadWrite, Category = SpatialPing)
	float LastMeasurementsWindowMin;
	UPROPERTY(BlueprintReadWrite, Category = SpatialPing)
	float LastMeasurementsWindowMax;
	UPROPERTY(BlueprintReadWrite, Category = SpatialPing)
	float LastMeasurementsWindow50thPercentile;
	UPROPERTY(BlueprintReadWrite, Category = SpatialPing)
	float LastMeasurementsWindow90thPercentile;
	UPROPERTY(BlueprintReadWrite, Category = SpatialPing)
	int WindowSize;

	UPROPERTY(BlueprintReadWrite, Category = SpatialPing)
	float TotalAvg;
	UPROPERTY(BlueprintReadWrite, Category = SpatialPing)
	float TotalMin;
	UPROPERTY(BlueprintReadWrite, Category = SpatialPing)
	float TotalMax;
	UPROPERTY(BlueprintReadWrite, Category = SpatialPing)
	int TotalNum;
};

/*
 Offers a configurable means of measuring round-trip latency in SpatialOS deployments.
 This component should be attached to a player controller.
 */
UCLASS(ClassGroup = (SpatialGDK), Meta = (BlueprintSpawnableComponent))
class SPATIALGDK_API USpatialPingComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()

public:
	// Determines whether the component starts with ping behavior enabled.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SpatialPing)
	bool bStartWithPingEnabled = true;

	// The minimum time, in seconds, between pings.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SpatialPing)
	float MinPingInterval = 0.25f;

	// The maximum time, in seconds, to wait for a reply before sending another ping.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SpatialPing)
	float TimeoutLimit = 4.0f;

	// The number of ping measurements recorded for the rolling average.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SpatialPing)
	int PingMeasurementsWindowSize = 20;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Returns whether this component can ping.
	UFUNCTION(BlueprintCallable, Category = "SpatialGDK|Ping")
	bool GetIsPingEnabled() const;

	// Set whether this component can ping.
	UFUNCTION(BlueprintCallable, Category = "SpatialGDK|Ping")
	void SetPingEnabled(bool bSetEnabled);

	// Returns the latest raw round-trip ping value in seconds.
	UFUNCTION(BlueprintCallable, Category = "SpatialGDK|Ping")
	float GetPing() const;

	// Returns the average, min, and max values for the last PingMeasurementsWindowSize measurements as well as the total average, min, and
	// max.
	UFUNCTION(BlueprintCallable, Category = "SpatialGDK|Ping")
	FSpatialPingAverageData GetAverageData() const;

	// Multicast delegate that will be broadcast whenever a new ping measurement is recorded.
	UPROPERTY(BlueprintAssignable, Category = "SpatialGDK|Ping")
	FOnRecordPing OnRecordPing;

private:
	float RoundTripPing;

	uint16 LastSentPingID;
	double LastSentPingTimestamp;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedPingID)
	uint16 ReplicatedPingID;

	uint32 TimeoutCount;

	bool bIsPingEnabled = false;

	FTimerHandle PingTickHandle;
	FTimerHandle PingTimerHandle;

	UPROPERTY()
	APlayerController* OwningController;

	void EnablePing();
	void DisablePing();

	void TickPingComponent();

	void SendNewPing();

	void OnPingTimeout();

	UFUNCTION()
	virtual void OnRep_ReplicatedPingID();

	UFUNCTION(Server, Unreliable, WithValidation)
	virtual void SendServerWorkerPingID(uint16 PingID);

	void RecordPing(float Ping);

	TArray<float> LastPingMeasurements;

	float TotalPing = 0.0f;
	float TotalMin = 1.0f;
	float TotalMax = 0.0f;
	int TotalNum = 0;
};
