// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialReceiver.h"

#include "Engine/Engine.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "EngineClasses/SpatialVirtualWorkerTranslationManager.h"
#include "Interop/Connection/SpatialEventTracer.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialPlayerSpawner.h"
#include "Interop/SpatialSender.h"
#include "SpatialConstants.h"
#include "Utils/ErrorCodeRemapping.h"
#include "Utils/RepLayoutUtils.h"
#include "Utils/SpatialDebugger.h"

DEFINE_LOG_CATEGORY(LogSpatialReceiver);

DECLARE_CYCLE_STAT(TEXT("Receiver LeaveCritSection"), STAT_ReceiverLeaveCritSection, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver RemoveEntity"), STAT_ReceiverRemoveEntity, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver AddComponent"), STAT_ReceiverAddComponent, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver ComponentUpdate"), STAT_ReceiverComponentUpdate, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver ApplyData"), STAT_ReceiverApplyData, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver ApplyHandover"), STAT_ReceiverApplyHandover, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver HandleRPC"), STAT_ReceiverHandleRPC, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver CommandRequest"), STAT_ReceiverCommandRequest, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver CommandResponse"), STAT_ReceiverCommandResponse, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver AuthorityChange"), STAT_ReceiverAuthChange, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver ReserveEntityIds"), STAT_ReceiverReserveEntityIds, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver CreateEntityResponse"), STAT_ReceiverCreateEntityResponse, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver EntityQueryResponse"), STAT_ReceiverEntityQueryResponse, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver SystemEntityCommandResponse"), STAT_ReceiverSystemEntityCommandResponse, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver FlushRemoveComponents"), STAT_ReceiverFlushRemoveComponents, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver ReceiveActor"), STAT_ReceiverReceiveActor, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver RemoveActor"), STAT_ReceiverRemoveActor, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver ApplyRPC"), STAT_ReceiverApplyRPC, STATGROUP_SpatialNet);
using namespace SpatialGDK;

DEFINE_LOG_CATEGORY_CLASS(CreateEntityHandler, LogCreateEntityHandler);

void USpatialReceiver::Init(USpatialNetDriver* InNetDriver, SpatialEventTracer* InEventTracer)
{
	NetDriver = InNetDriver;
	Sender = InNetDriver->Sender;
	PackageMap = InNetDriver->PackageMap;
	EventTracer = InEventTracer;
}

void USpatialReceiver::AddPendingReliableRPC(Worker_RequestId RequestId, TSharedRef<FReliableRPCForRetry> ReliableRPC)
{
	PendingReliableRPCs.Add(RequestId, ReliableRPC);
}

void USpatialReceiver::OnDisconnect(uint8 StatusCode, const FString& Reason)
{
	if (GEngine != nullptr)
	{
		GEngine->BroadcastNetworkFailure(NetDriver->GetWorld(), NetDriver, ENetworkFailure::FromDisconnectOpStatusCode(StatusCode), Reason);
	}
}
