// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SpatialAuthorityTestGameState.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialAuthorityTestGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:	
	ASpatialAuthorityTestGameState();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, Replicated, Category = "Default")
	int32 AuthorityOnBeginPlay;

	UPROPERTY(VisibleAnywhere, Replicated, Category = "Default")
	int32 AuthorityOnTick;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
};
