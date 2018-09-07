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

	if(WorkerLocator)
	{
		Worker_Locator_Destroy(WorkerLocator);
	}

	Super::FinishDestroy();
}

void USpatialWorkerConnection::Connect(bool bInitAsClient)
{
	if(ShouldConnectWithLocator())
	{
		ConnectToLocator();
	}
	else
	{
		ConnectToReceptionist(bInitAsClient);
	}

}

void USpatialWorkerConnection::ConnectToReceptionist(bool bConnectAsClient)
{
	if(ConfigReceptionist.WorkerType.IsEmpty())
	{
		ConfigReceptionist.WorkerType = bConnectAsClient ? TEXT("UnrealClient") : TEXT("UnrealWorker");
	}

	if(ConfigReceptionist.WorkerId.IsEmpty())
	{
		ConfigReceptionist.WorkerId = ConfigReceptionist.WorkerType + FGuid::NewGuid().ToString();
	}

	// TODO: Move creation of connection parameters into a function somehow
	Worker_ConnectionParameters ConnectionParams = Worker_DefaultConnectionParameters();
	FTCHARToUTF8 WorkerTypeCStr(*ConfigReceptionist.WorkerType);
	ConnectionParams.worker_type = WorkerTypeCStr.Get();
	ConnectionParams.enable_protocol_logging_at_startup = ConfigReceptionist.EnableProtocolLoggingAtStartup;

	Worker_ComponentVtable DefaultVtable = {};
	ConnectionParams.component_vtable_count = 0;
	ConnectionParams.default_component_vtable = &DefaultVtable;

	ConnectionParams.network.connection_type = ConfigReceptionist.LinkProtocol;
	ConnectionParams.network.use_external_ip = ConfigReceptionist.UseExternalIp;
	// end TODO

	Worker_ConnectionFuture* ConnectionFuture = Worker_ConnectAsync(
			TCHAR_TO_UTF8(*ConfigReceptionist.ReceptionistHost), ConfigReceptionist.ReceptionistPort,
			TCHAR_TO_UTF8(*ConfigReceptionist.WorkerId), &ConnectionParams);

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [ConnectionFuture, this]
	{
		WorkerConnection = Worker_ConnectionFuture_Get(ConnectionFuture, nullptr);

		Worker_ConnectionFuture_Destroy(ConnectionFuture);
		if (Worker_Connection_IsConnected(WorkerConnection))
		{
			AsyncTask(ENamedThreads::GameThread, [this] 
			{
				this->bIsConnected = true;
				this->OnConnected.ExecuteIfBound();
			});
		}
		else
		{
			Worker_OpList* OpList = Worker_Connection_GetOpList(WorkerConnection, 0);
			for(int i = 0; i < (int)OpList->op_count; i++)
			{
				if(OpList->ops[i].op_type == WORKER_OP_TYPE_DISCONNECT)
				{
					UE_LOG(LogTemp, Error, TEXT("Couldn't connect to SpatialOS: %s"), UTF8_TO_TCHAR(OpList->ops[i].disconnect.reason));
				}
			}
		}
	});
}

void USpatialWorkerConnection::ConnectToLocator()
{
	FTCHARToUTF8 ProjectNameCStr(*ConfigLocator.ProjectName);
	FTCHARToUTF8 LoginTokenCStr(*ConfigLocator.LoginToken);

	Worker_LoginTokenCredentials Credentials;
	Credentials.token = LoginTokenCStr.Get();

	Worker_LocatorParameters LocatorParams;
	LocatorParams.credentials_type = WORKER_LOCATOR_LOGIN_TOKEN_CREDENTIALS;
	LocatorParams.project_name = ProjectNameCStr.Get();
	LocatorParams.login_token = Credentials;

	WorkerLocator = Worker_Locator_Create(TCHAR_TO_UTF8(*ConfigLocator.LocatorHost), &LocatorParams);

	Worker_DeploymentListFuture* DeploymentListFuture = Worker_Locator_GetDeploymentListAsync(WorkerLocator);
	Worker_DeploymentListFuture_Get(DeploymentListFuture, nullptr, this,
		[](void* UserData, const Worker_DeploymentList* DeploymentList)
	{
		if (DeploymentList->error != nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("%s"), DeploymentList->error);
		}

		if (DeploymentList->deployment_count == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("Received empty list of deployments. Failed to connect."));
		}

		USpatialWorkerConnection* SpatialConnection = static_cast<USpatialWorkerConnection*>(UserData); 

		// TODO: Move creation of connection parameters into a function somehow
		Worker_ConnectionParameters ConnectionParams = Worker_DefaultConnectionParameters();
		FTCHARToUTF8 WorkerTypeCStr(*SpatialConnection->ConfigLocator.WorkerType);
		ConnectionParams.worker_type = WorkerTypeCStr.Get();
		ConnectionParams.enable_protocol_logging_at_startup = SpatialConnection->ConfigLocator.EnableProtocolLoggingAtStartup;

		Worker_ComponentVtable DefaultVtable = {};
		ConnectionParams.component_vtable_count = 0;
		ConnectionParams.default_component_vtable = &DefaultVtable;

		ConnectionParams.network.connection_type = SpatialConnection->ConfigLocator.LinkProtocol;
		ConnectionParams.network.use_external_ip = SpatialConnection->ConfigLocator.UseExternalIp;
		// end TODO

		Worker_ConnectionFuture* ConnectionFuture = Worker_Locator_ConnectAsync(SpatialConnection->WorkerLocator, DeploymentList->deployments[0].deployment_name,
			&ConnectionParams, nullptr, nullptr);

		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [ConnectionFuture, SpatialConnection]
		{
			SpatialConnection->WorkerConnection = Worker_ConnectionFuture_Get(ConnectionFuture, nullptr);

			Worker_ConnectionFuture_Destroy(ConnectionFuture);

			AsyncTask(ENamedThreads::GameThread, [SpatialConnection] 
			{ 
				SpatialConnection->bIsConnected = true;
				SpatialConnection->OnConnected.ExecuteIfBound();
			});
		});
	});
}

bool USpatialWorkerConnection::ShouldConnectWithLocator()
{
	const TCHAR* commandLine = FCommandLine::Get();
	FString str;
	return FParse::Value(commandLine, *FString("loginToken"), str);
}

bool USpatialWorkerConnection::IsConnected()
{
	return bIsConnected;
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
