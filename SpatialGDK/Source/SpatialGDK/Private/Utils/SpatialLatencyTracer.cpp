// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SpatialLatencyTracer.h"

#include "Async/Async.h"
#include "Engine/World.h"
#include "EngineClasses/SpatialGameInstance.h"
#include "Utils/SchemaUtils.h"

#include <sstream>

DEFINE_LOG_CATEGORY(LogSpatialLatencyTracing);

// Stream for piping trace lib output to UE output
class UEStream : public std::stringbuf
{
	int sync()
	{
		UE_LOG(LogSpatialLatencyTracing, Verbose, TEXT("%s"), *FString(str().c_str()));
		str("");
		return std::stringbuf::sync();
	}
};

namespace
{
	UEStream Stream;
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

bool USpatialLatencyTracer::BeginLatencyTrace(UObject* WorldContextObject, const AActor* Actor, const FString& FunctionName, const FString& TraceDesc)
{
#if TRACE_LIB_ACTIVE
	if (USpatialLatencyTracer* Tracer = GetTracer(WorldContextObject))
	{
		return Tracer->BeginLatencyTrace_Internal(Actor, FunctionName, TraceDesc);
	}
#endif // TRACE_LIB_ACTIVE
	return false;
}

bool USpatialLatencyTracer::ContinueLatencyTrace(UObject* WorldContextObject, const AActor* Actor, const FString& FunctionName, const FString& TraceDesc)
{
#if TRACE_LIB_ACTIVE
	if (USpatialLatencyTracer* Tracer = GetTracer(WorldContextObject))
	{
		return Tracer->ContinueLatencyTrace_Internal(Actor, FunctionName, TraceDesc);
	}
#endif // TRACE_LIB_ACTIVE
	return false;
}

bool USpatialLatencyTracer::EndLatencyTrace(UObject* WorldContextObject)
{
#if TRACE_LIB_ACTIVE
	if (USpatialLatencyTracer* Tracer = GetTracer(WorldContextObject))
	{
		return Tracer->EndLatencyTrace_Internal();
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

TraceKey USpatialLatencyTracer::GetTraceKey(const UObject* Obj, const UFunction* Function)
{
	FScopeLock Lock(&Mutex);

	ActorFuncKey FuncKey{ Cast<AActor>(Obj), Function };
	TraceKey ReturnKey = InvalidTraceKey;
	TrackingTraces.RemoveAndCopyValue(FuncKey, ReturnKey);
	return ReturnKey;
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

	check(GetActiveTrace() == nullptr);

	if (Schema_GetObjectCount(Obj, FieldId) > 0)
	{
		Schema_Object* TraceData = Schema_IndexObject(Obj, FieldId, 0);

		const uint8* TraceBytes = Schema_GetBytes(TraceData, SpatialConstants::UNREAL_RPC_TRACE_ID);
		const uint8* SpanBytes = Schema_GetBytes(TraceData, SpatialConstants::UNREAL_RPC_SPAN_ID);

		improbable::trace::TraceId _TraceId;
		memcpy(&_TraceId[0], TraceBytes, sizeof(improbable::trace::TraceId));

		improbable::trace::SpanId _SpanId;
		memcpy(&_SpanId[0], SpanBytes, sizeof(improbable::trace::SpanId));

		improbable::trace::SpanContext DestContext(_TraceId, _SpanId);

		FString SpanMsg = FormatMessage(TEXT("Read Trace From Schema Obj"));
		TraceSpan RetrieveTrace = improbable::trace::Span::StartSpanWithRemoteParent(TCHAR_TO_UTF8(*SpanMsg), DestContext);
		TraceMap.Add(ActiveTraceKey, MoveTemp(RetrieveTrace));

		return ActiveTraceKey;
	}

	return InvalidTraceKey;
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

bool USpatialLatencyTracer::BeginLatencyTrace_Internal(const AActor* Actor, const FString& FunctionName, const FString& TraceDesc)
{
	FScopeLock Lock(&Mutex);

	TraceKey Key = CreateNewTraceEntry(Actor, FunctionName);
	if (Key == InvalidTraceKey)
	{
		UE_LOG(LogSpatialLatencyTracing, Warning, TEXT("(%s) : Failed to create Actor/Func trace (%s)"), *WorkerId, *TraceDesc);
		return false;
	}

	FString SpanMsg = FormatMessage(TraceDesc);
	TraceSpan NewTrace = improbable::trace::Span::StartSpan(TCHAR_TO_UTF8(*SpanMsg), nullptr);

	WriteKeyFrameToTrace(&NewTrace, FString::Printf(TEXT("Begin trace : %s"), *FunctionName));

	TraceMap.Add(Key, MoveTemp(NewTrace));

	return true;
}

bool USpatialLatencyTracer::ContinueLatencyTrace_Internal(const AActor* Actor, const FString& FunctionName, const FString& TraceDesc)
{
	FScopeLock Lock(&Mutex);

	TraceSpan* ActiveTrace = GetActiveTrace();
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

	TraceMap.Add(Key, MoveTemp(*ActiveTrace));
	TraceMap.Remove(ActiveTraceKey);

	return true;
}

bool USpatialLatencyTracer::EndLatencyTrace_Internal()
{
	FScopeLock Lock(&Mutex);

	TraceSpan* ActiveTrace = GetActiveTrace();
	if (ActiveTrace == nullptr)
	{
		UE_LOG(LogSpatialLatencyTracing, Warning, TEXT("(%s) : No active trace to end"), *WorkerId);
		return false;
	}

	WriteKeyFrameToTrace(ActiveTrace, TEXT("End Trace"));
	ActiveTrace->End();

	TraceMap.Remove(ActiveTraceKey);

	return true;
}

TraceKey USpatialLatencyTracer::CreateNewTraceEntry(const AActor* Actor, const FString& FunctionName)
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
			UE_LOG(LogSpatialLatencyTracing, Warning, TEXT("(%s) : ActorFunc already exists for trace"), *WorkerId);
		}
	}

	return InvalidTraceKey;
}

USpatialLatencyTracer::TraceSpan* USpatialLatencyTracer::GetActiveTrace()
{
	return TraceMap.Find(ActiveTraceKey);
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

void USpatialLatencyTracer::SendTestTrace()
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
