// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/WorkerView.h"

#include "Containers/Array.h"
#include "Containers/Map.h"
#include "GenericPlatform/GenericPlatformMath.h"
#include "Math/NumericLimits.h"

namespace SpatialGDK
{
struct FRetryData
{
	int32 Retries;
	float BackOffTimeS;
	float BackOffIncrementS;
	float MaximumRetryTimeS;
	uint32 TimeoutMillis;

	FRetryData Advance() const
	{
		return { Retries - 1, FMath::Min(BackOffTimeS + BackOffIncrementS, MaximumRetryTimeS), BackOffIncrementS * 2, MaximumRetryTimeS,
				 TimeoutMillis };
	}
};

// Will retry until a successful response is received.
constexpr FRetryData RETRY_UNTIL_COMPLETE = { TNumericLimits<int32>::Max(), 0, 0.1f, 5.0f, 0 };

template <typename T, typename CommandData>
class TCommandRetryHandler
{
public:
	explicit TCommandRetryHandler(WorkerView* Worker)
		: TimeElapsedS(0.0)
		, Worker(Worker)
	{
	}

	void ProcessOps(float TimeAdvancedS)
	{
		for (auto& Op : Worker->GetViewDelta().GetWorkerMessages())
		{
			if (T::CanHandleOp(Op))
			{
				// todo - need to do something about this
				// possibly give an alternative messages api or put this in a connection handler
				HandleResponse(Op);
			}
		}

		TimeElapsedS += TimeAdvancedS;
		while (CommandsToSendHeap.Num() > 0)
		{
			FDataToSend& Command = CommandsToSendHeap[0];
			if (Command.TimeToSend > TimeElapsedS)
			{
				return;
			}

			SendRequest(Command.RequestId, MoveTemp(Command.Data), Command.Retry);

			CommandsToSendHeap.HeapPopDiscard(CompareByTimeToSend);
		}
	}

	void SendRequest(Worker_RequestId RequestId, CommandData Data, const FRetryData& RetryData)
	{
		if (RetryData.Retries > 0)
		{
			static_cast<T*>(this)->SendCommandRequest(RequestId, Data, RetryData.TimeoutMillis);
			RequestsInFlight.Emplace(RequestId, FDataInFlight{ MoveTemp(Data), RetryData });
		}
		else
		{
			static_cast<T*>(this)->SendCommandRequest(RequestId, MoveTemp(Data), RetryData.TimeoutMillis);
		}
	}

protected:
	struct FDataToSend
	{
		Worker_RequestId RequestId;
		CommandData Data;
		double TimeToSend;
		FRetryData Retry;
	};

	struct FDataInFlight
	{
		CommandData Data;
		FRetryData Retry;
	};

	static bool CompareByTimeToSend(const FDataToSend& Lhs, const FDataToSend& Rhs) { return Lhs.TimeToSend < Rhs.TimeToSend; }

	void HandleResponse(Worker_Op& Op)
	{
		Worker_RequestId RequestId = T::GetRequestId(Op);
		auto It = RequestsInFlight.CreateKeyIterator(RequestId);
		if (!It)
		{
			return;
		}

		FDataInFlight& Data = It.Value();

		// Enqueue a retry if appropriate.
		if (T::CanRetry(Op, Data.Retry) && Data.Retry.Retries >= 0)
		{
			// Effectively remove the op by setting its type to something invalid.
			Op.op_type = 0;
			CommandsToSendHeap.HeapPush(FDataToSend{ RequestId, MoveTemp(Data.Data), TimeElapsedS + Data.Retry.BackOffTimeS, Data.Retry },
										CompareByTimeToSend);
		}

		RequestsInFlight.Remove(RequestId);
	}

	double TimeElapsedS;
	TArray<FDataToSend> CommandsToSendHeap;
	TMap<Worker_RequestId_Key, FDataInFlight> RequestsInFlight;
	WorkerView* Worker;
};

} // namespace SpatialGDK
// Implementations for specific commands.
#include "SpatialView/CommandRetryHandlerImpl.h"
