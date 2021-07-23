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
	void Init(AActor& InActor);
	void Update();

	void CheckUnauthorisedDataChanges();

protected:
	UPROPERTY()
	AActor* Actor;
	int32 NumReplicatedProperties;
	TArray<uint32> ReplicatedPropertyHashes;

	void CreateHash();
};
