// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MultipleOwnershipPawn.generated.h"

UCLASS()
class AMultipleOwnershipPawn : public APawn
{
	GENERATED_BODY()

private:
	UPROPERTY()
	UStaticMeshComponent* CubeComponent;

	UPROPERTY()
	class USphereComponent* CollisionComponent;

public:
	AMultipleOwnershipPawn();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	int ReceivedRPCs;

	UFUNCTION(Server, Reliable)
	void ServerSendRPC();
};
