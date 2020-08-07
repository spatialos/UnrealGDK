// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "ReplicationGraph.h"

#include "SpatialReplicationGraph.generated.h"

class UActorChannel;
class UObject;

UCLASS(Transient)
class SPATIALGDK_API USpatialReplicationGraph : public UReplicationGraph
{
	GENERATED_BODY()

public:
	//~ Begin UReplicationGraph Interface
	virtual UActorChannel* GetOrCreateSpatialActorChannel(UObject* TargetObject) override;
	//~ End UReplicationGraph Interface
};
