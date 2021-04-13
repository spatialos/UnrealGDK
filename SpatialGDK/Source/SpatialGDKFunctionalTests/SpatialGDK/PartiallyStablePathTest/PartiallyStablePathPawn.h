// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DefaultPawn.h"

#include "PartiallyStablePathPawn.generated.h"

class APartiallyStablePathActor;
class UStaticMeshComponent;

UCLASS()
class APartiallyStablePathPawn : public ADefaultPawn
{
	GENERATED_BODY()

public:
	APartiallyStablePathPawn() {}

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerVerifyComponentReference(APartiallyStablePathActor* Actor, UStaticMeshComponent* Component);

	bool bServerRPCCalled = false;
	bool bRPCParamMatchesComponent = false;
};
