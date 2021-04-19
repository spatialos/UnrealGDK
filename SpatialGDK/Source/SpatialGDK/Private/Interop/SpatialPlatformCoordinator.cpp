// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialPlatformCoordinator.h"

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
#include "Interop/SpatialWorkerFlags.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Schema/AuthorityIntent.h"
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

DEFINE_LOG_CATEGORY(LogSpatialPlatformCoordinator);

using namespace SpatialGDK;

USpatialPlatformCoordinator::USpatialPlatformCoordinator() {}

USpatialPlatformCoordinator::~USpatialPlatformCoordinator()
{
	for (auto Request : UncompletedRequests)
	{
		Request.Value->OnProcessRequestComplete().Unbind();
		Request.Value->OnHeaderReceived().Unbind();
		Request.Value->OnRequestProgress().Unbind();
		Request.Value->OnRequestWillRetry().Unbind();
	}
}

void USpatialPlatformCoordinator::Init(UNetDriver* InDriver)
{
	CachedReadyStatus = false;
	Driver = Cast<USpatialNetDriver>(InDriver);
	Url = GetDefault<USpatialGDKSettings>()->SpatialPlatformUrl;
}

void USpatialPlatformCoordinator::StartSendingHeartbeat()
{
	const FString SpatialWorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();

	UE_LOG(LogSpatialPlatformCoordinator, Verbose, TEXT("%s - SpatialWorkerId:[%s]"), *FString(__FUNCTION__), *SpatialWorkerId);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HeartbeatRequest = FHttpModule::Get().CreateRequest();

	HeartbeatRequest->OnProcessRequestComplete().BindLambda(
		[this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
			UncompletedRequests.Remove(HeartBeatRequestKey);

			GetWorld()->GetTimerManager().ClearTimer(HeartBeatTimerHandler);
			GetWorld()->GetTimerManager().SetTimer(HeartBeatTimerHandler, this, &USpatialPlatformCoordinator::StartSendingHeartbeat,
												   GetDefault<USpatialGDKSettings>()->SpatialPlatformHeartbeatInterval, false);

			int32 HttpCode = Response->GetResponseCode();
			UE_LOG(LogSpatialPlatformCoordinator, Verbose, TEXT("%s - HTTP Code:[%d], Response:[%s]"), *FString(__FUNCTION__),
				   HttpCode, *Response->GetContentAsString());

			if (!bWasSuccessful || HttpCode != 200)
			{
				UE_LOG(LogSpatialPlatformCoordinator, Warning, TEXT("%s - Failed HTTP request, Response:[%s]"), *FString(__FUNCTION__),
					   *Response->GetContentAsString());
			}
		});

	HeartbeatRequest->OnRequestProgress().BindLambda([this](FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived) {});

	HeartbeatRequest->OnHeaderReceived().BindLambda(
		[this](FHttpRequestPtr Request, const FString& HeaderName, const FString& NewHeaderValue) {});

	HeartbeatRequest->OnRequestWillRetry().BindLambda([this](FHttpRequestPtr Request, FHttpResponsePtr Response, float SecondsToRetry) {});

	HeartbeatRequest->SetURL(Url + TEXT("/health/") + SpatialWorkerId);
	HeartbeatRequest->SetVerb("POST");
	HeartbeatRequest->SetHeader(TEXT("User-Agent"), "UnrealEngine-GDK-Agent");
	HeartbeatRequest->SetHeader("Content-Type", TEXT("application/json"));
	HeartbeatRequest->ProcessRequest();

	UncompletedRequests.Add(HeartBeatRequestKey, HeartbeatRequest);
}

void USpatialPlatformCoordinator::SendReadyStatus()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogSpatialPlatformCoordinator, Warning, TEXT("%s - World is null, queue this ready request"), *FString(__FUNCTION__));
		CachedReadyStatus = true;
		return;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogSpatialPlatformCoordinator, Warning, TEXT("%s - GameInstance is null, queue this ready request"), *FString(__FUNCTION__));
		CachedReadyStatus = true;
		return;
	}

	const FString SpatialWorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();
	if (SpatialWorkerId.TrimStartAndEnd().IsEmpty())
	{
		UE_LOG(LogSpatialPlatformCoordinator, Warning, TEXT("%s - SpatialWorkerId is empty, queue this ready request"),
			   *FString(__FUNCTION__));
		CachedReadyStatus = true;
		return;
	}

	CachedReadyStatus = false;

	UE_LOG(LogSpatialPlatformCoordinator, Verbose, TEXT("%s - SpatialWorkerId:[%s]"), *FString(__FUNCTION__), *SpatialWorkerId);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> ReadyStatusRequest = FHttpModule::Get().CreateRequest();

	ReadyStatusRequest->OnProcessRequestComplete().BindLambda(
		[this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
			UncompletedRequests.Remove(ReadyRequestKey);

			int32 HttpCode = Response->GetResponseCode();
			UE_LOG(LogSpatialPlatformCoordinator, Verbose, TEXT("%s - HTTP Code:[%d], Response:[%s]"), *FString(__FUNCTION__),
				   HttpCode, *Response->GetContentAsString());

			if (!bWasSuccessful || HttpCode != 200)
			{
				UE_LOG(LogSpatialPlatformCoordinator, Warning, TEXT("%s - Failed HTTP request, Response:[%s]"), *FString(__FUNCTION__),
					   *Response->GetContentAsString());

				GetWorld()->GetTimerManager().ClearTimer(ReadyTimerHandler);
				GetWorld()->GetTimerManager().SetTimer(ReadyTimerHandler, this, &USpatialPlatformCoordinator::SendReadyStatus, 5, false);
			}
		});

	ReadyStatusRequest->OnRequestProgress().BindLambda([this](FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived) {});

	ReadyStatusRequest->OnHeaderReceived().BindLambda(
		[this](FHttpRequestPtr Request, const FString& HeaderName, const FString& NewHeaderValue) {});

	ReadyStatusRequest->OnRequestWillRetry().BindLambda(
		[this](FHttpRequestPtr Request, FHttpResponsePtr Response, float SecondsToRetry) {});

	// ReadyStatusRequest->SetURL(Url + TEXT("/ready/"));
	ReadyStatusRequest->SetURL(Url + TEXT("/ready/") + SpatialWorkerId);
	ReadyStatusRequest->SetVerb("POST");
	ReadyStatusRequest->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	ReadyStatusRequest->SetHeader("Content-Type", TEXT("application/json"));
	ReadyStatusRequest->ProcessRequest();

	UncompletedRequests.Add(ReadyRequestKey, ReadyStatusRequest);
}

void USpatialPlatformCoordinator::StartPollingForGameserverStatus()
{
	USpatialWorkerFlags* SpatialWorkerFlags = Driver->SpatialWorkerFlags;
	const FString SpatialWorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();

	UE_LOG(LogSpatialPlatformCoordinator, Verbose, TEXT("%s - SpatialWorkerId:[%s]"), *FString(__FUNCTION__), *SpatialWorkerId);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> WorkerStatusPollingRequest = FHttpModule::Get().CreateRequest();

	WorkerStatusPollingRequest->OnProcessRequestComplete().BindLambda(
		[this, SpatialWorkerFlags](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
			UncompletedRequests.Remove(GameserverRequestKey);

			GetWorld()->GetTimerManager().ClearTimer(GameserverStatusTimerHandler);
			GetWorld()->GetTimerManager().SetTimer(GameserverStatusTimerHandler, this,
												   &USpatialPlatformCoordinator::StartPollingForGameserverStatus,
												   GetDefault<USpatialGDKSettings>()->SpatialPlatformServerStatusPollingInterval, false);

			int32 HttpCode = Response->GetResponseCode();
			UE_LOG(LogSpatialPlatformCoordinator, Verbose, TEXT("%s - HTTP Code:[%d], Response:[%s]"), *FString(__FUNCTION__),
				   HttpCode, *Response->GetContentAsString());

			if (bWasSuccessful && HttpCode == 200)
			{
				TSharedPtr<FJsonObject> RootObject;
				FString ResponseStr = Response->GetContentAsString();
				TSharedRef<TJsonReader<> > JsonReader = TJsonReaderFactory<>::Create(ResponseStr);
				if (!FJsonSerializer::Deserialize(JsonReader, RootObject))
				{
					UE_LOG(LogSpatialPlatformCoordinator, Warning, TEXT("%s - failed to parse json, Response:[%s]"), *FString(__FUNCTION__),
						   *Response->GetContentAsString());
					return;
				}

				TSharedPtr<FJsonObject> StatusJson = RootObject->GetObjectField("status");
				if (!StatusJson.IsValid())
				{
					UE_LOG(LogSpatialPlatformCoordinator, Warning, TEXT("%s - Missing field status, Response:[%s]"), *FString(__FUNCTION__),
						   *Response->GetContentAsString());
					return;
				}

				for (auto JsonValue : StatusJson->Values)
				{
					if (JsonValue.Value->Type != EJson::Number && JsonValue.Value->Type != EJson::String)
					{
						continue;
					}

					FString JsonFieldValueString = JsonValue.Value->AsString();

					FString OutFieldValue;
					SpatialWorkerFlags->GetWorkerFlag(JsonValue.Key, OutFieldValue);

					if (JsonFieldValueString != OutFieldValue)
					{
						SpatialWorkerFlags->SetWorkerFlag(JsonValue.Key, JsonFieldValueString);
					}
				}
			}
			else
			{
				UE_LOG(LogSpatialPlatformCoordinator, Warning, TEXT("%s - Failed HTTP request, Response:[%s], ResponseCode:[%d]"),
					   *FString(__FUNCTION__), *Response->GetContentAsString(), Response->GetResponseCode());
			}
		});

	WorkerStatusPollingRequest->OnRequestProgress().BindLambda(
		[this, SpatialWorkerFlags](FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived) {});

	WorkerStatusPollingRequest->OnHeaderReceived().BindLambda(
		[this](FHttpRequestPtr Request, const FString& HeaderName, const FString& NewHeaderValue) {});

	WorkerStatusPollingRequest->OnRequestWillRetry().BindLambda(
		[this](FHttpRequestPtr Request, FHttpResponsePtr Response, float SecondsToRetry) {});

	WorkerStatusPollingRequest->SetURL(Url + TEXT("/gameserver"));
	WorkerStatusPollingRequest->SetVerb("GET");
	WorkerStatusPollingRequest->SetHeader(TEXT("User-Agent"), "UnrealEngine-GDK-Agent");
	WorkerStatusPollingRequest->SetHeader("Content-Type", TEXT("application/json"));
	WorkerStatusPollingRequest->ProcessRequest();

	UncompletedRequests.Add(GameserverRequestKey, WorkerStatusPollingRequest);
}

void USpatialPlatformCoordinator::StartPollingForWorkerFlags()
{
	USpatialWorkerFlags* SpatialWorkerFlags = Driver->SpatialWorkerFlags;
	const FString SpatialWorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();

	UE_LOG(LogSpatialPlatformCoordinator, Verbose, TEXT("%s - SpatialWorkerId:[%s]"), *FString(__FUNCTION__), *SpatialWorkerId);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> WorkerFlagsPollingRequest = FHttpModule::Get().CreateRequest();

	WorkerFlagsPollingRequest->OnProcessRequestComplete().BindLambda(
		[this, SpatialWorkerFlags](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
			UncompletedRequests.Remove(WorkerflagsRequestKey);

			GetWorld()->GetTimerManager().ClearTimer(WorkerFlagsTimerHandler);
			GetWorld()->GetTimerManager().SetTimer(WorkerFlagsTimerHandler, this, &USpatialPlatformCoordinator::StartPollingForWorkerFlags,
												   GetDefault<USpatialGDKSettings>()->SpatialPlatformWorkerFlagsPollingInterval, false);

			int32 HttpCode = Response->GetResponseCode();
			UE_LOG(LogSpatialPlatformCoordinator, Verbose, TEXT("%s - HTTP Code:[%d], Response:[%s]"), *FString(__FUNCTION__),
				   HttpCode, *Response->GetContentAsString());

			if (bWasSuccessful && HttpCode == 200)
			{
				TSharedPtr<FJsonObject> RootObject;
				FString ResponseStr = Response->GetContentAsString();
				TSharedRef<TJsonReader<> > JsonReader = TJsonReaderFactory<>::Create(ResponseStr);
				if (!FJsonSerializer::Deserialize(JsonReader, RootObject))
				{
					UE_LOG(LogSpatialPlatformCoordinator, Warning, TEXT("%s - failed to parse json, Response:[%s]"), *FString(__FUNCTION__),
						   *Response->GetContentAsString());
					return;
				}

				TSharedPtr<FJsonObject> WorkerFlagsJson = RootObject->GetObjectField("worker_flags");
				if (!WorkerFlagsJson.IsValid())
				{
					UE_LOG(LogSpatialPlatformCoordinator, Warning, TEXT("%s - Missing field object_meta, Response:[%s]"),
						   *FString(__FUNCTION__), *Response->GetContentAsString());
					return;
				}

				for (auto JsonValue : WorkerFlagsJson->Values)
				{
					if (JsonValue.Value->Type != EJson::Number && JsonValue.Value->Type != EJson::String)
					{
						continue;
					}

					FString JsonFieldValueString = JsonValue.Value->AsString();

					FString OutFieldValue;
					SpatialWorkerFlags->GetWorkerFlag(JsonValue.Key, OutFieldValue);

					if (JsonFieldValueString != OutFieldValue)
					{
						SpatialWorkerFlags->SetWorkerFlag(JsonValue.Key, JsonFieldValueString);
					}
				}
			}
			else
			{
				UE_LOG(LogSpatialPlatformCoordinator, Warning, TEXT("%s - Failed HTTP request, Response:[%s], ResponseCode:[%d]"),
					   *FString(__FUNCTION__), *Response->GetContentAsString(), Response->GetResponseCode());
			}
		});

	WorkerFlagsPollingRequest->OnRequestProgress().BindLambda(
		[this, SpatialWorkerFlags](FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived) {});

	WorkerFlagsPollingRequest->OnHeaderReceived().BindLambda(
		[this](FHttpRequestPtr Request, const FString& HeaderName, const FString& NewHeaderValue) {});

	WorkerFlagsPollingRequest->OnRequestWillRetry().BindLambda(
		[this](FHttpRequestPtr Request, FHttpResponsePtr Response, float SecondsToRetry) {});

	WorkerFlagsPollingRequest->SetURL(Url + TEXT("/workerflags"));
	WorkerFlagsPollingRequest->SetVerb("GET");
	WorkerFlagsPollingRequest->SetHeader(TEXT("User-Agent"), "UnrealEngine-GDK-Agent");
	WorkerFlagsPollingRequest->SetHeader("Content-Type", TEXT("application/json"));
	WorkerFlagsPollingRequest->ProcessRequest();

	UncompletedRequests.Add(WorkerflagsRequestKey, WorkerFlagsPollingRequest);
}

bool USpatialPlatformCoordinator::CheckPlatformSwitch(bool bHeartBeat)
{
	FString strSwitch = FPlatformMisc::GetEnvironmentVariable(TEXT("bEnableSpatialPlatformCoordinator")).ToLower().TrimStartAndEnd();
	if (strSwitch.IsEmpty())
	{
		return false;
	}
	if (bHeartBeat == true && strSwitch != "true")
	{
		return false;
	}

	return true;
}
