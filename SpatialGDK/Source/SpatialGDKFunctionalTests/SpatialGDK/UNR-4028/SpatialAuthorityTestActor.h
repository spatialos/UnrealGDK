// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpatialAuthorityTestActor.generated.h"

class ASpatialFunctionalTest;
class ASpatialFunctionalTestFlowController;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialAuthorityTestActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpatialAuthorityTestActor();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, Category = "Default")
	ASpatialFunctionalTest* OwnerTest;

	UPROPERTY(VisibleAnywhere, Replicated, Category = "Default")
	int32 AuthorityOnBeginPlay;

	UPROPERTY(VisibleAnywhere, Replicated, Category = "Default")
	int32 AuthorityOnTick;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
