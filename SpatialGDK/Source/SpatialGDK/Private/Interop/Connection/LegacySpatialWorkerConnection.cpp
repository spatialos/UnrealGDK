// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/LegacySpatialWorkerConnection.h"
#include "SpatialView/OpList/WorkerConnectionOpList.h"

#include "Async/Async.h"
#include "SpatialGDKSettings.h"

DEFINE_LOG_CATEGORY(LogSpatialWorkerConnection);

using namespace SpatialGDK;

void ULegacySpatialWorkerConnection::SetConnection(Worker_Connection* WorkerConnectionIn)
{
	WorkerConnection = WorkerConnectionIn;

	CacheWorkerAttributes();

	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();
	if (!SpatialGDKSettings->bRunSpatialWorkerConnectionOnGameThread)
	{
		if (OpsProcessingThread == nullptr)
		{
			bool bCanWake = SpatialGDKSettings->bWorkerFlushAfterOutgoingNetworkOp;
			float WaitTimeS = 1.0f / (GetDefault<USpatialGDKSettings>()->OpsUpdateRate);
			int32 WaitTimeMs = static_cast<int32>(FTimespan::FromSeconds(WaitTimeS).GetTotalMilliseconds());
			if (WaitTimeMs <= 0)
			{
				UE_LOG(LogSpatialWorkerConnection, Warning, TEXT("Clamping wait time for worker ops thread to the minimum rate of 1ms."));
				WaitTimeMs = 1;
			}
			ThreadWaitCondition.Emplace(bCanWake, WaitTimeMs);

			InitializeOpsProcessingThread();
		}
	}
}

void ULegacySpatialWorkerConnection::FinishDestroy()
{
	UE_LOG(LogSpatialWorkerConnection, Log, TEXT("Destroying SpatialWorkerconnection."));

	DestroyConnection();

	Super::FinishDestroy();
}

void ULegacySpatialWorkerConnection::DestroyConnection()
{
	Stop(); // Stop OpsProcessingThread
	if (OpsProcessingThread != nullptr)
	{
		OpsProcessingThread->WaitForCompletion();
		OpsProcessingThread = nullptr;
	}

	ThreadWaitCondition.Reset(); // Set TOptional value to null

	if (WorkerConnection)
	{
		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [WorkerConnection = WorkerConnection] {
			Worker_Connection_Destroy(WorkerConnection);
		});

		WorkerConnection = nullptr;
	}

	NextRequestId = 0;
	KeepRunning.AtomicSet(true);
}

TArray<OpList> ULegacySpatialWorkerConnection::GetOpList()
{
	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();
	if (SpatialGDKSettings->bRunSpatialWorkerConnectionOnGameThread)
	{
		QueueLatestOpList();
	}

	TArray<OpList> OpLists;
	while (!OpListQueue.IsEmpty())
	{
		OpList OutOpList;
		OpListQueue.Dequeue(OutOpList);
		OpLists.Add(MoveTemp(OutOpList));
	}

	return OpLists;
}

Worker_RequestId ULegacySpatialWorkerConnection::SendReserveEntityIdsRequest(uint32_t NumOfEntities)
{
	QueueOutgoingMessage<FReserveEntityIdsRequest>(NumOfEntities);
	return NextRequestId++;
}

Worker_RequestId ULegacySpatialWorkerConnection::SendCreateEntityRequest(TArray<FWorkerComponentData> Components,
																		 const Worker_EntityId* EntityId)
{
	QueueOutgoingMessage<FCreateEntityRequest>(MoveTemp(Components), EntityId);
	return NextRequestId++;
}

Worker_RequestId ULegacySpatialWorkerConnection::SendDeleteEntityRequest(Worker_EntityId EntityId)
{
	QueueOutgoingMessage<FDeleteEntityRequest>(EntityId);
	return NextRequestId++;
}

void ULegacySpatialWorkerConnection::SendAddComponent(Worker_EntityId EntityId, FWorkerComponentData* ComponentData)
{
	QueueOutgoingMessage<FAddComponent>(EntityId, *ComponentData);
}

void ULegacySpatialWorkerConnection::SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	QueueOutgoingMessage<FRemoveComponent>(EntityId, ComponentId);
}

void ULegacySpatialWorkerConnection::SendComponentUpdate(Worker_EntityId EntityId, FWorkerComponentUpdate* ComponentUpdate)
{
	QueueOutgoingMessage<FComponentUpdate>(EntityId, *ComponentUpdate);
}

Worker_RequestId ULegacySpatialWorkerConnection::SendCommandRequest(Worker_EntityId EntityId, Worker_CommandRequest* Request,
																	uint32_t CommandId)
{
	QueueOutgoingMessage<FCommandRequest>(EntityId, *Request, CommandId);
	return NextRequestId++;
}

void ULegacySpatialWorkerConnection::SendCommandResponse(Worker_RequestId RequestId, Worker_CommandResponse* Response)
{
	QueueOutgoingMessage<FCommandResponse>(RequestId, *Response);
}

void ULegacySpatialWorkerConnection::SendCommandFailure(Worker_RequestId RequestId, const FString& Message)
{
	QueueOutgoingMessage<FCommandFailure>(RequestId, Message);
}

void ULegacySpatialWorkerConnection::SendLogMessage(const uint8_t Level, const FName& LoggerName, const TCHAR* Message)
{
	QueueOutgoingMessage<FLogMessage>(Level, LoggerName, Message);
}

void ULegacySpatialWorkerConnection::SendComponentInterest(Worker_EntityId EntityId, TArray<Worker_InterestOverride>&& ComponentInterest)
{
	QueueOutgoingMessage<FComponentInterest>(EntityId, MoveTemp(ComponentInterest));
}

Worker_RequestId ULegacySpatialWorkerConnection::SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery)
{
	QueueOutgoingMessage<FEntityQueryRequest>(*EntityQuery);
	return NextRequestId++;
}

void ULegacySpatialWorkerConnection::SendMetrics(SpatialMetrics Metrics)
{
	QueueOutgoingMessage<FMetrics>(MoveTemp(Metrics));
}

PhysicalWorkerName ULegacySpatialWorkerConnection::GetWorkerId() const
{
	return PhysicalWorkerName(UTF8_TO_TCHAR(Worker_Connection_GetWorkerId(WorkerConnection)));
}

const TArray<FString>& ULegacySpatialWorkerConnection::GetWorkerAttributes() const
{
	return CachedWorkerAttributes;
}

void ULegacySpatialWorkerConnection::CacheWorkerAttributes()
{
	const Worker_WorkerAttributes* Attributes = Worker_Connection_GetWorkerAttributes(WorkerConnection);

	CachedWorkerAttributes.Empty();

	if (Attributes->attributes == nullptr)
	{
		return;
	}

	for (uint32 Index = 0; Index < Attributes->attribute_count; ++Index)
	{
		CachedWorkerAttributes.Add(UTF8_TO_TCHAR(Attributes->attributes[Index]));
	}
}

uint32 ULegacySpatialWorkerConnection::Run()
{
	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();
	check(!SpatialGDKSettings->bRunSpatialWorkerConnectionOnGameThread);

	while (KeepRunning)
	{
		ThreadWaitCondition->Wait();
		QueueLatestOpList();
		ProcessOutgoingMessages();
	}

	return 0;
}

void ULegacySpatialWorkerConnection::Stop()
{
	KeepRunning.AtomicSet(false);
}

void ULegacySpatialWorkerConnection::InitializeOpsProcessingThread()
{
	check(IsInGameThread());

	OpsProcessingThread = FRunnableThread::Create(this, TEXT("SpatialWorkerConnectionWorker"), 0);
	check(OpsProcessingThread);
}

void ULegacySpatialWorkerConnection::QueueLatestOpList()
{
	OpList Ops = GetOpListFromConnection(WorkerConnection);

	if (Ops.Count > 0)
	{
		OpListQueue.Enqueue(MoveTemp(Ops));
	}
}

void ULegacySpatialWorkerConnection::ProcessOutgoingMessages()
{
	bool bSentData = false;
	while (!OutgoingMessagesQueue.IsEmpty())
	{
		bSentData = true;

		TUniquePtr<FOutgoingMessage> OutgoingMessage;
		OutgoingMessagesQueue.Dequeue(OutgoingMessage);

		OnDequeueMessage.Broadcast(OutgoingMessage.Get());

		static const Worker_UpdateParameters DisableLoopback{ /*loopback*/ WORKER_COMPONENT_UPDATE_LOOPBACK_NONE };

		switch (OutgoingMessage->Type)
		{
		case EOutgoingMessageType::ReserveEntityIdsRequest:
		{
			FReserveEntityIdsRequest* Message = static_cast<FReserveEntityIdsRequest*>(OutgoingMessage.Get());

			Worker_Connection_SendReserveEntityIdsRequest(WorkerConnection, Message->NumOfEntities, nullptr);
			break;
		}
		case EOutgoingMessageType::CreateEntityRequest:
		{
			FCreateEntityRequest* Message = static_cast<FCreateEntityRequest*>(OutgoingMessage.Get());

#if TRACE_LIB_ACTIVE
			// We have to unpack these as Worker_ComponentData is not the same as FWorkerComponentData
			TArray<Worker_ComponentData> UnpackedComponentData;
			UnpackedComponentData.SetNum(Message->Components.Num());
			for (int i = 0, Num = Message->Components.Num(); i < Num; i++)
			{
				UnpackedComponentData[i] = Message->Components[i];
			}
			Worker_ComponentData* ComponentData = UnpackedComponentData.GetData();
			uint32 ComponentCount = UnpackedComponentData.Num();
#else
			Worker_ComponentData* ComponentData = Message->Components.GetData();
			uint32 ComponentCount = Message->Components.Num();
#endif
			Worker_Connection_SendCreateEntityRequest(WorkerConnection, ComponentCount, ComponentData,
													  Message->EntityId.IsSet() ? &(Message->EntityId.GetValue()) : nullptr, nullptr);
			break;
		}
		case EOutgoingMessageType::DeleteEntityRequest:
		{
			FDeleteEntityRequest* Message = static_cast<FDeleteEntityRequest*>(OutgoingMessage.Get());

			Worker_Connection_SendDeleteEntityRequest(WorkerConnection, Message->EntityId, nullptr);
			break;
		}
		case EOutgoingMessageType::AddComponent:
		{
			FAddComponent* Message = static_cast<FAddComponent*>(OutgoingMessage.Get());

			Worker_Connection_SendAddComponent(WorkerConnection, Message->EntityId, &Message->Data, &DisableLoopback);
			break;
		}
		case EOutgoingMessageType::RemoveComponent:
		{
			FRemoveComponent* Message = static_cast<FRemoveComponent*>(OutgoingMessage.Get());

			Worker_Connection_SendRemoveComponent(WorkerConnection, Message->EntityId, Message->ComponentId, &DisableLoopback);
			break;
		}
		case EOutgoingMessageType::ComponentUpdate:
		{
			FComponentUpdate* Message = static_cast<FComponentUpdate*>(OutgoingMessage.Get());

			Worker_Connection_SendComponentUpdate(WorkerConnection, Message->EntityId, &Message->Update, &DisableLoopback);

			break;
		}
		case EOutgoingMessageType::CommandRequest:
		{
			FCommandRequest* Message = static_cast<FCommandRequest*>(OutgoingMessage.Get());

			static const Worker_CommandParameters DefaultCommandParams{};
			Worker_Connection_SendCommandRequest(WorkerConnection, Message->EntityId, &Message->Request, nullptr, &DefaultCommandParams);
			break;
		}
		case EOutgoingMessageType::CommandResponse:
		{
			FCommandResponse* Message = static_cast<FCommandResponse*>(OutgoingMessage.Get());

			Worker_Connection_SendCommandResponse(WorkerConnection, Message->RequestId, &Message->Response);
			break;
		}
		case EOutgoingMessageType::CommandFailure:
		{
			FCommandFailure* Message = static_cast<FCommandFailure*>(OutgoingMessage.Get());

			Worker_Connection_SendCommandFailure(WorkerConnection, Message->RequestId, TCHAR_TO_UTF8(*Message->Message));
			break;
		}
		case EOutgoingMessageType::LogMessage:
		{
			FLogMessage* Message = static_cast<FLogMessage*>(OutgoingMessage.Get());

			FTCHARToUTF8 LoggerName(*Message->LoggerName.ToString());
			FTCHARToUTF8 LogString(*Message->Message);

			Worker_LogMessage LogMessage{};
			LogMessage.level = Message->Level;
			LogMessage.logger_name = LoggerName.Get();
			LogMessage.message = LogString.Get();
			Worker_Connection_SendLogMessage(WorkerConnection, &LogMessage);
			break;
		}
		case EOutgoingMessageType::ComponentInterest:
		{
			FComponentInterest* Message = static_cast<FComponentInterest*>(OutgoingMessage.Get());

			Worker_Connection_SendComponentInterest(WorkerConnection, Message->EntityId, Message->Interests.GetData(),
													Message->Interests.Num());
			break;
		}
		case EOutgoingMessageType::EntityQueryRequest:
		{
			FEntityQueryRequest* Message = static_cast<FEntityQueryRequest*>(OutgoingMessage.Get());

			Worker_Connection_SendEntityQueryRequest(WorkerConnection, &Message->EntityQuery, nullptr);
			break;
		}
		case EOutgoingMessageType::Metrics:
		{
			FMetrics* Message = static_cast<FMetrics*>(OutgoingMessage.Get());

			Message->Metrics.SendToConnection(WorkerConnection);
			break;
		}
		default:
		{
			checkNoEntry();
			break;
		}
		}
	}

	// Flush worker API calls
	if (bSentData)
	{
		Worker_Connection_Alpha_Flush(WorkerConnection);
	}
}

void ULegacySpatialWorkerConnection::MaybeFlush()
{
	const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>();
	if (Settings->bWorkerFlushAfterOutgoingNetworkOp)
	{
		Flush();
	}
}

void ULegacySpatialWorkerConnection::Flush()
{
	const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>();
	if (Settings->bRunSpatialWorkerConnectionOnGameThread)
	{
		ProcessOutgoingMessages();
	}
	else if (ensure(ThreadWaitCondition.IsSet()))
	{
		ThreadWaitCondition->Wake(); // No-op if wake is not enabled.
	}
}

template <typename T, typename... ArgsType>
void ULegacySpatialWorkerConnection::QueueOutgoingMessage(ArgsType&&... Args)
{
	// TODO UNR-1271: As later optimization, we can change the queue to hold a union
	// of all outgoing message types, rather than having a pointer.
	auto Message = MakeUnique<T>(Forward<ArgsType>(Args)...);
	OnEnqueueMessage.Broadcast(Message.Get());
	OutgoingMessagesQueue.Enqueue(MoveTemp(Message));
}
