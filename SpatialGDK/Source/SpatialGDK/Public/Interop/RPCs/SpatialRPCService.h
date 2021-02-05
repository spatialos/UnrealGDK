// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "ClientServerRPCService.h"
#include "EngineClasses/SpatialNetBitWriter.h"
#include "Interop/Connection/SpatialEventTracer.h"
#include "Interop/Connection/SpatialGDKSpanId.h"
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

	void Flush();

	void ProcessIncomingRPCs();
	void ProcessOutgoingRPCs();

	void ProcessOrQueueIncomingRPC(const FUnrealObjectRef& InTargetObjectRef, RPCPayload InPayload,
								   TOptional<uint64> RPCIdForLinearEventTrace);

	EPushRPCResult PushRPC(Worker_EntityId EntityId, ERPCType Type, RPCPayload Payload, bool bCreatedEntity, UObject* Target = nullptr,
						   UFunction* Function = nullptr, const FSpatialGDKSpanId& SpanId = {});
	void PushOverflowedRPCs();

	struct UpdateToSend
	{
		Worker_EntityId EntityId;
		FWorkerComponentUpdate Update;
		FSpatialGDKSpanId SpanId;
	};
	TArray<UpdateToSend> GetRPCsAndAcksToSend();
	TArray<FWorkerComponentData> GetRPCComponentsOnEntityCreation(Worker_EntityId EntityId);

	void ClearPendingRPCs(Worker_EntityId EntityId);

	RPCPayload CreateRPCPayloadFromParams(UObject* TargetObject, const FUnrealObjectRef& TargetObjectRef, UFunction* Function,
										  ERPCType Type, void* Params) const;
	void ProcessOrQueueOutgoingRPC(const FUnrealObjectRef& InTargetObjectRef, RPCPayload&& InPayload);

private:
	EPushRPCResult PushRPCInternal(Worker_EntityId EntityId, ERPCType Type, PendingRPCPayload Payload, bool bCreatedEntity);

	FRPCErrorInfo ApplyRPC(const FPendingRPCParams& Params);
	// Note: It's like applying an RPC, but more secretive
	FRPCErrorInfo ApplyRPCInternal(UObject* TargetObject, UFunction* Function, const FPendingRPCParams& PendingRPCParams);
	FRPCErrorInfo SendRPC(const FPendingRPCParams& Params);
	bool SendRingBufferedRPC(UObject* TargetObject, UFunction* Function, const RPCPayload& Payload, USpatialActorChannel* Channel,
							 const FUnrealObjectRef& TargetObjectRef, const FSpatialGDKSpanId& SpanId);
#if !UE_BUILD_SHIPPING
	void TrackRPC(AActor* Actor, UFunction* Function, const RPCPayload& Payload, ERPCType RPCType);
#endif
	FSpatialNetBitWriter PackRPCDataToSpatialNetBitWriter(UFunction* Function, void* Parameters) const;

	USpatialNetDriver* NetDriver;
	USpatialLatencyTracer* SpatialLatencyTracer;
	SpatialEventTracer* EventTracer;

	FRPCContainer IncomingRPCs{ ERPCQueueType::Receive };
	FRPCContainer OutgoingRPCs{ ERPCQueueType::Send };

	FRPCStore RPCStore;
	ClientServerRPCService ClientServerRPCs;
	MulticastRPCService MulticastRPCs;

	// Keep around one of the passed subviews here in order to read the main view.
	const FSubView* AuthSubView;

	float LastIncomingProcessingTime;
	float LastOutgoingProcessingTime;

#if TRACE_LIB_ACTIVE
	void ProcessResultToLatencyTrace(const EPushRPCResult Result, const TraceKey Trace);
	TMap<EntityComponentId, TraceKey> PendingTraces;
#endif
};

} // namespace SpatialGDK
