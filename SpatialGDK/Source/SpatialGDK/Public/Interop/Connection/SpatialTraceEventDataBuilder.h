// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/Connection/SpatialTraceUniqueId.h"
#include "SpatialCommonTypes.h"
#include <WorkerSDK/improbable/c_trace.h>
#include <WorkerSDK/improbable/c_worker.h>

#define GDK_EVENT_NAMESPACE "unreal_gdk."

DECLARE_LOG_CATEGORY_EXTERN(FSpatialTraceEventDataBuilderLog, Log, All);

namespace worker
{
namespace c
{
struct Trace_EventData;
}
} // namespace worker

namespace SpatialGDK
{
struct SPATIALGDK_API FSpatialTraceEventName
{
	static const char* PushRPCEventName;
	static const char* ComponentUpdateEventName;
	static const char* SendRetireEntityEventName;
	static const char* PropertyChangedEventName;
	static const char* SendPropertyUpdateEventName;
	static const char* SendCreateEntityEventName;
	static const char* ReceiveCreateEntitySuccessEventName;
	static const char* ReceivePropertyUpdateEventName;
	static const char* AuthorityIntentUpdateEventName;
	static const char* ReceiveCommandResponseEventName;
	static const char* ReceiveCommandRequestEventName;
	static const char* ApplyRPCEventName;
	static const char* SendRPCEventName;
	static const char* QueueRPCEventName;
	static const char* ReceieveRPCEventName;
	static const char* MergeSendRPCEventName;
	static const char* ApplyCrossServerRPCEventName;
	static const char* SendCommandResponseEventName;
	static const char* SendCommandRequestEventName;
	static const char* SendCrossServerRPCEventName;
	static const char* ReceiveCrossServerRPCEventName;
	static const char* MergePropertyUpdateEventName;
	static const char* ReceiveCreateEntityEventName;
	static const char* ReceiveRemoveEntityEventName;
	static const char* AuthorityChangeEventName;
};

class SPATIALGDK_API FSpatialTraceEventDataBuilder
{
public:
	class SPATIALGDK_API FStringCache
	{
	public:
		int32 CombineStrings(const char* A, const char* B);

		int32 AddString(const char* String);
		int32 AddFString(const FString& String);
		int32 AddUInt32(uint32 Value);
		int32 AddUInt64(uint64 Value);
		int32 AddInt32(int32 Value);
		int32 AddInt64(int64 Value);

		const char* Get(int32 Handle) const;
		int32 GetBufferSize() const { return BufferSize; }

	private:
		static const int32 BufferSize = 1000;
		char Buffer[BufferSize];
		int32 NextIndex = 0;

		template <typename IntegerType, typename FmtType>
		int32 AddInteger(IntegerType Integer, const FmtType& Fmt)
		{
			if (NextIndex >= BufferSize)
			{
				return BufferSize - 1;
			}

			int32 InsertIndex = NextIndex;
			int32 RemainingSize = BufferSize - NextIndex;
			int32 BytesWritten = FCStringAnsi::Snprintf(&Buffer[NextIndex], RemainingSize, Fmt, Integer);
			BytesWritten += sizeof(char);
			NextIndex += BytesWritten;
			return InsertIndex;
		}
	};

	FSpatialTraceEventDataBuilder();
	~FSpatialTraceEventDataBuilder();

	void AddObject(const UObject* Object, const char* Key = "Object");
	void AddFunction(const UFunction* Function, const char* Key = "Function");
	void AddEntityId(const Worker_EntityId EntityId, const char* Key = "EntityId");
	void AddComponentId(const Worker_ComponentId ComponentId, const char* Key = "ComponentId");
	void AddFieldId(const uint32 FieldId, const char* Key = "FieldId");
	void AddWorkerId(const uint32 WorkerId, const char* Key = "WorkerId");
	void AddCommand(const char* Command, const char* Key = "Command");
	void AddRequestId(const int64 RequestId, const char* Key = "RequestId");
	void AddAuthority(const Worker_Authority Authority, const char* Key = "Authority");
	void AddLinearTraceId(const EventTraceUniqueId LinearTraceId, const char* Key = "LinearTraceId");

	void AddKeyValue(const char* Key, const char* Value);
	void AddKeyValue(const char* Key, const FString& Value);
	void AddKeyValue(const FString& Key, const FString& Value);
	void AddKeyValue(const char* Key, const uint32 Value);
	void AddKeyValue(const char* Key, const uint64 Value);
	void AddKeyValue(const char* Key, const int32 Value);
	void AddKeyValue(const char* Key, const int64 Value);
	void AddKeyValue(const char* Key, const bool bValue);

	const Trace_EventData* GetEventData() { return EventData; };

private:
	Trace_EventData* EventData;
	FStringCache StringConverter;

	void AddKeyValue(int32 KeyHandle, int32 ValueHandle);

	static const char* AuthorityToString(Worker_Authority Authority);
};

} // namespace SpatialGDK
