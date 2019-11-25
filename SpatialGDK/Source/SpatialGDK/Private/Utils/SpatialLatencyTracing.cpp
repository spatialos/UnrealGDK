// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SpatialLatencyTracing.h"

#include "EngineClasses/SpatialNetDriver.h"

//#ifdef TRACE_LIB_ACTIVE
#include "Async/Async.h"
#include "WorkerSDK/improbable/trace.h"

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
}

void USpatialLatencyTracing::Init(USpatialNetDriver* InNetDriver)
{
	NetDriver = InNetDriver;

	StackdriverExporter::Register({ "holocentroid-aimful-6523579", /*rpc_deadline_ms*/5000 });

	std::cout.rdbuf(&Stream);
	std::cerr.rdbuf(&Stream);

	StdoutExporter::Register();
}

bool USpatialLatencyTracing::BeginLatencyTrace(AActor* Actor, const FString& FunctionName, const FString& TraceDesc)
{
	UClass* ActorClass = Actor->GetClass();

	if (UFunction* Function = ActorClass->FindFunctionByName(*FunctionName))
	{
		const TraceKey Key(Actor, Function);
		if (!TraceMap.Find(Key))
		{
			Span NewTrace = Span::StartSpan(TCHAR_TO_UTF8(*TraceDesc), nullptr);

			TraceMap.Add(Key, MoveTemp(NewTrace));
			return true;
		}
	}

	return false;
}

bool USpatialLatencyTracing::ContinueLatencyTrace(AActor* Actor, const FString& FunctionName, const FString& TraceDesc)
{
	UClass* ActorClass = Actor->GetClass();

	UFunction* Function = ActorClass->FindFunctionByName(*FunctionName);

	return true;
}

bool USpatialLatencyTracing::EndLatencyTrace()
{
	return true;
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

TraceSpan* USpatialLatencyTracing::GetTrace(AActor* Actor, UFunction* Function)
{
	return TraceMap.Find(TraceKey( Actor, Function ));
}

TraceSpan* USpatialLatencyTracing::GetTrace(UObject* Obj, UFunction* Function)
{
	return GetTrace(Cast<AActor>(Obj), Function);
}

//#endif // TRACE_LIB_ACTOR
