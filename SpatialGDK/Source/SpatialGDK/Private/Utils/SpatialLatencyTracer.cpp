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

DECLARE_CYCLE_STAT(TEXT("ContinueLatencyTraceRPC_Internal"), STAT_ContinueLatencyTraceRPC_Internal, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("BeginLatencyTraceRPC_Internal"), STAT_BeginLatencyTraceRPC_Internal, STATGROUP_SpatialNet);


void PrintTrace(const improbable::trace::SpanContext& TraceContext, FString Prefix)
{
	auto t = TraceContext.trace_id();
	auto s = TraceContext.span_id();
	UE_LOG(LogTemp, Warning, TEXT("%s"), *Prefix);
	UE_LOG(LogTemp, Warning, TEXT("Trace %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d"), t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7], t[8], t[9], t[10], t[11], t[12], t[13], t[14], t[15]);
	UE_LOG(LogTemp, Warning, TEXT("Span %d %d %d %d %d %d %d %d"), s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7]);
}

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
	//ActiveTraceKey = InvalidTraceKey;
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

bool USpatialLatencyTracer::SetMessagePrefix(UObject* WorldContextObject, const FString& NewMessagePrefix)
{
#if TRACE_LIB_ACTIVE
	if (USpatialLatencyTracer* Tracer = GetTracer(WorldContextObject))
	{
		Tracer->MessagePrefix = NewMessagePrefix;
		return true;
	}
#endif // TRACE_LIB_ACTIVE
	return false;
}

bool USpatialLatencyTracer::BeginLatencyTrace(UObject* WorldContextObject, const FString& TraceDesc, FSpatialLatencyPayload& OutLatencyPayload)
{
#if TRACE_LIB_ACTIVE
	if (USpatialLatencyTracer* Tracer = GetTracer(WorldContextObject))
	{
		return Tracer->BeginLatencyTrace_Internal(TraceDesc, OutLatencyPayload);
	}
#endif // TRACE_LIB_ACTIVE
	return false;
}

bool USpatialLatencyTracer::ContinueLatencyTraceRPC(UObject* WorldContextObject, const AActor* Actor, const FString& FunctionName, const FString& TraceDesc, const FSpatialLatencyPayload& LatencyPayload, FSpatialLatencyPayload& OutLatencyPayload)
{
#if TRACE_LIB_ACTIVE
	if (USpatialLatencyTracer* Tracer = GetTracer(WorldContextObject))
	{
		return Tracer->ContinueLatencyTrace_Internal(Actor, FunctionName, ETraceType::RPC, TraceDesc, LatencyPayload, OutLatencyPayload);
	}
#endif // TRACE_LIB_ACTIVE
	return false;
}

bool USpatialLatencyTracer::ContinueLatencyTraceProperty(UObject* WorldContextObject, const AActor* Actor, const FString& PropertyName, const FString& TraceDesc, const FSpatialLatencyPayload& LatencyPayload, FSpatialLatencyPayload& OutLatencyPayload)
{
#if TRACE_LIB_ACTIVE
	if (USpatialLatencyTracer* Tracer = GetTracer(WorldContextObject))
	{
		return Tracer->ContinueLatencyTrace_Internal(Actor, PropertyName, ETraceType::Property, TraceDesc, LatencyPayload, OutLatencyPayload);
	}
#endif // TRACE_LIB_ACTIVE
	return false;
}

bool USpatialLatencyTracer::ContinueLatencyTraceTagged(UObject* WorldContextObject, const AActor* Actor, const FString& Tag, const FString& TraceDesc, const FSpatialLatencyPayload& LatencyPayload, FSpatialLatencyPayload& OutLatencyPayload)
{
#if TRACE_LIB_ACTIVE
	if (USpatialLatencyTracer* Tracer = GetTracer(WorldContextObject))
	{
		return Tracer->ContinueLatencyTrace_Internal(Actor, Tag, ETraceType::Tagged, TraceDesc, LatencyPayload, OutLatencyPayload);
	}
#endif // TRACE_LIB_ACTIVE
	return false;
}

bool USpatialLatencyTracer::EndLatencyTrace(UObject* WorldContextObject, const FSpatialLatencyPayload& LatencyPayload)
{
#if TRACE_LIB_ACTIVE
	if (USpatialLatencyTracer* Tracer = GetTracer(WorldContextObject))
	{
		return Tracer->EndLatencyTrace_Internal(LatencyPayload);
	}
#endif // TRACE_LIB_ACTIVE
	return false;
}

// bool USpatialLatencyTracer::IsLatencyTraceActive(UObject* WorldContextObject)
// {
// #if TRACE_LIB_ACTIVE
// 	if (USpatialLatencyTracer* Tracer = GetTracer(WorldContextObject))
// 	{
// 		return Tracer->IsLatencyTraceActive_Internal();
// 	}
// #endif // TRACE_LIB_ACTIVE
// 	return false;
// }

FSpatialLatencyPayload USpatialLatencyTracer::RetrievePayload(UObject* WorldContextObject, const AActor* Actor, const FString& Tag)
{
#if TRACE_LIB_ACTIVE
	if (USpatialLatencyTracer* Tracer = GetTracer(WorldContextObject))
	{
		return Tracer->RetrievePayload_Internal(Actor, Tag);
	}
#endif
	return FSpatialLatencyPayload{};
}

USpatialLatencyTracer* USpatialLatencyTracer::GetTracer(UObject* WorldContextObject)
{
#if TRACE_LIB_ACTIVE
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (World == nullptr)
	{
		World = GWorld;
	}

	if (USpatialGameInstance* GameInstance = World->GetGameInstance<USpatialGameInstance>())
	{
		return GameInstance->GetSpatialLatencyTracer();
	}
#endif
	return nullptr;
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
	TrackingRPCs.RemoveAndCopyValue(FuncKey, ReturnKey);
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

TraceKey USpatialLatencyTracer::RetrievePendingTrace(const UObject* Obj, const FString& Tag)
{
	FScopeLock Lock(&Mutex);

	ActorTagKey EventKey{ Cast<AActor>(Obj), Tag };
	TraceKey ReturnKey = InvalidTraceKey;
	TrackingTags.RemoveAndCopyValue(EventKey, ReturnKey);
	return ReturnKey;
}

// void USpatialLatencyTracer::MarkActiveLatencyTrace(const TraceKey Key)
// {
// 	// We can safely set this to the active trace, even if Key is invalid, as other functionality
// 	// is gated on the ActiveTraceKey being present in the TraceMap
// 	ActiveTraceKey = Key;
// }

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

		if (RootTraces.Find(Key) == nullptr)
		{
			Trace->End();
			TraceMap.Remove(Key);
		}
		//TracesOnWire.Add(MoveTemp(*Trace));
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
		PrintTrace(TraceContext, FormatMessage(TEXT("Write to schema")));

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
		PrintTrace(DestContext, FormatMessage(TEXT("ReadTraceFromSchemaObject")));
		FString Stored = FormatMessage(TEXT("Stored"));

		// TODO: find previous trace
		TraceKey Key = InvalidTraceKey;

		for (const auto& TracePair : TraceMap)
		{
			const TraceKey& _Key = TracePair.Key;
			const TraceSpan& Span = TracePair.Value;

			//if (memcmp(Span.context().trace_id().data(), Payload.TraceId.GetData(), sizeof(Payload.TraceId) == 0))
			if (Span.context().trace_id() == DestContext.trace_id())
			{
				Key = _Key;
				break;
			}
		}

		if (Key != InvalidTraceKey)
		{
			//TraceSpan& StoredTrace = TracesOnWire[Idx];
			TraceSpan* Span = TraceMap.Find(Key);

			WriteKeyFrameToTrace(Span, TEXT("Read Trace From Schema Obj"));
		}
		else
		{
			FString SpanMsg = FormatMessage(TEXT("Read Trace From Schema Obj"));
			TraceSpan RetrieveTrace = improbable::trace::Span::StartSpanWithRemoteParent(TCHAR_TO_UTF8(*SpanMsg), DestContext);

			Key = GenerateNewTraceKey();
			TraceMap.Add(Key, MoveTemp(RetrieveTrace));
		}

		return Key;

		/*
		uint32 Idx = TracesOnWire.IndexOfByPredicate([&DestContext, &Stored](const TraceSpan& CurrSpan)
		{
			PrintTrace(CurrSpan.context(), Stored);
			return CurrSpan.context().trace_id() == DestContext.trace_id();
		});

		if (Idx != INDEX_NONE)
		{
			TraceSpan& StoredTrace = TracesOnWire[Idx];

			WriteKeyFrameToTrace(&StoredTrace, TEXT("Read Trace From Schema Obj"));

			//const TraceKey Key = GenerateNewTraceKey();
			//TraceMap.Add(Key, MoveTemp(StoredTrace));

			//TracesOnWire.RemoveAtSwap(Idx);
		}
		else
		{
			FString SpanMsg = FormatMessage(TEXT("Read Trace From Schema Obj"));
			TraceSpan RetrieveTrace = improbable::trace::Span::StartSpanWithRemoteParent(TCHAR_TO_UTF8(*SpanMsg), DestContext);

			const TraceKey Key = GenerateNewTraceKey();
			TraceMap.Add(Key, MoveTemp(RetrieveTrace));

			return Key;
		}
		*/
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
		UE_LOG(LogSpatialLatencyTracing, Warning, TEXT("Payload SpanId does not contain the correct number of span bytes. %d found"), Payload.SpanId.Num());
		return InvalidTraceKey;
	}

	improbable::trace::SpanContext DestContext = ReadSpanContext(Payload.TraceId.GetData(), Payload.SpanId.GetData());
	PrintTrace(DestContext, FormatMessage(TEXT("ReadTraceFromSpatialPayload")));
	FString Stored = FormatMessage(TEXT("Stored"));

	uint32 Idx = TracesOnWire.IndexOfByPredicate([&DestContext, &Stored](const TraceSpan& CurrSpan)
	{
		PrintTrace(CurrSpan.context(), Stored);
		return CurrSpan.context().trace_id() == DestContext.trace_id();
	});

	const TraceKey Key = GenerateNewTraceKey();

	if (Idx != INDEX_NONE)
	{
		TraceSpan& StoredTrace = TracesOnWire[Idx];

		WriteKeyFrameToTrace(&StoredTrace, TEXT("Read Trace From Payload Obj"));

		TraceMap.Add(Key, MoveTemp(StoredTrace));

		TracesOnWire.RemoveAtSwap(Idx);
	}
	else
	{
		FString SpanMsg = FormatMessage(TEXT("Read Trace From Payload Obj"));
		TraceSpan RetrieveTrace = improbable::trace::Span::StartSpanWithRemoteParent(TCHAR_TO_UTF8(*SpanMsg), DestContext);

		TraceMap.Add(Key, MoveTemp(RetrieveTrace));
	}

	return Key;
}

FSpatialLatencyPayload USpatialLatencyTracer::RetrievePayload_Internal(const UObject* Obj, const FString& Tag)
{
	FScopeLock Lock(&Mutex);

	 TraceKey Trace = RetrievePendingTrace(Obj, Tag);
	 if (Trace != InvalidTraceKey)
	 {
		 if (const TraceSpan* Span = TraceMap.Find(Trace))
		 {
			 const improbable::trace::SpanContext& TraceContext = Span->context();

			 TArray<uint8> TraceBytes = TArray<uint8_t>((const uint8_t*)&TraceContext.trace_id()[0], sizeof(improbable::trace::TraceId));
			 TArray<uint8> SpanBytes = TArray<uint8_t>((const uint8_t*)&TraceContext.span_id()[0], sizeof(improbable::trace::SpanId));
			 return FSpatialLatencyPayload(MoveTemp(TraceBytes), MoveTemp(SpanBytes));
		 }
	 }
	 return {};
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
		WriteToLatencyTrace(ComponentUpdate->Update.Trace, TEXT("Moved componentUpdate to Worker queue"));
	}
	else if (Message->Type == SpatialGDK::EOutgoingMessageType::AddComponent)
	{
		const SpatialGDK::FAddComponent* ComponentAdd = static_cast<const SpatialGDK::FAddComponent*>(Message);
		WriteToLatencyTrace(ComponentAdd->Data.Trace, TEXT("Moved componentAdd to Worker queue"));
	}
	else if (Message->Type == SpatialGDK::EOutgoingMessageType::CreateEntityRequest)
	{
		const SpatialGDK::FCreateEntityRequest* CreateEntityRequest = static_cast<const SpatialGDK::FCreateEntityRequest*>(Message);
		for (auto& Component : CreateEntityRequest->Components)
		{
			WriteToLatencyTrace(Component.Trace, TEXT("Moved createEntityRequest to Worker queue"));
		}
	}
}

void USpatialLatencyTracer::OnDequeueMessage(const SpatialGDK::FOutgoingMessage* Message)
{
	if (Message->Type == SpatialGDK::EOutgoingMessageType::ComponentUpdate)
	{
		const SpatialGDK::FComponentUpdate* ComponentUpdate = static_cast<const SpatialGDK::FComponentUpdate*>(Message);
		EndLatencyTrace(ComponentUpdate->Update.Trace, TEXT("Sent componentUpdate to Worker SDK"));
	}
	else if (Message->Type == SpatialGDK::EOutgoingMessageType::AddComponent)
	{
		const SpatialGDK::FAddComponent* ComponentAdd = static_cast<const SpatialGDK::FAddComponent*>(Message);
		EndLatencyTrace(ComponentAdd->Data.Trace, TEXT("Sent componentAdd to Worker SDK"));
	}
	else if (Message->Type == SpatialGDK::EOutgoingMessageType::CreateEntityRequest)
	{
		const SpatialGDK::FCreateEntityRequest* CreateEntityRequest = static_cast<const SpatialGDK::FCreateEntityRequest*>(Message);
		for (auto& Component : CreateEntityRequest->Components)
		{
			EndLatencyTrace(Component.Trace, TEXT("Sent createEntityRequest to Worker SDK"));
		}
	}
}

bool USpatialLatencyTracer::BeginLatencyTrace_Internal(const FString& TraceDesc, FSpatialLatencyPayload& OutLatencyPayload)
{	 
	// TODO: UNR-2787 - Improve mutex-related latency
	// This functions might spike because of the Mutex below
	SCOPE_CYCLE_COUNTER(STAT_BeginLatencyTraceRPC_Internal);
	FScopeLock Lock(&Mutex);

	FString SpanMsg = FormatMessage(TraceDesc);
	TraceSpan NewTrace = improbable::trace::Span::StartSpan(TCHAR_TO_UTF8(*SpanMsg), nullptr);

	// Construct payload data from trace
	const improbable::trace::SpanContext& TraceContext = NewTrace.context();
	PrintTrace(TraceContext, FormatMessage(TEXT("Begin trace")));

	{
		TArray<uint8> TraceBytes = TArray<uint8_t>((const uint8_t*)&TraceContext.trace_id()[0], sizeof(improbable::trace::TraceId));
		TArray<uint8> SpanBytes = TArray<uint8_t>((const uint8_t*)&TraceContext.span_id()[0], sizeof(improbable::trace::SpanId));
		OutLatencyPayload = FSpatialLatencyPayload(MoveTemp(TraceBytes), MoveTemp(SpanBytes));
	}

	// Add to internal tracking
	TraceKey Key = GenerateNewTraceKey();
	TraceMap.Add(Key, MoveTemp(NewTrace));

	RootTraces.Add(Key);
	OutLatencyPayload.Key = Key;
	//PayloadToTraceKeys.Add(MakeTuple(OutLatencyPayload, Key));

	return true;
}

bool USpatialLatencyTracer::ContinueLatencyTrace_Internal(const AActor* Actor, const FString& Target, ETraceType::Type Type, const FString& TraceDesc, const FSpatialLatencyPayload& LatencyPayload, FSpatialLatencyPayload& OutLatencyPayload)
{
	// TODO: UNR-2787 - Improve mutex-related latency
	// This functions might spike because of the Mutex below
	SCOPE_CYCLE_COUNTER(STAT_ContinueLatencyTraceRPC_Internal);
	if (Actor == nullptr)
	{
		return false;
	}

	FScopeLock Lock(&Mutex);

	OutLatencyPayload = LatencyPayload;
	if (OutLatencyPayload.Key == InvalidTraceKey)
	{
		ResolveKeyInLatencyPayload(OutLatencyPayload);
	}

	const TraceKey Key = OutLatencyPayload.Key;
	const TraceSpan* ActiveTrace = TraceMap.Find(Key);

	//TraceKey ActiveTraceKey = InvalidTraceKey;
	//TraceSpan* ActiveTrace = GetActiveTraceOrReadPayload(LatencyPayload, ActiveTraceKey);
	if (ActiveTrace == nullptr)
	{
		UE_LOG(LogSpatialLatencyTracing, Warning, TEXT("(%s) : No active trace to continue (%s)"), *WorkerId, *TraceDesc);
		return false;
	}

	//TraceKey Key = CreateNewTraceEntry(Actor, Target, Type, Key);
	if (!CreateNewTraceEntry(Actor, Target, Type, Key))
	{
		UE_LOG(LogSpatialLatencyTracing, Warning, TEXT("(%s) : Failed to create Actor/Func trace (%s)"), *WorkerId, *TraceDesc);
		return false;
	}

// 	if (Key == InvalidTraceKey)
// 	{
// 		UE_LOG(LogSpatialLatencyTracing, Warning, TEXT("(%s) : Failed to create Actor/Func trace (%s)"), *WorkerId, *TraceDesc);
// 		return false;
// 	}

	WriteKeyFrameToTrace(ActiveTrace, TCHAR_TO_UTF8(*TraceDesc));
	WriteKeyFrameToTrace(ActiveTrace, FString::Printf(TEXT("Continue trace %s : %s"), *UEnum::GetValueAsString(Type), *Target));

// 	// For non-spatial tracing
// 	const improbable::trace::SpanContext& TraceContext = ActiveTrace->context();
// 
// 	{
// 		TArray<uint8> TraceBytes = TArray<uint8_t>((const uint8_t*)&TraceContext.trace_id()[0], sizeof(improbable::trace::TraceId));
// 		TArray<uint8> SpanBytes = TArray<uint8_t>((const uint8_t*)&TraceContext.span_id()[0], sizeof(improbable::trace::SpanId));
// 		OutLatencyPayloadContinue = FSpatialLatencyPayload(MoveTemp(TraceBytes), MoveTemp(SpanBytes));
// 	}

	// Move the active trace to a new tracked trace
	//TraceSpan TempSpan(MoveTemp(*ActiveTrace));
	//TraceMap.Add(Key, MoveTemp(TempSpan));
	//TraceMap.Remove(ActiveTraceKey);
	//ActiveTraceKey = InvalidTraceKey;
	//PayloadToTraceKeys.Remove(LatencyPayload);
	//PayloadToTraceKeys.Add(MakeTuple(OutLatencyPayloadContinue, Key)); // Add continued payload to tracking map. 

	if (!GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking())
	{
		// We can't do any deeper tracing in the stack here so terminate these traces here
		if (Type == ETraceType::RPC || Type == ETraceType::Property)
		{
			EndLatencyTrace(Key, TEXT("End of native tracing"));
		}

		ClearTrackingInformation();
	}

	return true;
}

bool USpatialLatencyTracer::EndLatencyTrace_Internal(const FSpatialLatencyPayload& LatencyPayload)
{
	FScopeLock Lock(&Mutex);

	//TraceKey ActiveTraceKey = InvalidTraceKey;
	//TraceSpan* ActiveTrace = GetActiveTraceOrReadPayload(LatencyPayload, ActiveTraceKey);

	FSpatialLatencyPayload NonConstLatencyPayload = LatencyPayload;
	if (NonConstLatencyPayload.Key == InvalidTraceKey)
	{
		ResolveKeyInLatencyPayload(NonConstLatencyPayload);
	}

	const TraceKey Key = NonConstLatencyPayload.Key;
	const TraceSpan* ActiveTrace = TraceMap.Find(Key);

	if (ActiveTrace == nullptr)
	{
		UE_LOG(LogSpatialLatencyTracing, Warning, TEXT("(%s) : No active trace to end"), *WorkerId);
		return false;
	}

	WriteKeyFrameToTrace(ActiveTrace, TEXT("End Trace"));
	PrintTrace(ActiveTrace->context(), FormatMessage(TEXT("End trace")));
	ActiveTrace->End();

	//PayloadToTraceKeys.Remove(LatencyPayload);
	TraceMap.Remove(Key);
	RootTraces.Remove(Key);
	//ActiveTraceKey = InvalidTraceKey;

	if (!GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking())
	{
		// We can't do any deeper tracing in the stack here so terminate these traces here
		ClearTrackingInformation();
	}

	return true;
}

// bool USpatialLatencyTracer::IsLatencyTraceActive_Internal()
// {
// 	return (ActiveTraceKey != InvalidTraceKey);
// }

void USpatialLatencyTracer::ClearTrackingInformation()
{
	TraceMap.Reset();
	TrackingRPCs.Reset();
	TrackingProperties.Reset();
	TrackingTags.Reset();
	//PayloadToTraceKeys.Reset();
}

bool USpatialLatencyTracer::CreateNewTraceEntry(const AActor* Actor, const FString& Target, const ETraceType::Type Type, const TraceKey Key)
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
					//const TraceKey _TraceKey = GenerateNewTraceKey();
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
					//const TraceKey _TraceKey = GenerateNewTraceKey();
					TrackingProperties.Add(APKey, Key);
					//TrackingProperties.Add(Key, _TraceKey);
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
					//const TraceKey _TraceKey = GenerateNewTraceKey();
					TrackingTags.Add(ATKey, Key);
					return true;
					//return _TraceKey;
				}
				UE_LOG(LogSpatialLatencyTracing, Warning, TEXT("(%s) : ActorProperty already exists for trace"), *WorkerId);
			}
			break;
		}
	}

	return false;
}

TraceKey USpatialLatencyTracer::GenerateNewTraceKey()
{
	return NextTraceKey++;
}

// USpatialLatencyTracer::TraceSpan* USpatialLatencyTracer::GetActiveTrace()
// {
// 	return TraceMap.Find(ActiveTraceKey);
// }

USpatialLatencyTracer::TraceSpan* USpatialLatencyTracer::GetActiveTraceOrReadPayload(const FSpatialLatencyPayload& Payload, TraceKey& OutKey)
{
// 	if (TraceKey* ExistingKey = PayloadToTraceKeys.Find(Payload)) // This occurs if the root was created on this machine 
// 	{
// 		if (USpatialLatencyTracer::TraceSpan* Span = TraceMap.Find(*ExistingKey))
// 		{
// 			return Span;
// 		}
// 		else
// 		{
// 			UE_LOG(LogSpatialLatencyTracing, Warning, TEXT("(%s) : Could not find existing payload in TraceMap despite being tracked in the payload map."), *WorkerId);
// 		}
// 	}

	//USpatialLatencyTracer::TraceSpan* ActiveTrace = GetActiveTrace();
	USpatialLatencyTracer::TraceSpan* ActiveTrace = nullptr;
	if (ActiveTrace == nullptr)
	{
		// Try read the trace from the payload
		TraceKey Key = ReadTraceFromSpatialPayload(Payload);
		if (Key != InvalidTraceKey)
		{
			OutKey = Key;
			//MarkActiveLatencyTrace(Key);
			//ActiveTrace = GetActiveTrace();
			ActiveTrace = TraceMap.Find(Key);
		}
		else
		{
			UE_LOG(LogSpatialLatencyTracing, Warning, TEXT("(%s) : Could not read trace from payload. The payload was likely invalid."), *WorkerId);
		}
	}
	return ActiveTrace;
}

bool USpatialLatencyTracer::ResolveKeyInLatencyPayload(FSpatialLatencyPayload& Payload)
{
	// Key isn't set, so find it in the trace map
	for (const auto& TracePair : TraceMap)
	{
		const TraceKey& Key = TracePair.Key;
		const TraceSpan& Span = TracePair.Value;

		if (memcmp(Span.context().trace_id().data(), Payload.TraceId.GetData(), sizeof(Payload.TraceId)) == 0)
		{
			Payload.Key = Key;
			break;
		}
	}

	if (Payload.Key == InvalidTraceKey)
	{
		// Uninitialized key, generate and add to map
		Payload.Key = GenerateNewTraceKey();

		improbable::trace::SpanContext DestContext = ReadSpanContext(Payload.TraceId.GetData(), Payload.SpanId.GetData());

		FString SpanMsg = FormatMessage(TEXT("Read Trace From Payload Obj TODO"));
		TraceSpan RetrieveTrace = improbable::trace::Span::StartSpanWithRemoteParent(TCHAR_TO_UTF8(*SpanMsg), DestContext);

		TraceMap.Add(Payload.Key, MoveTemp(RetrieveTrace));
	}

	return true;
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
	return FString::Printf(TEXT("%s(%s) : %s"), *MessagePrefix, *WorkerId.Left(18), *Message);
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
