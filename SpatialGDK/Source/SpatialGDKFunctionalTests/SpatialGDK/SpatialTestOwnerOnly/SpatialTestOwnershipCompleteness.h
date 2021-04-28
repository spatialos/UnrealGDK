// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "SpatialFunctionalTest.h"

#include "GameFramework/GameModeBase.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Misc/AutomationTest.h"
#include "TestMaps/GeneratedTestMap.h"

#include "SpatialTestOwnershipCompleteness.generated.h"

UCLASS()
class AOwnershipCompletenessGameMode : public AGameModeBase
{
	GENERATED_BODY()

	AOwnershipCompletenessGameMode();
};

UCLASS()
class AOwnershipCompletenessPawn : public APawn
{
	GENERATED_BODY()
};

UCLASS()
class AOwnershipCompletenessController : public APlayerController
{
	GENERATED_BODY()
};

UCLASS()
class AOwnershipCompletenessTestPawnActor : public AActor
{
	GENERATED_BODY()

public:
	AOwnershipCompletenessTestPawnActor() { bReplicates = true; }
	// UPROPERTY(Replicated)
	int OwnerOnlyValue = 0;
};

UCLASS()
class AOwnershipCompletenessTestControllerActor : public AActor
{
	GENERATED_BODY()

public:
	AOwnershipCompletenessTestControllerActor() { bReplicates = true; }
	// UPROPERTY(Replicated)
	int OwnerOnlyValue = 0;
};

UCLASS()
class AOwnershipCompletenessTestFreeActor : public AActor
{
	GENERATED_BODY()

public:
	AOwnershipCompletenessTestFreeActor() { bReplicates = true; }
	// UPROPERTY(Replicated)
	int OwnerOnlyValue = 0;
};

UCLASS()
class UOwnershipCompletenessGeneratedMap : public UGeneratedTestMap
{
	GENERATED_BODY()

	UOwnershipCompletenessGeneratedMap();

	virtual void CreateCustomContentForMap() override;
};

UCLASS()
class AOwnershipCompletenessTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	AOwnershipCompletenessTest();

private:
	virtual void PrepareTest() override;

	UPROPERTY(Transient)
	AOwnershipCompletenessController* Controller;

	UPROPERTY(Transient)
	AOwnershipCompletenessPawn* Pawn;

	UPROPERTY(Transient)
	AOwnershipCompletenessTestPawnActor* PawnActor;

	UPROPERTY(Transient)
	AOwnershipCompletenessTestControllerActor* ControllerActor;

	UPROPERTY(Transient)
	AOwnershipCompletenessTestFreeActor* FreeActor;
};
