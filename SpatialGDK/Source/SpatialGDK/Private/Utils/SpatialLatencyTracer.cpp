// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SpatialLatencyTracer.h"

#include "Async/Async.h"
#include "Engine/World.h"
#include "EngineClasses/SpatialGameInstance.h"
#include "GeneralProjectSettings.h"
#include "Interop/Connection/OutgoingMessages.h"
#include "Utils/SchemaUtils.h"

#include <sstream>

DEFINE_LOG_CATEGORY(LogSpatialLatencyTracing);

namespace
{
	// Stream for piping trace lib output to UE output
	class UEStream : public std::stringbuf
	{
		int sync() override
		{
			UE_LOG(LogSpatialLatencyTracing, Verbose, TEXT("%s"), *FString(str().c_str()));
			str("");
			return std::stringbuf::sync();
		}

	public:
		virtual ~UEStream() override
		{
			sync();
		}
	};

	UEStream Stream;

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

USpatialLatencyTracer::USpatialLatencyTracer()
{
#if TRACE_LIB_ACTIVE
	ActiveTraceKey = InvalidTraceKey;
	ResetWorkerId();
#endif
}

void USpatialLatencyTracer::RegisterProject(UObject* WorldContextObject, const FString& ProjectId)
{
#if TRACE_LIB_ACTIVE
	using namespace improbable::exporters::trace;

	StackdriverExporter::Register({ TCHAR_TO_UTF8(*ProjectId) });

	std::cout.rdbuf(&Stream);
	std::cerr.rdbuf(&Stream);

	StdoutExporter::Register();
#endif // TRACE_LIB_ACTIVE
}

bool USpatialLatencyTracer::BeginLatencyTrace(UObject* WorldContextObject, const AActor* Actor, const FString& FunctionName, const FString& TraceDesc, FSpatialLatencyPayload& OutLatencyPayload)
{
#if TRACE_LIB_ACTIVE
	if (USpatialLatencyTracer* Tracer = GetTracer(WorldContextObject))
	{
		return Tracer->BeginLatencyTrace_Internal(Actor, FunctionName, TraceDesc, OutLatencyPayload);
	}
#endif // TRACE_LIB_ACTIVE
	return false;
}

bool USpatialLatencyTracer::ContinueLatencyTrace(UObject* WorldContextObject, const AActor* Actor, const FString& FunctionName, const FString& TraceDesc, const FSpatialLatencyPayload& LatencyPayLoad, FSpatialLatencyPayload& OutLatencyPayloadContinue)
{
#if TRACE_LIB_ACTIVE
	if (USpatialLatencyTracer* Tracer = GetTracer(WorldContextObject))
	{
		return Tracer->ContinueLatencyTrace_Internal(Actor, FunctionName, TraceDesc, LatencyPayLoad, OutLatencyPayloadContinue);
	}
#endif // TRACE_LIB_ACTIVE
	return false;
}

bool USpatialLatencyTracer::EndLatencyTrace(UObject* WorldContextObject, const FSpatialLatencyPayload& LatencyPayLoad)
{
#if TRACE_LIB_ACTIVE
	if (USpatialLatencyTracer* Tracer = GetTracer(WorldContextObject))
	{
		return Tracer->EndLatencyTrace_Internal(LatencyPayLoad);
	}
#endif // TRACE_LIB_ACTIVE
	return false;
}

bool USpatialLatencyTracer::IsLatencyTraceActive(UObject* WorldContextObject)
{
#if TRACE_LIB_ACTIVE
	if (USpatialLatencyTracer* Tracer = GetTracer(WorldContextObject))
	{
		return Tracer->IsLatencyTraceActive_Internal();
	}
#endif // TRACE_LIB_ACTIVE
	return false;
}

#if TRACE_LIB_ACTIVE
bool USpatialLatencyTracer::IsValidKey(const TraceKey Key)
{
	FScopeLock Lock(&Mutex);
	return TraceMap.Find(Key);
}

TraceKey USpatialLatencyTracer::RetrievePendingTrace(const UObject* Obj, const UFunction* Function)
{
	FScopeLock Lock(&Mutex);

	ActorFuncKey FuncKey{ Cast<AActor>(Obj), Function };
	TraceKey ReturnKey = InvalidTraceKey;
	TrackingTraces.RemoveAndCopyValue(FuncKey, ReturnKey);
	return ReturnKey;
}

TraceKey USpatialLatencyTracer::RetrievePendingTrace(const UObject* Obj, const UProperty* Property)
{
	FScopeLock Lock(&Mutex);

	ActorPropertyKey PropKey{ Cast<AActor>(Obj), Property };
	TraceKey ReturnKey = InvalidTraceKey;
	TrackingProperties.RemoveAndCopyValue(PropKey, ReturnKey);
	return ReturnKey;
}

void USpatialLatencyTracer::MarkActiveLatencyTrace(const TraceKey Key)
{
	// We can safely set this to the active trace, even if Key is invalid, as other functionality
	// is gated on the ActiveTraceKey being present in the TraceMap
	ActiveTraceKey = Key;
}

void USpatialLatencyTracer::WriteToLatencyTrace(const TraceKey Key, const FString& TraceDesc)
{
	FScopeLock Lock(&Mutex);

	if (TraceSpan* Trace = TraceMap.Find(Key))
	{
		WriteKeyFrameToTrace(Trace, TraceDesc);
	}
}

void USpatialLatencyTracer::EndLatencyTrace(const TraceKey Key, const FString& TraceDesc)
{
	FScopeLock Lock(&Mutex);

	if (TraceSpan* Trace = TraceMap.Find(Key))
	{
		WriteKeyFrameToTrace(Trace, TraceDesc);

		Trace->End();
		TraceMap.Remove(Key);
	}
}

void USpatialLatencyTracer::WriteTraceToSchemaObject(const TraceKey Key, Schema_Object* Obj, const Schema_FieldId FieldId)
{
	FScopeLock Lock(&Mutex);

	if (TraceSpan* Trace = TraceMap.Find(Key))
	{
		Schema_Object* TraceObj = Schema_AddObject(Obj, FieldId);

		const improbable::trace::SpanContext& TraceContext = Trace->context();
		improbable::trace::TraceId _TraceId = TraceContext.trace_id();
		improbable::trace::SpanId _SpanId = TraceContext.span_id();

		SpatialGDK::AddBytesToSchema(TraceObj, SpatialConstants::UNREAL_RPC_TRACE_ID, &_TraceId[0], _TraceId.size());
		SpatialGDK::AddBytesToSchema(TraceObj, SpatialConstants::UNREAL_RPC_SPAN_ID, &_SpanId[0], _SpanId.size());
	}
}

TraceKey USpatialLatencyTracer::ReadTraceFromSchemaObject(Schema_Object* Obj, const Schema_FieldId FieldId)
{
	FScopeLock Lock(&Mutex);

	if (Schema_GetObjectCount(Obj, FieldId) > 0)
	{
		Schema_Object* TraceData = Schema_IndexObject(Obj, FieldId, 0);

		const uint8* TraceBytes = Schema_GetBytes(TraceData, SpatialConstants::UNREAL_RPC_TRACE_ID);
		const uint8* SpanBytes = Schema_GetBytes(TraceData, SpatialConstants::UNREAL_RPC_SPAN_ID);

		improbable::trace::SpanContext DestContext = ReadSpanContext(TraceBytes, SpanBytes);

		FString SpanMsg = FormatMessage(TEXT("Read Trace From Schema Obj"));
		TraceSpan RetrieveTrace = improbable::trace::Span::StartSpanWithRemoteParent(TCHAR_TO_UTF8(*SpanMsg), DestContext);

		const TraceKey Key = GenerateNewTraceKey();
		TraceMap.Add(Key, MoveTemp(RetrieveTrace));

		return Key;
	}

	return InvalidTraceKey;
}

TraceKey USpatialLatencyTracer::ReadTraceFromSpatialPayload(const FSpatialLatencyPayload& Payload)
{
	FScopeLock Lock(&Mutex);

	if (Payload.TraceId.Num() != sizeof(improbable::trace::TraceId))
	{
		UE_LOG(LogSpatialLatencyTracing, Warning, TEXT("Payload TraceId does not contain the correct number of trace bytes. %d found"), Payload.TraceId.Num());
		return InvalidTraceKey;
	}

	if (Payload.SpanId.Num() != sizeof(improbable::trace::SpanId))
	{
		UE_LOG(LogSpatialLatencyTracing, Warning, TEXT("Payload TraceId does not contain the correct number of span bytes. %d found"), Payload.SpanId.Num());
		return InvalidTraceKey;
	}

	improbable::trace::SpanContext DestContext = ReadSpanContext(Payload.TraceId.GetData(), Payload.SpanId.GetData());

	FString SpanMsg = FormatMessage(TEXT("Read Trace From Payload Obj"));
	TraceSpan RetrieveTrace = improbable::trace::Span::StartSpanWithRemoteParent(TCHAR_TO_UTF8(*SpanMsg), DestContext);

	const TraceKey Key = GenerateNewTraceKey();
	TraceMap.Add(Key, MoveTemp(RetrieveTrace));

	return Key;
}

void USpatialLatencyTracer::ResetWorkerId()
{
	WorkerId = TEXT("DeviceId_") + FPlatformMisc::GetDeviceId();
}

void USpatialLatencyTracer::OnEnqueueMessage(const SpatialGDK::FOutgoingMessage* Message)
{
	if (Message->Type == SpatialGDK::EOutgoingMessageType::ComponentUpdate)
	{
		const SpatialGDK::FComponentUpdate* ComponentUpdate = static_cast<const SpatialGDK::FComponentUpdate*>(Message);
		WriteToLatencyTrace(ComponentUpdate->Trace, TEXT("Moved componentUpdate to Worker queue"));
	}
	else if (Message->Type == SpatialGDK::EOutgoingMessageType::AddComponent)
	{
		const SpatialGDK::FAddComponent* ComponentAdd = static_cast<const SpatialGDK::FAddComponent*>(Message);
		WriteToLatencyTrace(ComponentAdd->Trace, TEXT("Moved componentAdd to Worker queue"));
	}
}

void USpatialLatencyTracer::OnDequeueMessage(const SpatialGDK::FOutgoingMessage* Message)
{
	if (Message->Type == SpatialGDK::EOutgoingMessageType::ComponentUpdate)
	{
		const SpatialGDK::FComponentUpdate* ComponentUpdate = static_cast<const SpatialGDK::FComponentUpdate*>(Message);
		EndLatencyTrace(ComponentUpdate->Trace, TEXT("Sent componentUpdate to Worker SDK"));
	}
	else if (Message->Type == SpatialGDK::EOutgoingMessageType::AddComponent)
	{
		const SpatialGDK::FAddComponent* ComponentAdd = static_cast<const SpatialGDK::FAddComponent*>(Message);
		EndLatencyTrace(ComponentAdd->Trace, TEXT("Sent componentAdd to Worker SDK"));
	}

}

USpatialLatencyTracer* USpatialLatencyTracer::GetTracer(UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (World == nullptr)
	{
		World = GWorld;
	}

	if (USpatialGameInstance* GameInstance = World->GetGameInstance<USpatialGameInstance>())
	{
		return GameInstance->GetSpatialLatencyTracer();
	}

	return nullptr;
}

bool USpatialLatencyTracer::BeginLatencyTrace_Internal(const AActor* Actor, const FString& FunctionOrProperty, const FString& TraceDesc, FSpatialLatencyPayload& OutLatencyPayload)
{
	FScopeLock Lock(&Mutex);

	TraceKey Key = CreateNewTraceEntry(Actor, FunctionOrProperty);
	if (Key == InvalidTraceKey)
	{
		UE_LOG(LogSpatialLatencyTracing, Warning, TEXT("(%s) : Failed to create Actor/Func trace (%s)"), *WorkerId, *TraceDesc);
		return false;
	}

	FString SpanMsg = FormatMessage(TraceDesc);
	TraceSpan NewTrace = improbable::trace::Span::StartSpan(TCHAR_TO_UTF8(*SpanMsg), nullptr);

	WriteKeyFrameToTrace(&NewTrace, FString::Printf(TEXT("Begin trace : %s"), *FunctionOrProperty));

	// For non-spatial tracing
	const improbable::trace::SpanContext& TraceContext = NewTrace.context();

	{
		TArray<uint8> TraceBytes = TArray<uint8_t>((const uint8_t*)&TraceContext.trace_id()[0], sizeof(improbable::trace::TraceId));
		TArray<uint8> SpanBytes = TArray<uint8_t>((const uint8_t*)&TraceContext.span_id()[0], sizeof(improbable::trace::SpanId));
		OutLatencyPayload = FSpatialLatencyPayload(MoveTemp(TraceBytes), MoveTemp(SpanBytes));
	}

	TraceMap.Add(Key, MoveTemp(NewTrace));

	if (!GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking())
	{
		// We can't do any deeper tracing in the stack here so terminate these traces here
		ClearTrackingInformation();
	}

	return true;
}

bool USpatialLatencyTracer::ContinueLatencyTrace_Internal(const AActor* Actor, const FString& FunctionName, const FString& TraceDesc, const FSpatialLatencyPayload& LatencyPayload, FSpatialLatencyPayload& OutLatencyPayloadContinue)
{
	if (Actor == nullptr)
	{
		return InvalidTraceKey;
	}

	FScopeLock Lock(&Mutex);
		
	TraceSpan* ActiveTrace = GetActiveTraceOrReadPayload(LatencyPayload);
	if (ActiveTrace == nullptr)
	{
		UE_LOG(LogSpatialLatencyTracing, Warning, TEXT("(%s) : No active trace to continue (%s)"), *WorkerId, *TraceDesc);
		return false;
	}

	TraceKey Key = CreateNewTraceEntry(Actor, FunctionName);
	if (Key == InvalidTraceKey)
	{
		UE_LOG(LogSpatialLatencyTracing, Warning, TEXT("(%s) : Failed to create Actor/Func trace (%s)"), *WorkerId, *TraceDesc);
		return false;
	}

	WriteKeyFrameToTrace(ActiveTrace, TCHAR_TO_UTF8(*TraceDesc));
	WriteKeyFrameToTrace(ActiveTrace, FString::Printf(TEXT("Continue trace : %s"), *FunctionName));

	// For non-spatial tracing
	const improbable::trace::SpanContext& TraceContext = ActiveTrace->context();

	{
		TArray<uint8> TraceBytes = TArray<uint8_t>((const uint8_t*)&TraceContext.trace_id()[0], sizeof(improbable::trace::TraceId));
		TArray<uint8> SpanBytes = TArray<uint8_t>((const uint8_t*)&TraceContext.span_id()[0], sizeof(improbable::trace::SpanId));
		OutLatencyPayloadContinue = FSpatialLatencyPayload(MoveTemp(TraceBytes), MoveTemp(SpanBytes));
	}

	TraceMap.Add(Key, MoveTemp(ActiveTrace));
	TraceMap.Remove(ActiveTraceKey);
	ActiveTraceKey = InvalidTraceKey;

	if (!GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking())
	{
		// We can't do any deeper tracing in the stack here so terminate these traces here
		ClearTrackingInformation();
	}

	return true;
}

bool USpatialLatencyTracer::EndLatencyTrace_Internal(const FSpatialLatencyPayload& LatencyPayload)
{
	FScopeLock Lock(&Mutex);

	TraceSpan* ActiveTrace = GetActiveTraceOrReadPayload(LatencyPayload);

	if (ActiveTrace == nullptr)
	{
		UE_LOG(LogSpatialLatencyTracing, Warning, TEXT("(%s) : No active trace to end"), *WorkerId);
		return false;
	}

	WriteKeyFrameToTrace(ActiveTrace, TEXT("End Trace"));
	ActiveTrace->End();

	TraceMap.Remove(ActiveTraceKey);
	ActiveTraceKey = InvalidTraceKey;

	if (!GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking())
	{
		// We can't do any deeper tracing in the stack here so terminate these traces here
		ClearTrackingInformation();
	}

	return true;
}

bool USpatialLatencyTracer::IsLatencyTraceActive_Internal()
{
	return (ActiveTraceKey != InvalidTraceKey);
}

void USpatialLatencyTracer::ClearTrackingInformation()
{
	TraceMap.Reset();
	TrackingTraces.Reset();
	TrackingProperties.Reset();
}

TraceKey USpatialLatencyTracer::CreateNewTraceEntry(const AActor* Actor, const FString& FunctionOrProperty)
{
	if (Actor == nullptr)
	{
		return InvalidTraceKey;
	}

	if (UClass* ActorClass = Actor->GetClass())
	{
		if (const UFunction* Function = ActorClass->FindFunctionByName(*FunctionOrProperty))
		{
			ActorFuncKey Key{ Actor, Function };
			if (TrackingTraces.Find(Key) == nullptr)
			{
				const TraceKey _TraceKey = GenerateNewTraceKey();
				TrackingTraces.Add(Key, _TraceKey);
				return _TraceKey;
			}
			UE_LOG(LogSpatialLatencyTracing, Warning, TEXT("(%s) : ActorFunc already exists for trace"), *WorkerId);
		}
		else if (const UProperty* Property = ActorClass->FindPropertyByName(*FunctionOrProperty))
		{
			ActorPropertyKey Key{ Actor, Property };
			if (TrackingProperties.Find(Key) == nullptr)
			{
				const TraceKey _TraceKey = GenerateNewTraceKey();
				TrackingProperties.Add(Key, _TraceKey);
				return _TraceKey;
			}
			UE_LOG(LogSpatialLatencyTracing, Warning, TEXT("(%s) : ActorProperty already exists for trace"), *WorkerId);
		}
	}

	return InvalidTraceKey;
}

TraceKey USpatialLatencyTracer::GenerateNewTraceKey()
{
	return NextTraceKey++;
}

USpatialLatencyTracer::TraceSpan* USpatialLatencyTracer::GetActiveTrace()
{
	return TraceMap.Find(ActiveTraceKey);
}

USpatialLatencyTracer::TraceSpan* USpatialLatencyTracer::GetActiveTraceOrReadPayload(const FSpatialLatencyPayload& Payload)
{
	USpatialLatencyTracer::TraceSpan* ActiveTrace = GetActiveTrace();
	if (ActiveTrace == nullptr)
	{
		// Try read the trace from the payload
		TraceKey Key = ReadTraceFromSpatialPayload(Payload);
		if (Key != InvalidTraceKey)
		{
			MarkActiveLatencyTrace(Key);
			ActiveTrace = GetActiveTrace();
		}
		else
		{
			UE_LOG(LogSpatialLatencyTracing, Warning, TEXT("(%s) : Could not read trace from payload. The payload was likely invalid."), *WorkerId);
		}
	}
	return ActiveTrace;
}

void USpatialLatencyTracer::WriteKeyFrameToTrace(const TraceSpan* Trace, const FString& TraceDesc)
{
	if (Trace != nullptr)
	{
		FString TraceMsg = FormatMessage(TraceDesc);
		improbable::trace::Span::StartSpan(TCHAR_TO_UTF8(*TraceMsg), Trace).End();
	}
}

FString USpatialLatencyTracer::FormatMessage(const FString& Message) const
{
	return FString::Printf(TEXT("(%s) : %s"), *WorkerId.Left(18), *Message);
}

#endif // TRACE_LIB_ACTIVE

void USpatialLatencyTracer::Debug_SendTestTrace()
{
#if TRACE_LIB_ACTIVE
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, []
	{
		using namespace improbable::trace;

		std::cout << "Sending test trace" << std::endl;

		Span RootSpan = Span::StartSpan("Example Span", nullptr);

		{
			Span SubSpan1 = Span::StartSpan("Sub span 1", &RootSpan);
			FPlatformProcess::Sleep(1);
			SubSpan1.End();
		}

		{
			Span SubSpan2 = Span::StartSpan("Sub span 2", &RootSpan);
			FPlatformProcess::Sleep(1);
			SubSpan2.End();
		}

		FPlatformProcess::Sleep(1);

		// recreate Span from context
		const SpanContext& SourceContext = RootSpan.context();
		auto TraceId = SourceContext.trace_id();
		auto SpanId = SourceContext.span_id();
		RootSpan.End();

		SpanContext DestContext(TraceId, SpanId);

		{
			Span SubSpan3 = Span::StartSpanWithRemoteParent("SubSpan 3", DestContext);
			SubSpan3.AddAnnotation("Starting sub span");
			FPlatformProcess::Sleep(1);
			SubSpan3.End();
		}
	});
#endif // TRACE_LIB_ACTIVE
}
