// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialWorkerConnection.h"

#include "Async/Async.h"

void USpatialWorkerConnection::FinishDestroy()
{
	if (WorkerConnection)
	{
		Worker_Connection_Destroy(WorkerConnection);
		WorkerConnection = nullptr;
	}

	Super::FinishDestroy();
}

void USpatialWorkerConnection::Connect(ReceptionistConfig Config)
{
	// TODO: Move this back to the helper function below
	Worker_ComponentVtable DefaultVtable = {};

	Worker_ConnectionParameters ConnectionParams = Worker_DefaultConnectionParameters();
	FTCHARToUTF8 WorkerTypeCStr(*Config.WorkerType);
	ConnectionParams.worker_type = WorkerTypeCStr.Get();
	ConnectionParams.enable_protocol_logging_at_startup = Config.EnableProtocolLoggingAtStartup;
	ConnectionParams.component_vtable_count = 0;
	ConnectionParams.default_component_vtable = &DefaultVtable;
	ConnectionParams.network.connection_type = Config.LinkProtocol;
	ConnectionParams.network.use_external_ip = Config.UseExternalIp;

	Worker_ConnectionFuture* ConnectionFuture = Worker_ConnectAsync(
		TCHAR_TO_UTF8(*Config.ReceptionistHost), Config.ReceptionistPort,
		TCHAR_TO_UTF8(*Config.WorkerId), &ConnectionParams);

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [ConnectionFuture, this]
	{
		WorkerConnection = Worker_ConnectionFuture_Get(ConnectionFuture, nullptr);

		Worker_ConnectionFuture_Destroy(ConnectionFuture);

		if (Worker_Connection_IsConnected(WorkerConnection))
		{
			AsyncTask(ENamedThreads::GameThread, [this] {
				OnConnected.ExecuteIfBound();
				bIsConnected = true;
			});
		}
		else
		{
			// TODO: Poll ops for disconnected reason and try to reconnect
		}
	});
}

void USpatialWorkerConnection::Connect(LocatorConfig Config)
{
	Worker_ConnectionParameters ConnectionParams = CreateConnectionParameters(Config);

	Locator = Worker_Locator_Create(TCHAR_TO_UTF8(*Config.LocatorHost), &Config.LocatorParameters);

	Worker_DeploymentListFuture* DeploymentListFuture = Worker_Locator_GetDeploymentListAsync(Locator);
	/*Worker_DeploymentListFuture_Get(DeploymentListFuture, nullptr, nullptr,
		[ConnectionParams, this](void* UserData, Worker_DeploymentList* DeploymentList)
	{
		if (DeploymentList->error != nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("%s"), DeploymentList->error);
		}

		if (DeploymentList->deployment_count == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("Received empty list of deployments. Failed to connect."));
		}

		Worker_ConnectionFuture* ConnectionFuture = Worker_Locator_ConnectAsync(Locator, DeploymentList->deployments[0].deployment_name,
			&ConnectionParams, nullptr, nullptr);

		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [ConnectionFuture, this]
		{
			this->Connection = Worker_ConnectionFuture_Get(ConnectionFuture, nullptr);

			Worker_ConnectionFuture_Destroy(ConnectionFuture);

			AsyncTask(ENamedThreads::GameThread, [this] { this->OnConnected.ExecuteIfBound(); });
		});
	});*/
}

bool USpatialWorkerConnection::IsConnected()
{
	return bIsConnected;
}

Worker_ConnectionParameters USpatialWorkerConnection::CreateConnectionParameters(ConnectionConfig& Config)
{
	Worker_NetworkParameters NetworkParams;
	NetworkParams.connection_type = Config.LinkProtocol;
	NetworkParams.use_external_ip = Config.UseExternalIp;

	Worker_ComponentVtable DefaultVtable = {};

	Worker_ConnectionParameters ConnectionParams;
	ConnectionParams.worker_type = TCHAR_TO_UTF8(*Config.WorkerType);
	ConnectionParams.enable_protocol_logging_at_startup = Config.EnableProtocolLoggingAtStartup;
	ConnectionParams.default_component_vtable = &DefaultVtable;
	ConnectionParams.network = NetworkParams;

	return ConnectionParams;
}

Worker_OpList* USpatialWorkerConnection::GetOpList()
{
	return Worker_Connection_GetOpList(WorkerConnection, 0);
}

Worker_RequestId USpatialWorkerConnection::SendReserveEntityIdRequest()
{
	return Worker_Connection_SendReserveEntityIdRequest(WorkerConnection, nullptr);
}

Worker_RequestId USpatialWorkerConnection::SendCreateEntityRequest(uint32_t ComponentCount, const Worker_ComponentData* Components, const Worker_EntityId* EntityId)
{
	return Worker_Connection_SendCreateEntityRequest(WorkerConnection, ComponentCount, Components, EntityId, nullptr);
}

Worker_RequestId USpatialWorkerConnection::SendDeleteEntityRequest(Worker_EntityId EntityId)
{
	return Worker_Connection_SendDeleteEntityRequest(WorkerConnection, EntityId, nullptr);
}

void USpatialWorkerConnection::SendComponentUpdate(Worker_EntityId EntityId, const Worker_ComponentUpdate* ComponentUpdate)
{
	Worker_Connection_SendComponentUpdate(WorkerConnection, EntityId, ComponentUpdate);
}

Worker_RequestId USpatialWorkerConnection::SendCommandRequest(Worker_EntityId EntityId, const Worker_CommandRequest* Request, uint32_t CommandId)
{
	Worker_CommandParameters CommandParams{};
	return Worker_Connection_SendCommandRequest(WorkerConnection, EntityId, Request, CommandId, nullptr, &CommandParams);
}

void USpatialWorkerConnection::SendCommandResponse(Worker_RequestId RequestId, const Worker_CommandResponse* Response)
{
	return Worker_Connection_SendCommandResponse(WorkerConnection, RequestId, Response);
}

void USpatialWorkerConnection::SendLogMessage(const uint8_t Level, const char* LoggerName, const char* Message)
{
	Worker_LogMessage LogMessage{};
	LogMessage.level = Level;
	LogMessage.logger_name = LoggerName;
	LogMessage.message = Message;

	Worker_Connection_SendLogMessage(WorkerConnection, &LogMessage);
}
