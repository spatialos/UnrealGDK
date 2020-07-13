// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SpatialLatencyTracer.h"

#include "Async/Async.h"
#include "Engine/World.h"
#include "EngineClasses/SpatialGameInstance.h"

#include <sstream>

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

	UEStream UStream;
}  // anonymous namespace

USpatialLatencyTracer::USpatialLatencyTracer()
{
#if TRACE_LIB_ACTIVE
	Tracer = MakeShared<SpatialGDK::SpatialLatencyTracerData, ESPMode::ThreadSafe>();
#endif
}

void USpatialLatencyTracer::RegisterProject(UObject* WorldContextObject, const FString& ProjectId)
{
#if TRACE_LIB_ACTIVE
	using namespace improbable::exporters::trace;

	StackdriverExporter::Register({ TCHAR_TO_UTF8(*ProjectId) });

	if (UE_GET_LOG_VERBOSITY(LogSpatialLatencyTracing) >= ELogVerbosity::Verbose)
	{
		std::cout.rdbuf(&UStream);
		std::cerr.rdbuf(&UStream);

		StdoutExporter::Register();
	}
#endif // TRACE_LIB_ACTIVE
}

bool USpatialLatencyTracer::SetTraceMetadata(UObject* WorldContextObject, const FString& NewTraceMetadata)
{
#if TRACE_LIB_ACTIVE
	if (SpatialGDK::TracerSharedPtr Tracer = GetTracer(WorldContextObject))
	{
		Tracer->TraceMetadata = NewTraceMetadata;
		return true;
	}
#endif // TRACE_LIB_ACTIVE
	return false;
}

bool USpatialLatencyTracer::BeginLatencyTrace(UObject* WorldContextObject, const FString& TraceDesc, FSpatialLatencyPayload& OutLatencyPayload)
{
#if TRACE_LIB_ACTIVE
	if (SpatialGDK::TracerSharedPtr Tracer = GetTracer(WorldContextObject))
	{
		return Tracer->BeginLatencyTrace_Internal(TraceDesc, OutLatencyPayload);
	}
#endif // TRACE_LIB_ACTIVE
	return false;
}

bool USpatialLatencyTracer::ContinueLatencyTraceRPC(UObject* WorldContextObject, const AActor* Actor, const FString& FunctionName, const FString& TraceDesc, const FSpatialLatencyPayload& LatencyPayload, FSpatialLatencyPayload& OutContinuedLatencyPayload)
{
#if TRACE_LIB_ACTIVE
	if (SpatialGDK::TracerSharedPtr Tracer = GetTracer(WorldContextObject))
	{
		return Tracer->ContinueLatencyTrace_Internal(Actor, FunctionName, ETraceType::RPC, TraceDesc, LatencyPayload, OutContinuedLatencyPayload);
	}
#endif // TRACE_LIB_ACTIVE
	return false;
}

bool USpatialLatencyTracer::ContinueLatencyTraceProperty(UObject* WorldContextObject, const AActor* Actor, const FString& PropertyName, const FString& TraceDesc, const FSpatialLatencyPayload& LatencyPayload, FSpatialLatencyPayload& OutContinuedLatencyPayload)
{
#if TRACE_LIB_ACTIVE
	if (SpatialGDK::TracerSharedPtr Tracer = GetTracer(WorldContextObject))
	{
		return Tracer->ContinueLatencyTrace_Internal(Actor, PropertyName, ETraceType::Property, TraceDesc, LatencyPayload, OutContinuedLatencyPayload);
	}
#endif // TRACE_LIB_ACTIVE
	return false;
}

bool USpatialLatencyTracer::ContinueLatencyTraceTagged(UObject* WorldContextObject, const AActor* Actor, const FString& Tag, const FString& TraceDesc, const FSpatialLatencyPayload& LatencyPayload, FSpatialLatencyPayload& OutContinuedLatencyPayload)
{
#if TRACE_LIB_ACTIVE
	if (SpatialGDK::TracerSharedPtr Tracer = GetTracer(WorldContextObject))
	{
		return Tracer->ContinueLatencyTrace_Internal(Actor, Tag, ETraceType::Tagged, TraceDesc, LatencyPayload, OutContinuedLatencyPayload);
	}
#endif // TRACE_LIB_ACTIVE
	return false;
}

bool USpatialLatencyTracer::EndLatencyTrace(UObject* WorldContextObject, const FSpatialLatencyPayload& LatencyPayload)
{
#if TRACE_LIB_ACTIVE
	if (SpatialGDK::TracerSharedPtr Tracer = GetTracer(WorldContextObject))
	{
		return Tracer->EndLatencyTrace_Internal(LatencyPayload);
	}
#endif // TRACE_LIB_ACTIVE
	return false;
}

FSpatialLatencyPayload USpatialLatencyTracer::RetrievePayload(UObject* WorldContextObject, const AActor* Actor, const FString& Tag)
{
#if TRACE_LIB_ACTIVE
	if (SpatialGDK::TracerSharedPtr Tracer = GetTracer(WorldContextObject))
	{
		return Tracer->RetrievePayload_Internal(Actor, Tag);
	}
#endif
	return FSpatialLatencyPayload{};
}

SpatialGDK::TracerSharedPtr USpatialLatencyTracer::GetTracer(UObject* WorldContextObject)
{
#if TRACE_LIB_ACTIVE
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (World == nullptr)
	{
		World = GWorld;
	}

	if (USpatialGameInstance* GameInstance = World->GetGameInstance<USpatialGameInstance>())
	{
		return GameInstance->GetSpatialLatencyTracer()->Tracer;
	}
#endif
	return nullptr;
}

SpatialGDK::TracerSharedPtr USpatialLatencyTracer::GetTracer()
{
	return Tracer;
}

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

void USpatialLatencyTracer::SetWorkerId(const FString& NewWorkerId)
{
#if TRACE_LIB_ACTIVE
	if (Tracer.IsValid())
	{
		Tracer->SetWorkerId(NewWorkerId);
	}
#endif // TRACE_LIB_ACTIVE
}

void USpatialLatencyTracer::ResetWorkerId()
{
#if TRACE_LIB_ACTIVE
	if (Tracer.IsValid())
	{
		Tracer->ResetWorkerId();
	}
#endif // TRACE_LIB_ACTIVE
}
