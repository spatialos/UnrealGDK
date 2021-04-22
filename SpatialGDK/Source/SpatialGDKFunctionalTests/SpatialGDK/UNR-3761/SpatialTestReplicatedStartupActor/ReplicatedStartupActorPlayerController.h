// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ReplicatedStartupActorPlayerController.generated.h"

class ASpatialTestReplicatedStartupActor;
UCLASS()
class AReplicatedStartupActorPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(Server, Reliable)
	void ClientToServerRPC(ASpatialTestReplicatedStartupActor* Test, AActor* ReplicatedActor);

	UFUNCTION(Server, Reliable)
	void ResetBoolean(ASpatialTestReplicatedStartupActor* Test);
};
