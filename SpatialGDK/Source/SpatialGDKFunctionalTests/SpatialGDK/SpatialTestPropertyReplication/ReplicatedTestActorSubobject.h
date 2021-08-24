// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "ReplicatedTestActor.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/ReplicatedTestActorBase.h"
#include "ReplicatedTestActorSubobject.generated.h"

class AReplicatedTestActor;

UCLASS()
class UReplicatedSubobject : public UActorComponent
{
	GENERATED_BODY()
public:
	UReplicatedSubobject();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	int TestReplicatedProperty;
};

UCLASS()
class AReplicatedTestActorSubobject : public AReplicatedTestActorBase
{
	GENERATED_BODY()

public:
	AReplicatedTestActorSubobject();

	void OnAuthorityGained() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Subobjects
	UPROPERTY(Replicated)
	UReplicatedSubobject* ReplicatedSubobject;
};
