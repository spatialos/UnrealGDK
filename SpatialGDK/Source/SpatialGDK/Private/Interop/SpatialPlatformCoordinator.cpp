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
	for (auto Request : CachedRequests)
	{
		Request.Value->OnProcessRequestComplete().Unbind();
		Request.Value->OnHeaderReceived().Unbind();
		Request.Value->OnRequestProgress().Unbind();
		Request.Value->OnRequestWillRetry().Unbind();
	}
}

void USpatialPlatformCoordinator::Init(UNetDriver* InDriver)
{
	Driver = Cast<USpatialNetDriver>(InDriver);
	Url = GetDefault<USpatialGDKSettings>()->SpatialPlatformUrl;
}

void USpatialPlatformCoordinator::StartSendingHeartbeat()
{
	CHECK_PLATFORM_SWITCH(true);

	USpatialWorkerFlags* SpatialWorkerFlags = Driver->SpatialWorkerFlags;
	const FString SpatialWorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();
	FString NewSpatialWorkerId = SpatialWorkerId + "";

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HeartbeatRequest = FHttpModule::Get().CreateRequest();

	HeartbeatRequest->OnProcessRequestComplete().BindLambda(
		[this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
			CachedRequests.Remove(HeartBeatRequestKey);

			GetWorld()->GetTimerManager().ClearTimer(HeartBeatTimerHandler);
			GetWorld()->GetTimerManager().SetTimer(HeartBeatTimerHandler, this, &USpatialPlatformCoordinator::StartSendingHeartbeat,
												   GetDefault<USpatialGDKSettings>()->SpatialPlatformHeartbeatInterval, false);

			if (bWasSuccessful)
			{
				TSharedPtr<FJsonObject> RootObject;
				FString ResponseStr = Response->GetContentAsString();
				TSharedRef<TJsonReader<> > JsonReader = TJsonReaderFactory<>::Create(ResponseStr);
				if (!FJsonSerializer::Deserialize(JsonReader, RootObject))
				{
					return;
				}
			}
			else
			{
				UE_LOG(LogSpatialPlatformCoordinator, Warning, TEXT("%s - Failed HTTP request, Response:[%s]"), *FString(__FUNCTION__),
					   *Response->GetContentAsString());
			}
		});

	HeartbeatRequest->OnRequestProgress().BindLambda(
		[this, SpatialWorkerFlags](FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived) {});

	HeartbeatRequest->OnHeaderReceived().BindLambda(
		[this](FHttpRequestPtr Request, const FString& HeaderName, const FString& NewHeaderValue) {});

	HeartbeatRequest->OnRequestWillRetry().BindLambda([this](FHttpRequestPtr Request, FHttpResponsePtr Response, float SecondsToRetry) {});

	HeartbeatRequest->SetURL(Url + TEXT("/health/") + NewSpatialWorkerId);
	HeartbeatRequest->SetVerb("POST");
	HeartbeatRequest->SetHeader(TEXT("User-Agent"), "UnrealEngine-GDK-Agent");
	HeartbeatRequest->SetHeader("Content-Type", TEXT("application/json"));
	HeartbeatRequest->ProcessRequest();

	CachedRequests.Add(HeartBeatRequestKey, HeartbeatRequest);
}

void USpatialPlatformCoordinator::SendReadyStatus()
{
	CHECK_PLATFORM_SWITCH(false);
	
	USpatialWorkerFlags* SpatialWorkerFlags = Driver->SpatialWorkerFlags;
	const FString SpatialWorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();
	FString NewSpatialWorkerId = SpatialWorkerId + "";

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> ReadyStatusRequest = FHttpModule::Get().CreateRequest();

	ReadyStatusRequest->OnProcessRequestComplete().BindLambda(
		[this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
			CachedRequests.Remove(ReadyRequestKey);

			if (bWasSuccessful) {}
			else
			{
				UE_LOG(LogSpatialPlatformCoordinator, Warning, TEXT("%s - Failed HTTP request, Response:[%s]"), *FString(__FUNCTION__),
					   *Response->GetContentAsString());
			}
		});

	ReadyStatusRequest->OnRequestProgress().BindLambda(
		[this, SpatialWorkerFlags](FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived) {});

	ReadyStatusRequest->OnHeaderReceived().BindLambda(
		[this](FHttpRequestPtr Request, const FString& HeaderName, const FString& NewHeaderValue) {});

	ReadyStatusRequest->OnRequestWillRetry().BindLambda(
		[this](FHttpRequestPtr Request, FHttpResponsePtr Response, float SecondsToRetry) {});

	ReadyStatusRequest->SetURL(Url + TEXT("/ready/") + NewSpatialWorkerId);
	ReadyStatusRequest->SetVerb("POST");
	ReadyStatusRequest->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	ReadyStatusRequest->SetHeader("Content-Type", TEXT("application/json"));
	ReadyStatusRequest->ProcessRequest();

	CachedRequests.Add(ReadyRequestKey, ReadyStatusRequest);
}

void USpatialPlatformCoordinator::StartPollingForGameserverStatus()
{
	CHECK_PLATFORM_SWITCH(false);

	USpatialWorkerFlags* SpatialWorkerFlags = Driver->SpatialWorkerFlags;
	const FString SpatialWorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();
	FString NewSpatialWorkerId = SpatialWorkerId + "";

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> WorkerStatusPollingRequest = FHttpModule::Get().CreateRequest();

	WorkerStatusPollingRequest->OnProcessRequestComplete().BindLambda(
		[this, SpatialWorkerFlags](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
			CachedRequests.Remove(GameserverRequestKey);

			GetWorld()->GetTimerManager().ClearTimer(GameserverStatusTimerHandler);
			GetWorld()->GetTimerManager().SetTimer(GameserverStatusTimerHandler, this,
												   &USpatialPlatformCoordinator::StartPollingForGameserverStatus,
												   GetDefault<USpatialGDKSettings>()->SpatialPlatformServerStatusPollingInterval, false);

			if (bWasSuccessful)
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

	CachedRequests.Add(GameserverRequestKey, WorkerStatusPollingRequest);
}

void USpatialPlatformCoordinator::StartWatchingForGameserverStatus()
{
	CHECK_PLATFORM_SWITCH(false);

	USpatialWorkerFlags* SpatialWorkerFlags = Driver->SpatialWorkerFlags;

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> WatcherStatusPollingRequest = FHttpModule::Get().CreateRequest();
	WatcherStatusPollingRequest->OnProcessRequestComplete().BindLambda(
		[this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {});
	WatcherStatusPollingRequest->OnRequestProgress().BindLambda(
		[this, SpatialWorkerFlags](FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived) {
			FHttpResponsePtr Response = Request->GetResponse();
			FString ResponseStr = Response->GetContentAsString();
		});
	WatcherStatusPollingRequest->OnHeaderReceived().BindLambda(
		[this](FHttpRequestPtr Request, const FString& HeaderName, const FString& NewHeaderValue) {});
	WatcherStatusPollingRequest->OnRequestWillRetry().BindLambda(
		[this](FHttpRequestPtr Request, FHttpResponsePtr Response, float SecondsToRetry) {});

	WatcherStatusPollingRequest->SetURL(Url + TEXT("/watch/gameserver"));
	WatcherStatusPollingRequest->SetVerb("GET");
	WatcherStatusPollingRequest->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	WatcherStatusPollingRequest->SetHeader("Content-Type", TEXT("application/json"));
	WatcherStatusPollingRequest->ProcessRequest();
}

void USpatialPlatformCoordinator::StartPollingForWorkerFlags()
{
	CHECK_PLATFORM_SWITCH(false);

	USpatialWorkerFlags* SpatialWorkerFlags = Driver->SpatialWorkerFlags;
	const FString SpatialWorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();
	FString NewSpatialWorkerId = SpatialWorkerId + "";

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> WorkerFlagsPollingRequest = FHttpModule::Get().CreateRequest();

	WorkerFlagsPollingRequest->OnProcessRequestComplete().BindLambda(
		[this, SpatialWorkerFlags](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
			CachedRequests.Remove(WorkerflagsRequestKey);

			GetWorld()->GetTimerManager().ClearTimer(WorkerFlagsTimerHandler);
			GetWorld()->GetTimerManager().SetTimer(WorkerFlagsTimerHandler, this, &USpatialPlatformCoordinator::StartPollingForWorkerFlags,
												   GetDefault<USpatialGDKSettings>()->SpatialPlatformWorkerFlagsPollingInterval, false);

			if (bWasSuccessful)
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

	CachedRequests.Add(WorkerflagsRequestKey, WorkerFlagsPollingRequest);
}
