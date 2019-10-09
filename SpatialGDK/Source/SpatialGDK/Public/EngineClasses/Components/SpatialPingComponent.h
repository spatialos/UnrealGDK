// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "SpatialPingComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialPingComponent, Log, All);

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
	float MinPingInterval = 1.0f;

	// The maximum time, in seconds, to wait for a reply before sending another ping.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SpatialPing)
	float TimeoutLimit = 4.0f;

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

	UFUNCTION()
	void TickPingComponent();

	UFUNCTION()
	void SendNewPing();

	UFUNCTION()
	void OnPingTimeout();

	UFUNCTION()
	virtual void OnRep_ReplicatedPingID();

	UFUNCTION(Server, Unreliable, WithValidation)
	virtual void SendServerWorkerPingID(uint16 PingID);
};
