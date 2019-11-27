// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SpatialLatencyTracing.h"

#include "Async/Async.h"
#include "Utils/SchemaUtils.h"

#include <sstream>

DEFINE_LOG_CATEGORY(LogSpatialLatencyTracing);

// Stream for piping trace lib output to UE output
class UEStream : public std::stringbuf
{
	int sync()
	{
		UE_LOG(LogSpatialLatencyTracing, Log, TEXT("%s"), *FString(str().c_str()));
		str("");
		return std::stringbuf::sync();
	}
};

#if TRACE_LIB_ACTIVE
using namespace improbable::exporters::trace;
using namespace improbable::trace;

TMap<USpatialLatencyTracing::ActorFuncKey, TraceKey> USpatialLatencyTracing::TrackingTraces;
TMap<TraceKey, USpatialLatencyTracing::TraceSpan> USpatialLatencyTracing::TraceMap;
FCriticalSection USpatialLatencyTracing::Mutex;
#endif

namespace
{
	UEStream Stream;
}

void USpatialLatencyTracing::RegisterProject(const FString& ProjectId)
{
#if TRACE_LIB_ACTIVE
	StackdriverExporter::Register({ TCHAR_TO_UTF8(*ProjectId) });

	std::cout.rdbuf(&Stream);
	std::cerr.rdbuf(&Stream);

	StdoutExporter::Register();
#endif // TRACE_LIB_ACTIVE
}

bool USpatialLatencyTracing::BeginLatencyTrace(const AActor* Actor, const FString& FunctionName, const FString& TraceDesc)
{
#if TRACE_LIB_ACTIVE
	FScopeLock Lock(&Mutex);

	TraceKey Key = CreateNewTraceEntry(Actor, FunctionName);
	if (Key == InvalidTraceKey)
	{
		return false;
	}

	TraceSpan NewTrace = Span::StartSpan(TCHAR_TO_UTF8(*TraceDesc), nullptr);

	WriteKeyFrameToTrace(&NewTrace, FString::Printf(TEXT("Begin trace : %s"), *FunctionName));

	TraceMap.Add(Key, MoveTemp(NewTrace));
	
	return true;
#else
	return false;
#endif // TRACE_LIB_ACTIVE
}

bool USpatialLatencyTracing::ContinueLatencyTrace(const AActor* Actor, const FString& FunctionName, const FString& TraceDesc)
{
#if TRACE_LIB_ACTIVE
	FScopeLock Lock(&Mutex);

	TraceSpan* ActiveTrace = GetActiveTrace();
	if (ActiveTrace == nullptr)
	{
		return false;
	}

	TraceKey Key = CreateNewTraceEntry(Actor, FunctionName);
	if (Key == InvalidTraceKey)
	{
		return false;
	}

	WriteKeyFrameToTrace(ActiveTrace, TCHAR_TO_UTF8(*TraceDesc));
	WriteKeyFrameToTrace(ActiveTrace, FString::Printf(TEXT("Continue trace : %s"), *FunctionName));

	TraceMap.Add(Key, MoveTemp(*ActiveTrace));
	TraceMap.Remove(ActiveTraceKey);

	return true;
#else
	return false;
#endif // TRACE_LIB_ACTIVE
}

bool USpatialLatencyTracing::EndLatencyTrace()
{
#if TRACE_LIB_ACTIVE
	FScopeLock Lock(&Mutex);

	TraceSpan* ActiveTrace = GetActiveTrace();
	if (ActiveTrace == nullptr)
	{
		return false;
	}

	WriteKeyFrameToTrace(ActiveTrace, TEXT("End Trace"));
	ActiveTrace->End();

	TraceMap.Remove(ActiveTraceKey);

	return true;
#else
	return false;
#endif // TRACE_LIB_ACTIVE
}

#if TRACE_LIB_ACTIVE
bool USpatialLatencyTracing::IsValidKey(const TraceKey& Key)
{
	FScopeLock Lock(&Mutex);
	return TraceMap.Find(Key);
}

TraceKey USpatialLatencyTracing::GetTraceKey(const UObject* Obj, const UFunction* Function)
{
	FScopeLock Lock(&Mutex);

	ActorFuncKey FuncKey{ Cast<AActor>(Obj), Function };
	TraceKey ReturnKey = InvalidTraceKey;
	TrackingTraces.RemoveAndCopyValue(FuncKey, ReturnKey);
	return ReturnKey;
}

void USpatialLatencyTracing::WriteToLatencyTrace(const TraceKey& Key, const FString& TraceDesc)
{
	FScopeLock Lock(&Mutex);

	if (TraceSpan* Trace = TraceMap.Find(Key))
	{
		WriteKeyFrameToTrace(Trace, TraceDesc);
	}
}

void USpatialLatencyTracing::EndLatencyTrace(const TraceKey& Key, const FString& TraceDesc)
{
	FScopeLock Lock(&Mutex);

	if (TraceSpan* Trace = TraceMap.Find(Key))
	{
		WriteKeyFrameToTrace(Trace, TraceDesc);

		Trace->End();
		TraceMap.Remove(Key);
	}
}

void USpatialLatencyTracing::WriteTraceToSchemaObject(const TraceKey& Key, Schema_Object* Obj)
{
	FScopeLock Lock(&Mutex);

	if (TraceSpan* Trace = TraceMap.Find(Key))
	{
		const SpanContext& TraceContext = Trace->context();
		TraceId _TraceId = TraceContext.trace_id();
		SpanId _SpanId = TraceContext.span_id();

		SpatialGDK::AddBytesToSchema(Obj, SpatialConstants::UNREAL_RPC_TRACE_ID, &_TraceId[0], _TraceId.size());
		SpatialGDK::AddBytesToSchema(Obj, SpatialConstants::UNREAL_RPC_SPAN_ID, &_SpanId[0], _SpanId.size());
	}
}

TraceKey USpatialLatencyTracing::ReadTraceFromSchemaObject(Schema_Object* Obj)
{
	FScopeLock Lock(&Mutex);

	check(GetActiveTrace() == nullptr);

	const uint8* TraceBytes = Schema_GetBytes(Obj, SpatialConstants::UNREAL_RPC_TRACE_ID);
	const uint8* SpanBytes = Schema_GetBytes(Obj, SpatialConstants::UNREAL_RPC_SPAN_ID);

	TraceId _TraceId;
	memcpy(&_TraceId[0], TraceBytes, sizeof(TraceId));

	SpanId _SpanId;
	memcpy(&_SpanId[0], SpanBytes, sizeof(SpanId));

	SpanContext DestContext(_TraceId, _SpanId);

	TraceSpan RetrieveTrace = Span::StartSpanWithRemoteParent("Read Trace From Schema Obj", DestContext);
	TraceMap.Add(ActiveTraceKey, MoveTemp(RetrieveTrace));

	return ActiveTraceKey;
}

TraceKey USpatialLatencyTracing::CreateNewTraceEntry(const AActor* Actor, const FString& FunctionName)
{
	static TraceKey NextTraceKey = 1;

	if (UClass* ActorClass = Actor->GetClass())
	{
		if (UFunction* Function = ActorClass->FindFunctionByName(*FunctionName))
		{
			ActorFuncKey Key{ Actor, Function };
			if (TrackingTraces.Find(Key) == nullptr)
			{
				TrackingTraces.Add(Key, NextTraceKey);
				return NextTraceKey++;
			}
		}
	}

	return InvalidTraceKey;
}

USpatialLatencyTracing::TraceSpan* USpatialLatencyTracing::GetActiveTrace()
{
	return TraceMap.Find(ActiveTraceKey);
}

void USpatialLatencyTracing::WriteKeyFrameToTrace(const TraceSpan* Trace, const FString& TraceDesc)
{
	if (Trace != nullptr)
	{
		Span SubTrace = Span::StartSpan(TCHAR_TO_UTF8(*TraceDesc), Trace);
		SubTrace.End();
	}
}
#endif // TRACE_LIB_ACTIVE

void USpatialLatencyTracing::SendTestTrace()
{
#if TRACE_LIB_ACTIVE
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, []
	{
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
