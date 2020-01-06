// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

// TODO(Alex): move `Worker_` code to SpatialWorkerConnection
#include "WorkerConnection.h"

#include "SpatialWorkerConnection.h"

#include "HAL/RunnableThread.h"
#include "Async/Async.h"

UWorkerConnection::UWorkerConnection(const FObjectInitializer & ObjectInitializer /*= FObjectInitializer::Get()*/)
{
	WorkerConnectionCallbacks = NewObject<UWorkerConnectionCallbacks>();
	WorkerConnectionImpl = MakeUnique<USpatialWorkerConnection>();
}

void UWorkerConnection::FinishDestroy()
{
	DestroyConnection();

	Super::FinishDestroy();
}

void UWorkerConnection::DestroyConnection()
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

PhysicalWorkerName UWorkerConnection::GetWorkerId() const
{
	return WorkerConnectionImpl->GetWorkerId();
}

Worker_RequestId UWorkerConnection::SendCommandRequest(Worker_EntityId EntityId, const Worker_CommandRequest* Request, uint32_t CommandId)
{
	return WorkerConnectionImpl->SendCommandRequest(EntityId, Request, CommandId);
}

void UWorkerConnection::SendComponentUpdate(Worker_EntityId EntityId, const Worker_ComponentUpdate* ComponentUpdate, const TraceKey Key /*= USpatialLatencyTracer::InvalidTraceKey*/)
{
	WorkerConnectionImpl->SendComponentUpdate(EntityId, ComponentUpdate, Key);
}

Worker_RequestId UWorkerConnection::SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery)
{
	return WorkerConnectionImpl->SendEntityQueryRequest(EntityQuery);
}

Worker_RequestId UWorkerConnection::SendReserveEntityIdsRequest(uint32_t NumOfEntities)
{
	return WorkerConnectionImpl->SendReserveEntityIdsRequest(NumOfEntities);
}

Worker_RequestId UWorkerConnection::SendDeleteEntityRequest(Worker_EntityId EntityId)
{
	return WorkerConnectionImpl->SendDeleteEntityRequest(EntityId);
}

Worker_RequestId UWorkerConnection::SendCreateEntityRequest(TArray<Worker_ComponentData>&& Components, const Worker_EntityId* EntityId)
{
	return WorkerConnectionImpl->SendCreateEntityRequest(MoveTemp(Components), EntityId);
}

void UWorkerConnection::SendAddComponent(Worker_EntityId EntityId, Worker_ComponentData* ComponentData)
{
	WorkerConnectionImpl->SendAddComponent(EntityId, ComponentData);
}

void UWorkerConnection::SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	WorkerConnectionImpl->SendRemoveComponent(EntityId, ComponentId);
}

void UWorkerConnection::SendCommandResponse(Worker_RequestId RequestId, const Worker_CommandResponse* Response)
{
	WorkerConnectionImpl->SendCommandResponse(RequestId, Response);
}

void UWorkerConnection::SendComponentInterest(Worker_EntityId EntityId, TArray<Worker_InterestOverride>&& ComponentInterest)
{
	WorkerConnectionImpl->SendComponentInterest(EntityId, MoveTemp(ComponentInterest));
}

void UWorkerConnection::SendMetrics(const SpatialGDK::SpatialMetrics& Metrics)
{
	WorkerConnectionImpl->SendMetrics(Metrics);
}

void UWorkerConnection::SendLogMessage(uint8_t Level, const FName& LoggerName, const TCHAR* Message)
{
	WorkerConnectionImpl->SendLogMessage(Level, LoggerName, Message);
}

const TArray<FString>& UWorkerConnection::GetWorkerAttributes() const
{
	return WorkerConnectionImpl->GetWorkerAttributes();
}

void UWorkerConnection::SetConnectionType(ESpatialConnectionType InConnectionType)
{
	WorkerConnectionImpl->SetConnectionType(InConnectionType);
}

void UWorkerConnection::Connect(bool bInitAsClient, uint32 PlayInEditorID)
{
	if (bIsConnected)
	{
		check(bInitAsClient == bConnectAsClient);
		AsyncTask(ENamedThreads::GameThread, [WeakThis = TWeakObjectPtr<UWorkerConnection>(this)]
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

	Worker_ConnectionFuture* ConnectionFuture = nullptr;
	switch (WorkerConnectionImpl->GetConnectionType())
	{
	case ESpatialConnectionType::Receptionist:
		ConnectionFuture = WorkerConnectionImpl->ConnectToReceptionist(PlayInEditorID, bConnectAsClient);
		break;
	case ESpatialConnectionType::Locator:
		ConnectionFuture = WorkerConnectionImpl->ConnectToLocator(bConnectAsClient);
		break;
	}

	FinishConnecting(ConnectionFuture);
}

bool UWorkerConnection::IsConnected() const
{
	return bIsConnected;
}

TArray<Worker_OpList*> UWorkerConnection::GetOpList()
{
	return WorkerConnectionImpl->GetOpList();
}

void UWorkerConnection::InitializeOpsProcessingThread()
{
	check(IsInGameThread());

	OpsProcessingThread = FRunnableThread::Create(this, TEXT("SpatialWorkerConnectionWorker"), 0);
	check(OpsProcessingThread);
}

void UWorkerConnection::OnConnectionSuccess()
{
	bIsConnected = true;

	if (OpsProcessingThread == nullptr)
	{
		InitializeOpsProcessingThread();
	}

	WorkerConnectionCallbacks->OnConnectedCallback.ExecuteIfBound();
}

void UWorkerConnection::OnConnectionFailure()
{
	bIsConnected = false;

	uint8_t ConnectionStatusCode;
	FString ErrorMessage;
	WorkerConnectionImpl->GetErrorCodeAndMessage(ConnectionStatusCode, ErrorMessage);
	WorkerConnectionCallbacks->OnFailedToConnectCallback.ExecuteIfBound(ConnectionStatusCode, ErrorMessage);
}

bool UWorkerConnection::Init()
{
	OpsUpdateInterval = 1.0f / GetDefault<USpatialGDKSettings>()->OpsUpdateRate;

	return true;
}

uint32 UWorkerConnection::Run()
{
	while (KeepRunning)
	{
		FPlatformProcess::Sleep(OpsUpdateInterval);

		WorkerConnectionImpl->QueueLatestOpList();

		WorkerConnectionImpl->ProcessOutgoingMessages();
	}

	return 0;
}

void UWorkerConnection::Stop()
{
	KeepRunning.AtomicSet(false);
}

void UWorkerConnection::FinishConnecting(Worker_ConnectionFuture* ConnectionFuture)
{
	TWeakObjectPtr<UWorkerConnection> WeakWorkerConnection(this);

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [ConnectionFuture, WeakWorkerConnection]
	{
		Worker_Connection* NewCAPIWorkerConnection = Worker_ConnectionFuture_Get(ConnectionFuture, nullptr);
		Worker_ConnectionFuture_Destroy(ConnectionFuture);

		AsyncTask(ENamedThreads::GameThread, [WeakWorkerConnection, NewCAPIWorkerConnection]
		{
			UWorkerConnection* WorkerConnection = WeakWorkerConnection.Get();

			if (WorkerConnection == nullptr)
			{
				return;
			}

			WorkerConnection->WorkerConnectionImpl->WorkerConnection = NewCAPIWorkerConnection;

			if (Worker_Connection_IsConnected(NewCAPIWorkerConnection))
			{
				WorkerConnection->WorkerConnectionImpl->CacheWorkerAttributes();
				WorkerConnection->OnConnectionSuccess();
			}
			else
			{
				// TODO: Try to reconnect - UNR-576
				WorkerConnection->OnConnectionFailure();
			}
		});
	});
}
