// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

// TODO(Alex): move `Worker_` code to SpatialWorkerConnection
#include "WorkerConnection.h"

#include "SpatialWorkerConnection.h"

#include "HAL/RunnableThread.h"
#include "Async/Async.h"

USpatialWorkerConnection::USpatialWorkerConnection(const FObjectInitializer & ObjectInitializer /*= FObjectInitializer::Get()*/)
{
	WorkerConnectionCallbacks = NewObject<USpatialWorkerConnectionCallbacks>();
	WorkerConnectionImpl = MakeUnique<RealWorkerConnection>();
}

void USpatialWorkerConnection::FinishDestroy()
{
	DestroyConnection();

	Super::FinishDestroy();
}

void USpatialWorkerConnection::DestroyConnection()
{
	Stop();
	if (OpsProcessingThread != nullptr)
	{
		OpsProcessingThread->WaitForCompletion();
		OpsProcessingThread = nullptr;
	}

	if (WorkerConnectionImpl != nullptr)
	{
		// TODO(Alex): is it needed
		WorkerConnectionImpl->DestroyConnection();
		WorkerConnectionImpl = nullptr;
	}

	bIsConnected = false;
	KeepRunning.AtomicSet(true);
}

PhysicalWorkerName USpatialWorkerConnection::GetWorkerId() const
{
	return WorkerConnectionImpl->GetWorkerId();
}

void USpatialWorkerConnection::CacheWorkerAttributes()
{
	WorkerConnectionImpl->CacheWorkerAttributes();
}

Worker_RequestId USpatialWorkerConnection::SendCommandRequest(Worker_EntityId EntityId, const Worker_CommandRequest* Request, uint32_t CommandId)
{
	return WorkerConnectionImpl->SendCommandRequest(EntityId, Request, CommandId);
}

void USpatialWorkerConnection::SendComponentUpdate(Worker_EntityId EntityId, const Worker_ComponentUpdate* ComponentUpdate, const TraceKey Key /*= USpatialLatencyTracer::InvalidTraceKey*/)
{
	WorkerConnectionImpl->SendComponentUpdate(EntityId, ComponentUpdate, Key);
}

Worker_RequestId USpatialWorkerConnection::SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery)
{
	return WorkerConnectionImpl->SendEntityQueryRequest(EntityQuery);
}

Worker_RequestId USpatialWorkerConnection::SendReserveEntityIdsRequest(uint32_t NumOfEntities)
{
	return WorkerConnectionImpl->SendReserveEntityIdsRequest(NumOfEntities);
}

Worker_RequestId USpatialWorkerConnection::SendDeleteEntityRequest(Worker_EntityId EntityId)
{
	return WorkerConnectionImpl->SendDeleteEntityRequest(EntityId);
}

Worker_RequestId USpatialWorkerConnection::SendCreateEntityRequest(TArray<Worker_ComponentData>&& Components, const Worker_EntityId* EntityId)
{
	return WorkerConnectionImpl->SendCreateEntityRequest(MoveTemp(Components), EntityId);
}

void USpatialWorkerConnection::SendAddComponent(Worker_EntityId EntityId, Worker_ComponentData* ComponentData)
{
	WorkerConnectionImpl->SendAddComponent(EntityId, ComponentData);
}

void USpatialWorkerConnection::SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	WorkerConnectionImpl->SendRemoveComponent(EntityId, ComponentId);
}

void USpatialWorkerConnection::SendCommandResponse(Worker_RequestId RequestId, const Worker_CommandResponse* Response)
{
	WorkerConnectionImpl->SendCommandResponse(RequestId, Response);
}

void USpatialWorkerConnection::SendCommandFailure(Worker_RequestId RequestId, const FString& Message)
{
	WorkerConnectionImpl->SendCommandFailure(RequestId, Message);
}

void USpatialWorkerConnection::SendComponentInterest(Worker_EntityId EntityId, TArray<Worker_InterestOverride>&& ComponentInterest)
{
	WorkerConnectionImpl->SendComponentInterest(EntityId, MoveTemp(ComponentInterest));
}

void USpatialWorkerConnection::SendMetrics(const SpatialGDK::SpatialMetrics& Metrics)
{
	WorkerConnectionImpl->SendMetrics(Metrics);
}

void USpatialWorkerConnection::SendLogMessage(uint8_t Level, const FName& LoggerName, const TCHAR* Message)
{
	WorkerConnectionImpl->SendLogMessage(Level, LoggerName, Message);
}

const TArray<FString>& USpatialWorkerConnection::GetWorkerAttributes() const
{
	return WorkerConnectionImpl->GetWorkerAttributes();
}

void USpatialWorkerConnection::SetConnectionType(ESpatialConnectionType InConnectionType)
{
	WorkerConnectionImpl->SetConnectionType(InConnectionType);
}

void USpatialWorkerConnection::Connect(bool bInitAsClient, uint32 PlayInEditorID)
{
	if (bIsConnected)
	{
		check(bInitAsClient == bConnectAsClient);
		AsyncTask(ENamedThreads::GameThread, [WeakThis = TWeakObjectPtr<USpatialWorkerConnection>(this)]
			{
				if (WeakThis.IsValid())
				{
					WeakThis->OnConnectionSuccess();
				}
				else
				{
					UE_LOG(LogSpatialWorkerConnection, Error, TEXT("SpatialWorkerConnection is not valid but was already connected."));
				}
			});
		return;
	}

	bConnectAsClient = bInitAsClient;
	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();
	if (SpatialGDKSettings->bUseDevelopmentAuthenticationFlow && bInitAsClient)
	{
		LocatorConfig.WorkerType = SpatialConstants::DefaultClientWorkerType.ToString();
		LocatorConfig.UseExternalIp = true;
		WorkerConnectionImpl->StartDevelopmentAuth(SpatialGDKSettings->DevelopmentAuthenticationToken, bConnectAsClient);
		return;
	}
	else
	{
		TWeakObjectPtr<USpatialWorkerConnection> WeakWorkerConnection(this);
		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [PlayInEditorID, bInitAsClient, WeakWorkerConnection]
		{
			USpatialWorkerConnection* WorkerConnection = WeakWorkerConnection.Get();
			if (WorkerConnection == nullptr)
			{
				return;
			}

			Worker_Connection* NewCAPIWorkerConnection = WorkerConnection->WorkerConnectionImpl->Connect(PlayInEditorID, bInitAsClient);

			if (NewCAPIWorkerConnection != nullptr)
			{
				AsyncTask(ENamedThreads::GameThread, [WeakWorkerConnection, NewCAPIWorkerConnection]
				{
					USpatialWorkerConnection* WorkerConnection = WeakWorkerConnection.Get();
					if (WorkerConnection == nullptr)
					{
						return;
					}

					WorkerConnection->WorkerConnectionImpl->WorkerConnection = NewCAPIWorkerConnection;
					WorkerConnection->CacheWorkerAttributes();
					WorkerConnection->OnConnectionSuccess();
				});
			}
			else
			{
				// TODO: Try to reconnect - UNR-576
				WorkerConnection->OnConnectionFailure();
			}
		});
	}
}

bool USpatialWorkerConnection::IsConnected() const
{
	return bIsConnected;
}

TArray<Worker_OpList*> USpatialWorkerConnection::GetOpList()
{
	return WorkerConnectionImpl->GetOpList();
}

void USpatialWorkerConnection::InitializeOpsProcessingThread()
{
	check(IsInGameThread());

	OpsProcessingThread = FRunnableThread::Create(this, TEXT("SpatialWorkerConnectionWorker"), 0);
	check(OpsProcessingThread);
}

void USpatialWorkerConnection::OnConnectionSuccess()
{
	bIsConnected = true;

	if (OpsProcessingThread == nullptr)
	{
		InitializeOpsProcessingThread();
	}

	WorkerConnectionCallbacks->OnConnectedCallback.ExecuteIfBound();
}

void USpatialWorkerConnection::OnConnectionFailure()
{
	bIsConnected = false;

	uint8_t ConnectionStatusCode;
	FString ErrorMessage;
	WorkerConnectionImpl->GetErrorCodeAndMessage(ConnectionStatusCode, ErrorMessage);
	WorkerConnectionCallbacks->OnFailedToConnectCallback.ExecuteIfBound(ConnectionStatusCode, ErrorMessage);
}

bool USpatialWorkerConnection::Init()
{
	OpsUpdateInterval = 1.0f / GetDefault<USpatialGDKSettings>()->OpsUpdateRate;

	return true;
}

uint32 USpatialWorkerConnection::Run()
{
	while (KeepRunning)
	{
		FPlatformProcess::Sleep(OpsUpdateInterval);

		WorkerConnectionImpl->QueueLatestOpList();

		WorkerConnectionImpl->ProcessOutgoingMessages();
	}

	return 0;
}

void USpatialWorkerConnection::Stop()
{
	KeepRunning.AtomicSet(false);
}
