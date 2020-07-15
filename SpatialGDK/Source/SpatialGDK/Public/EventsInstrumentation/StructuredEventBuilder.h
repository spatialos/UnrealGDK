#pragma once

#include "NetworkEvents.h"

class StructuredEventBuilder
{
public:
	template <typename TNetEvent>
	static TNetEvent ConstructNetEvent(FString Direction)
	{
		static_assert(std::is_base_of<FNetworkEvent, TNetEvent>::value, "TNetEvent must inherit from FNetworkEvent.");

		TNetEvent NetEvent;
		NetEvent.Network.MessageDirection = Direction;

		return NetEvent;
	}

	template <typename TNetEvent>
	static TNetEvent ConstructNetSendEvent()
	{
		return ConstructNetEvent<TNetEvent>(TEXT("SEND"));
	}

	template <typename TNetEvent>
	static TNetEvent ConstructNetReceiveEvent()
	{
		return ConstructNetEvent<TNetEvent>(TEXT("RECEIVE"));
	}

	static FActorData ConstructActorData(const AActor* Actor, Worker_EntityId EntityId)
	{
		FActorData Data;
		Data.EntityId = EntityId;
		if (Actor != nullptr)
		{
			Data.Type = Actor->GetClass()->GetName();
			Data.Name = Actor->GetFullName();
		}
		return Data;
	}

	static FSubobjectData ConstructSubobjectData(const UObject* Subobject)
	{
		FSubobjectData Data;
		if (Subobject != nullptr)
		{
			Data.Type = Subobject->GetClass()->GetName();
			Data.Name = Subobject->GetFullName();
		}
		return Data;
	}

	static FRPCData ConstructUserRPCData(const UFunction* Function, TraceKey TraceId, Worker_RequestId LocalRequestId, FString ErrorMessage, uint64 StatusCode, uint8 RetryAttempt)
	{
		FRPCData Data;
		Data.Type = TEXT("USER");
		Data.LocalRequestId = LocalRequestId;
		Data.TraceKey = TraceId;
		Data.ErrorMessage = ErrorMessage;
		Data.StatusCode = StatusCode;
		Data.RetryAttempt = RetryAttempt;
		if(StatusCode != 0)
		{
			Data.Success = StatusCode == WORKER_STATUS_CODE_SUCCESS;
		}
		if (Function != nullptr)
		{
			Data.Name = Function->GetName();
		}
		
		return Data;
	}
	static FRPCData ConstructGdkRPCData(FString CommandName, Worker_RequestId LocalRequestId, FString ErrorMessage, uint64 StatusCode, uint8 RetryAttempt)
	{
		FRPCData Data;
		Data.Type = TEXT("GDK");
		Data.LocalRequestId = LocalRequestId;
		Data.Name = CommandName;
		Data.ErrorMessage = ErrorMessage;
		Data.StatusCode = StatusCode;
		Data.RetryAttempt = RetryAttempt;
		if (StatusCode != 0)
		{
			Data.Success = StatusCode == WORKER_STATUS_CODE_SUCCESS;
		}

		return Data;
	}
};
