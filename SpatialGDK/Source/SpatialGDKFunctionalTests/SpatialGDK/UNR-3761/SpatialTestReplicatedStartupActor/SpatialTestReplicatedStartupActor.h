// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "TestMaps/GeneratedTestMap.h"
#include "SpatialTestReplicatedStartupActor.generated.h"

class AReplicatedStartupActor;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestReplicatedStartupActor : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialTestReplicatedStartupActor();

	virtual void PrepareTest() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	bool bIsValidReference;

	AReplicatedStartupActor* ReplicatedStartupActor;
};

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API USpatialTestReplicatedStartupActorMap : public UGeneratedTestMap
{
	GENERATED_BODY()

public:
	USpatialTestReplicatedStartupActorMap();

protected:
	virtual void CreateCustomContentForMap() override;
};
