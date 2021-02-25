// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialSender.h"

#include "GameFramework/PlayerController.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/Connection/SpatialEventTracer.h"
#include "Interop/Connection/SpatialTraceEventBuilder.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialReceiver.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Schema/ServerWorker.h"
#include "SpatialConstants.h"
#include "Utils/RepLayoutUtils.h"
#include "Utils/SpatialDebugger.h"
#include "Utils/SpatialStatics.h"

using namespace SpatialGDK;

DECLARE_CYCLE_STAT(TEXT("Sender SendComponentUpdates"), STAT_SpatialSenderSendComponentUpdates, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Sender ResetOutgoingUpdate"), STAT_SpatialSenderResetOutgoingUpdate, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Sender QueueOutgoingUpdate"), STAT_SpatialSenderQueueOutgoingUpdate, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Sender UpdateInterestComponent"), STAT_SpatialSenderUpdateInterestComponent, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Sender SendRPC"), STAT_SpatialSenderSendRPC, STATGROUP_SpatialNet);

void USpatialSender::Init(USpatialNetDriver* InNetDriver, FTimerManager* InTimerManager, SpatialRPCService* InRPCService,
						  SpatialEventTracer* InEventTracer)
{
	Connection = InNetDriver->Connection;
	Receiver = InNetDriver->Receiver;
	PackageMap = InNetDriver->PackageMap;
	ClassInfoManager = InNetDriver->ClassInfoManager;
	TimerManager = InTimerManager;
	RPCService = InRPCService;
	EventTracer = InEventTracer;
}

void USpatialSender::SendCommandResponse(Worker_RequestId RequestId, Worker_CommandResponse& Response, const FSpatialGDKSpanId& CauseSpanId)
{
	FSpatialGDKSpanId SpanId;
	if (EventTracer != nullptr)
	{
		SpanId = EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateSendCommandResponse(RequestId, true),
										 /* Causes */ CauseSpanId.GetConstId(), /* NumCauses */ 1);
	}

	Connection->SendCommandResponse(RequestId, &Response, SpanId);
}

void USpatialSender::SendEmptyCommandResponse(Worker_ComponentId ComponentId, Schema_FieldId CommandIndex, Worker_RequestId RequestId,
											  const FSpatialGDKSpanId& CauseSpanId)
{
	Worker_CommandResponse Response = {};
	Response.component_id = ComponentId;
	Response.command_index = CommandIndex;
	Response.schema_type = Schema_CreateCommandResponse();

	FSpatialGDKSpanId SpanId;
	if (EventTracer != nullptr)
	{
		SpanId =
			EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateSendCommandResponse(RequestId, true), CauseSpanId.GetConstId(), 1);
	}

	Connection->SendCommandResponse(RequestId, &Response, SpanId);
}

void USpatialSender::SendCommandFailure(Worker_RequestId RequestId, const FString& Message, const FSpatialGDKSpanId& CauseSpanId)
{
	FSpatialGDKSpanId SpanId;
	if (EventTracer != nullptr)
	{
		SpanId =
			EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateSendCommandResponse(RequestId, false), CauseSpanId.GetConstId(), 1);
	}

	Connection->SendCommandFailure(RequestId, Message, SpanId);
}
