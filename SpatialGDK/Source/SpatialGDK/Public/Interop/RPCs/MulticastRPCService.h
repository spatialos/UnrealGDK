// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "RPCStore.h"
#include "Schema/MulticastRPCs.h"
#include "Schema/RPCPayload.h"
#include "SpatialView/SubView.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMulticastRPCService, Log, All);

class USpatialLatencyTracer;
class USpatialNetDriver;
struct RPCRingBuffer;

namespace SpatialGDK
{
class SPATIALGDK_API MulticastRPCService
{
public:
	MulticastRPCService(const ExtractRPCDelegate InExtractRPCCallback, const FSubView& InSubView, FRPCStore& InRPCStore);

	void AdvanceView();
	void ProcessChanges();

private:
	// Process relevant view delta changes.
	void EntityAdded(FSpatialEntityId EntityId);
	void EntityRefresh(FSpatialEntityId EntityId);
	void ComponentUpdate(FSpatialEntityId EntityId, Worker_ComponentId ComponentId);
	void AuthorityGained(FSpatialEntityId EntityId, Worker_ComponentSetId ComponentSetId);
	void AuthorityLost(FSpatialEntityId EntityId, Worker_ComponentSetId ComponentSetId);

	// Maintain local state of multicast RPCs.
	void PopulateDataStore(FSpatialEntityId EntityId);
	void ApplyComponentUpdate(FSpatialEntityId EntityId, Schema_ComponentUpdate* Update);

	// Multicast system responses to state changes.
	void OnCheckoutMulticastRPCComponentOnEntity(FSpatialEntityId EntityId);
	void OnRemoveMulticastRPCComponentForEntity(FSpatialEntityId EntityId);
	void OnEndpointAuthorityGained(FSpatialEntityId EntityId, Worker_ComponentSetId ComponentSetId);
	void OnEndpointAuthorityLost(FSpatialEntityId EntityId, Worker_ComponentSetId ComponentSetId);

	// Calls ExtractRPCCallback for each RPC it extracts from a given component. If the callback returns false,
	// stops retrieving RPCs.
	void ExtractRPCs(FSpatialEntityId EntityId);

	ExtractRPCDelegate ExtractRPCCallback;
	const FSubView* SubView;

	FRPCStore* RPCStore;

	// Deserialized state store for multicast components.
	TMap<Worker_EntityId_Key, MulticastRPCs> MulticastDataStore;

	// This is local, not written into schema.
	TMap<Worker_EntityId_Key, uint64> LastSeenMulticastRPCIds;
};

} // namespace SpatialGDK
