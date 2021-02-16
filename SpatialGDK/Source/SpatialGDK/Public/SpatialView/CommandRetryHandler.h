// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/OpList/OpList.h"
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

	void RetryAndBackOff()
	{
		--Retries;
		BackOffTimeS = FMath::Min(BackOffTimeS + BackOffIncrementS, MaximumRetryTimeS);
		BackOffIncrementS *= 2;
	}

	void RetryWithoutBackOff() { --Retries; }

	void StopRetries() { Retries = -1; }
};

// Will retry until it's done or no longer makes sense.
constexpr FRetryData RETRY_UNTIL_COMPLETE = { TNumericLimits<int32>::Max(), 0, 0.1f, 5.0f, 0 };
constexpr FRetryData RETRY_MAX_TIMES = { SpatialConstants::MAX_NUMBER_COMMAND_ATTEMPTS, 0, 0.1f, 5.0f, 0 };
constexpr FRetryData NO_RETRIES = { 0, 0, 0.f, 0.f, 0 };

template <typename T>
class TCommandRetryHandler
{
public:
	using DataType = typename T::CommandData;

	void ProcessOps(float TimeAdvancedS, OpList& Ops, WorkerView& View)
	{
		TimeElapsedS += TimeAdvancedS;

		for (uint32 i = 0; i < Ops.Count; ++i)
		{
			if (T::CanHandleOp(Ops.Ops[i]))
			{
				HandleResponse(Ops.Ops[i]);
			}
		}

		while (CommandsToSendHeap.Num() > 0)
		{
			FDataToSend& Command = CommandsToSendHeap[0];
			if (Command.TimeToSend > TimeElapsedS)
			{
				return;
			}

			SendRequest(Command.RequestId, MoveTemp(Command.Data), Command.Retry, View);

			CommandsToSendHeap.HeapPopDiscard(CompareByTimeToSend);
		}
	}

	void SendRequest(Worker_RequestId RequestId, DataType Data, const FRetryData& RetryData, WorkerView& View)
	{
		if (RetryData.Retries > 0)
		{
			T::SendCommandRequest(RequestId, Data, RetryData.TimeoutMillis, View);
			RequestsInFlight.Emplace(RequestId, FDataInFlight{ MoveTemp(Data), RetryData });
		}
		else
		{
			T::SendCommandRequest(RequestId, MoveTemp(Data), RetryData.TimeoutMillis, View);
		}
	}

protected:
	struct FDataToSend
	{
		Worker_RequestId RequestId;
		DataType Data;
		double TimeToSend;
		FRetryData Retry;
	};

	struct FDataInFlight
	{
		DataType Data;
		FRetryData Retry;
	};

	static bool CompareByTimeToSend(const FDataToSend& Lhs, const FDataToSend& Rhs) { return Lhs.TimeToSend < Rhs.TimeToSend; }

	void HandleResponse(Worker_Op& Op)
	{
		Worker_RequestId& RequestId = T::GetRequestId(Op);
		auto It = RequestsInFlight.CreateKeyIterator(RequestId);
		if (!It)
		{
			return;
		}

		FDataInFlight& Data = It.Value();

		// Update the retry data and enqueue a retry if appropriate.
		T::UpdateRetries(Op, Data.Retry);
		if (Data.Retry.Retries >= 0)
		{
			CommandsToSendHeap.HeapPush(FDataToSend{ RequestId, MoveTemp(Data.Data), TimeElapsedS + Data.Retry.BackOffTimeS, Data.Retry },
										CompareByTimeToSend);
			// Effectively remove the op by setting its request ID to something invalid.
			RequestId *= -1;
		}

		It.RemoveCurrent();
	}

	double TimeElapsedS = 0.0;
	TArray<FDataToSend> CommandsToSendHeap;
	TMap<Worker_RequestId_Key, FDataInFlight> RequestsInFlight;
};

} // namespace SpatialGDK
// Implementations for specific commands.
#include "SpatialView/CommandRetryHandlerImpl.h"
