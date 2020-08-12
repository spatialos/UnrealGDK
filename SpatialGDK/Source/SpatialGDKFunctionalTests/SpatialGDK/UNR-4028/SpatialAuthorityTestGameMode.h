// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SpatialAuthorityTestGameMode.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialAuthorityTestGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpatialAuthorityTestGameMode();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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
