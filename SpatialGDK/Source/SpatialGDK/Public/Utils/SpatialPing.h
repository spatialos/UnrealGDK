// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"

#include "SpatialPing.generated.h"

UCLASS()
class SPATIALGDK_API ASpatialPing : public AInfo
{
	GENERATED_UCLASS_BODY()

public:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	float RTPing;

	uint32 LastSentPingID;
	uint32 LastReceivedPingID;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedPingID)
	uint32 ReplicatedPingID;

	TMap<uint32, float> PingSentTimes;

	TArray<uint32> SentPingIDs;
	TArray<float> SentPingTimes;

	void SendNewPing();

	UFUNCTION()
	virtual void OnRep_ReplicatedPingID();

	UFUNCTION(Server, Unreliable, WithValidation)
	virtual void SendServerWorkerPingID(const uint32 PingID);
};
