// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "SpatialConstants.h"
#include "SpatialShadowActor.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialShadowActor, Log, All);

UCLASS(Transient)
class SPATIALGDK_API USpatialShadowActor : public UObject
{
	GENERATED_BODY()

public:
	USpatialShadowActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	void Init(const Worker_EntityId InEntityId, AActor* InActor);

	void CheckUnauthorisedDataChanges(const Worker_EntityId InEntityId, AActor* InActor);

	void Update(const Worker_EntityId InEntityId, AActor* InActor);

private:
	Worker_EntityId EntityId;
	AActor* Actor;
	FString ReplicatedPropertyHash;
	FString CreateHash(AActor* InActor);

	// Store a list of actors class names that currently violate the non-auth changes so that the user is not spammed.
	// TODO: link PR to investigate these cases
	UPROPERTY()
	TArray<FString> SuppressedActors;
};
