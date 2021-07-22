// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/ReplicatedTestActorBase.h"
#include "ReplicatedTestActorBasic.generated.h"

UCLASS()
class AReplicatedTestActorBasic : public AReplicatedTestActorBase
{
	GENERATED_BODY()

public:
	AReplicatedTestActorBasic();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Basic replicated property types
	UPROPERTY(Replicated)
	int ReplicatedIntProperty;

	UPROPERTY(Replicated)
	double ReplicatedFloatProperty;

	UPROPERTY(Replicated)
	bool bReplicatedBoolProperty;

	UPROPERTY(Replicated)
	FString ReplicatedStringProperty;

	// Basic non-replicated property types
	UPROPERTY()
	int IntProperty;

	UPROPERTY()
	double FloatProperty;

	UPROPERTY()
	bool bBoolProperty;

	UPROPERTY()
	FString StringProperty;
};
