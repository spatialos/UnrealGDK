// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "SpatialPing.generated.h"

UCLASS()
class SPATIALGDK_API ASpatialPing
	: public AInfo
{
	GENERATED_UCLASS_BODY()

public:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	UPROPERTY(Replicated)
	uint32 LastReceivedPingID;

	TMap<uint32, float> PingSentTimes;


};
