
#include "SpatialConnection.h"
#include <Async.h>

void USpatialConnection::Connect(ReceptionistConfig Config)
{
	// TODO: Move this back to the helper function below
	Worker_ComponentVtable DefaultVtable = {};

	Worker_ConnectionParameters ConnectionParams = Worker_DefaultConnectionParameters();
	ConnectionParams.worker_type = TCHAR_TO_ANSI(*Config.WorkerType);
	ConnectionParams.enable_protocol_logging_at_startup = Config.EnableProtocolLoggingAtStartup;
	ConnectionParams.component_vtable_count = 0;
	ConnectionParams.default_component_vtable = &DefaultVtable;
	ConnectionParams.network.connection_type = Config.LinkProtocol;
	ConnectionParams.network.use_external_ip = Config.UseExternalIp;

	Worker_ConnectionFuture* ConnectionFuture = Worker_ConnectAsync(
		TCHAR_TO_ANSI(*Config.ReceptionistHost), Config.ReceptionistPort,
		TCHAR_TO_ANSI(*Config.WorkerId), &ConnectionParams);

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [ConnectionFuture, this]
	{
		this->Connection = Worker_ConnectionFuture_Get(ConnectionFuture, nullptr);

		Worker_ConnectionFuture_Destroy(ConnectionFuture);

		AsyncTask(ENamedThreads::GameThread, [this]{ this->OnConnected.ExecuteIfBound(); });
	});
}

void USpatialConnection::Connect(LocatorConfig Config)
{
	Worker_ConnectionParameters ConnectionParams = CreateConnectionParameters(Config);

	Locator = Worker_Locator_Create(TCHAR_TO_ANSI(*Config.LocatorHost), &Config.LocatorParameters);

	Worker_DeploymentListFuture* DeploymentListFuture = Worker_Locator_GetDeploymentListAsync(Locator);
	/*Worker_DeploymentListFuture_Get(DeploymentListFuture, nullptr, nullptr, 
		[ConnectionParams, this](void* UserData, Worker_DeploymentList* DeploymentList)
	{
		if(DeploymentList->error != nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("%s"), DeploymentList->error);
		}

		if(DeploymentList->deployment_count == 0)
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

bool USpatialConnection::IsConnected()
{
	if(Connection != nullptr)
	{
		return Worker_Connection_IsConnected(Connection) != 0;
	}

	return false;
}

Worker_ConnectionParameters USpatialConnection::CreateConnectionParameters(ConnectionConfig& Config)
{
	Worker_NetworkParameters NetworkParams;
	NetworkParams.connection_type = Config.LinkProtocol;
	NetworkParams.use_external_ip = Config.UseExternalIp;

	Worker_ComponentVtable DefaultVtable = {};

	Worker_ConnectionParameters ConnectionParams;
	ConnectionParams.worker_type = TCHAR_TO_ANSI(*Config.WorkerType);
	ConnectionParams.enable_protocol_logging_at_startup = Config.EnableProtocolLoggingAtStartup;
	ConnectionParams.default_component_vtable = &DefaultVtable;
	ConnectionParams.network = NetworkParams;

	return ConnectionParams;
}

Worker_OpList* USpatialConnection::GetOpList(uint32_t TimeoutMillis)
{
	return Worker_Connection_GetOpList(Connection, TimeoutMillis);
}

Worker_RequestId USpatialConnection::SendReserveEntityIdRequest(const uint32_t* TimeoutMillis)
{
	return Worker_Connection_SendReserveEntityIdRequest(Connection, TimeoutMillis);
}

Worker_RequestId USpatialConnection::SendCreateEntityRequest(uint32_t ComponentCount, const Worker_ComponentData* Components, const Worker_EntityId* EntityId, const uint32_t* TimeoutMillis)
{
	return Worker_Connection_SendCreateEntityRequest(Connection, ComponentCount, Components, EntityId, TimeoutMillis);
}

Worker_RequestId USpatialConnection::SendDeleteEntityRequest(Worker_EntityId EntityId, const uint32_t* TimeoutMillis)
{
	return Worker_Connection_SendDeleteEntityRequest(Connection, EntityId, TimeoutMillis);
}

void USpatialConnection::SendComponentUpdate(Worker_EntityId EntityId, const Worker_ComponentUpdate* ComponentUpdate)
{
	Worker_Connection_SendComponentUpdate(Connection, EntityId, ComponentUpdate);
}

Worker_RequestId USpatialConnection::SendCommandRequest(Worker_EntityId EntityId, const Worker_CommandRequest* Request, uint32_t CommandId, const uint32_t* TimeoutMillis, const Worker_CommandParameters* CommandParameters)
{
	return Worker_Connection_SendCommandRequest(Connection, EntityId, Request, CommandId, TimeoutMillis, CommandParameters);
}

void USpatialConnection::SendCommandResponse(Worker_RequestId RequestId, const Worker_CommandResponse* Response)
{
	return Worker_Connection_SendCommandResponse(Connection, RequestId, Response);
}