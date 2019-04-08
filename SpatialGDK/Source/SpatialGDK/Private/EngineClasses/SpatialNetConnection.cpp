// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialNetConnection.h"

#include "Engine/World.h"
#include "TimerManager.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Gameframework/Pawn.h"
#include "Gameframework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialReceiver.h"
#include "Interop/SpatialSender.h"
#include "SpatialConstants.h"
#include "SpatialGDKSettings.h"

#include <WorkerSDK/improbable/c_schema.h>

DEFINE_LOG_CATEGORY(LogSpatialNetConnection);

USpatialNetConnection::USpatialNetConnection(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, PlayerControllerEntity(SpatialConstants::INVALID_ENTITY_ID)
	, CurrentPingID(0)
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

#if ENGINE_MINOR_VERSION <= 20
void USpatialNetConnection::LowLevelSend(void * Data, int32 CountBytes, int32 CountBits)
{
	//Intentionally does not call Super::
}
#else
void USpatialNetConnection::LowLevelSend(void* Data, int32 CountBits, FOutPacketTraits& Traits)
{
	//Intentionally does not call Super::
}
#endif

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

void USpatialNetConnection::UpdateLevelVisibility(const FName& PackageName, bool bIsVisible)
{
	UNetConnection::UpdateLevelVisibility(PackageName, bIsVisible);

	// We want to update our interest as fast as possible
	// So we send an Interest update immediately.
	UpdateActorInterest(Cast<AActor>(PlayerController));
	UpdateActorInterest(Cast<AActor>(PlayerController->GetPawn()));
}

void USpatialNetConnection::UpdateActorInterest(AActor* Actor)
{
	if (Actor == nullptr)
	{
		return;
	}

	USpatialSender* Sender = Cast<USpatialNetDriver>(Driver)->Sender;

	Sender->UpdateInterestComponent(Actor);
	for (const auto& Child : Actor->Children)
	{
		UpdateActorInterest(Child);
	}
}

void USpatialNetConnection::InitHeartbeat(FTimerManager* InTimerManager, Worker_EntityId InPlayerControllerEntity)
{
	checkf(PlayerControllerEntity == SpatialConstants::INVALID_ENTITY_ID || PlayerControllerEntity == InPlayerControllerEntity, TEXT("InitPing: PlayerControllerEntity already set: %lld. New entity: %lld"), PlayerControllerEntity, InPlayerControllerEntity);
	PlayerControllerEntity = InPlayerControllerEntity;
	TimerManager = TimerManager == nullptr ? InTimerManager : TimerManager;

	if (Driver->IsServer())
	{
		SetHeartbeatTimeoutTimer();

		// Set up heartbeat event callback
		TWeakObjectPtr<USpatialNetConnection> ConnectionPtr = this;
		Cast<USpatialNetDriver>(Driver)->Receiver->AddHeartbeatDelegate(PlayerControllerEntity, HeartbeatDelegate::CreateLambda([ConnectionPtr](const Worker_ComponentUpdateOp& Op)
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

		USpatialWorkerConnection* Connection = Cast<USpatialNetDriver>(Driver)->Connection;
		if (Connection->IsConnected())
		{
			Connection->SendComponentUpdate(PlayerControllerEntity, &ComponentUpdate);
		}
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

void USpatialNetConnection::InitPing(FTimerManager* InTimerManager, Worker_EntityId InPlayerControllerEntity)
{
	checkf(PlayerControllerEntity == SpatialConstants::INVALID_ENTITY_ID || PlayerControllerEntity == InPlayerControllerEntity, TEXT("InitPing: PlayerControllerEntity already set: %lld. New entity: %lld"), PlayerControllerEntity, InPlayerControllerEntity);
	PlayerControllerEntity = InPlayerControllerEntity;
	TimerManager = TimerManager == nullptr ? InTimerManager : TimerManager;

	TWeakObjectPtr<USpatialNetConnection> ConnectionPtr = this;

	if (Driver->IsServer())
	{
		Cast<USpatialNetDriver>(Driver)->Receiver->AddClientPongDelegate(PlayerControllerEntity, ClientPongDelegate::CreateLambda([ConnectionPtr](const Worker_ComponentUpdateOp& Op)
		{
			if (!ConnectionPtr.IsValid())
			{
				return;
			}

			float ReceivedTimestamp = ConnectionPtr->GetWorld()->GetTimeSeconds();

			Schema_Object* EventsObject = Schema_GetComponentUpdateEvents(Op.update.schema_type);
			uint32 EventCount = Schema_GetObjectCount(EventsObject, SpatialConstants::PING_EVENT_ID);

			if (EventCount > 0 && ConnectionPtr->PlayerController)
			{
				if (APlayerState* PlayerState = Cast<APlayerController>(ConnectionPtr->PlayerController)->PlayerState)
				{
					uint32 ID = Schema_GetUint32(EventsObject, SpatialConstants::PING_ID_OFFSET_ID);
					float TimeSent;
					if (ConnectionPtr->SentPingTimestamps.RemoveAndCopyValue(ID, TimeSent))
					{
						// Set player state ExactPing in msecs, ExactPing is not replicated
						PlayerState->ExactPing = (ReceivedTimestamp - TimeSent) * 1000.0f;

						// In native Unreal, Ping property on player state is replicated
						// (compressed for replication by dividing ExactPing by 4)
						PlayerState->Ping = static_cast<int32>(PlayerState->ExactPing * 0.25f);
					}
				}
			}
		}));

		TimerManager->SetTimer(PingTimer, [this]()
		{
			RemoveExpiredPings();
			SendPingOrPong(CurrentPingID++, SpatialConstants::SERVER_PING_COMPONENT_ID);
		}, GetDefault<USpatialGDKSettings>()->PingIntervalSeconds, true, 0.0f);
	}
	else
	{
		Cast<USpatialNetDriver>(Driver)->Receiver->AddServerPingDelegate(PlayerControllerEntity, ServerPingDelegate::CreateLambda([ConnectionPtr](const Worker_ComponentUpdateOp& Op)
		{
			if (ConnectionPtr.IsValid())
			{
				Schema_Object* EventsObject = Schema_GetComponentUpdateEvents(Op.update.schema_type);
				uint32 EventCount = Schema_GetObjectCount(EventsObject, SpatialConstants::PING_EVENT_ID);

				if (EventCount > 0)
				{
					uint32 ID = Schema_GetUint32(EventsObject, SpatialConstants::PING_ID_OFFSET_ID);
					ConnectionPtr->SendPingOrPong(ID, SpatialConstants::CLIENT_PONG_COMPONENT_ID);
				}
			}
		}));
	}
}

void USpatialNetConnection::DisablePing()
{
	if (TimerManager && PingTimer.IsValid())
	{
		TimerManager->ClearTimer(PingTimer);
	}
	SentPingTimestamps.Empty();

	PlayerControllerEntity = SpatialConstants::INVALID_ENTITY_ID;
}

void USpatialNetConnection::SendPingOrPong(uint32 PingId, Worker_ComponentId ComponentId)
{
	Worker_ComponentUpdate ComponentUpdate = {};

	ComponentUpdate.component_id = ComponentId;
	ComponentUpdate.schema_type = Schema_CreateComponentUpdate(ComponentId);
	Schema_Object* EventsObject = Schema_GetComponentUpdateEvents(ComponentUpdate.schema_type);
	Schema_AddObject(EventsObject, SpatialConstants::PING_EVENT_ID);
	Schema_AddUint32(EventsObject, SpatialConstants::PING_ID_OFFSET_ID, PingId);

	USpatialWorkerConnection* Connection = Cast<USpatialNetDriver>(Driver)->Connection;
	if (Connection->IsConnected())
	{
		Connection->SendComponentUpdate(PlayerControllerEntity, &ComponentUpdate);

		if (Driver->IsServer())
		{
			SentPingTimestamps.Add(PingId, GetWorld()->GetTimeSeconds());
		}
	}
}

void USpatialNetConnection::RemoveExpiredPings()
{
	float Timeout = GetDefault<USpatialGDKSettings>()->PingTimeoutSeconds;
	float TimeNow = GetWorld()->GetTimeSeconds();

	for (auto It = SentPingTimestamps.CreateIterator(); It; ++It)
	{
		if (TimeNow - It->Value > Timeout)
		{
			It.RemoveCurrent();
		}
	}
}
