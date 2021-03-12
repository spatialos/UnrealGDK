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

void USpatialPlatformCoordinator::Init(UNetDriver* InDriver)
{
	Driver = Cast<USpatialNetDriver>(InDriver);
	Url = GetDefault<USpatialGDKSettings>()->SpatialPlatformUrl;
}

void USpatialPlatformCoordinator::StartSendingHeartbeat()
{
	if (!GetDefault<USpatialGDKSettings>()->bEnableSpatialPlatformCoordinator)
	{
		return;
	}

	USpatialWorkerFlags* SpatialWorkerFlags = Driver->SpatialWorkerFlags;
	const FString SpatialWorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();
	FString NewSpatialWorkerId = SpatialWorkerId + "";

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> WorkerStatusPollingRequest = FHttpModule::Get().CreateRequest();

	UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));

	WorkerStatusPollingRequest->OnProcessRequestComplete().BindLambda(
		[this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
			GetWorld()->GetTimerManager().ClearTimer(HeartBeatTimerHandler);
			GetWorld()->GetTimerManager().SetTimer(HeartBeatTimerHandler, this, &USpatialPlatformCoordinator::StartSendingHeartbeat,
												   GetDefault<USpatialGDKSettings>()->SpatialPlatformHeartbeatInterval, false);

			if (bWasSuccessful)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s - Successful HTTP request, Response:[%s]"), *FString(__FUNCTION__),
					   *Response->GetContentAsString());

				TSharedPtr<FJsonObject> RootObject;
				FHttpResponsePtr Response = Request->GetResponse();
				FString ResponseStr = Response->GetContentAsString();
				TSharedRef<TJsonReader<> > JsonReader = TJsonReaderFactory<>::Create(ResponseStr);
				if (!FJsonSerializer::Deserialize(JsonReader, RootObject))
				{
					return;
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s - Failed HTTP request, Response:[%s]"), *FString(__FUNCTION__),
					   *Response->GetContentAsString());
			}
		});

	WorkerStatusPollingRequest->OnRequestProgress().BindLambda(
		[this, SpatialWorkerFlags](FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived) {
			UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));
		});

	WorkerStatusPollingRequest->OnHeaderReceived().BindLambda(
		[this](FHttpRequestPtr Request, const FString& HeaderName, const FString& NewHeaderValue) {
			UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));
		});

	WorkerStatusPollingRequest->OnRequestWillRetry().BindLambda(
		[this](FHttpRequestPtr Request, FHttpResponsePtr Response, float SecondsToRetry) {
			UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));
		});

	WorkerStatusPollingRequest->SetURL(Url + TEXT("/health/") + NewSpatialWorkerId);
	WorkerStatusPollingRequest->SetVerb("POST");
	WorkerStatusPollingRequest->SetHeader(TEXT("User-Agent"), "UnrealEngine-GDK-Agent");
	WorkerStatusPollingRequest->SetHeader("Content-Type", TEXT("application/json"));
	WorkerStatusPollingRequest->ProcessRequest();
}

void USpatialPlatformCoordinator::SendReadyStatus()
{
	if (!GetDefault<USpatialGDKSettings>()->bEnableSpatialPlatformCoordinator)
	{
		return;
	}

	USpatialWorkerFlags* SpatialWorkerFlags = Driver->SpatialWorkerFlags;
	const FString SpatialWorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();
	FString NewSpatialWorkerId = SpatialWorkerId + "";

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> WorkerStatusPollingRequest = FHttpModule::Get().CreateRequest();

	WorkerStatusPollingRequest->OnProcessRequestComplete().BindLambda(
		[this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
			UE_LOG(LogTemp, Warning, TEXT("%s - Response:[%s]"), *FString(__FUNCTION__), *Response->GetContentAsString());
		});

	WorkerStatusPollingRequest->OnRequestProgress().BindLambda(
		[this, SpatialWorkerFlags](FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived) {
			FHttpResponsePtr Response = Request->GetResponse();
			FString ResponseStr = Response->GetContentAsString();
			UE_LOG(LogTemp, Warning, TEXT("%s - Response:[%s]"), *FString(__FUNCTION__), *ResponseStr);
		});

	WorkerStatusPollingRequest->OnHeaderReceived().BindLambda(
		[this](FHttpRequestPtr Request, const FString& HeaderName, const FString& NewHeaderValue) {
			UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));
		});

	WorkerStatusPollingRequest->OnRequestWillRetry().BindLambda(
		[this](FHttpRequestPtr Request, FHttpResponsePtr Response, float SecondsToRetry) {
			UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));
		});

	WorkerStatusPollingRequest->SetURL(Url + TEXT("/health/") + NewSpatialWorkerId);
	WorkerStatusPollingRequest->SetVerb("GET");
	WorkerStatusPollingRequest->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	WorkerStatusPollingRequest->SetHeader("Content-Type", TEXT("application/json"));
	WorkerStatusPollingRequest->ProcessRequest();
}

void USpatialPlatformCoordinator::StartPollingForGameserverStatus()
{
	if (!GetDefault<USpatialGDKSettings>()->bEnableSpatialPlatformCoordinator)
	{
		return;
	}

	USpatialWorkerFlags* SpatialWorkerFlags = Driver->SpatialWorkerFlags;
	const FString SpatialWorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();
	FString NewSpatialWorkerId = SpatialWorkerId + "";

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> WorkerStatusPollingRequest = FHttpModule::Get().CreateRequest();

	UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));

	WorkerStatusPollingRequest->OnProcessRequestComplete().BindLambda(
		[this, SpatialWorkerFlags](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
			GetWorld()->GetTimerManager().ClearTimer(GameserverStatusTimerHandler);
			GetWorld()->GetTimerManager().SetTimer(GameserverStatusTimerHandler, this,
												   &USpatialPlatformCoordinator::StartPollingForGameserverStatus,
												   GetDefault<USpatialGDKSettings>()->SpatialPlatformServerStatusPollingInterval, false);

			if (bWasSuccessful)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s - Successful HTTP request, Response:[%s]"), *FString(__FUNCTION__),
					   *Response->GetContentAsString());

				TSharedPtr<FJsonObject> RootObject;
				FHttpResponsePtr Response = Request->GetResponse();
				FString ResponseStr = Response->GetContentAsString();
				TSharedRef<TJsonReader<> > JsonReader = TJsonReaderFactory<>::Create(ResponseStr);
				if (!FJsonSerializer::Deserialize(JsonReader, RootObject))
				{
					return;
				}

				TSharedPtr<FJsonObject> ObjectMetaJson = RootObject->GetObjectField("object_meta");
				if (!ObjectMetaJson.IsValid())
				{
					return;
				}

				TSharedPtr<FJsonObject> AnnotationsJson = ObjectMetaJson->GetObjectField("annotations");
				if (!AnnotationsJson.IsValid())
				{
					return;
				}

				for (auto JsonValue : AnnotationsJson->Values)
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

				TSharedPtr<FJsonObject> StatusJson = RootObject->GetObjectField("status");
				if (!StatusJson.IsValid())
				{
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
				UE_LOG(LogTemp, Warning, TEXT("%s - Failed HTTP request, Response:[%s], ResponseCode:[%d]"), *FString(__FUNCTION__),
					   *Response->GetContentAsString(), Response->GetResponseCode());
			}
		});

	WorkerStatusPollingRequest->OnRequestProgress().BindLambda(
		[this, SpatialWorkerFlags](FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived) {
			UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));
		});

	WorkerStatusPollingRequest->OnHeaderReceived().BindLambda(
		[this](FHttpRequestPtr Request, const FString& HeaderName, const FString& NewHeaderValue) {
			UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));
		});

	WorkerStatusPollingRequest->OnRequestWillRetry().BindLambda(
		[this](FHttpRequestPtr Request, FHttpResponsePtr Response, float SecondsToRetry) {
			UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));
		});

	// WorkerStatusPollingRequest->SetURL(Url + TEXT("/gameserver"));
	WorkerStatusPollingRequest->SetURL(Url + TEXT("/test.php"));
	WorkerStatusPollingRequest->SetVerb("GET");
	WorkerStatusPollingRequest->SetHeader(TEXT("User-Agent"), "UnrealEngine-GDK-Agent");
	WorkerStatusPollingRequest->SetHeader("Content-Type", TEXT("application/json"));
	WorkerStatusPollingRequest->ProcessRequest();
}

void USpatialPlatformCoordinator::StartWatchingForGameserverStatus()
{
	if (!GetDefault<USpatialGDKSettings>()->bEnableSpatialPlatformCoordinator)
	{
		return;
	}

	USpatialWorkerFlags* SpatialWorkerFlags = Driver->SpatialWorkerFlags;

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> WorkerStatusPollingRequest = FHttpModule::Get().CreateRequest();
	WorkerStatusPollingRequest->OnProcessRequestComplete().BindLambda(
		[this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
			UE_LOG(LogTemp, Warning, TEXT("%s - Response:[%s]"), *FString(__FUNCTION__), *Response->GetContentAsString());
		});
	WorkerStatusPollingRequest->OnRequestProgress().BindLambda(
		[this, SpatialWorkerFlags](FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived) {
			FHttpResponsePtr Response = Request->GetResponse();
			FString ResponseStr = Response->GetContentAsString();
			UE_LOG(LogTemp, Warning, TEXT("%s - Response:[%s]"), *FString(__FUNCTION__), *ResponseStr);
		});
	WorkerStatusPollingRequest->OnHeaderReceived().BindLambda(
		[this](FHttpRequestPtr Request, const FString& HeaderName, const FString& NewHeaderValue) {
			UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));
		});
	WorkerStatusPollingRequest->OnRequestWillRetry().BindLambda(
		[this](FHttpRequestPtr Request, FHttpResponsePtr Response, float SecondsToRetry) {
			UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));
		});

	WorkerStatusPollingRequest->SetURL(Url + TEXT("/watch/gameserver"));
	WorkerStatusPollingRequest->SetVerb("GET");
	WorkerStatusPollingRequest->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	WorkerStatusPollingRequest->SetHeader("Content-Type", TEXT("application/json"));
	WorkerStatusPollingRequest->ProcessRequest();
}
