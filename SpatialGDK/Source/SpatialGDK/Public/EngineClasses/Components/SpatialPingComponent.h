// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "SpatialPingComponent.generated.h"

/*
 Offers a configurable means of measuring round-trip latency in SpatialOS deployments.
 This component should be attached to a player controller.
 */
UCLASS(ClassGroup = (SpatialGDK), Meta = (BlueprintSpawnableComponent))
class SPATIALGDK_API USpatialPingComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SpatialPing)
	bool bStartWithPingEnabled = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SpatialPing)
	float PingFrequency = 0.5f;

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

private:
	float RoundTripPing;

	TArray<int16> SentPingIDs;
	TArray<double> SentPingTimestamps;

	int16 LastSentPingID;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedPingID)
	int16 ReplicatedPingID;

	bool bIsPingEnabled = false;

	FTimerHandle PingTimerHandle;

	APlayerController* OwningController;

	void EnablePing();

	void DisablePing();

	UFUNCTION()
	void TickPingComponent();

	void SendNewPing();

	UFUNCTION()
	virtual void OnRep_ReplicatedPingID();

	UFUNCTION(Server, Unreliable, WithValidation)
	virtual void SendServerWorkerPingID(int16 PingID);
};
