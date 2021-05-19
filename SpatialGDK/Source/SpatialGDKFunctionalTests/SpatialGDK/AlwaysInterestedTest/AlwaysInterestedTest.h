// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "AlwaysInterestedTest.generated.h"

class AAlwaysInterestedTestActor;
class ASmallNCDActor;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AAlwaysInterestedTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	AAlwaysInterestedTest();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PrepareTest() override;

	UPROPERTY(Replicated)
	AAlwaysInterestedTestActor* ActorWithAlwaysInterestedProperty;

	UPROPERTY(Replicated)
	ASmallNCDActor* InterestedInThisReplicatedActor;

	UPROPERTY(Replicated)
	ASmallNCDActor* NotInterestedInThisReplicatedActor;

	UPROPERTY(Replicated)
	ASmallNCDActor* OtherInterestedInThisReplicatedActor;

	// Only valid on server workers
	FVector LocalWorkerPosition;
	FVector OtherWorkerPosition;

	// Other tests rely on the pawns to be in the starting position, so cache and reset at end of test.
	FVector OriginalPawn1Position;
	FVector OriginalPawn2Position;
};
