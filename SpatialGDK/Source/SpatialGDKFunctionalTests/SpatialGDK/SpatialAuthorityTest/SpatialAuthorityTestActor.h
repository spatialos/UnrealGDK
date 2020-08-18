// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

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
	ASpatialAuthorityTestActor();

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
