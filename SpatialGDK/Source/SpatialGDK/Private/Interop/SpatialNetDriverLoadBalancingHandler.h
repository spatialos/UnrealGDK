// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Utils/SpatialLoadBalancingHandler.h"
#include "Engine/NetworkObjectList.h"

// Specialization of the load balancing handler for the SpatialNetDriver.
class FSpatialNetDriverLoadBalancingHandler : public TSpatialLoadBalancingHandler<FSpatialNetDriverLoadBalancingHandler>
{
	friend TSpatialLoadBalancingHandler<FSpatialNetDriverLoadBalancingHandler>;

public:

	FSpatialNetDriverLoadBalancingHandler(USpatialNetDriver* InNetDriver, TArray<FNetworkObjectInfo*>& InOutNetworkObjects);

	void HandleLoadBalancing();

protected:

	// Adaptor over an array of FNetworkObjectInfo to have a range-for compatible iterator over AActor.
	struct FNetworkObjectsArrayAdaptor
	{
		struct Iterator
		{
			Iterator(TArray<FNetworkObjectInfo*>::RangedForIteratorType Iterator)
				:IteratorImpl(Iterator)
			{}

			AActor* operator*() const { return (*IteratorImpl)->Actor; }
			void operator ++() { ++IteratorImpl; }
			bool operator != (Iterator const& iRHS) const { return IteratorImpl != iRHS.IteratorImpl; }

			TArray<FNetworkObjectInfo*>::RangedForIteratorType IteratorImpl;
		};

		FNetworkObjectsArrayAdaptor(TArray<FNetworkObjectInfo*>& InNetworkObjects)
			: NetworkObjects(InNetworkObjects)
		{}

		Iterator begin() { return Iterator(NetworkObjects.begin()); }
		Iterator end() { return Iterator(NetworkObjects.end()); }

		TArray<FNetworkObjectInfo*>& NetworkObjects;
	};

	FNetworkObjectsArrayAdaptor GetActorsBeingReplicated();

	void RemoveAdditionalActor(AActor* Actor);

	void AddActorToReplicate(AActor* Actor);

	TArray<AActor*>& GetDependentActors(AActor* Actor);

	TSet<FNetworkObjectInfo*> AdditionalActorsToReplicate;

	TArray<FNetworkObjectInfo*>& NetworkObjects;
};
