// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpatialFunctionalTest.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialAuthorityTestActor.h"
#include "SpatialAuthorityTestReplicatedActor.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialAuthorityTestReplicatedActor : public ASpatialAuthorityTestActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpatialAuthorityTestReplicatedActor();

	//void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//ASpatialFunctionalTest* OwnerTest;

	//UPROPERTY(Replicated)
	//ASpatialFunctionalTestFlowController* AuthorityOnBeginPlay;

	//UPROPERTY(Replicated)
	//ASpatialFunctionalTestFlowController* AuthorityOnTick;

protected:
	// Called when the game starts or when spawned
	//virtual void BeginPlay() override;

public:	
	// Called every frame
	//virtual void Tick(float DeltaTime) override;
};
