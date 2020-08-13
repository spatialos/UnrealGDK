// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SpatialAuthorityTestGameMode.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialAuthorityTestGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:	
	ASpatialAuthorityTestGameMode();

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
