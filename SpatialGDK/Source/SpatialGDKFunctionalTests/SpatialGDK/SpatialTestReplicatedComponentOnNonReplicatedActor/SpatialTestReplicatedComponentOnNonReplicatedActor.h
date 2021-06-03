// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialFunctionalTest.h"
#include "TestMaps/GeneratedTestMap.h"

#include "SpatialTestReplicatedComponentOnNonReplicatedActor.generated.h"

UCLASS()
class UReplicatedComponentOnNonReplicatedActorGeneratedMap : public UGeneratedTestMap
{
	GENERATED_BODY()

public:
	UReplicatedComponentOnNonReplicatedActorGeneratedMap();

	virtual void CreateCustomContentForMap() override;
};

UCLASS()
class UTestReplicatedComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UTestReplicatedComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	int Dummy;
};

UCLASS()
class ATestNonReplicatedActor : public AActor
{
	GENERATED_BODY()
public:
	ATestNonReplicatedActor();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostInitializeComponents() override;

	UPROPERTY(Replicated, VisibleAnywhere, Category = Test)
	UTestReplicatedComponent* ReplicatedDefaultReference;

	UPROPERTY(VisibleAnywhere, Category = Test)
	UTestReplicatedComponent* NonReplicatedDefaultReference;

	UPROPERTY(Replicated, Transient, VisibleAnywhere, Category = Test)
	UTestReplicatedComponent* ReplicatedRuntimeReference;

	UPROPERTY(Transient, VisibleAnywhere, Category = Test)
	UTestReplicatedComponent* NonReplicatedRuntimeReference;
};

USTRUCT()
struct FTestReplicatedComponentOnNonReplicatedActorInfo
{
	GENERATED_BODY()

	UPROPERTY()
	UTestReplicatedComponent* Component;

	UPROPERTY()
	FVector OriginActorLocation;
};

UCLASS()
class ATestReplicatedActor : public AActor
{
	GENERATED_BODY()
public:
	ATestReplicatedActor();

	virtual void OnAuthorityGained() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	TArray<UTestReplicatedComponent*> ActorInfos;

	UPROPERTY(Replicated)
	FWorkerDefinition OriginWorkerDefinition;
};

UCLASS()
class ASpatialTestReplicatedComponentOnNonReplicatedActor : public ASpatialFunctionalTest
{
	GENERATED_BODY()

	virtual void PrepareTest() override;

	UPROPERTY(Transient)
	TArray<ATestNonReplicatedActor*> TestActors;

	UPROPERTY(Transient)
	TArray<ATestReplicatedActor*> ReplicatedActors;

	UPROPERTY(Transient)
	ATestReplicatedActor* LocallyAuthReplicatedActor;

	float counter = 0;
};
