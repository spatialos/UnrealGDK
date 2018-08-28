// Fill out your copyright notice in the Description page of Project Settings.

#include "SpatialPlayerSpawner.h"
#include "SocketSubsystem.h"
#include "SpatialNetConnection.h"
#include "SpatialConstants.h"
#include "SpatialNetDriver.h"
#include "TimerManager.h"

#include "SchemaHelpers.h"

#include <improbable/c_worker.h>
#include <improbable/c_schema.h>

using namespace worker;

DEFINE_LOG_CATEGORY(LogSpatialGDKPlayerSpawner);

void USpatialPlayerSpawner::Init(USpatialNetDriver* NetDriver, FTimerManager* TimerManager)
{
	this->NetDriver = NetDriver;
	this->TimerManager = TimerManager;

	NumberOfAttempts = 0;
}

void USpatialPlayerSpawner::ReceivePlayerSpawnRequest(std::string& URL, Worker_RequestId RequestId )
{
	FString URLString = UTF8_TO_TCHAR(URL.c_str());
	//URLString.Append(TEXT("?workerId=")).Append(UTF8_TO_TCHAR(op.CallerWorkerId.c_str()));

	NetDriver->AcceptNewPlayer(FURL(nullptr, *URLString, TRAVEL_Absolute), false);

	Worker_CommandResponse CommandResponse = {};
	CommandResponse.component_id = SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID;
	//CommandResponse.schema_type = Schema_CreateCommandRequest(SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID, 1);
	//Schema_Object* ResponseObject = Schema_GetCommandResponseObject(CommandResponse.schema_type);
	//Schema_AddBool(ResponseObject, 1, true);

	//Worker_Connection_SendCommandResponse(NetDriver->Connection, RequestId, &CommandResponse);
}

void USpatialPlayerSpawner::SendPlayerSpawnRequest()
{
	FURL DummyURL;

	Worker_CommandRequest CommandRequest = {};
	CommandRequest.component_id = SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID;
	CommandRequest.schema_type = Schema_CreateCommandRequest(SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID, 1);
	Schema_Object* RequestObject = Schema_GetCommandRequestObject(CommandRequest.schema_type);
	Schema_AddString(RequestObject, 1, TCHAR_TO_ANSI(*DummyURL.ToString(true)));

	Worker_CommandParameters CommandParams = {};
	Worker_Connection_SendCommandRequest(NetDriver->Connection, SpatialConstants::SPAWNER_ENTITY_ID, &CommandRequest, 1, nullptr, &CommandParams);

	++NumberOfAttempts;
}

void USpatialPlayerSpawner::ReceivePlayerSpawnResponse()
{
	//if(op.StatusCode == worker::StatusCode::kSuccess)
	//{
	//	UE_LOG(LogSpatialGDKPlayerSpawner, Display, TEXT("Player spawned sucessfully"));
	//}
	//else if(NumberOfAttempts < SpatialConstants::MAX_NUMBER_COMMAND_ATTEMPTS)
	//{
	//	UE_LOG(LogSpatialGDKPlayerSpawner, Warning, TEXT("Player spawn request failed: \"%s\""),
	//		*FString(op.Message.c_str()));

	//	FTimerHandle RetryTimer;
	//	FTimerDelegate TimerCallback;
	//	TimerCallback.BindLambda([this, RetryTimer]()
	//	{
	//		this->SendPlayerSpawnRequest();
	//	});

	//	TimerManager->SetTimer(RetryTimer, TimerCallback, SpatialConstants::GetCommandRetryWaitTimeSeconds(NumberOfAttempts), false);
	//}
	//else
	//{
	//	UE_LOG(LogSpatialGDKPlayerSpawner, Fatal, TEXT("Player spawn request failed too many times. (%u attempts)"),
	//		SpatialConstants::MAX_NUMBER_COMMAND_ATTEMPTS)
	//}
}
