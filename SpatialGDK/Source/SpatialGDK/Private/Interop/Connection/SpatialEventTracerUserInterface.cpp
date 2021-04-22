// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialEventTracerUserInterface.h"

#include "Engine/Engine.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"

#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialClassInfoManager.h"

DEFINE_LOG_CATEGORY(LogSpatialEventTracerUserInterface);

namespace
{
TArray<FSpatialGDKSpanId> ConvertSpanIds(const TArray<FUserSpanId>& Causes)
{
	TArray<FSpatialGDKSpanId> CauseSpanIds;
	for (const FUserSpanId& UserSpanIdCause : Causes)
	{
		if (!UserSpanIdCause.IsValid())
		{
			UE_LOG(LogSpatialEventTracerUserInterface, Warning,
				   TEXT("USpatialEventTracerUserInterface::CreateSpanIdWithCauses - Invalid input cause"));
			continue;
		}

		FSpatialGDKSpanId CauseSpanId = SpatialGDK::SpatialEventTracer::UserSpanIdToGDKSpanId(UserSpanIdCause);
		CauseSpanIds.Add(CauseSpanId);
	}
	return MoveTemp(CauseSpanIds);
}
} // namespace

FUserSpanId USpatialEventTracerUserInterface::TraceEvent(UObject* WorldContextObject, const FString& EventType, const FString& EventMessage, TMap<FString, FString> Data)
{
	SpatialGDK::SpatialEventTracer* EventTracer = GetEventTracer(WorldContextObject);
	if (EventTracer == nullptr)
	{
		return {};
	}

	FSpatialGDKSpanId SpanId = EventTracer->TraceEvent(TCHAR_TO_ANSI(*EventType), TCHAR_TO_ANSI(*EventMessage), /* Causes */ nullptr, /* NumCauses */ 0,
		[Data](SpatialGDK::FSpatialTraceEventDataBuilder& EventBuilder) {
			for (const auto& Pair : Data)
			{
				EventBuilder.AddKeyValue(Pair.Key, Pair.Value);
			}
	});

	return SpatialGDK::SpatialEventTracer::GDKSpanIdToUserSpanId(SpanId);
}

FUserSpanId USpatialEventTracerUserInterface::TraceEventWithCauses(UObject* WorldContextObject, const FString& EventType, const FString& EventMessage, TMap<FString, FString> Data,
																   const TArray<FUserSpanId>& Causes)
{
	SpatialGDK::SpatialEventTracer* EventTracer = GetEventTracer(WorldContextObject);
	if (EventTracer == nullptr)
	{
		return {};
	}

	TArray<FSpatialGDKSpanId> CauseSpanIds = ConvertSpanIds(Causes);
	FSpatialGDKSpanId SpanId = EventTracer->TraceEvent(TCHAR_TO_ANSI(*EventType), TCHAR_TO_ANSI(*EventMessage), CauseSpanIds.GetData()->GetId(), CauseSpanIds.Num(),
		[Data](SpatialGDK::FSpatialTraceEventDataBuilder& EventBuilder) {
			for (const auto& Pair : Data)
			{
				EventBuilder.AddKeyValue(Pair.Key, Pair.Value);
			}
	});

	return SpatialGDK::SpatialEventTracer::GDKSpanIdToUserSpanId(SpanId);
}

void USpatialEventTracerUserInterface::TraceRPC(UObject* WorldContextObject, FEventTracerRPCDelegate Delegate,
												const FUserSpanId& UserSpanId)
{
	SpatialGDK::SpatialEventTracer* EventTracer = GetEventTracer(WorldContextObject);
	if (EventTracer == nullptr)
	{
		Delegate.Execute();
		return;
	}

	if (!UserSpanId.IsValid())
	{
		Delegate.Execute();
		return;
	}

	FSpatialGDKSpanId SpanId = SpatialGDK::SpatialEventTracer::UserSpanIdToGDKSpanId(UserSpanId);
	EventTracer->AddToStack(SpanId);
	Delegate.Execute();
	EventTracer->PopFromStack();
}

bool USpatialEventTracerUserInterface::GetActiveSpanId(UObject* WorldContextObject, FUserSpanId& OutUserSpanId)
{
	const SpatialGDK::SpatialEventTracer* EventTracer = GetEventTracer(WorldContextObject);
	if (EventTracer == nullptr)
	{
		return false;
	}

	if (EventTracer->IsStackEmpty())
	{
		return false;
	}

	FSpatialGDKSpanId SpanId = EventTracer->GetFromStack();
	OutUserSpanId = SpatialGDK::SpatialEventTracer::GDKSpanIdToUserSpanId(SpanId);
	return true;
}

void USpatialEventTracerUserInterface::TraceProperty(UObject* WorldContextObject, UObject* Object, const FUserSpanId& UserSpanId)
{
	SpatialGDK::SpatialEventTracer* EventTracer = GetEventTracer(WorldContextObject);
	if (EventTracer == nullptr)
	{
		return;
	}

	USpatialNetDriver* NetDriver = GetSpatialNetDriver(WorldContextObject);
	if (NetDriver == nullptr)
	{
		return;
	}

	if (!UserSpanId.IsValid())
	{
		return;
	}

	FSpatialGDKSpanId SpanId = SpatialGDK::SpatialEventTracer::UserSpanIdToGDKSpanId(UserSpanId);
	EventTracer->AddLatentPropertyUpdateSpanId(Object, SpanId);
}

SpatialGDK::SpatialEventTracer* USpatialEventTracerUserInterface::GetEventTracer(UObject* WorldContextObject)
{
	const USpatialNetDriver* NetDriver = GetSpatialNetDriver(WorldContextObject);
	if (NetDriver == nullptr || NetDriver->Connection == nullptr)
	{
		UE_LOG(LogSpatialEventTracerUserInterface, Error,
			   TEXT("USpatialEventTracerUserInterface::GetEventTracer - NetDriver or Connection is null"));
		return nullptr;
	}

	return NetDriver->Connection->GetEventTracer();
}

USpatialNetDriver* USpatialEventTracerUserInterface::GetSpatialNetDriver(UObject* WorldContextObject)
{
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (World == nullptr)
	{
		UE_LOG(LogSpatialEventTracerUserInterface, Error,
			   TEXT("USpatialEventTracerUserInterface::GetSpatialNetDriver - World is null, will use GWorld instead"));
		World = GWorld;
	}

	USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(World->GetNetDriver());
	return NetDriver;
}
