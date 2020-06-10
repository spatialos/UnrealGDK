// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RPCTestInterface.h"
#include "RPCInInterfaceActor.generated.h"

UCLASS(Blueprintable)
class SPATIALGDKFUNCTIONALTESTS_API ARPCInInterfaceActor : public AActor, public IRPCTestInterface
{
	GENERATED_BODY()

public:
	ARPCInInterfaceActor();

	bool bRPCReceived = false;

	UFUNCTION(Client, Reliable)
	virtual void RPCInInterface() override;

protected:
	virtual void RPCInInterface_Implementation() override;
};
