// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "SpatialPingComponent.generated.h"

UCLASS(ClassGroup = (SpatialGDK), Meta = (BlueprintSpawnableComponent))
class SPATIALGDK_API USpatialPingComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SpatialPing)
	bool bCanPing = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SpatialPing)
	float PingFrequency = 0.5f;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = SpatialPing)
	float GetPing() const;

private:
	float RTPing;

	uint32 LastSentPingID;
	uint32 LastReceivedPingID;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedPingID)
	uint32 ReplicatedPingID;

	FTimerHandle PingTimerHandle;

	UFUNCTION()
	void TickPingComponent();

	TArray<uint32> SentPingIDs;
	TArray<float> SentPingTimes;

	void SendNewPing();

	UFUNCTION()
	virtual void OnRep_ReplicatedPingID();

	UFUNCTION(Server, Unreliable, WithValidation)
	virtual void SendServerWorkerPingID(const uint32 PingID);
};
