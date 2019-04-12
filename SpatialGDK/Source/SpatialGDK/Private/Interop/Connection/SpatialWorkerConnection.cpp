// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialWorkerConnection.h"

#include "Async/Async.h"
#include "Engine/World.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Misc/Paths.h"
#include "SpatialGDKSettings.h"
#include "UnrealEngine.h"
#include "Utils/ErrorCodeRemapping.h"

DEFINE_LOG_CATEGORY(LogSpatialWorkerConnection);

void USpatialWorkerConnection::FinishDestroy()
{
	DestroyConnection();

	Super::FinishDestroy();
}

void USpatialWorkerConnection::DestroyConnection()
{
	Stop(); // Stop worker thread
	if (Thread != nullptr)
	{
		Thread->WaitForCompletion();
		Thread = nullptr;
	}

	if (WorkerConnection)
	{
		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [WorkerConnection = WorkerConnection]
		{
			Worker_Connection_Destroy(WorkerConnection);
		});

		WorkerConnection = nullptr;
	}

	if (WorkerLocator)
	{
		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [WorkerLocator = WorkerLocator]
		{
			Worker_Alpha_Locator_Destroy(WorkerLocator);
		});

		WorkerLocator = nullptr;
	}

	bIsConnected = false;
	NextRequestId = 0;
	KeepRunning.AtomicSet(true);
}

void USpatialWorkerConnection::Connect(bool bInitAsClient)
{
	if (bIsConnected)
	{
		OnConnectionSuccess();
		return;
	}

	switch (GetConnectionType())
	{
	case SpatialConnectionType::Receptionist:
		ConnectToReceptionist(bInitAsClient);
		break;
	case SpatialConnectionType::Locator:
		ConnectToLocator();
		break;
	}
}

void USpatialWorkerConnection::ConnectToReceptionist(bool bConnectAsClient)
{
	if (ReceptionistConfig.WorkerType.IsEmpty())
	{
		ReceptionistConfig.WorkerType = bConnectAsClient ? SpatialConstants::ClientWorkerType : SpatialConstants::ServerWorkerType;
		UE_LOG(LogSpatialWorkerConnection, Warning, TEXT("No worker type specified through commandline, defaulting to %s"), *ReceptionistConfig.WorkerType);
	}

	if (ReceptionistConfig.WorkerId.IsEmpty())
	{
		ReceptionistConfig.WorkerId = ReceptionistConfig.WorkerType + FGuid::NewGuid().ToString();
	}

	// TODO UNR-1271: Move creation of connection parameters into a function somehow
	Worker_ConnectionParameters ConnectionParams = Worker_DefaultConnectionParameters();
	FTCHARToUTF8 WorkerTypeCStr(*ReceptionistConfig.WorkerType);
	ConnectionParams.worker_type = WorkerTypeCStr.Get();
	ConnectionParams.enable_protocol_logging_at_startup = ReceptionistConfig.EnableProtocolLoggingAtStartup;

	FString FinalProtocolLoggingPrefix;
	if (!ReceptionistConfig.ProtocolLoggingPrefix.IsEmpty())
	{
		FinalProtocolLoggingPrefix = ReceptionistConfig.ProtocolLoggingPrefix;
	}
	else
	{
		FinalProtocolLoggingPrefix = ReceptionistConfig.WorkerId;
	}
	FTCHARToUTF8 ProtocolLoggingPrefixCStr(*FinalProtocolLoggingPrefix);
	ConnectionParams.protocol_logging.log_prefix = ProtocolLoggingPrefixCStr.Get();

	Worker_ComponentVtable DefaultVtable = {};
	ConnectionParams.component_vtable_count = 0;
	ConnectionParams.default_component_vtable = &DefaultVtable;

	ConnectionParams.network.connection_type = ReceptionistConfig.LinkProtocol;
	ConnectionParams.network.use_external_ip = ReceptionistConfig.UseExternalIp;
	ConnectionParams.network.tcp.multiplex_level = ReceptionistConfig.TcpMultiplexLevel;
	// end TODO

	Worker_ConnectionFuture* ConnectionFuture = Worker_ConnectAsync(
		TCHAR_TO_UTF8(*ReceptionistConfig.ReceptionistHost), ReceptionistConfig.ReceptionistPort,
		TCHAR_TO_UTF8(*ReceptionistConfig.WorkerId), &ConnectionParams);

	FinishConnecting(ConnectionFuture);
}

void USpatialWorkerConnection::ConnectToLocator()
{
	if (LocatorConfig.WorkerType.IsEmpty())
	{
		LocatorConfig.WorkerType = SpatialConstants::ClientWorkerType;
		UE_LOG(LogSpatialWorkerConnection, Warning, TEXT("No worker type specified through commandline, defaulting to %s"), *LocatorConfig.WorkerType);
	}

	if (LocatorConfig.WorkerId.IsEmpty())
	{
		LocatorConfig.WorkerId = LocatorConfig.WorkerType + FGuid::NewGuid().ToString();
	}

	FTCHARToUTF8 PlayerIdentityTokenCStr(*LocatorConfig.PlayerIdentityToken);
	FTCHARToUTF8 LoginTokenCStr(*LocatorConfig.LoginToken);

	Worker_Alpha_LocatorParameters LocatorParams = {};
	LocatorParams.player_identity.player_identity_token = PlayerIdentityTokenCStr.Get();
	LocatorParams.player_identity.login_token = LoginTokenCStr.Get();

	// Connect to the locator on the default port(0 will choose the default)
	WorkerLocator = Worker_Alpha_Locator_Create(TCHAR_TO_UTF8(*LocatorConfig.LocatorHost), 0, &LocatorParams);

	// TODO UNR-1271: Move creation of connection parameters into a function somehow
	Worker_ConnectionParameters ConnectionParams = Worker_DefaultConnectionParameters();
	FTCHARToUTF8 WorkerTypeCStr(*LocatorConfig.WorkerType);
	ConnectionParams.worker_type = WorkerTypeCStr.Get();
	ConnectionParams.enable_protocol_logging_at_startup = LocatorConfig.EnableProtocolLoggingAtStartup;

	Worker_ComponentVtable DefaultVtable = {};
	ConnectionParams.component_vtable_count = 0;
	ConnectionParams.default_component_vtable = &DefaultVtable;

	ConnectionParams.network.connection_type = LocatorConfig.LinkProtocol;
	ConnectionParams.network.use_external_ip = LocatorConfig.UseExternalIp;
	ConnectionParams.network.tcp.multiplex_level = LocatorConfig.TcpMultiplexLevel;

	FString ProtocolLogDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectLogDir()) + TEXT("protocol-log-");
	ConnectionParams.protocol_logging.log_prefix = TCHAR_TO_UTF8(*ProtocolLogDir);
	// end TODO

	Worker_ConnectionFuture* ConnectionFuture = Worker_Alpha_Locator_ConnectAsync(WorkerLocator, &ConnectionParams);

	FinishConnecting(ConnectionFuture);
}

void USpatialWorkerConnection::FinishConnecting(Worker_ConnectionFuture* ConnectionFuture)
{
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [ConnectionFuture, this]
	{
		Worker_Connection* NewWorkerConnection = Worker_ConnectionFuture_Get(ConnectionFuture, nullptr);
		Worker_ConnectionFuture_Destroy(ConnectionFuture);

		AsyncTask(ENamedThreads::GameThread, [this, NewWorkerConnection]
		{
			this->WorkerConnection = NewWorkerConnection;

			CacheWorkerAttributes();

			if (Worker_Connection_IsConnected(NewWorkerConnection))
			{
				this->OnConnectionSuccess();
			}
			else
			{
				// TODO: Try to reconnect - UNR-576
				this->OnConnectionFailure();
			}
		});
	});
}

SpatialConnectionType USpatialWorkerConnection::GetConnectionType() const
{
	if (!LocatorConfig.PlayerIdentityToken.IsEmpty())
	{
		return SpatialConnectionType::Locator;
	}
	else
	{
		return SpatialConnectionType::Receptionist;
	}
}

TArray<Worker_OpList*> USpatialWorkerConnection::GetOpList()
{
	TArray<Worker_OpList*> OpLists;
	while (!OpListQueue.IsEmpty())
	{
		Worker_OpList* OutOpList;
		OpListQueue.Dequeue(OutOpList);
		OpLists.Add(OutOpList);
	}

	return OpLists;
}

Worker_RequestId USpatialWorkerConnection::SendReserveEntityIdsRequest(uint32_t NumOfEntities)
{
	QueueOutgoingMessage<FReserveEntityIdsRequest>(NumOfEntities);
	return NextRequestId++;
}

Worker_RequestId USpatialWorkerConnection::SendCreateEntityRequest(TArray<Worker_ComponentData>&& Components, const Worker_EntityId* EntityId)
{
	QueueOutgoingMessage<FCreateEntityRequest>(MoveTemp(Components), EntityId);
	return NextRequestId++;
}

Worker_RequestId USpatialWorkerConnection::SendDeleteEntityRequest(Worker_EntityId EntityId)
{
	QueueOutgoingMessage<FDeleteEntityRequest>(EntityId);
	return NextRequestId++;
}

void USpatialWorkerConnection::SendComponentUpdate(Worker_EntityId EntityId, const Worker_ComponentUpdate* ComponentUpdate)
{
	QueueOutgoingMessage<FComponentUpdate>(EntityId, *ComponentUpdate);
}

Worker_RequestId USpatialWorkerConnection::SendCommandRequest(Worker_EntityId EntityId, const Worker_CommandRequest* Request, uint32_t CommandId)
{
	QueueOutgoingMessage<FCommandRequest>(EntityId, *Request, CommandId);
	return NextRequestId++;
}

void USpatialWorkerConnection::SendCommandResponse(Worker_RequestId RequestId, const Worker_CommandResponse* Response)
{
	QueueOutgoingMessage<FCommandResponse>(RequestId, *Response);
}

void USpatialWorkerConnection::SendCommandFailure(Worker_RequestId RequestId, const FString& Message)
{
	QueueOutgoingMessage<FCommandFailure>(RequestId, Message);
}

void USpatialWorkerConnection::SendLogMessage(const uint8_t Level, const FName& LoggerName, const TCHAR* Message)
{
	QueueOutgoingMessage<FLogMessage>(Level, LoggerName, Message);
}

void USpatialWorkerConnection::SendComponentInterest(Worker_EntityId EntityId, TArray<Worker_InterestOverride>&& ComponentInterest)
{
	QueueOutgoingMessage<FComponentInterest>(EntityId, MoveTemp(ComponentInterest));
}

Worker_RequestId USpatialWorkerConnection::SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery)
{
	QueueOutgoingMessage<FEntityQueryRequest>(*EntityQuery);
	return NextRequestId++;
}

void USpatialWorkerConnection::SendMetrics(const Worker_Metrics* Metrics)
{
	QueueOutgoingMessage<FMetrics>(*Metrics);
}

FString USpatialWorkerConnection::GetWorkerId() const
{
	return FString(UTF8_TO_TCHAR(Worker_Connection_GetWorkerId(WorkerConnection)));
}

const TArray<FString>& USpatialWorkerConnection::GetWorkerAttributes() const
{
	return CachedWorkerAttributes;
}

void USpatialWorkerConnection::CacheWorkerAttributes()
{
	const Worker_WorkerAttributes* Attributes = Worker_Connection_GetWorkerAttributes(WorkerConnection);

	CachedWorkerAttributes.Empty();
	for (uint32 Index = 0; Index < Attributes->attribute_count; ++Index)
	{
		CachedWorkerAttributes.Add(UTF8_TO_TCHAR(Attributes->attributes[Index]));
	}
}

USpatialNetDriver* USpatialWorkerConnection::GetSpatialNetDriverChecked() const
{
	UGameInstance* GameInstance = Cast<UGameInstance>(GetOuter());
	UNetDriver* NetDriver = GameInstance->GetWorld()->GetNetDriver();

	// On the client, the world might not be completely set up.
	// in this case we can use the PendingNetGame to get the NetDriver
	if (NetDriver == nullptr)
	{
		NetDriver = GameInstance->GetWorldContext()->PendingNetGame->GetNetDriver();
	}

	USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(NetDriver);
	checkf(SpatialNetDriver, TEXT("SpatialNetDriver was invalid while accessing SpatialNetDriver!"));
	return SpatialNetDriver;
}

void USpatialWorkerConnection::OnConnectionSuccess()
{
	bIsConnected = true;
	InitializeWorkerThread();
	GetSpatialNetDriverChecked()->HandleOnConnected();
}

void USpatialWorkerConnection::OnPreConnectionFailure(const FString& Reason)
{
	bIsConnected = false;
	GetSpatialNetDriverChecked()->HandleOnConnectionFailed(Reason);
}

void USpatialWorkerConnection::OnConnectionFailure()
{
	bIsConnected = false;

	UGameInstance* GameInstance = Cast<UGameInstance>(GetOuter());
	if (GEngine != nullptr && GameInstance->GetWorld() != nullptr)
	{
		uint8_t ConnectionStatusCode = Worker_Connection_GetConnectionStatusCode(WorkerConnection);
		const FString ErrorMessage(UTF8_TO_TCHAR(Worker_Connection_GetConnectionStatusDetailString(WorkerConnection)));

		GEngine->BroadcastNetworkFailure(GameInstance->GetWorld(), GetSpatialNetDriverChecked(), ENetworkFailure::FromDisconnectOpStatusCode(ConnectionStatusCode), *ErrorMessage);
	}
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

		QueueLatestOpList();

		ProcessOutgoingMessages();
	}

	return 0;
}

void USpatialWorkerConnection::Stop()
{
	KeepRunning.AtomicSet(false);
}

void USpatialWorkerConnection::InitializeWorkerThread()
{
	check(IsInGameThread());

	Thread = FRunnableThread::Create(this, TEXT("SpatialWorkerConnectionWorker"), 0);
	check(Thread);
}

void USpatialWorkerConnection::QueueLatestOpList()
{
	OpListQueue.Enqueue(Worker_Connection_GetOpList(WorkerConnection, 0));
}

void USpatialWorkerConnection::ProcessOutgoingMessages()
{
	while (!OutgoingMessagesQueue.IsEmpty())
	{
		TUniquePtr<FOutgoingMessage> OutgoingMessage;
		OutgoingMessagesQueue.Dequeue(OutgoingMessage);

		switch (OutgoingMessage->Type)
		{
		case EOutgoingMessageType::ReserveEntityIdsRequest:
		{
			FReserveEntityIdsRequest* Message = static_cast<FReserveEntityIdsRequest*>(OutgoingMessage.Get());

			Worker_Connection_SendReserveEntityIdsRequest(WorkerConnection,
				Message->NumOfEntities,
				nullptr);
			break;
		}
		case EOutgoingMessageType::CreateEntityRequest:
		{
			FCreateEntityRequest* Message = static_cast<FCreateEntityRequest*>(OutgoingMessage.Get());

			Worker_Connection_SendCreateEntityRequest(WorkerConnection,
				Message->Components.Num(),
				Message->Components.GetData(),
				Message->EntityId.IsSet() ? &(Message->EntityId.GetValue()) : nullptr,
				nullptr);
			break;
		}
		case EOutgoingMessageType::DeleteEntityRequest:
		{
			FDeleteEntityRequest* Message = static_cast<FDeleteEntityRequest*>(OutgoingMessage.Get());

			Worker_Connection_SendDeleteEntityRequest(WorkerConnection,
				Message->EntityId,
				nullptr);
			break;
		}
		case EOutgoingMessageType::ComponentUpdate:
		{
			FComponentUpdate* Message = static_cast<FComponentUpdate*>(OutgoingMessage.Get());

			static const Worker_Alpha_UpdateParameters DisableLoopback{ false /* loopback */ };
			Worker_Alpha_Connection_SendComponentUpdate(WorkerConnection,
				Message->EntityId,
				&Message->Update,
				&DisableLoopback);
			break;
		}
		case EOutgoingMessageType::CommandRequest:
		{
			FCommandRequest* Message = static_cast<FCommandRequest*>(OutgoingMessage.Get());

			static const Worker_CommandParameters DefaultCommandParams{};
			Worker_Connection_SendCommandRequest(WorkerConnection,
				Message->EntityId,
				&Message->Request,
				Message->CommandId,
				nullptr,
				&DefaultCommandParams);
			break;
		}
		case EOutgoingMessageType::CommandResponse:
		{
			FCommandResponse* Message = static_cast<FCommandResponse*>(OutgoingMessage.Get());

			Worker_Connection_SendCommandResponse(WorkerConnection,
				Message->RequestId,
				&Message->Response);
			break;
		}
		case EOutgoingMessageType::CommandFailure:
		{
			FCommandFailure* Message = static_cast<FCommandFailure*>(OutgoingMessage.Get());

			Worker_Connection_SendCommandFailure(WorkerConnection,
				Message->RequestId,
				TCHAR_TO_UTF8(*Message->Message));
			break;
		}
		case EOutgoingMessageType::LogMessage:
		{
			FLogMessage* Message = static_cast<FLogMessage*>(OutgoingMessage.Get());

			Worker_LogMessage LogMessage{};
			LogMessage.level = Message->Level;
			LogMessage.logger_name = TCHAR_TO_UTF8(*Message->LoggerName.ToString());
			LogMessage.message = TCHAR_TO_UTF8(*Message->Message);
			Worker_Connection_SendLogMessage(WorkerConnection, &LogMessage);
			break;
		}
		case EOutgoingMessageType::ComponentInterest:
		{
			FComponentInterest* Message = static_cast<FComponentInterest*>(OutgoingMessage.Get());

			Worker_Connection_SendComponentInterest(WorkerConnection,
				Message->EntityId,
				Message->Interests.GetData(),
				Message->Interests.Num());
			break;
		}
		case EOutgoingMessageType::EntityQueryRequest:
		{
			FEntityQueryRequest* Message = static_cast<FEntityQueryRequest*>(OutgoingMessage.Get());

			Worker_Connection_SendEntityQueryRequest(WorkerConnection,
				&Message->EntityQuery,
				nullptr);
			break;
		}
		case EOutgoingMessageType::Metrics:
		{
			FMetrics* Message = static_cast<FMetrics*>(OutgoingMessage.Get());

			Worker_Connection_SendMetrics(WorkerConnection, &Message->Metrics);
			break;
		}
		default:
		{
			checkNoEntry();
			break;
		}
		}
	}
}

template <typename T, typename... ArgsType>
void USpatialWorkerConnection::QueueOutgoingMessage(ArgsType&&... Args)
{
	// TODO UNR-1271: As later optimization, we can change the queue to hold a union
	// of all outgoing message types, rather than having a pointer.
	OutgoingMessagesQueue.Enqueue(MakeUnique<T>(Forward<ArgsType>(Args)...));
}
