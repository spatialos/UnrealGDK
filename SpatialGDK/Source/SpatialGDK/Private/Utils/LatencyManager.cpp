// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/LatencyManager.h"

#include "Engine/World.h"
#include "TimerManager.h"

#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialReceiver.h"
#include "Interop/SpatialSender.h"
#include "SpatialConstants.h"
#include "SpatialGDKSettings.h"

ULatencyManager::ULatencyManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, PlayerControllerEntity(SpatialConstants::INVALID_ENTITY_ID)
	, CurrentPingID(0)
{
}

void ULatencyManager::Init(USpatialNetDriver* InDriver, USpatialNetConnection* InConnection)
{
	Driver = InDriver;
	NetConnection = InConnection;
}

void ULatencyManager::Enable(FTimerManager* InTimerManager, Worker_EntityId InPlayerControllerEntity)
{
	checkf(PlayerControllerEntity == SpatialConstants::INVALID_ENTITY_ID, TEXT("InitPing: PlayerControllerEntity already set: %lld. New entity: %lld"), PlayerControllerEntity, InPlayerControllerEntity);
	PlayerControllerEntity = InPlayerControllerEntity;
	TimerManager = InTimerManager;

	TWeakObjectPtr<USpatialNetConnection> ConnectionPtr = NetConnection;

	if (Driver->IsServer())
	{
		InitServerPing();
	}
	else
	{
		InitClientPong();
	}
}

void ULatencyManager::InitServerPing()
{
	TWeakObjectPtr<USpatialNetConnection> ConnectionPtr = NetConnection;

	Cast<USpatialNetDriver>(Driver)->Receiver->AddClientPongDelegate(PlayerControllerEntity, ClientPongDelegate::CreateLambda([ConnectionPtr, this](const Worker_ComponentUpdateOp& Op)
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
				if (SentPingTimestamps.RemoveAndCopyValue(ID, TimeSent))
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

	TimerManager->SetTimer(LatencyTimer, [this]()
	{
		RemoveExpiredPings();
		SendPingOrPong(CurrentPingID++, SpatialConstants::SERVER_PING_COMPONENT_ID);
	}, GetDefault<USpatialGDKSettings>()->PingIntervalSeconds, true, 0.0f);
}

void ULatencyManager::InitClientPong()
{
	TWeakObjectPtr<USpatialNetConnection> ConnectionPtr = NetConnection;

	Cast<USpatialNetDriver>(Driver)->Receiver->AddServerPingDelegate(PlayerControllerEntity, ServerPingDelegate::CreateLambda([ConnectionPtr, this](const Worker_ComponentUpdateOp& Op)
	{
		if (ConnectionPtr.IsValid())
		{
			Schema_Object* EventsObject = Schema_GetComponentUpdateEvents(Op.update.schema_type);
			uint32 EventCount = Schema_GetObjectCount(EventsObject, SpatialConstants::PING_EVENT_ID);

			if (EventCount > 0)
			{
				uint32 ID = Schema_GetUint32(EventsObject, SpatialConstants::PING_ID_OFFSET_ID);
				SendPingOrPong(ID, SpatialConstants::CLIENT_PONG_COMPONENT_ID);
			}
		}
	}));
}

void ULatencyManager::Disable()
{
	if (TimerManager && LatencyTimer.IsValid())
	{
		TimerManager->ClearTimer(LatencyTimer);
	}
	SentPingTimestamps.Empty();

	PlayerControllerEntity = SpatialConstants::INVALID_ENTITY_ID;
}

void ULatencyManager::SendPingOrPong(uint32 PingId, Worker_ComponentId ComponentId)
{
	Worker_ComponentUpdate ComponentUpdate = {};

	ComponentUpdate.component_id = ComponentId;
	ComponentUpdate.schema_type = Schema_CreateComponentUpdate(ComponentId);
	Schema_Object* EventsObject = Schema_GetComponentUpdateEvents(ComponentUpdate.schema_type);
	Schema_AddObject(EventsObject, SpatialConstants::PING_EVENT_ID);
	Schema_AddUint32(EventsObject, SpatialConstants::PING_ID_OFFSET_ID, PingId);

	USpatialWorkerConnection* WorkerConnection = Cast<USpatialNetDriver>(Driver)->Connection;
	if (WorkerConnection->IsConnected())
	{
		WorkerConnection->SendComponentUpdate(PlayerControllerEntity, &ComponentUpdate);

		if (Driver->IsServer())
		{
			SentPingTimestamps.Add(PingId, GetWorld()->GetTimeSeconds());
		}
	}
}

void ULatencyManager::RemoveExpiredPings()
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
