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

TMap<TraceKey, TraceSpan> USpatialLatencyTracing::TraceMap;

namespace
{
	UEStream Stream;
	TraceKey ActiveTraceKey{ nullptr, nullptr };
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
	TraceKey Key;
	if (!CreateTraceKey(Actor, FunctionName, Key))
	{
		return false;
	}

	if (TraceMap.Find(Key))
	{
		return false;
	}

	TraceSpan NewTrace = Span::StartSpan(TCHAR_TO_UTF8(*TraceDesc), nullptr);

	AddKeyFrameToTrace(&NewTrace, FString::Printf(TEXT("Begin trace : %s"), *FunctionName));

	TraceMap.Add(Key, MoveTemp(NewTrace));
	return true;
}

bool USpatialLatencyTracing::ContinueLatencyTrace(const AActor* Actor, const FString& FunctionName, const FString& TraceDesc)
{
	TraceKey Key;
	if (!CreateTraceKey(Actor, FunctionName, Key))
	{
		return false;
	}

	if (TraceMap.Find(Key))
	{
		return false;
	}

	TraceSpan* ActiveTrace = GetActiveTrace();
	if (ActiveTrace == nullptr)
	{
		return false;
	}

	AddKeyFrameToTrace(ActiveTrace, TCHAR_TO_UTF8(*TraceDesc));
	AddKeyFrameToTrace(ActiveTrace, FString::Printf(TEXT("Continue trace : %s"), *FunctionName));

	TraceMap.Add(Key, MoveTemp(*ActiveTrace));
	TraceMap.Remove(ActiveTraceKey);

	return true;
}

bool USpatialLatencyTracing::EndLatencyTrace()
{
	TraceSpan* ActiveTrace = GetActiveTrace();
	if (ActiveTrace == nullptr)
	{
		return false;
	}

	AddKeyFrameToTrace(ActiveTrace, TEXT("End Trace"));
	ActiveTrace->End();

	TraceMap.Remove(ActiveTraceKey);

	return true;
}

void USpatialLatencyTracing::EndLatencyTrace(const TraceSpan* Trace, const FString& TraceDesc)
{

}

TraceKey USpatialLatencyTracing::CreateTraceKey(const UObject* Obj, const UFunction* Function)
{
	return TraceKey(Cast<AActor>(Obj), Function);
}

bool USpatialLatencyTracing::IsValidKey(const TraceKey& Key)
{
	bool bValid = TraceMap.Find(Key);
	return bValid;
}

void USpatialLatencyTracing::AddKeyFrameToTrace(const TraceSpan* Trace, const FString& TraceDesc)
{
	if (Trace != nullptr)
	{
		Span SubTrace = Span::StartSpan(TCHAR_TO_UTF8(*TraceDesc), Trace);
		SubTrace.End();
	}
}

void USpatialLatencyTracing::WriteToSchemaObject(Schema_Object* Obj, const TraceKey& Key)
{
	if (TraceSpan* Trace = TraceMap.Find(Key))
	{
		const SpanContext& TraceContext = Trace->context();
		TraceId _TraceId = TraceContext.trace_id();
		SpanId _SpanId = TraceContext.span_id();

		SpatialGDK::AddBytesToSchema(Obj, 1, &_TraceId[0], _TraceId.size());
		SpatialGDK::AddBytesToSchema(Obj, 2, &_SpanId[0], _SpanId.size());
	}
}

void USpatialLatencyTracing::ReadFromSchemaObject(Schema_Object* Obj)
{
	check(GetActiveTrace() == nullptr);

	uint32 TraceSize = Schema_GetBytesLength(Obj, 1);
	const uint8* TraceBytes = Schema_GetBytes(Obj, 1);
	uint32 SpanSize = Schema_GetBytesLength(Obj, 2);
	const uint8* SpanBytes = Schema_GetBytes(Obj, 2);

	TraceId _TraceId;
	memcpy(&_TraceId[0], TraceBytes, TraceSize);

	SpanId _SpanId;
	memcpy(&_SpanId[0], SpanBytes, SpanSize);

	SpanContext DestContext(_TraceId, _SpanId);

	TraceSpan RetrieveTrace = Span::StartSpanWithRemoteParent("Read Trace From Schema Obj", DestContext);
	TraceMap.Add(ActiveTraceKey, MoveTemp(RetrieveTrace));
}

bool USpatialLatencyTracing::CreateTraceKey(const AActor* Actor, const FString& FunctionName, TraceKey& OutKey)
{
	if (UClass* ActorClass = Actor->GetClass())
	{
		if (UFunction* Function = ActorClass->FindFunctionByName(*FunctionName))
		{
			OutKey = TraceKey( Actor, Function );
			return true;
		}
	}

	return false;
}

TraceSpan* USpatialLatencyTracing::GetActiveTrace()
{
	return TraceMap.Find(ActiveTraceKey);
}


void USpatialLatencyTracing::SendTestTrace()
{
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, []
	{
		std::cout << "Sending test trace" << std::endl;

		Span RootSpan = Span::StartSpan("Example Span", nullptr);
		//RootSpan.AddAnnotation("Starting root span");

		{
			Span SubSpan1 = Span::StartSpan("Sub span 1", &RootSpan);
			//SubSpan1.AddAnnotation("Starting sub span");

			FPlatformProcess::Sleep(1);

			SubSpan1.End();
		}

		{
			Span SubSpan2 = Span::StartSpan("Sub span 2", &RootSpan);
			//SubSpan2.AddAnnotation("Starting sub span");

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
