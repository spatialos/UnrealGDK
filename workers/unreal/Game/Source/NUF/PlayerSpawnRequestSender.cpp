// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "PlayerSpawnRequestSender.h"
#include "CoreMinimal.h"
#include "SpatialOS.h"
#include "SpatialConstants.h"
#include "TimerManager.h"
#include <functional>

DEFINE_LOG_CATEGORY(LogSpatialOSPlayerSpawner);

const float FPlayerSpawnRequestSender::FIRST_RETRY_WAIT_SECONDS = 0.2f;
const uint32 FPlayerSpawnRequestSender::MAX_NUMBER_ATTEMPTS = 5u;

FPlayerSpawnRequestSender::FPlayerSpawnRequestSender() 
	: TimerManager(nullptr)
	, Connection(nullptr)
	, View(nullptr)
	, NumberOfAttempts(0u)
{
}

FPlayerSpawnRequestSender::~FPlayerSpawnRequestSender()
{
	if (ResponseCallbackKey.IsSet()) 
	{
		View->Remove(ResponseCallbackKey.GetValue());
	}
}

void FPlayerSpawnRequestSender::RequestPlayer(USpatialOS* InSpatialOS, FTimerManager* InTimerManager, const FURL& Url)
{
	TimerManager = InTimerManager;
	Connection = InSpatialOS->GetConnection().Pin().Get();
	View = InSpatialOS->GetView().Pin().Get();
	Request = SpawnPlayerCommand::Request{ TCHAR_TO_UTF8(*Url.ToString()) };
	NumberOfAttempts = 0;

	SendPlayerSpawnRequest();
}

void FPlayerSpawnRequestSender::SendPlayerSpawnRequest() 
{
	Connection->SendCommandRequest<SpawnPlayerCommand>(SpatialConstants::SPAWNER_ENTITY_ID, Request, 0);
	++NumberOfAttempts;

	ResponseCallbackKey = View->OnCommandResponse<SpawnPlayerCommand>(
		std::bind(&FPlayerSpawnRequestSender::HandlePlayerSpawnResponse, this, std::placeholders::_1));
}

void FPlayerSpawnRequestSender::HandlePlayerSpawnResponse(
	const worker::CommandResponseOp<SpawnPlayerCommand>& Op) 
{
	View->Remove(ResponseCallbackKey.GetValue());
	ResponseCallbackKey.Reset();
	if (Op.StatusCode == worker::StatusCode::kSuccess) 
	{
		UE_LOG(LogSpatialOSPlayerSpawner, Display, TEXT("Player spawned sucessfully"));
	}
	else if (NumberOfAttempts < MAX_NUMBER_ATTEMPTS) 
	{
		UE_LOG(LogSpatialOSPlayerSpawner, Warning, TEXT("Player spawn request failed: \"%s\""), 
			*FString(Op.Message.c_str()));

		FTimerHandle RetryTimer;
		FTimerDelegate TimerCallback;
		TimerCallback.BindLambda([this, RetryTimer]() 
		{
			SendPlayerSpawnRequest();
		});

		TimerManager->SetTimer(RetryTimer, TimerCallback, GetRetryWaitTimeSeconds(), false);
	} 
	else 
	{
		UE_LOG(LogSpatialOSPlayerSpawner, Fatal, TEXT("Player spawn request failed too many times. (%u attempts)"), 
			MAX_NUMBER_ATTEMPTS)
	}
}

float FPlayerSpawnRequestSender::GetRetryWaitTimeSeconds()
{
	// Double the time to wait on each failure
	uint32 WaitTimeExponentialFactor = 1u << (NumberOfAttempts - 1);
	return FIRST_RETRY_WAIT_SECONDS * WaitTimeExponentialFactor;
}
