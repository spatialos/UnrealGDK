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

	void Init(AActor& InActor);
	void Update(AActor& InActor);

	void CheckUnauthorisedDataChanges();

protected:
	UPROPERTY()
	AActor* Actor;

	int32 NumReplicatedProperties;
	TArray<uint32> ReplicatedPropertyHashes;

	void CreateHash(const AActor& InActor);
};
