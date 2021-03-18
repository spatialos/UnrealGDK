// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Components/SceneComponent.h"
#include "CoreMinimal.h"
#include "TestUnrealComponents.generated.h"

/**
 * Simple component that only contains normal replicated data
 */
UCLASS()
class UDataOnlyComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UDataOnlyComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	int32 DataReplicatedVar;
};

/**
 * Simple component that contains data and handover replicated variables
 */
UCLASS()
class UDataAndHandoverComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UDataAndHandoverComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	int32 DataReplicatedVar;

	UPROPERTY(Handover)
	int32 HandoverReplicatedVar;
};

/**
 * Simple component that contains data and owner only replicated variables
 */
UCLASS()
class UDataAndOwnerOnlyComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UDataAndOwnerOnlyComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	int32 DataReplicatedVar;

	UPROPERTY(Replicated)
	int32 OwnerOnlyReplicatedVar;
};

/**
 * Simple component that contains data and inital only replicated variables
 */
UCLASS()
class UDataAndInitialOnlyComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UDataAndInitialOnlyComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	int32 DataReplicatedVar;

	UPROPERTY(Replicated)
	int32 InitialOnlyReplicatedVar;
};
