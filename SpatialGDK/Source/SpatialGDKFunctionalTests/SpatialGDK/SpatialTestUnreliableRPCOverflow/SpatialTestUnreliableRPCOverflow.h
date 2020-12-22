// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestUnreliableRPCOverflow.generated.h"

class AUnreliableRPCTestActor;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestUnreliableRPCOverflow : public ASpatialFunctionalTest
{
	GENERATED_BODY()
public:
	ASpatialTestUnreliableRPCOverflow();

	virtual void PrepareTest() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	uint32 RPCLimitCount;

	UPROPERTY(Replicated)
	AUnreliableRPCTestActor* TestActor = nullptr;
};
