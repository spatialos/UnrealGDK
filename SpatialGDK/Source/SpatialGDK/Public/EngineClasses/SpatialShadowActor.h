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
	USpatialShadowActor() = default;

	void Init(const Worker_EntityId InEntityId, AActor* InActor);

	void CheckUnauthorisedDataChanges(const Worker_EntityId InEntityId, const AActor* InActor);

	void Update(const Worker_EntityId InEntityId, AActor* InActor);

protected:
	Worker_EntityId EntityId;
	UPROPERTY()
	AActor* Actor;
	FString ReplicatedPropertyHash;
	FString CreateHash(const AActor* InActor);
};
