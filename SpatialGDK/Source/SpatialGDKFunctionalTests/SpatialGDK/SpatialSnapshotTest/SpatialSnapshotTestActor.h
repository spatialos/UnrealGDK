// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Actor.h"
#include "SpatialSnapshotTestActor.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialSnapshotTestActor : public AActor
{
	GENERATED_BODY()
public:
	ASpatialSnapshotTestActor();

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
