// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/LatencyManager.h"

#include "Engine/World.h"

#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialReceiver.h"
#include "SpatialConstants.h"

ULatencyManager::ULatencyManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, PlayerControllerEntity(SpatialConstants::INVALID_ENTITY_ID)
{
}

void ULatencyManager::Enable(Worker_EntityId InPlayerControllerEntity)
{
	checkf(PlayerControllerEntity == SpatialConstants::INVALID_ENTITY_ID, TEXT("LatencyManager::Enable : PlayerControllerEntity already set: %lld. New entity: %lld"), PlayerControllerEntity, InPlayerControllerEntity);
	PlayerControllerEntity = InPlayerControllerEntity;

	NetConnection = Cast<USpatialNetConnection>(GetOuter());
	NetDriver = Cast<USpatialNetDriver>(NetConnection->GetDriver());
	LastPingSent = GetWorld()->RealTimeSeconds;

	auto Delegate = TBaseDelegate<void, Worker_ComponentUpdateOp &>::CreateLambda([this](const Worker_ComponentUpdateOp& Op)
	{
		float ReceivedTimestamp = GetWorld()->RealTimeSeconds;

		if (!NetConnection.IsValid())
		{
			return;
		}

		Schema_Object* EventsObject = Schema_GetComponentUpdateEvents(Op.update.schema_type);
		uint32 EventCount = Schema_GetObjectCount(EventsObject, SpatialConstants::PING_PONG_EVENT_ID);

		if (EventCount > 0)
		{
			NetConnection->PlayerController->PlayerState->UpdatePing(ReceivedTimestamp - LastPingSent);
			SendPingOrPong(NetDriver->IsServer() ? SpatialConstants::SERVER_PING_COMPONENT_ID : SpatialConstants::CLIENT_PONG_COMPONENT_ID);
		}
	});


	if (NetDriver->IsServer())
	{
		NetDriver->Receiver->AddClientPongDelegate(PlayerControllerEntity, Delegate);
		SendPingOrPong(SpatialConstants::SERVER_PING_COMPONENT_ID);
	}
	else
	{
		NetDriver->Receiver->AddServerPingDelegate(PlayerControllerEntity, Delegate);
	}
}

void ULatencyManager::Disable()
{
	PlayerControllerEntity = SpatialConstants::INVALID_ENTITY_ID;
}

void ULatencyManager::SendPingOrPong(Worker_ComponentId ComponentId)
{
	Worker_ComponentUpdate ComponentUpdate = {};

	ComponentUpdate.component_id = ComponentId;
	ComponentUpdate.schema_type = Schema_CreateComponentUpdate(ComponentId);
	Schema_Object* EventsObject = Schema_GetComponentUpdateEvents(ComponentUpdate.schema_type);
	Schema_AddObject(EventsObject, SpatialConstants::PING_PONG_EVENT_ID);

	USpatialWorkerConnection* WorkerConnection = NetDriver->Connection;
	if (WorkerConnection->IsConnected())
	{
		WorkerConnection->SendComponentUpdate(PlayerControllerEntity, &ComponentUpdate);
		LastPingSent = GetWorld()->RealTimeSeconds;
	}
}
