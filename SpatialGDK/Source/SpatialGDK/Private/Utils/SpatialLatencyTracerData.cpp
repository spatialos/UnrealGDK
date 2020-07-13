// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SpatialLatencyTracerData.h"

#include "Async/Async.h"
#include "Engine/World.h"
#include "EngineClasses/SpatialGameInstance.h"
#include "GeneralProjectSettings.h"
#include "Interop/Connection/OutgoingMessages.h"
#include "Utils/SchemaUtils.h"

#include <sstream>

DEFINE_LOG_CATEGORY(LogSpatialLatencyTracing);

DECLARE_CYCLE_STAT(TEXT("ContinueLatencyTraceRPC_Internal"), STAT_ContinueLatencyTraceRPC_Internal, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("BeginLatencyTraceRPC_Internal"), STAT_BeginLatencyTraceRPC_Internal, STATGROUP_SpatialNet);

namespace
{
#if TRACE_LIB_ACTIVE
	improbable::trace::SpanContext ReadSpanContext(const void* TraceBytes, const void* SpanBytes)
	{
		improbable::trace::TraceId _TraceId;
		memcpy(&_TraceId[0], TraceBytes, sizeof(improbable::trace::TraceId));

		improbable::trace::SpanId _SpanId;
		memcpy(&_SpanId[0], SpanBytes, sizeof(improbable::trace::SpanId));

		return improbable::trace::SpanContext(_TraceId, _SpanId);
	}
#endif
}  // anonymous namespace

#if TRACE_LIB_ACTIVE
namespace SpatialGDK
{

SpatialLatencyTracerData::SpatialLatencyTracerData()
{
	ResetWorkerId();
	FParse::Value(FCommandLine::Get(), TEXT("traceMetadata"), TraceMetadata);
}

TraceKey SpatialLatencyTracerData::RetrievePendingTrace(const UObject* Obj, const UFunction* Function)
{
	FScopeLock Lock(&Mutex);

	ActorFuncKey FuncKey{ Cast<AActor>(Obj), Function };
	TraceKey ReturnKey = InvalidTraceKey;
	TrackingRPCs.RemoveAndCopyValue(FuncKey, ReturnKey);
	return ReturnKey;
}

TraceKey SpatialLatencyTracerData::RetrievePendingTrace(const UObject* Obj, const UProperty* Property)
{
	FScopeLock Lock(&Mutex);

	ActorPropertyKey PropKey{ Cast<AActor>(Obj), Property };
	TraceKey ReturnKey = InvalidTraceKey;
	TrackingProperties.RemoveAndCopyValue(PropKey, ReturnKey);
	return ReturnKey;
}

TraceKey SpatialLatencyTracerData::RetrievePendingTrace(const UObject* Obj, const FString& Tag)
{
	FScopeLock Lock(&Mutex);

	ActorTagKey EventKey{ Cast<AActor>(Obj), Tag };
	TraceKey ReturnKey = InvalidTraceKey;
	TrackingTags.RemoveAndCopyValue(EventKey, ReturnKey);
	return ReturnKey;
}

void SpatialLatencyTracerData::WriteToLatencyTrace(const TraceKey Key, const FString& TraceDesc)
{
	FScopeLock Lock(&Mutex);

	if (TraceSpan* Trace = TraceMap.Find(Key))
	{
		WriteKeyFrameToTrace(Trace, TraceDesc);
	}
}

void SpatialLatencyTracerData::WriteAndEndTrace(const TraceKey Key, const FString& TraceDesc, bool bOnlyEndIfTraceRootIsRemote)
{
	FScopeLock Lock(&Mutex);

	if (TraceSpan* Trace = TraceMap.Find(Key))
	{
		WriteKeyFrameToTrace(Trace, TraceDesc);

		// Check RootTraces to verify if this trace was started locally. If it was, we don't End the trace yet, but
		// wait for an explicit call to EndLatencyTrace.
		if (!bOnlyEndIfTraceRootIsRemote || RootTraces.Find(Key) == nullptr)
		{
			Trace->End();
			TraceMap.Remove(Key);
		}
	}
}

void SpatialLatencyTracerData::WriteTraceToSchemaObject(const TraceKey Key, Schema_Object* Obj, const Schema_FieldId FieldId)
{
	FScopeLock Lock(&Mutex);

	if (TraceSpan* Trace = TraceMap.Find(Key))
	{
		Schema_Object* TraceObj = Schema_AddObject(Obj, FieldId);

		const improbable::trace::SpanContext& TraceContext = Trace->context();
		improbable::trace::TraceId _TraceId = TraceContext.trace_id();
		improbable::trace::SpanId _SpanId = TraceContext.span_id();

		AddBytesToSchema(TraceObj, SpatialConstants::UNREAL_RPC_TRACE_ID, &_TraceId[0], _TraceId.size());
		AddBytesToSchema(TraceObj, SpatialConstants::UNREAL_RPC_SPAN_ID, &_SpanId[0], _SpanId.size());
	}
}

TraceKey SpatialLatencyTracerData::ReadTraceFromSchemaObject(Schema_Object* Obj, const Schema_FieldId FieldId)
{
	FScopeLock Lock(&Mutex);

	if (Schema_GetObjectCount(Obj, FieldId) > 0)
	{
		Schema_Object* TraceData = Schema_IndexObject(Obj, FieldId, 0);

		const uint8* TraceBytes = Schema_GetBytes(TraceData, SpatialConstants::UNREAL_RPC_TRACE_ID);
		const uint8* SpanBytes = Schema_GetBytes(TraceData, SpatialConstants::UNREAL_RPC_SPAN_ID);

		improbable::trace::SpanContext DestContext = ReadSpanContext(TraceBytes, SpanBytes);

		TraceKey Key = InvalidTraceKey;

		for (const auto& TracePair : TraceMap)
		{
			const TraceKey& _Key = TracePair.Key;
			const TraceSpan& Span = TracePair.Value;

			if (Span.context().trace_id() == DestContext.trace_id())
			{
				Key = _Key;
				break;
			}
		}

		if (Key != InvalidTraceKey)
		{
			TraceSpan* Span = TraceMap.Find(Key);

			WriteKeyFrameToTrace(Span, TEXT("Local Trace - Schema Obj Read"));
		}
		else
		{
			FString SpanMsg = FormatMessage(TEXT("Remote Parent Trace - Schema Obj Read"));
			TraceSpan RetrieveTrace = improbable::trace::Span::StartSpanWithRemoteParent(TCHAR_TO_UTF8(*SpanMsg), DestContext);

			Key = GenerateNewTraceKey();
			TraceMap.Add(Key, MoveTemp(RetrieveTrace));
		}

		return Key;
	}

	return InvalidTraceKey;
}

FSpatialLatencyPayload SpatialLatencyTracerData::RetrievePayload_Internal(const UObject* Obj, const FString& Tag)
{
	FScopeLock Lock(&Mutex);

	 TraceKey Key = RetrievePendingTrace(Obj, Tag);
	 if (Key != InvalidTraceKey)
	 {
		 if (const TraceSpan* Span = TraceMap.Find(Key))
		 {
			 const improbable::trace::SpanContext& TraceContext = Span->context();

			 TArray<uint8> TraceBytes = TArray<uint8_t>((const uint8_t*)&TraceContext.trace_id()[0], sizeof(improbable::trace::TraceId));
			 TArray<uint8> SpanBytes = TArray<uint8_t>((const uint8_t*)&TraceContext.span_id()[0], sizeof(improbable::trace::SpanId));
			 return FSpatialLatencyPayload(MoveTemp(TraceBytes), MoveTemp(SpanBytes), Key);
		 }
	 }
	 return {};
}

void SpatialLatencyTracerData::ResetWorkerId()
{
	WorkerId = TEXT("DeviceId_") + FPlatformMisc::GetDeviceId();
}

void SpatialLatencyTracerData::OnEnqueueMessage(const FOutgoingMessage* Message)
{
	if (Message->Type == EOutgoingMessageType::ComponentUpdate)
	{
		const FComponentUpdate* ComponentUpdate = static_cast<const FComponentUpdate*>(Message);
		WriteToLatencyTrace(ComponentUpdate->Update.Trace, TEXT("Moved componentUpdate to Worker queue"));

		FScopeLock Lock(&Mutex);

		if (TraceSpan* Trace = TraceMap.Find(ComponentUpdate->Update.Trace))
		{
			UE_LOG(LogSpatialLatencyTracing, Warning, TEXT("MCS: Enqueue called for %d"), ComponentUpdate->Update.Trace);
		}
	}
	else if (Message->Type == EOutgoingMessageType::AddComponent)
	{
		const FAddComponent* ComponentAdd = static_cast<const FAddComponent*>(Message);
		WriteToLatencyTrace(ComponentAdd->Data.Trace, TEXT("Moved componentAdd to Worker queue"));
	}
	else if (Message->Type == EOutgoingMessageType::CreateEntityRequest)
	{
		const FCreateEntityRequest* CreateEntityRequest = static_cast<const FCreateEntityRequest*>(Message);
		for (auto& Component : CreateEntityRequest->Components)
		{
			WriteToLatencyTrace(Component.Trace, TEXT("Moved createEntityRequest to Worker queue"));
		}
	}
}

void SpatialLatencyTracerData::OnDequeueMessage(const FOutgoingMessage* Message)
{
	if (Message->Type == EOutgoingMessageType::ComponentUpdate)
	{
		const FComponentUpdate* ComponentUpdate = static_cast<const FComponentUpdate*>(Message);

		if (TraceSpan* Trace = TraceMap.Find(ComponentUpdate->Update.Trace))
		{
			UE_LOG(LogSpatialLatencyTracing, Warning, TEXT("MCS: Dequeue called for %d"), ComponentUpdate->Update.Trace);
		}

		WriteAndEndTrace(ComponentUpdate->Update.Trace, TEXT("Sent componentUpdate to Worker SDK"), true);
	}
	else if (Message->Type == EOutgoingMessageType::AddComponent)
	{
		const FAddComponent* ComponentAdd = static_cast<const FAddComponent*>(Message);
		WriteAndEndTrace(ComponentAdd->Data.Trace, TEXT("Sent componentAdd to Worker SDK"), true);
	}
	else if (Message->Type == EOutgoingMessageType::CreateEntityRequest)
	{
		const FCreateEntityRequest* CreateEntityRequest = static_cast<const FCreateEntityRequest*>(Message);
		for (auto& Component : CreateEntityRequest->Components)
		{
			WriteAndEndTrace(Component.Trace, TEXT("Sent createEntityRequest to Worker SDK"), true);
		}
	}
}

bool SpatialLatencyTracerData::BeginLatencyTrace_Internal(const FString& TraceDesc, FSpatialLatencyPayload& OutLatencyPayload)
{	 
	// TODO: UNR-2787 - Improve mutex-related latency
	// This functions might spike because of the Mutex below
	SCOPE_CYCLE_COUNTER(STAT_BeginLatencyTraceRPC_Internal);
	FScopeLock Lock(&Mutex);

	FString SpanMsg = FormatMessage(TraceDesc, true);
	TraceSpan NewTrace = improbable::trace::Span::StartSpan(TCHAR_TO_UTF8(*SpanMsg), nullptr);

	// Construct payload data from trace
	const improbable::trace::SpanContext& TraceContext = NewTrace.context();

	{
		TArray<uint8> TraceBytes = TArray<uint8_t>((const uint8_t*)&TraceContext.trace_id()[0], sizeof(improbable::trace::TraceId));
		TArray<uint8> SpanBytes = TArray<uint8_t>((const uint8_t*)&TraceContext.span_id()[0], sizeof(improbable::trace::SpanId));
		OutLatencyPayload = FSpatialLatencyPayload(MoveTemp(TraceBytes), MoveTemp(SpanBytes), GenerateNewTraceKey());
	}

	// Add to internal tracking
	TraceMap.Add(OutLatencyPayload.Key, MoveTemp(NewTrace));

	// Store traces started on this worker, so we can persist them until they've been round trip returned.
	RootTraces.Add(OutLatencyPayload.Key);

	return true;
}

bool SpatialLatencyTracerData::ContinueLatencyTrace_Internal(const AActor* Actor, const FString& Target, ETraceType::Type Type, const FString& TraceDesc, const FSpatialLatencyPayload& LatencyPayload, FSpatialLatencyPayload& OutLatencyPayload)
{
	// TODO: UNR-2787 - Improve mutex-related latency
	// This functions might spike because of the Mutex below
	SCOPE_CYCLE_COUNTER(STAT_ContinueLatencyTraceRPC_Internal);
	if (Actor == nullptr)
	{
		return false;
	}

	// We do minimal internal tracking for native rpcs/properties
	const bool bInternalTracking = GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking() || Type == ETraceType::Tagged;

	FScopeLock Lock(&Mutex);

	OutLatencyPayload = LatencyPayload;
	if (OutLatencyPayload.Key == InvalidTraceKey)
	{
		ResolveKeyInLatencyPayload(OutLatencyPayload);
	}

	const TraceKey Key = OutLatencyPayload.Key;
	const TraceSpan* ActiveTrace = TraceMap.Find(Key);
	if (ActiveTrace == nullptr)
	{
		UE_LOG(LogSpatialLatencyTracing, Warning, TEXT("(%s) : No active trace to continue (%s)"), *WorkerId, *TraceDesc);
		return false;
	}

	if (bInternalTracking)
	{
		if (!AddTrackingInfo(Actor, Target, Type, Key))
		{
			UE_LOG(LogSpatialLatencyTracing, Warning, TEXT("(%s) : Failed to create Actor/Func trace (%s)"), *WorkerId, *TraceDesc);
			return false;
		}
	}

	WriteKeyFrameToTrace(ActiveTrace, FString::Printf(TEXT("Continue [%s] %s - %s"), *TraceDesc, *UEnum::GetValueAsString(Type), *Target));

	// If we're not doing any further tracking, end the trace
	if (!bInternalTracking)
	{
		WriteAndEndTrace(Key, TEXT("Native - End of Tracking"), true);
	}

	return true;
}

bool SpatialLatencyTracerData::EndLatencyTrace_Internal(const FSpatialLatencyPayload& LatencyPayload)
{
	FScopeLock Lock(&Mutex);

	// Create temp payload to resolve key
	FSpatialLatencyPayload LocalLatencyPayload = LatencyPayload;
	if (LocalLatencyPayload.Key == InvalidTraceKey)
	{
		ResolveKeyInLatencyPayload(LocalLatencyPayload);
	}

	const TraceKey Key = LocalLatencyPayload.Key;
	const TraceSpan* ActiveTrace = TraceMap.Find(Key);
	if (ActiveTrace == nullptr)
	{
		UE_LOG(LogSpatialLatencyTracing, Warning, TEXT("(%s) : No active trace to end"), *WorkerId);
		return false;
	}

	WriteKeyFrameToTrace(ActiveTrace, TEXT("End"));

	ActiveTrace->End();

	TraceMap.Remove(Key);
	RootTraces.Remove(Key);

	return true;
}

bool SpatialLatencyTracerData::AddTrackingInfo(const AActor* Actor, const FString& Target, const ETraceType::Type Type, const TraceKey Key)
{
	if (Actor == nullptr)
	{
		return false;
	}

	if (UClass* ActorClass = Actor->GetClass())
	{
		switch (Type)
		{
		case ETraceType::RPC:
			if (const UFunction* Function = ActorClass->FindFunctionByName(*Target))
			{
				ActorFuncKey AFKey{ Actor, Function };
				if (TrackingRPCs.Find(AFKey) == nullptr)
				{
					TrackingRPCs.Add(AFKey, Key);
					return true;
				}
				UE_LOG(LogSpatialLatencyTracing, Warning, TEXT("(%s) : ActorFunc already exists for trace"), *WorkerId);
			}
			break;
		case ETraceType::Property:
			if (const UProperty* Property = ActorClass->FindPropertyByName(*Target))
			{
				ActorPropertyKey APKey{ Actor, Property };
				if (TrackingProperties.Find(APKey) == nullptr)
				{
					TrackingProperties.Add(APKey, Key);
					return true;
				}
				UE_LOG(LogSpatialLatencyTracing, Warning, TEXT("(%s) : ActorProperty already exists for trace"), *WorkerId);
			}
			break;
		case ETraceType::Tagged:
			{
				ActorTagKey ATKey{ Actor, Target };
				if (TrackingTags.Find(ATKey) == nullptr)
				{
					TrackingTags.Add(ATKey, Key);
					return true;
				}
				UE_LOG(LogSpatialLatencyTracing, Warning, TEXT("(%s) : ActorTag already exists for trace"), *WorkerId);
			}
			break;
		}
	}

	return false;
}

TraceKey SpatialLatencyTracerData::GenerateNewTraceKey()
{
	return NextTraceKey++;
}

void SpatialLatencyTracerData::ResolveKeyInLatencyPayload(FSpatialLatencyPayload& Payload)
{
	// Key isn't set, so attempt to find it in the trace map
	for (const auto& TracePair : TraceMap)
	{
		const TraceKey& Key = TracePair.Key;
		const TraceSpan& Span = TracePair.Value;

		if (memcmp(Span.context().trace_id().data(), Payload.TraceId.GetData(), Payload.TraceId.Num()) == 0)
		{
			WriteKeyFrameToTrace(&Span, TEXT("Local Trace - Payload Obj Read"));
			Payload.Key = Key;
			break;
		}
	}

	if (Payload.Key == InvalidTraceKey)
	{
		// Uninitialized key, generate and add to map
		Payload.Key = GenerateNewTraceKey();

		improbable::trace::SpanContext DestContext = ReadSpanContext(Payload.TraceId.GetData(), Payload.SpanId.GetData());

		FString SpanMsg = FormatMessage(TEXT("Remote Parent Trace - Payload Obj Read"));
		TraceSpan RetrieveTrace = improbable::trace::Span::StartSpanWithRemoteParent(TCHAR_TO_UTF8(*SpanMsg), DestContext);

		TraceMap.Add(Payload.Key, MoveTemp(RetrieveTrace));
	}
}

void SpatialLatencyTracerData::WriteKeyFrameToTrace(const TraceSpan* Trace, const FString& TraceDesc)
{
	if (Trace != nullptr)
	{
		FString TraceMsg = FormatMessage(TraceDesc);
		improbable::trace::Span::StartSpan(TCHAR_TO_UTF8(*TraceMsg), Trace).End();
	}
}

FString SpatialLatencyTracerData::FormatMessage(const FString& Message, bool bIncludeMetadata) const
{
	if (bIncludeMetadata)
	{
		return FString::Printf(TEXT("%s (%s : %s)"), *Message, *TraceMetadata, *WorkerId.Left(18));
	}
	else
	{
		return FString::Printf(TEXT("%s (%s)"), *Message, *WorkerId.Left(18));
	}
}
} // namespace SpatialGDK
#endif // TRACE_LIB_ACTIVE
