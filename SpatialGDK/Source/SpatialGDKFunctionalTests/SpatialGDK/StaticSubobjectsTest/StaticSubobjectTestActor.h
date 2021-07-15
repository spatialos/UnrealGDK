// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/ReplicatedTestActorBase.h"

#include "StaticSubobjectTestActor.generated.h"

UCLASS()
class AStaticSubobjectTestActor : public AReplicatedTestActorBase
{
	GENERATED_BODY()

public:
	AStaticSubobjectTestActor();

	void InitialiseTestIntProperty();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	int TestIntProperty;

	UPROPERTY(Replicated)
	USceneComponent* TestStaticComponent1;

	UPROPERTY(Replicated)
	USceneComponent* TestStaticComponent2;
};
