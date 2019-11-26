// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SpatialLatencyTracing.h"

#include "EngineClasses/SpatialNetDriver.h"

//#ifdef TRACE_LIB_ACTIVE
#include "Async/Async.h"
#include "WorkerSDK/improbable/trace.h"

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

using namespace improbable::exporters::trace;
using namespace improbable::trace;

TMap<ActorFuncTrack, TraceKey> USpatialLatencyTracing::TrackingTraces;
TMap<TraceKey, TraceSpan> USpatialLatencyTracing::TraceMap;
FCriticalSection USpatialLatencyTracing::Mutex;

namespace
{
	UEStream Stream;
	const TraceKey ActiveTraceKey = 0;
	const TraceKey InvalidTraceKey = -1;
}

void USpatialLatencyTracing::RegisterProject(const FString& ProjectId)
{
	StackdriverExporter::Register({ TCHAR_TO_UTF8(*ProjectId) });

	std::cout.rdbuf(&Stream);
	std::cerr.rdbuf(&Stream);

	StdoutExporter::Register();
}

bool USpatialLatencyTracing::BeginLatencyTrace(const AActor* Actor, const FString& FunctionName, const FString& TraceDesc)
{
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
}

bool USpatialLatencyTracing::ContinueLatencyTrace(const AActor* Actor, const FString& FunctionName, const FString& TraceDesc)
{
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
}

bool USpatialLatencyTracing::EndLatencyTrace()
{
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
}

bool USpatialLatencyTracing::IsValidKey(const TraceKey& Key)
{
	bool bValid = TraceMap.Find(Key);
	return bValid;
}

TraceKey USpatialLatencyTracing::GetTraceKey(const UObject* Obj, const UFunction* Function)
{
	FScopeLock Lock(&Mutex);

	ActorFuncTrack FuncKey{ Cast<AActor>(Obj), Function };
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

void USpatialLatencyTracing::WriteToSchemaObject(Schema_Object* Obj, const TraceKey& Key)
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

TraceKey USpatialLatencyTracing::ReadFromSchemaObject(Schema_Object* Obj)
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
			ActorFuncTrack Key{ Actor, Function };
			if (TrackingTraces.Find(Key) == nullptr)
			{
				TrackingTraces.Add(Key, NextTraceKey);
				return NextTraceKey++;
			}
		}
	}

	return InvalidTraceKey;
}

TraceSpan* USpatialLatencyTracing::GetActiveTrace()
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

void USpatialLatencyTracing::SendTestTrace()
{
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
}

//#endif // TRACE_LIB_ACTOR
