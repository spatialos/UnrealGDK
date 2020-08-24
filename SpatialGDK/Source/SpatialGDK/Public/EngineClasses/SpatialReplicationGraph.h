// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "ReplicationGraph.h"

#include "SpatialCommonTypes.h"
#include "Utils/SpatialLoadBalancingHandler.h"

#include "SpatialReplicationGraph.generated.h"

class UActorChannel;
class UObject;

UCLASS(Transient)
class SPATIALGDK_API USpatialReplicationGraph : public UReplicationGraph
{
	GENERATED_BODY()

public:
	virtual void OnOwnerUpdated(AActor* Actor, AActor* OldOwner);

	FGlobalActorReplicationInfoMap& GetGlobalActorReplicationInfoMap() { return GlobalActorReplicationInfoMap; }

protected:
	//~ Begin UReplicationGraph Interface
	virtual UActorChannel* GetOrCreateSpatialActorChannel(UObject* TargetObject) override;

	virtual void PreReplicateActors(UNetReplicationGraphConnection* ConnectionManager) override;

	virtual void PostReplicateActors(UNetReplicationGraphConnection* ConnectionManager) override;
	//~ End UReplicationGraph Interface

	TUniquePtr<FSpatialLoadBalancingHandler> LoadBalancingHandler;
};
