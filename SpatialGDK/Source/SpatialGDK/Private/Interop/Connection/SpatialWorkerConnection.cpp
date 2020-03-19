// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialWorkerConnection.h"

#include "Async/Async.h"
#include "SpatialGDKSettings.h"

DEFINE_LOG_CATEGORY(LogSpatialWorkerConnection);

using namespace SpatialGDK;

void USpatialWorkerConnection::SetConnection(Worker_Connection* WorkerConnectionIn)
{
	WorkerConnection = WorkerConnectionIn;

	CacheWorkerAttributes();

	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();    
	if (!SpatialGDKSettings->bRunSpatialWorkerConnectionOnGameThread)  
	{
		if (OpsProcessingThread == nullptr)
		{
			InitializeOpsProcessingThread();
		}
	}
}

void USpatialWorkerConnection::FinishDestroy()
{
	UE_LOG(LogSpatialWorkerConnection, Log, TEXT("Destroying SpatialWorkerconnection."));

	DestroyConnection();

	Super::FinishDestroy();
}

void USpatialWorkerConnection::DestroyConnection()
{
	Stop(); // Stop OpsProcessingThread
	if (OpsProcessingThread != nullptr)
	{
		OpsProcessingThread->WaitForCompletion();
		OpsProcessingThread = nullptr;
	}

	if (WorkerConnection)
	{
		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [WorkerConnection = WorkerConnection]
		{
			Worker_Connection_Destroy(WorkerConnection);
		});

		WorkerConnection = nullptr;
	}

	NextRequestId = 0;
	KeepRunning.AtomicSet(true);
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

Worker_RequestId USpatialWorkerConnection::SendCreateEntityRequest(TArray<FWorkerComponentData>&& Components, const Worker_EntityId* EntityId)
{
	QueueOutgoingMessage<FCreateEntityRequest>(MoveTemp(Components), EntityId);
	return NextRequestId++;
}

Worker_RequestId USpatialWorkerConnection::SendDeleteEntityRequest(Worker_EntityId EntityId)
{
	QueueOutgoingMessage<FDeleteEntityRequest>(EntityId);
	return NextRequestId++;
}

void USpatialWorkerConnection::SendAddComponent(Worker_EntityId EntityId, FWorkerComponentData* ComponentData)
{
	QueueOutgoingMessage<FAddComponent>(EntityId, *ComponentData);
}

void USpatialWorkerConnection::SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	QueueOutgoingMessage<FRemoveComponent>(EntityId, ComponentId);
}

void USpatialWorkerConnection::SendComponentUpdate(Worker_EntityId EntityId, const FWorkerComponentUpdate* ComponentUpdate)
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

void USpatialWorkerConnection::SendMetrics(const SpatialMetrics& Metrics)
{
	QueueOutgoingMessage<FMetrics>(Metrics);
}

PhysicalWorkerName USpatialWorkerConnection::GetWorkerId() const
{
	return PhysicalWorkerName(UTF8_TO_TCHAR(Worker_Connection_GetWorkerId(WorkerConnection)));
}

const TArray<FString>& USpatialWorkerConnection::GetWorkerAttributes() const
{
	return CachedWorkerAttributes;
}

void USpatialWorkerConnection::CacheWorkerAttributes()
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

bool USpatialWorkerConnection::Init()
{
	OpsUpdateInterval = 1.0f / GetDefault<USpatialGDKSettings>()->OpsUpdateRate;

	return true;
}

uint32 USpatialWorkerConnection::Run()
{
	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();
	check(!SpatialGDKSettings->bRunSpatialWorkerConnectionOnGameThread);

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

void USpatialWorkerConnection::InitializeOpsProcessingThread()
{
	check(IsInGameThread());

	OpsProcessingThread = FRunnableThread::Create(this, TEXT("SpatialWorkerConnectionWorker"), 0);
	check(OpsProcessingThread);
}

void USpatialWorkerConnection::QueueLatestOpList()
{
	Worker_OpList* OpList = Worker_Connection_GetOpList(WorkerConnection, 0);
	if (OpList->op_count > 0)
	{
		OpListQueue.Enqueue(OpList);
	}
	else
	{
		Worker_OpList_Destroy(OpList);
	}
}

void USpatialWorkerConnection::ProcessOutgoingMessages()
{
	while (!OutgoingMessagesQueue.IsEmpty())
	{
		TUniquePtr<FOutgoingMessage> OutgoingMessage;
		OutgoingMessagesQueue.Dequeue(OutgoingMessage);

		OnDequeueMessage.Broadcast(OutgoingMessage.Get());

		static const Worker_UpdateParameters DisableLoopback{ /*loopback*/ WORKER_COMPONENT_UPDATE_LOOPBACK_NONE };

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
			Worker_Connection_SendCreateEntityRequest(WorkerConnection,
				ComponentCount,
				ComponentData,
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
		case EOutgoingMessageType::AddComponent:
		{
			FAddComponent* Message = static_cast<FAddComponent*>(OutgoingMessage.Get());

			Worker_Connection_SendAddComponent(WorkerConnection,
				Message->EntityId,
				&Message->Data,
				&DisableLoopback);
			break;
		}
		case EOutgoingMessageType::RemoveComponent:
		{
			FRemoveComponent* Message = static_cast<FRemoveComponent*>(OutgoingMessage.Get());

			Worker_Connection_SendRemoveComponent(WorkerConnection,
				Message->EntityId,
				Message->ComponentId,
				&DisableLoopback);
			break;
		}
		case EOutgoingMessageType::ComponentUpdate:
		{
			FComponentUpdate* Message = static_cast<FComponentUpdate*>(OutgoingMessage.Get());

			Worker_Connection_SendComponentUpdate(WorkerConnection,
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

			// Do the conversion here so we can store everything on the stack.
			Worker_Metrics WorkerMetrics;

			WorkerMetrics.load = Message->Metrics.Load.IsSet() ? &Message->Metrics.Load.GetValue() : nullptr;

			TArray<Worker_GaugeMetric> WorkerGaugeMetrics;
			WorkerGaugeMetrics.SetNum(Message->Metrics.GaugeMetrics.Num());
			for (int i = 0; i < Message->Metrics.GaugeMetrics.Num(); i++)
			{
				WorkerGaugeMetrics[i].key = Message->Metrics.GaugeMetrics[i].Key.c_str();
				WorkerGaugeMetrics[i].value = Message->Metrics.GaugeMetrics[i].Value;
			}

			WorkerMetrics.gauge_metric_count = static_cast<uint32_t>(WorkerGaugeMetrics.Num());
			WorkerMetrics.gauge_metrics = WorkerGaugeMetrics.GetData();

			TArray<Worker_HistogramMetric> WorkerHistogramMetrics;
			TArray<TArray<Worker_HistogramMetricBucket>> WorkerHistogramMetricBuckets;
			WorkerHistogramMetrics.SetNum(Message->Metrics.HistogramMetrics.Num());
			for (int i = 0; i < Message->Metrics.HistogramMetrics.Num(); i++)
			{
				WorkerHistogramMetrics[i].key = Message->Metrics.HistogramMetrics[i].Key.c_str();
				WorkerHistogramMetrics[i].sum = Message->Metrics.HistogramMetrics[i].Sum;

				WorkerHistogramMetricBuckets[i].SetNum(Message->Metrics.HistogramMetrics[i].Buckets.Num());
				for (int j = 0; j < Message->Metrics.HistogramMetrics[i].Buckets.Num(); j++)
				{
					WorkerHistogramMetricBuckets[i][j].upper_bound = Message->Metrics.HistogramMetrics[i].Buckets[j].UpperBound;
					WorkerHistogramMetricBuckets[i][j].samples = Message->Metrics.HistogramMetrics[i].Buckets[j].Samples;
				}

				WorkerHistogramMetrics[i].bucket_count = static_cast<uint32_t>(WorkerHistogramMetricBuckets[i].Num());
				WorkerHistogramMetrics[i].buckets = WorkerHistogramMetricBuckets[i].GetData();
			}

			WorkerMetrics.histogram_metric_count = static_cast<uint32_t>(WorkerHistogramMetrics.Num());
			WorkerMetrics.histogram_metrics = WorkerHistogramMetrics.GetData();

			Worker_Connection_SendMetrics(WorkerConnection, &WorkerMetrics);
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
	auto Message = MakeUnique<T>(Forward<ArgsType>(Args)...);
	OnEnqueueMessage.Broadcast(Message.Get());
	OutgoingMessagesQueue.Enqueue(MoveTemp(Message));
}
