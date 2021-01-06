// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "Interop/SpatialClassInfoManager.h"
#include "RPCStore.h"
#include "Schema/CrossServerEndpoint.h"
#include "Schema/RPCPayload.h"
#include "SpatialView/SubView.h"
#include "Utils/CrossServerUtils.h"
#include "Utils/RPCRingBuffer.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCrossServerRPCService, Log, All);

class USpatialLatencyTracer;
class USpatialStaticComponentView;
class USpatialNetDriver;
struct RPCRingBuffer;

namespace SpatialGDK
{
struct FRPCStore;

struct CrossServerEndpoints
{
	// Locally authoritative state
	CrossServer::RPCSchedule ReceiverSchedule;
	CrossServer::WriterState SenderState;
	CrossServer::ReaderState ReceiverACKState;

	// Observed state
	TOptional<CrossServerEndpointACK> ACKedRPCs;
	TOptional<CrossServerEndpoint> ReceivedRPCs;
};

class SPATIALGDK_API CrossServerRPCService
{
public:
	CrossServerRPCService(const ActorCanExtractRPCDelegate InCanExtractRPCDelegate, const ExtractRPCDelegate InExtractRPCCallback,
						  const FSubView& InSubView, FRPCStore& InRPCStore);

	void AdvanceView();
	void ProcessChanges();

	EPushRPCResult PushCrossServerRPC(Worker_EntityId EntityId, const RPCSender& Sender, const PendingRPCPayload& Payload,
									  bool bCreatedEntity);

	void WriteCrossServerACKFor(Worker_EntityId Receiver, const RPCSender& Sender);
	void FlushPendingClearedFields(TPair<EntityComponentId, PendingUpdate>& UpdateToSend);

private:
	// Process relevant view delta changes.
	void EntityAdded(const Worker_EntityId EntityId);
	void ComponentUpdate(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId, Schema_ComponentUpdate* Update);
	void ProcessComponentChange(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId);

	// Maintain local state of client server RPCs.
	void PopulateDataStore(Worker_EntityId EntityId);

	// Client server RPC system responses to state changes.
	void OnEndpointAuthorityGained(Worker_EntityId EntityId, Worker_ComponentId ComponentId);

	// The component with the given component ID was updated, and so there is an RPC to be handled.
	void HandleRPC(const Worker_EntityId EntityId, const CrossServerEndpoint&);
	//// Calls ExtractRPCCallback for each RPC it extracts from a given component. If the callback returns false,
	//// stops retrieving RPCs.
	void ExtractCrossServerRPCs(Worker_EntityId EntityId, const CrossServerEndpoint&);
	void UpdateSentRPCsACKs(Worker_EntityId, const CrossServerEndpointACK&);
	void CleanupACKsFor(Worker_EntityId EndpointId, const CrossServerEndpoint&);
	//
	//// Helpers
	static bool IsCrossServerEndpoint(Worker_ComponentId ComponentId);

	ActorCanExtractRPCDelegate CanExtractRPCDelegate;
	ExtractRPCDelegate ExtractRPCCallback;
	const FSubView* SubView;

	FRPCStore* RPCStore;

	// Deserialized state store for client/server RPC components.
	TMap<Worker_EntityId_Key, CrossServerEndpoints> CrossServerDataStore;
};

} // namespace SpatialGDK
