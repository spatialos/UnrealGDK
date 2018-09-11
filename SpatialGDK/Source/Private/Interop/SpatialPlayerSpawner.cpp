// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialPlayerSpawner.h"

#include "SocketSubsystem.h"
#include "TimerManager.h"

#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"

#include <improbable/c_schema.h>
#include <improbable/c_worker.h>

using namespace worker;

DEFINE_LOG_CATEGORY(LogSpatialGDKPlayerSpawner);

void USpatialPlayerSpawner::Init(USpatialNetDriver* NetDriver, FTimerManager* TimerManager)
{
	this->NetDriver = NetDriver;
	this->TimerManager = TimerManager;

	NumberOfAttempts = 0;
}

void USpatialPlayerSpawner::ReceivePlayerSpawnRequest(FString URLString, const char* CallerWorkerId, Worker_RequestId RequestId )
{
	URLString.Append(TEXT("?workerId=")).Append(UTF8_TO_TCHAR(CallerWorkerId));

	NetDriver->AcceptNewPlayer(FURL(nullptr, *URLString, TRAVEL_Absolute), false);

	Worker_CommandResponse CommandResponse = {};
	CommandResponse.component_id = SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID;
	CommandResponse.schema_type = Schema_CreateCommandResponse(SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID, 1);
	Schema_Object* ResponseObject = Schema_GetCommandResponseObject(CommandResponse.schema_type);
	Schema_AddBool(ResponseObject, 1, true);

	NetDriver->Connection->SendCommandResponse(RequestId, &CommandResponse);
}

void USpatialPlayerSpawner::SendPlayerSpawnRequest()
{
	FURL DummyURL;

	Worker_CommandRequest CommandRequest = {};
	CommandRequest.component_id = SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID;
	CommandRequest.schema_type = Schema_CreateCommandRequest(SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID, 1);
	Schema_Object* RequestObject = Schema_GetCommandRequestObject(CommandRequest.schema_type);
	Schema_AddString(RequestObject, 1, DummyURL.ToString(true));

	NetDriver->Connection->SendCommandRequest(SpatialConstants::SPAWNER_ENTITY_ID, &CommandRequest, 1);

	++NumberOfAttempts;
}

void USpatialPlayerSpawner::ReceivePlayerSpawnResponse(Worker_CommandResponseOp& Op)
{
	UE_LOG(LogTemp, Warning, TEXT("Got a response"));

	if (Op.status_code == WORKER_STATUS_CODE_SUCCESS)
	{
		UE_LOG(LogSpatialGDKPlayerSpawner, Display, TEXT("Player spawned sucessfully"));
	}
	else if (NumberOfAttempts < SpatialConstants::MAX_NUMBER_COMMAND_ATTEMPTS)
	{
		UE_LOG(LogSpatialGDKPlayerSpawner, Warning, TEXT("Player spawn request failed: \"%s\""),
			UTF8_TO_TCHAR(Op.message));

		FTimerHandle RetryTimer;
		TimerManager->SetTimer(RetryTimer, [this]()
		{
			SendPlayerSpawnRequest();
		}, SpatialConstants::GetCommandRetryWaitTimeSeconds(NumberOfAttempts), false);
	}
	else
	{
		UE_LOG(LogSpatialGDKPlayerSpawner, Fatal, TEXT("Player spawn request failed too many times. (%u attempts)"),
			SpatialConstants::MAX_NUMBER_COMMAND_ATTEMPTS)
	}
}
