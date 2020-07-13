// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "TestReplicatedStartupActor.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ATestReplicatedStartupActor : public ASpatialFunctionalTest
{
	GENERATED_BODY()
	
public:	
	ATestReplicatedStartupActor();

	virtual void BeginPlay() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void ClientToServerRPC(AActor* ReplicatedActor, ATestReplicatedStartupActor* Test);

	UFUNCTION(Client, Reliable)
	void ServerToClientRPC(AActor* ReplicatedActor, ATestReplicatedStartupActor* Test);

	float ElapsedTime;

	UPROPERTY(Replicated)
	bool bIsValidReference;
};
