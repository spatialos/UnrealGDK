// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/ReplicatedTestActorBase.h"

#include "DynamicSubObjectTestActor.generated.h"

UCLASS()
class ADynamicSubObjectTestActor : public AReplicatedTestActorBase
{
	GENERATED_BODY()

public:
	ADynamicSubObjectTestActor();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void InitialiseTestIntProperty();

	UPROPERTY(Replicated)
	int TestIntProperty;
};
