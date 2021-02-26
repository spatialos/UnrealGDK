// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "TestMaps/GeneratedTestMap.h"
#include "SpatialReplicatedStartupActorTest.generated.h"

class AReplicatedStartupActor;

// Currently broken: UNR-4222 
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialReplicatedStartupActorTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialReplicatedStartupActorTest();

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
	USpatialTestReplicatedStartupActorMap()
		: UGeneratedTestMap(EMapCategory::NO_CI, TEXT("SpatialReplicatedStartupActorTest"))
	{
	}

protected:
	virtual void CreateCustomContentForMap() override;
};
