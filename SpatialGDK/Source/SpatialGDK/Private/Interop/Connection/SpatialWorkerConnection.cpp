// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialWorkerConnection.h"
#if WITH_EDITOR
#include "Interop/Connection/EditorWorkerController.h"
#endif

#include "Async/Async.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Interop/GlobalStateManager.h"
#include "Misc/Paths.h"

#include "EngineClasses/SpatialGameInstance.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "SpatialGDKSettings.h"
#include "Utils/EntityPool.h"
#include "Utils/ErrorCodeRemapping.h"
#include <gdk/spatialos_connection_handler.h>
#include <gdk/initial_op_list_connection_handler.h>
#include <memory>

DEFINE_LOG_CATEGORY(LogSpatialWorkerConnection);

using namespace SpatialGDK;

void USpatialWorkerConnection::Init(USpatialGameInstance* InGameInstance)
{
	GameInstance = InGameInstance;
}

void USpatialWorkerConnection::FinishDestroy()
{
	DestroyConnection();

	Super::FinishDestroy();
}

void USpatialWorkerConnection::DestroyConnection()
{
	bIsConnected = false;
	NextRequestId = 0;
}

void USpatialWorkerConnection::Connect(bool bInitAsClient)
{
#include "Interop/GlobalStateManager.h"
	if (bIsConnected)
	{
		return;
	}

	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();
	if (SpatialGDKSettings->bUseDevelopmentAuthenticationFlow && bInitAsClient)
	{
		LocatorConfig.WorkerType = SpatialConstants::DefaultClientWorkerType.ToString();
		LocatorConfig.UseExternalIp = true;
		StartDevelopmentAuth(SpatialGDKSettings->DevelopmentAuthenticationToken);
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

void USpatialWorkerConnection::OnLoginTokens(void* UserData, const Worker_Alpha_LoginTokensResponse* LoginTokens)
{
	if (LoginTokens->status.code != WORKER_CONNECTION_STATUS_CODE_SUCCESS)
	{
		UE_LOG(LogSpatialWorkerConnection, Error, TEXT("Failed to get login token, StatusCode: %d, Error: %s"), LoginTokens->status.code, UTF8_TO_TCHAR(LoginTokens->status.detail));
		return;
	}

	if (LoginTokens->login_token_count == 0)
	{
		UE_LOG(LogSpatialWorkerConnection, Warning, TEXT("No deployment found to connect to. Did you add the 'dev_login' tag to the deployment you want to connect to?"));
		return;
	}

	UE_LOG(LogSpatialWorkerConnection, Verbose, TEXT("Successfully received LoginTokens, Count: %d"), LoginTokens->login_token_count);
	USpatialWorkerConnection* Connection = static_cast<USpatialWorkerConnection*>(UserData);
	const FString& DeploymentToConnect = GetDefault<USpatialGDKSettings>()->DevelopmentDeploymentToConnect;
	// If not set, use the first deployment. It can change every query if you have multiple items available, because the order is not guaranteed.
	if (DeploymentToConnect.IsEmpty())
	{
		Connection->LocatorConfig.LoginToken = FString(LoginTokens->login_tokens[0].login_token);
	}
	else
	{
		for (uint32 i = 0; i < LoginTokens->login_token_count; i++)
		{
			FString DeploymentName = FString(LoginTokens->login_tokens[i].deployment_name);
			if (DeploymentToConnect.Compare(DeploymentName) == 0)
			{
				Connection->LocatorConfig.LoginToken = FString(LoginTokens->login_tokens[i].login_token);
				break;
			}
		}
	}
	Connection->ConnectToLocator();
}

void USpatialWorkerConnection::OnPlayerIdentityToken(void* UserData, const Worker_Alpha_PlayerIdentityTokenResponse* PIToken)
{
	if (PIToken->status.code != WORKER_CONNECTION_STATUS_CODE_SUCCESS)
	{
		UE_LOG(LogSpatialWorkerConnection, Error, TEXT("Failed to get PlayerIdentityToken, StatusCode: %d, Error: %s"), PIToken->status.code, UTF8_TO_TCHAR(PIToken->status.detail));
		return;
	}

	UE_LOG(LogSpatialWorkerConnection, Log, TEXT("Successfully received PIToken: %s"), UTF8_TO_TCHAR(PIToken->player_identity_token));
	USpatialWorkerConnection* Connection = static_cast<USpatialWorkerConnection*>(UserData);
	Connection->LocatorConfig.PlayerIdentityToken = UTF8_TO_TCHAR(PIToken->player_identity_token);
	Worker_Alpha_LoginTokensRequest LTParams{};
	LTParams.player_identity_token = PIToken->player_identity_token;
	FTCHARToUTF8 WorkerType(*Connection->LocatorConfig.WorkerType);
	LTParams.worker_type = WorkerType.Get();
	LTParams.use_insecure_connection = false;

	if (Worker_Alpha_LoginTokensResponseFuture* LTFuture = Worker_Alpha_CreateDevelopmentLoginTokensAsync(TCHAR_TO_UTF8(*Connection->LocatorConfig.LocatorHost), SpatialConstants::LOCATOR_PORT, &LTParams))
	{
		Worker_Alpha_LoginTokensResponseFuture_Get(LTFuture, nullptr, Connection, &USpatialWorkerConnection::OnLoginTokens);
	}
}

void USpatialWorkerConnection::StartDevelopmentAuth(FString DevAuthToken)
{
	Worker_Alpha_PlayerIdentityTokenRequest PITParams{};
	FTCHARToUTF8 DAToken(*DevAuthToken);
	FTCHARToUTF8 PlayerId(*SpatialConstants::DEVELOPMENT_AUTH_PLAYER_ID);
	PITParams.development_authentication_token = DAToken.Get();
	PITParams.player_id = PlayerId.Get();
	PITParams.display_name = "";
	PITParams.metadata = "";
	PITParams.use_insecure_connection = false;

	if (Worker_Alpha_PlayerIdentityTokenResponseFuture* PITFuture = Worker_Alpha_CreateDevelopmentPlayerIdentityTokenAsync(TCHAR_TO_UTF8(*LocatorConfig.LocatorHost), SpatialConstants::LOCATOR_PORT, &PITParams))
	{
		Worker_Alpha_PlayerIdentityTokenResponseFuture_Get(PITFuture, nullptr, this, &USpatialWorkerConnection::OnPlayerIdentityToken);
	}
}

void USpatialWorkerConnection::ConnectToReceptionist(bool bConnectAsClient)
{
	if (ReceptionistConfig.WorkerType.IsEmpty())
	{
		ReceptionistConfig.WorkerType = bConnectAsClient ? SpatialConstants::DefaultClientWorkerType.ToString() : SpatialConstants::DefaultServerWorkerType.ToString();
		UE_LOG(LogSpatialWorkerConnection, Warning, TEXT("No worker type specified through commandline, defaulting to %s"), *ReceptionistConfig.WorkerType);
	}

#if WITH_EDITOR
	SpatialGDKServices::InitWorkers(bConnectAsClient, GetSpatialNetDriverChecked()->PlayInEditorID, ReceptionistConfig.WorkerId);
#endif

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

	ConnectionParams.enable_dynamic_components = true;
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
		LocatorConfig.WorkerType = SpatialConstants::DefaultClientWorkerType.ToString();
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
	auto* WorkerLocator = Worker_Alpha_Locator_Create(TCHAR_TO_UTF8(*LocatorConfig.LocatorHost), 0, &LocatorParams);

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

	ConnectionParams.enable_dynamic_components = true;
	// end TODO

	Worker_ConnectionFuture* ConnectionFuture = Worker_Alpha_Locator_ConnectAsync(WorkerLocator, &ConnectionParams);
	Worker_Alpha_Locator_Destroy(WorkerLocator);

	FinishConnecting(ConnectionFuture);
}

void USpatialWorkerConnection::FinishConnecting(Worker_ConnectionFuture* ConnectionFuture)
{
	TWeakObjectPtr<USpatialWorkerConnection> WeakSpatialWorkerConnection(this);

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, ConnectionFuture, WeakSpatialWorkerConnection]
	{
		Worker_Connection* NewCAPIWorkerConnection = Worker_ConnectionFuture_Get(ConnectionFuture, nullptr);
		Worker_ConnectionFuture_Destroy(ConnectionFuture);

		AsyncTask(ENamedThreads::GameThread, [this, WeakSpatialWorkerConnection, NewCAPIWorkerConnection]
		{
			USpatialWorkerConnection* SpatialWorkerConnection = WeakSpatialWorkerConnection.Get();

			if (SpatialWorkerConnection == nullptr)
			{
				return;
			}

			gdk::ComponentRanges Ranges;
			const auto GeneratedStart = SpatialConstants::STARTING_GENERATED_COMPONENT_ID;
			// Generated components range.
			Ranges.TryAddComponentRange(gdk::Range{ GeneratedStart, GeneratedStart + 100000 });
			// Non-generated components range.
			Ranges.TryAddComponentRange(gdk::Range{ SpatialConstants::MAX_EXTERNAL_SCHEMA_ID + 1, GeneratedStart - 1});
			// External component range
			Ranges.TryAddComponentRange(gdk::Range{ SpatialConstants::MIN_EXTERNAL_SCHEMA_ID, SpatialConstants::MAX_EXTERNAL_SCHEMA_ID});
			// Standard library
			Ranges.TryAddComponentRange(gdk::Range{ 1, SpatialConstants::MIN_EXTERNAL_SCHEMA_ID - 1 });
			auto InitialOpsFunction = [this](gdk::OpList* OpList, gdk::ExtractedOpList* ExtractedOpList)
			{
				if (GetSpatialNetDriverChecked()->bConnectAsClient)
				{
					return true;
				}
				auto* EntityPool = GetSpatialNetDriverChecked()->EntityPool;
				auto* GlobalStateManager = GetSpatialNetDriverChecked()->GlobalStateManager;
				if (EntityPool == nullptr || GlobalStateManager == nullptr)
				{
					return false;
				}
				const gdk::ComponentId StartUpId = SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID;
				if (GlobalStateManager->IsReadyToCallBeginPlay() && EntityPool->IsReady())
				{
					GlobalStateManager->TriggerBeginPlay();
					return true;
				}
				for (size_t i = 0; i< OpList->GetCount(); ++i)
				{
					auto& op = (*OpList)[i];
					if (op.op_type == WORKER_OP_TYPE_DISCONNECT)
					{
						ExtractedOpList->AddOp(OpList, i);
					}
					if (op.op_type == WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE && !EntityPool->IsReady())
					{
						ExtractedOpList->AddOp(OpList, i);
					}
					if (op.op_type == WORKER_OP_TYPE_ADD_COMPONENT && op.add_component.data.component_id == StartUpId && !GlobalStateManager->IsReadyToCallBeginPlay())
					{
						ExtractedOpList->AddOp(OpList, i);
					}
					if (op.op_type == WORKER_OP_TYPE_COMPONENT_UPDATE && op.component_update.update.component_id == StartUpId && !GlobalStateManager->IsReadyToCallBeginPlay())
					{
						ExtractedOpList->AddOp(OpList, i);
					}
					if (op.op_type == WORKER_OP_TYPE_AUTHORITY_CHANGE && op.authority_change.component_id == StartUpId && !GlobalStateManager->IsReadyToCallBeginPlay())
					{
						ExtractedOpList->AddOp(OpList, i);
					}
				}
				return false;
			};

			auto SpatialOSConnectionHandler = std::make_unique<gdk::SpatialOsConnectionHandler>(NewCAPIWorkerConnection);
			auto InitialOpsConnectionHandler = std::make_unique<gdk::InitialOpListConnectionHandler>(MoveTemp(SpatialOSConnectionHandler), InitialOpsFunction);
			SpatialWorkerConnection->Worker = MakeUnique<gdk::SpatialOsWorker>(MoveTemp(InitialOpsConnectionHandler), Ranges);

			if (Worker_Connection_IsConnected(NewCAPIWorkerConnection))
			{
				SpatialWorkerConnection->CacheWorkerAttributes();
				SpatialWorkerConnection->OnConnectionSuccess(NewCAPIWorkerConnection);
			}
			else
			{
				// TODO: Try to reconnect - UNR-576
				SpatialWorkerConnection->OnConnectionFailure(NewCAPIWorkerConnection);
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

	return OpLists;
}

const gdk::SpatialOsWorker& USpatialWorkerConnection::GetWorker() const
{
	return *Worker.Get();
}

Worker_RequestId USpatialWorkerConnection::SendReserveEntityIdsRequest(uint32_t NumOfEntities)
{
	Worker->SendReserveEntityIdsRequest(NextRequestId, NumOfEntities, 0);
	return NextRequestId++;
}

Worker_RequestId USpatialWorkerConnection::SendCreateEntityRequest(TArray<Worker_ComponentData>&& Components, const Worker_EntityId* EntityId)
{
	gdk::EntityState EntityState;
	for (auto& Component : Components)
	{
		EntityState.AddComponent(gdk::ComponentData{ Component.schema_type });
	}

	gdk::EntityId Id = EntityId == nullptr ? 0 : *EntityId;
	Worker->SendCreateEntityRequest(NextRequestId, MoveTemp(EntityState), Id, 0);
	return NextRequestId++;
}

Worker_RequestId USpatialWorkerConnection::SendDeleteEntityRequest(Worker_EntityId EntityId)
{
	Worker->SendDeleteEntityRequest(NextRequestId, EntityId, 0);
	return NextRequestId++;
}

void USpatialWorkerConnection::SendAddComponent(Worker_EntityId EntityId, Worker_ComponentData* ComponentData)
{
	Worker->AddComponent(EntityId, gdk::ComponentData{ ComponentData->schema_type });
}

void USpatialWorkerConnection::SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	Worker->RemoveComponent(EntityId, ComponentId);
}

void USpatialWorkerConnection::SendComponentUpdate(Worker_EntityId EntityId, const Worker_ComponentUpdate* ComponentUpdate)
{
	Worker->SendUpdate(EntityId, gdk::ComponentUpdate{ ComponentUpdate->schema_type });
}

Worker_RequestId USpatialWorkerConnection::SendCommandRequest(Worker_EntityId EntityId, const Worker_CommandRequest* Request, uint32_t CommandId)
{
	Worker->SendCommandRequest(EntityId, NextRequestId, gdk::CommandRequest{ Request->schema_type }, 0);
	return NextRequestId++;
}

void USpatialWorkerConnection::SendCommandResponse(Worker_RequestId RequestId, const Worker_CommandResponse* Response)
{
	Worker->SendCommandResponse(RequestId, gdk::CommandResponse{ Response->schema_type });
}

void USpatialWorkerConnection::SendCommandFailure(Worker_RequestId RequestId, const FString& Message)
{
	Worker->SendCommandFailure(RequestId, std::string{TCHAR_TO_UTF8(*Message)});
}

void USpatialWorkerConnection::SendLogMessage(const uint8_t Level, const FName& LoggerName, const TCHAR* Message)
{
	Worker->SendLogMessage(static_cast<Worker_LogLevel>(Level), 0, std::string{ TCHAR_TO_UTF8(*LoggerName.ToString()) }, std::string{ TCHAR_TO_UTF8(Message) });
}

void USpatialWorkerConnection::SendComponentInterest(Worker_EntityId EntityId, TArray<Worker_InterestOverride>&& ComponentInterest)
{
	std::vector<Worker_InterestOverride> InterestOverrides;
	InterestOverrides.reserve(ComponentInterest.Num());
	for (const auto& Interest : ComponentInterest)
	{
		InterestOverrides.emplace_back(Interest);
	}
	Worker->SendInterestChange(EntityId, MoveTemp(InterestOverrides));
}

Worker_RequestId USpatialWorkerConnection::SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery)
{
	Worker->SendEntityQueryRequest(NextRequestId, gdk::EntityQuery{*EntityQuery}, 0);
	return NextRequestId++;
}

void USpatialWorkerConnection::SendMetrics(const Worker_Metrics& Metrics)
{
	Worker->SendMetricsMessage(gdk::Metrics{ Metrics });
}

FString USpatialWorkerConnection::GetWorkerId() const
{
	return FString(UTF8_TO_TCHAR(Worker->GetWorkerId().c_str()));
}

const TArray<FString>& USpatialWorkerConnection::GetWorkerAttributes() const
{
	return CachedWorkerAttributes;
}

void USpatialWorkerConnection::FlushMessageToSend()
{
	Worker->FlushSend();
}

void USpatialWorkerConnection::Advance()
{
	Worker->Advance();
}

void USpatialWorkerConnection::CacheWorkerAttributes()
{
	CachedWorkerAttributes.Empty();
	for (const auto& attribute : Worker->GetWorkerAttributes())
	{
		CachedWorkerAttributes.Emplace(UTF8_TO_TCHAR( attribute.c_str()));
	}
}

USpatialNetDriver* USpatialWorkerConnection::GetSpatialNetDriverChecked() const
{
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

void USpatialWorkerConnection::OnConnectionSuccess(Worker_Connection* Connection)
{
	bIsConnected = true;

	GetSpatialNetDriverChecked()->OnConnectedToSpatialOS();
	GameInstance->HandleOnConnected();
}

void USpatialWorkerConnection::OnPreConnectionFailure(const FString& Reason)
{
	bIsConnected = false;
	GameInstance->HandleOnConnectionFailed(Reason);
}

void USpatialWorkerConnection::OnConnectionFailure(Worker_Connection* Connection)
{
	bIsConnected = false;

	if (GEngine != nullptr && GameInstance->GetWorld() != nullptr)
	{
		uint8_t ConnectionStatusCode = Worker_Connection_GetConnectionStatusCode(Connection);
		const FString ErrorMessage(UTF8_TO_TCHAR(Worker_Connection_GetConnectionStatusDetailString(Connection)));

		GEngine->BroadcastNetworkFailure(GameInstance->GetWorld(), GetSpatialNetDriverChecked(), ENetworkFailure::FromDisconnectOpStatusCode(ConnectionStatusCode), *ErrorMessage);
	}
}
