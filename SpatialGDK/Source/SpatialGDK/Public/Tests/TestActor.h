// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "TestActor.generated.h"

UCLASS()
class ATestActor : public AActor
{
	GENERATED_BODY()

	ATestActor() { bReplicates = true; }

public:
	UFUNCTION(Server, Reliable)
	void TestServerRPC();
	void TestServerRPC_Implementation(){};
};
