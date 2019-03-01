// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialNetConnection.h"

#include "TimerManager.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/SpatialReceiver.h"
#include "SpatialConstants.h"
#include "SpatialGDKSettings.h"

#include <WorkerSDK/improbable/c_schema.h>

DEFINE_LOG_CATEGORY(LogSpatialNetConnection);

USpatialNetConnection::USpatialNetConnection(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, PlayerControllerEntity(SpatialConstants::INVALID_ENTITY_ID)
{
	InternalAck = 1;
}

void USpatialNetConnection::BeginDestroy()
{
	DisableHeartbeat();

	Super::BeginDestroy();
}

void USpatialNetConnection::InitBase(UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL, EConnectionState InState, int32 InMaxPacket /*= 0*/, int32 InPacketOverhead /*= 0*/)
{
	Super::InitBase(InDriver, InSocket, InURL, InState, InMaxPacket, InPacketOverhead);

	if (Cast<USpatialNetDriver>(InDriver)->PackageMap == nullptr)
	{
		// This should only happen if we're setting up the special "SpatialOS" connection.
		auto PackageMapClient = NewObject<USpatialPackageMapClient>(this);
		PackageMapClient->Initialize(this, InDriver->GuidCache);
		PackageMap = PackageMapClient;
		Cast<USpatialNetDriver>(InDriver)->PackageMap = PackageMapClient;
	}
	else
	{
		PackageMap = Cast<USpatialNetDriver>(InDriver)->PackageMap;
	}
}

void USpatialNetConnection::LowLevelSend(void * Data, int32 CountBytes, int32 CountBits)
{
	//Intentionally does not call Super::
}

bool USpatialNetConnection::ClientHasInitializedLevelFor(const AActor* TestActor) const
{
	check(Driver->IsServer());
	return true;
	//Intentionally does not call Super::
}

void USpatialNetConnection::Tick()
{
	// Since we're not receiving actual Unreal packets, Unreal may time out the connection. Timeouts are handled by SpatialOS, so we're setting these values here to keep Unreal happy.
	// Note that in the case of InternalAck (UnrealWorker) the engine does this (and more) in Super.
	if (!InternalAck)
	{
		LastReceiveTime = Driver->Time;
		LastReceiveRealtime = FPlatformTime::Seconds();
		LastGoodPacketRealtime = FPlatformTime::Seconds();
	}
	Super::Tick();
}

int32 USpatialNetConnection::IsNetReady(bool Saturate)
{
	// TODO: UNR-664 - Currently we do not report the number of bits sent when replicating, this means channel saturation cannot be checked properly.
	// This will always return true until we solve this.
	return true;
}

void USpatialNetConnection::InitHeartbeat(FTimerManager* InTimerManager, Worker_EntityId InPlayerControllerEntity)
{
	checkf(PlayerControllerEntity == SpatialConstants::INVALID_ENTITY_ID, TEXT("InitHeartbeat: PlayerControllerEntity already set: %lld. New entity: %lld"), PlayerControllerEntity, InPlayerControllerEntity);
	PlayerControllerEntity = InPlayerControllerEntity;
	TimerManager = InTimerManager;

	if (Driver->IsServer())
	{
		SetHeartbeatTimeoutTimer();

		// Set up heartbeat event callback
		TWeakObjectPtr<USpatialNetConnection> ConnectionPtr = this;
		Cast<USpatialNetDriver>(Driver)->Receiver->AddHeartbeatDelegate(PlayerControllerEntity, HeartbeatDelegate::CreateLambda([ConnectionPtr](Worker_ComponentUpdateOp& Op)
		{
			if (ConnectionPtr.IsValid())
			{
				Schema_Object* EventsObject = Schema_GetComponentUpdateEvents(Op.update.schema_type);
				uint32 EventCount = Schema_GetObjectCount(EventsObject, SpatialConstants::HEARTBEAT_EVENT_ID);
				if (EventCount > 0)
				{
					if (EventCount > 1)
					{
						UE_LOG(LogSpatialNetConnection, Log, TEXT("Received multiple heartbeat events in a single component update, entity %lld."), ConnectionPtr->PlayerControllerEntity);
					}

					ConnectionPtr->OnHeartbeat();
				}
			}
		}));
	}
	else
	{
		SetHeartbeatEventTimer();
	}
}

void USpatialNetConnection::SetHeartbeatTimeoutTimer()
{
	TimerManager->SetTimer(HeartbeatTimer, [this]()
	{
		// This client timed out. Disconnect it and trigger OnDisconnected logic.
		CleanUp();
	}, GetDefault<USpatialGDKSettings>()->HeartbeatTimeoutSeconds, false);
}

void USpatialNetConnection::SetHeartbeatEventTimer()
{
	TimerManager->SetTimer(HeartbeatTimer, [this]()
	{
		Worker_ComponentUpdate ComponentUpdate = {};

		ComponentUpdate.component_id = SpatialConstants::HEARTBEAT_COMPONENT_ID;
		ComponentUpdate.schema_type = Schema_CreateComponentUpdate(SpatialConstants::HEARTBEAT_COMPONENT_ID);
		Schema_Object* EventsObject = Schema_GetComponentUpdateEvents(ComponentUpdate.schema_type);
		Schema_AddObject(EventsObject, SpatialConstants::HEARTBEAT_EVENT_ID);

		Cast<USpatialNetDriver>(Driver)->Connection->SendComponentUpdate(PlayerControllerEntity, &ComponentUpdate);
	}, GetDefault<USpatialGDKSettings>()->HeartbeatIntervalSeconds, true, 0.0f);
}

void USpatialNetConnection::DisableHeartbeat()
{
	// Remove the heartbeat callback
	if (TimerManager != nullptr && HeartbeatTimer.IsValid())
	{
		TimerManager->ClearTimer(HeartbeatTimer);
	}
	PlayerControllerEntity = SpatialConstants::INVALID_ENTITY_ID;
}

void USpatialNetConnection::OnHeartbeat()
{
	SetHeartbeatTimeoutTimer();
}
