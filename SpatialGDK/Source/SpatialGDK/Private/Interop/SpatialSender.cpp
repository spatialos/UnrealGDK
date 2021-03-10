// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialSender.h"

#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Net/NetworkProfiler.h"
#include "Runtime/Launch/Resources/Version.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialLoadBalanceEnforcer.h"
#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialNetDriverDebugContext.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/Connection/SpatialEventTracer.h"
#include "Interop/Connection/SpatialTraceEventBuilder.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/GlobalStateManager.h"
#include "Interop/SpatialReceiver.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Schema/AuthorityIntent.h"
#include "Schema/CrossServerEndpoint.h"
#include "Schema/Interest.h"
#include "Schema/RPCPayload.h"
#include "Schema/ServerWorker.h"
#include "Schema/StandardLibrary.h"
#include "Schema/Tombstone.h"
#include "SpatialConstants.h"
#include "Utils/ComponentFactory.h"
#include "Utils/EntityFactory.h"
#include "Utils/InterestFactory.h"
#include "Utils/RepLayoutUtils.h"
#include "Utils/SpatialActorUtils.h"
#include "Utils/SpatialDebugger.h"
#include "Utils/SpatialLatencyTracer.h"
#include "Utils/SpatialMetrics.h"
#include "Utils/SpatialStatics.h"

DEFINE_LOG_CATEGORY(LogSpatialSender);

using namespace SpatialGDK;

DECLARE_CYCLE_STAT(TEXT("Sender SendComponentUpdates"), STAT_SpatialSenderSendComponentUpdates, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Sender ResetOutgoingUpdate"), STAT_SpatialSenderResetOutgoingUpdate, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Sender QueueOutgoingUpdate"), STAT_SpatialSenderQueueOutgoingUpdate, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Sender UpdateInterestComponent"), STAT_SpatialSenderUpdateInterestComponent, STATGROUP_SpatialNet);

void USpatialSender::Init(USpatialNetDriver* InNetDriver, FTimerManager* InTimerManager, SpatialEventTracer* InEventTracer)
{
	NetDriver = InNetDriver;
	StaticComponentView = InNetDriver->StaticComponentView;
	Connection = InNetDriver->Connection;
	PackageMap = InNetDriver->PackageMap;
	ClassInfoManager = InNetDriver->ClassInfoManager;
	TimerManager = InTimerManager;
	EventTracer = InEventTracer;
}

bool USpatialSender::ValidateOrExit_IsSupportedClass(const FString& PathName)
{
	// Level blueprint classes could have a PIE prefix, this will remove it.
	FString RemappedPathName = PathName;
#if ENGINE_MINOR_VERSION >= 26
	GEngine->NetworkRemapPath(NetDriver->GetSpatialOSNetConnection(), RemappedPathName, false /*bIsReading*/);
#else
	GEngine->NetworkRemapPath(NetDriver, RemappedPathName, false /*bIsReading*/);
#endif

	return ClassInfoManager->ValidateOrExit_IsSupportedClass(RemappedPathName);
}

void USpatialSender::UpdatePartitionEntityInterestAndPosition()
{
	check(Connection != nullptr);
	check(NetDriver != nullptr);
	check(NetDriver->VirtualWorkerTranslator != nullptr
		  && NetDriver->VirtualWorkerTranslator->GetClaimedPartitionId() != SpatialConstants::INVALID_ENTITY_ID);
	check(NetDriver->LoadBalanceStrategy != nullptr && NetDriver->LoadBalanceStrategy->IsReady());

	Worker_PartitionId PartitionId = NetDriver->VirtualWorkerTranslator->GetClaimedPartitionId();
	VirtualWorkerId VirtualId = NetDriver->VirtualWorkerTranslator->GetLocalVirtualWorkerId();

	// Update the interest. If it's ready and not null, also adds interest according to the load balancing strategy.
	FWorkerComponentUpdate InterestUpdate =
		NetDriver->InterestFactory
			->CreatePartitionInterest(NetDriver->LoadBalanceStrategy, VirtualId, NetDriver->DebugCtx != nullptr /*bDebug*/)
			.CreateInterestUpdate();

	Connection->SendComponentUpdate(PartitionId, &InterestUpdate);

	// Also update the position of the partition entity to the center of the load balancing region.
	FWorkerComponentUpdate Update =
		Position::CreatePositionUpdate(Coordinates::FromFVector(NetDriver->LoadBalanceStrategy->GetWorkerEntityPosition()));
	Connection->SendComponentUpdate(PartitionId, &Update);
}

void USpatialSender::SendAuthorityIntentUpdate(const AActor& Actor, VirtualWorkerId NewAuthoritativeVirtualWorkerId) const
{
	const Worker_EntityId EntityId = PackageMap->GetEntityIdFromObject(&Actor);
	check(EntityId != SpatialConstants::INVALID_ENTITY_ID);

	AuthorityIntent* AuthorityIntentComponent = StaticComponentView->GetComponentData<AuthorityIntent>(EntityId);
	check(AuthorityIntentComponent != nullptr);
	checkf(AuthorityIntentComponent->VirtualWorkerId != NewAuthoritativeVirtualWorkerId,
		   TEXT("Attempted to update AuthorityIntent twice to the same value. Actor: %s. Entity ID: %lld. Virtual worker: '%d'"),
		   *GetNameSafe(&Actor), EntityId, NewAuthoritativeVirtualWorkerId);

	AuthorityIntentComponent->VirtualWorkerId = NewAuthoritativeVirtualWorkerId;
	UE_LOG(LogSpatialSender, Log,
		   TEXT("(%s) Sending AuthorityIntent update for entity id %d. Virtual worker '%d' should become authoritative over %s"),
		   *NetDriver->Connection->GetWorkerId(), EntityId, NewAuthoritativeVirtualWorkerId, *GetNameSafe(&Actor));

	FWorkerComponentUpdate Update = AuthorityIntentComponent->CreateAuthorityIntentUpdate();

	FSpatialGDKSpanId SpanId;
	if (EventTracer != nullptr)
	{
		SpanId = EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateAuthorityIntentUpdate(NewAuthoritativeVirtualWorkerId, &Actor));
	}

	Connection->SendComponentUpdate(EntityId, &Update, SpanId);

	// Notify the enforcer directly on the worker that sends the component update, as the update will short circuit.
	// This should always happen with USLB.
	NetDriver->LoadBalanceEnforcer->ShortCircuitMaybeRefreshAuthorityDelegation(EntityId);

	if (NetDriver->SpatialDebuggerSystem.IsValid())
	{
		NetDriver->SpatialDebuggerSystem->ActorAuthorityIntentChanged(EntityId, NewAuthoritativeVirtualWorkerId);
	}
}
