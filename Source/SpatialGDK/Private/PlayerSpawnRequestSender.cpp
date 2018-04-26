// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "PlayerSpawnRequestSender.h"
#include "CoreMinimal.h"
#include "SpatialConstants.h"
#include "SpatialOS.h"
#include "TimerManager.h"
#include <functional>

DEFINE_LOG_CATEGORY(LogSpatialOSPlayerSpawner);

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
		ResponseCallbackKey.Reset();
	}
}

void FPlayerSpawnRequestSender::RequestPlayer(USpatialOS *InSpatialOS, FTimerManager *InTimerManager, const FURL &Url)
{
	TimerManager = InTimerManager;
	Connection = InSpatialOS->GetConnection().Pin().Get();
	View = InSpatialOS->GetView().Pin().Get();
	Request = SpawnPlayerCommand::Request{TCHAR_TO_UTF8(*Url.ToString())};
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
	const worker::CommandResponseOp<SpawnPlayerCommand> &Op)
{
	View->Remove(ResponseCallbackKey.GetValue());
	ResponseCallbackKey.Reset();
	if (Op.StatusCode == worker::StatusCode::kSuccess)
	{
		UE_LOG(LogSpatialOSPlayerSpawner, Display, TEXT("Player spawned sucessfully"));
	}
	else if (NumberOfAttempts < SpatialConstants::MAX_NUMBER_COMMAND_ATTEMPTS)
	{
		UE_LOG(LogSpatialOSPlayerSpawner, Warning, TEXT("Player spawn request failed: \"%s\""), *FString(Op.Message.c_str()));

		FTimerHandle RetryTimer;
		FTimerDelegate TimerCallback;
		TimerCallback.BindLambda([this, RetryTimer]() {
			SendPlayerSpawnRequest();
		});

		TimerManager->SetTimer(RetryTimer, TimerCallback, SpatialConstants::GetCommandRetryWaitTimeSeconds(NumberOfAttempts), false);
	}
	else
	{
		UE_LOG(LogSpatialOSPlayerSpawner, Fatal, TEXT("Player spawn request failed too many times. (%u attempts)"), SpatialConstants::MAX_NUMBER_COMMAND_ATTEMPTS)
	}
}
