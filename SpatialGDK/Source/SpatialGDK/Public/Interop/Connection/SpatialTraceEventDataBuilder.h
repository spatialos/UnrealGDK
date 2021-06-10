// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/Connection/SpatialTraceUniqueId.h"
#include "SpatialCommonTypes.h"
#include <WorkerSDK/improbable/c_trace.h>
#include <WorkerSDK/improbable/c_worker.h>

#define GDK_EVENT_NAMESPACE "unreal_gdk."

#define PUSH_RPC_EVENT_NAME GDK_EVENT_NAMESPACE "push_rpc"
#define COMPONENT_UPDATE_EVENT_NAME GDK_EVENT_NAMESPACE "component_update"
#define SEND_RETIRE_ENTITY_EVENT_NAME GDK_EVENT_NAMESPACE "send_retire_entity"
#define PROPERTY_CHANGED_EVENT_NAME GDK_EVENT_NAMESPACE "property_changed"
#define SEND_PROPERTY_UPDATE_EVENT_NAME GDK_EVENT_NAMESPACE "send_property_update"
#define SEND_CREATE_ENTITY_EVENT_NAME GDK_EVENT_NAMESPACE "send_create_entity"
#define RECEIVE_CREATE_ENTITY_SUCCESS_EVENT_NAME GDK_EVENT_NAMESPACE "receive_create_entity_success"
#define RECEIVE_PROPERTY_UPDATE_EVENT_NAME GDK_EVENT_NAMESPACE "receive_property_update"
#define AUTHORITY_INTENT_UPDATE_EVENT_NAME GDK_EVENT_NAMESPACE "authority_intent_update"
#define RECEIVE_COMMAND_RESPONSE_EVENT_NAME GDK_EVENT_NAMESPACE "receive_command_response"
#define RECEIVE_COMMAND_REQUEST_EVENT_NAME GDK_EVENT_NAMESPACE "receive_command_request"
#define APPLY_RPC_EVENT_NAME GDK_EVENT_NAMESPACE "apply_rpc"
#define SEND_RPC_EVENT_NAME GDK_EVENT_NAMESPACE "send_rpc"
#define QUEUE_RPC_EVENT_NAME GDK_EVENT_NAMESPACE "queue_rpc"
#define RECEIVE_RPC_EVENT_NAME GDK_EVENT_NAMESPACE "receive_rpc"
#define MERGE_SEND_RPCS_EVENT_NAME GDK_EVENT_NAMESPACE "merge_send_rpcs"
#define APPLY_CROSS_SERVER_RPC_EVENT_NAME GDK_EVENT_NAMESPACE "apply_cross_server_rpc"
#define SEND_COMMAND_RESPONSE_EVENT_NAME GDK_EVENT_NAMESPACE "send_command_response"
#define SEND_COMMAND_REQEUST_EVENT_NAME GDK_EVENT_NAMESPACE "send_command_request"
#define SEND_CROSS_SERVER_RPC_EVENT_NAME GDK_EVENT_NAMESPACE "send_cross_server_rpc"
#define MERGE_PROPERTY_EVENT_NAME GDK_EVENT_NAMESPACE "merge_property_update"
#define RECEIVE_CREATE_ENTITY_EVENT_NAME GDK_EVENT_NAMESPACE "receive_create_entity"
#define SEND_COMMAND_REQUEST_EVENT_NAME GDK_EVENT_NAMESPACE "send_command_request"
#define RECEIVE_REMOVE_ENTITY_EVENT_NAME GDK_EVENT_NAMESPACE "receive_remove_entity"
#define AUTHORITY_CHANGE_EVENT_NAME GDK_EVENT_NAMESPACE "authority_change"
#define RECEIVE_CROSS_SERVER_RPC_EVENT_NAME GDK_EVENT_NAMESPACE "receive_cross_server_rpc"
#define MERGE_SEND_RPC_EVENT_NAME GDK_EVENT_NAMESPACE "merge_send_rpcs"
#define MERGE_PROPERTY_UPDATE_EVENT_NAME GDK_EVENT_NAMESPACE "merge_property_update"

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
		static constexpr int32 BufferSize = 1000;
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
			int32 CharSize = sizeof(char);
			int32 BytesWritten = CharSize * (FCStringAnsi::Snprintf(&Buffer[NextIndex], RemainingSize, Fmt, Integer) + 1);
			NextIndex += BytesWritten;
			return InsertIndex;
		}
	};

	FSpatialTraceEventDataBuilder();
	~FSpatialTraceEventDataBuilder();

	void AddObject(const UObject* Object, const char* Key = "object");
	void AddFunction(const UFunction* Function, const char* Key = "function");
	void AddEntityId(const Worker_EntityId EntityId, const char* Key = "entity_id");
	void AddComponentId(const Worker_ComponentId ComponentId, const char* Key = "component_id");
	void AddComponentSetId(const Worker_ComponentSetId ComponentId, const char* Key = "component_set_id");
	void AddFieldId(const uint32 FieldId, const char* Key = "field_id");
	void AddWorkerId(const uint32 WorkerId, const char* Key = "worker_id");
	void AddCommand(const char* Command, const char* Key = "command");
	void AddRequestId(const int64 RequestId, const char* Key = "request_id");
	void AddAuthority(const Worker_Authority Authority, const char* Key = "authority");
	void AddLinearTraceId(const EventTraceUniqueId LinearTraceId, const char* Key = "linear_trace_id");

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
