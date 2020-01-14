// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialWorkerTestConnection.h"
#if WITH_EDITOR
#include "Interop/Connection/EditorWorkerController.h"
#endif

#include "WorkerOpListSerializer.h"

#include "Async/Async.h"
#include "Misc/Paths.h"

#include "SpatialGDKSettings.h"
#include "Utils/ErrorCodeRemapping.h"

using namespace SpatialGDK;

DEFINE_LOG_CATEGORY(LogSpatialWorkerTestConnection);

USpatialWorkerTestConnection::~USpatialWorkerTestConnection()
{
}

void USpatialWorkerTestConnection::DestroyConnection()
{
}

Worker_Connection* USpatialWorkerTestConnection::Connect(uint32 PlayInEditorID, bool bConnectAsClient)
{
	return nullptr;
}

void USpatialWorkerTestConnection::SetConnection(Worker_Connection* InConnection)
{
}

void USpatialWorkerTestConnection::StartDevelopmentAuth(FString DevAuthToken, bool bInConnectToLocatorAsClient)
{
}

Worker_ConnectionFuture* USpatialWorkerTestConnection::ConnectToReceptionist(uint32 PlayInEditorID, bool bConnectAsClient)
{
	return nullptr;
}

Worker_ConnectionFuture* USpatialWorkerTestConnection::ConnectToLocator(bool bConnectAsClient)
{
	return nullptr;
}

ESpatialConnectionType USpatialWorkerTestConnection::GetConnectionType() const
{
	return ConnectionType;
}

void USpatialWorkerTestConnection::SetConnectionType(ESpatialConnectionType InConnectionType)
{
}

void USpatialWorkerTestConnection::GetErrorCodeAndMessage(uint8_t& OutConnectionStatusCode, FString& OutErrorMessage) const
{
}

TArray<Worker_OpList*> USpatialWorkerTestConnection::GetOpList()
{
	//static const double StartTime = FPlatformTime::Seconds();
	//const double TimePassed = FPlatformTime::Seconds() - StartTime;
	//if (TimePassed < 5.0)
	//{
	//	return TArray<Worker_OpList*>();
	//}

	TArray<Worker_OpList*> OpLists;
	//for (const auto& OpList : SerializedOpLists.OpLists)
	// TODO(Alex): const value here
	int CurrentOpIndex = OpToProcessNum;

	//while((CurrentOpIndex % 10 != 0) && (CurrentOpIndex < SerializedOpLists.OpLists.Num())) 
	while(CurrentOpIndex < SerializedOpLists.OpLists.Num()) 
	{
		const UE4_OpLists::UE4_OpList& OpList = SerializedOpLists.OpLists[CurrentOpIndex];

		Worker_OpList* CurrOpList = new Worker_OpList{};
		CurrOpList->op_count = OpList.Num();
		CurrOpList->ops = new Worker_Op[CurrOpList->op_count];
		for (uint32_t i = 0; i < CurrOpList->op_count; i++)
		{
			Worker_Op& OpDst = CurrOpList->ops[i];
			OpDst = UE4_OpToWorker_Op(OpList[i]);
		}

		OpLists.Push(CurrOpList);

		CurrentOpIndex++;
		break;
	}
	OpToProcessNum = CurrentOpIndex + 1;

	return OpLists;
}

Worker_RequestId USpatialWorkerTestConnection::SendReserveEntityIdsRequest(uint32_t NumOfEntities)
{
	return 0;
}

Worker_RequestId USpatialWorkerTestConnection::SendCreateEntityRequest(TArray<Worker_ComponentData>&& Components, const Worker_EntityId* EntityId)
{
	return 0;
}

Worker_RequestId USpatialWorkerTestConnection::SendDeleteEntityRequest(Worker_EntityId EntityId)
{
	return 0;
}

void USpatialWorkerTestConnection::SendAddComponent(Worker_EntityId EntityId, Worker_ComponentData* ComponentData)
{
}

void USpatialWorkerTestConnection::SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
}

void USpatialWorkerTestConnection::SendComponentUpdate(Worker_EntityId EntityId, const Worker_ComponentUpdate* ComponentUpdate, const TraceKey Key)
{
}

Worker_RequestId USpatialWorkerTestConnection::SendCommandRequest(Worker_EntityId EntityId, const Worker_CommandRequest* Request, uint32_t CommandId)
{
	return 0;
}

void USpatialWorkerTestConnection::SendCommandResponse(Worker_RequestId RequestId, const Worker_CommandResponse* Response)
{
}

void USpatialWorkerTestConnection::SendCommandFailure(Worker_RequestId RequestId, const FString& Message)
{
}

void USpatialWorkerTestConnection::SendLogMessage(const uint8_t Level, const FName& LoggerName, const TCHAR* Message)
{
}

void USpatialWorkerTestConnection::SendComponentInterest(Worker_EntityId EntityId, TArray<Worker_InterestOverride>&& ComponentInterest)
{
}

Worker_RequestId USpatialWorkerTestConnection::SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery)
{
	return 0;
}

void USpatialWorkerTestConnection::SendMetrics(const SpatialMetrics& Metrics)
{
}

PhysicalWorkerName USpatialWorkerTestConnection::GetWorkerId() const
{
	return PhysicalWorkerName{};
}

const TArray<FString>& USpatialWorkerTestConnection::GetWorkerAttributes() const
{
	return CachedWorkerAttributes;
}

void USpatialWorkerTestConnection::CacheWorkerAttributes()
{
}

void USpatialWorkerTestConnection::QueueLatestOpList()
{
}

void USpatialWorkerTestConnection::ProcessOutgoingMessages()
{
}
