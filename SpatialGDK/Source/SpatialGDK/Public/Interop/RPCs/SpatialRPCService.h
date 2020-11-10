// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "ClientServerRPCService.h"
#include "Interop/Connection/SpatialEventTracer.h"
#include "Interop/SpatialClassInfoManager.h"
#include "MulticastRPCService.h"
#include "RPCStore.h"
#include "Schema/ClientEndpoint.h"
#include "Schema/RPCPayload.h"
#include "SpatialView/SubView.h"
#include "Utils/RPCContainer.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialRPCService, Log, All);

class USpatialLatencyTracer;
class USpatialStaticComponentView;
class USpatialNetDriver;
struct RPCRingBuffer;

namespace SpatialGDK
{
class SPATIALGDK_API SpatialRPCService
{
public:
	explicit SpatialRPCService(const FSubView& InActorAuthSubView, const FSubView& InActorNonAuthSubView,
							   USpatialLatencyTracer* InSpatialLatencyTracer, SpatialEventTracer* InEventTracer,
							   USpatialNetDriver* InNetDriver);

	void AdvanceView();
	void ProcessChanges(const float NetDriverTime);

	void ProcessIncomingRPCs();

	void ProcessOrQueueIncomingRPC(const FUnrealObjectRef& InTargetObjectRef, RPCPayload InPayload);

	EPushRPCResult PushRPC(Worker_EntityId EntityId, ERPCType Type, RPCPayload Payload, bool bCreatedEntity, UObject* Target = nullptr,
						   UFunction* Function = nullptr);
	void PushOverflowedRPCs();

	struct UpdateToSend
	{
		Worker_EntityId EntityId;
		FWorkerComponentUpdate Update;
		TOptional<Trace_SpanId> SpanId;
	};
	TArray<UpdateToSend> GetRPCsAndAcksToSend();
	TArray<FWorkerComponentData> GetRPCComponentsOnEntityCreation(Worker_EntityId EntityId);

	void ClearPendingRPCs(Worker_EntityId EntityId);

private:
	EPushRPCResult PushRPCInternal(Worker_EntityId EntityId, ERPCType Type, const PendingRPCPayload& Payload, bool bCreatedEntity);

	FRPCErrorInfo ApplyRPC(const FPendingRPCParams& Params);
	// Note: It's like applying an RPC, but more secretive
	FRPCErrorInfo ApplyRPCInternal(UObject* TargetObject, UFunction* Function, const FPendingRPCParams& PendingRPCParams);

	USpatialNetDriver* NetDriver;
	USpatialLatencyTracer* SpatialLatencyTracer;
	SpatialEventTracer* EventTracer;
	FRPCContainer IncomingRPCs{ ERPCQueueType::Receive };

	FRPCStore RPCStore;
	ClientServerRPCService ClientServerRPCs;
	MulticastRPCService MulticastRPCs;

	// Keep around one of the passed subviews here in order to read the main view.
	const FSubView* AuthSubView;

	float LastProcessingTime;

#if TRACE_LIB_ACTIVE
	void ProcessResultToLatencyTrace(const EPushRPCResult Result, const TraceKey Trace);
	TMap<EntityComponentId, TraceKey> PendingTraces;
#endif
};

} // namespace SpatialGDK
