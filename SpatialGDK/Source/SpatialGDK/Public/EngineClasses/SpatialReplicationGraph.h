// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "ReplicationGraph.h"

#include "SpatialCommonTypes.h"

#include "SpatialReplicationGraph.generated.h"

class UActorChannel;
class UObject;

class FSpatialReplicationGraphLoadBalancingHandler;

UCLASS(Transient)
class SPATIALGDK_API USpatialReplicationGraph : public UReplicationGraph
{
	GENERATED_BODY()
	friend FSpatialReplicationGraphLoadBalancingHandler;

public:

	~USpatialReplicationGraph();

	virtual void OnOwnerUpdated(AActor* Actor, AActor* OldOwner);

protected:

	//~ Begin UReplicationGraph Interface
	virtual UActorChannel* GetOrCreateSpatialActorChannel(UObject* TargetObject) override;

	virtual void PreReplicateActors(UNetReplicationGraphConnection& ConnectionManager) override;

	virtual void PostReplicateActors(UNetReplicationGraphConnection& ConnectionManager) override;
	//~ End UReplicationGraph Interface

	FGlobalActorReplicationInfoMap& GetGlobalActorReplicationInfoMap() { return GlobalActorReplicationInfoMap; }

	FSpatialReplicationGraphLoadBalancingHandler* LoadBalancingHandler = nullptr;
};
