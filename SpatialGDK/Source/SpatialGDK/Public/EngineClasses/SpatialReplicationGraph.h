// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Utils/SpatialLoadBalancingHandler.h"

#include "ReplicationGraph.h"

#include "SpatialReplicationGraph.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialReplicationGraph, Log, All);

class UActorChannel;
class UObject;

UCLASS(Transient)
class SPATIALGDK_API USpatialReplicationGraph : public UReplicationGraph
{
	GENERATED_BODY()

public:
	virtual void InitForNetDriver(UNetDriver*) override;
	virtual void OnOwnerUpdated(AActor* Actor, AActor* OldOwner);

	FGlobalActorReplicationInfoMap& GetGlobalActorReplicationInfoMap() { return GlobalActorReplicationInfoMap; }

	TArray<AActor*> GatherClientInterestedActors(UNetConnection* NetConnection);

protected:
	//~ Begin UReplicationGraph Interface
	virtual UActorChannel* GetOrCreateSpatialActorChannel(UObject* TargetObject) override;

	virtual void PreReplicateActors(UNetReplicationGraphConnection* ConnectionManager) override;

	virtual void PostReplicateActors(UNetReplicationGraphConnection* ConnectionManager) override;
	//~ End UReplicationGraph Interface

	void SetUseNarrowPhaseNCDInterestCulling(bool bToggle) { bUseNarrowPhaseNCDInterestCulling = bToggle; }

	TUniquePtr<FSpatialLoadBalancingHandler> LoadBalancingHandler;

private:
	TArray<AActor*> ExtractClientInterestActorsFromGather(UNetReplicationGraphConnection* ConnectionManager,
														  FGatheredReplicationActorLists& GatheredReplicationListsForConnection,
														  FNetViewerArray& Viewers);
	void GatherDependentActors(FPerConnectionActorInfoMap& ConnectionActorInfoMap, FGlobalActorReplicationInfo& GlobalActorInfo,
							   TArray<AActor*>& ClientInterestedActors);

	bool bUseNarrowPhaseNCDInterestCulling = true;
};
