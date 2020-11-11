// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialEventTracerUserInterface.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/Connection/SpatialEventTracer.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialClassInfoManager.h"

DEFINE_LOG_CATEGORY(LogSpatialEventTracerUserInterface);

FUserSpanId USpatialEventTracerUserInterface::CreateSpanId(UObject* WorldContextObject)
{
	const SpatialGDK::SpatialEventTracer* EventTracer = GetEventTracer(WorldContextObject);
	if (EventTracer == nullptr || !EventTracer->IsEnabled())
	{
		return {};
	}

	return SpatialGDK::SpatialEventTracer::GDKSpanIdToUserSpanId(EventTracer->CreateSpan().GetValue());
}

FUserSpanId USpatialEventTracerUserInterface::CreateSpanIdWithCauses(UObject* WorldContextObject, const TArray<FUserSpanId>& Causes)
{
	const SpatialGDK::SpatialEventTracer* EventTracer = GetEventTracer(WorldContextObject);
	if (EventTracer == nullptr || !EventTracer->IsEnabled())
	{
		return {};
	}

	TArray<FSpatialGDKSpanId> SpanIds;
	for (const FUserSpanId& UserSpanIdCause : Causes)
	{
		if (!UserSpanIdCause.IsValid())
		{
			UE_LOG(LogSpatialEventTracerUserInterface, Warning,
				   TEXT("USpatialEventTracerUserInterface::CreateSpanIdWithCauses - Invalid input cause"));
			continue;
		}

		TOptional<FSpatialGDKSpanId> CauseSpanId = SpatialGDK::SpatialEventTracer::UserSpanIdToGDKSpanId(UserSpanIdCause);
		if (CauseSpanId.IsSet())
		{
			SpanIds.Add(CauseSpanId.GetValue());
		}
	}

	FMultiGDKSpanIdAllocator SpanIdAllocator = FMultiGDKSpanIdAllocator(SpanIds);
	return SpatialGDK::SpatialEventTracer::GDKSpanIdToUserSpanId(EventTracer->CreateSpan(SpanIdAllocator.GetBuffer(), SpanIdAllocator.GetNumSpanIds()).GetValue());
}

void USpatialEventTracerUserInterface::TraceEvent(UObject* WorldContextObject, const FUserSpanId& UserSpanId,
												  const FSpatialTraceEvent& SpatialTraceEvent)
{
	SpatialGDK::SpatialEventTracer* EventTracer = GetEventTracer(WorldContextObject);
	if (EventTracer == nullptr)
	{
		return;
	}

	if (!UserSpanId.IsValid())
	{
		return;
	}

	TOptional<FSpatialGDKSpanId> SpanId = SpatialGDK::SpatialEventTracer::UserSpanIdToGDKSpanId(UserSpanId);
	if (!SpanId.IsSet())
	{
		return;
	}

	EventTracer->TraceEvent(SpatialTraceEvent, SpanId.GetValue());
}

void USpatialEventTracerUserInterface::TraceRPC(UObject* WorldContextObject, FEventTracerRPCDelegate Delegate,
												const FUserSpanId& UserSpanId)
{
	SpatialGDK::SpatialEventTracer* EventTracer = GetEventTracer(WorldContextObject);
	if (EventTracer == nullptr || !EventTracer->IsEnabled())
	{
		Delegate.Execute();
		return;
	}

	if (!UserSpanId.IsValid())
	{
		Delegate.Execute();
		return;
	}

	TOptional<FSpatialGDKSpanId> SpanId = SpatialGDK::SpatialEventTracer::UserSpanIdToGDKSpanId(UserSpanId);
	if (!SpanId.IsSet())
	{
		Delegate.Execute();
		return;
	}

	EventTracer->AddToStack(SpanId.GetValue());
	Delegate.Execute();
	EventTracer->PopFromStack();
}

bool USpatialEventTracerUserInterface::GetActiveSpanId(UObject* WorldContextObject, FUserSpanId& OutUserSpanId)
{
	const SpatialGDK::SpatialEventTracer* EventTracer = GetEventTracer(WorldContextObject);
	if (EventTracer == nullptr || !EventTracer->IsEnabled())
	{
		return false;
	}

	if (EventTracer->IsStackEmpty())
	{
		return false;
	}

	TOptional<FSpatialGDKSpanId> SpanId = EventTracer->GetFromStack();
	if (!SpanId.IsSet())
	{
		return false;
	}

	OutUserSpanId = SpatialGDK::SpatialEventTracer::GDKSpanIdToUserSpanId(SpanId.GetValue());
	return true;
}

void USpatialEventTracerUserInterface::TraceProperty(UObject* WorldContextObject, UObject* Object, const FUserSpanId& UserSpanId)
{
	SpatialGDK::SpatialEventTracer* EventTracer = GetEventTracer(WorldContextObject);
	if (EventTracer == nullptr || !EventTracer->IsEnabled())
	{
		return;
	}

	USpatialNetDriver* NetDriver = GetSpatialNetDriver(WorldContextObject);
	if (NetDriver == nullptr)
	{
		return;
	}

	TOptional<FSpatialGDKSpanId> SpanId = SpatialGDK::SpatialEventTracer::UserSpanIdToGDKSpanId(UserSpanId);
	if (!SpanId.IsSet())
	{
		return;
	}

	EventTracer->AddLatentPropertyUpdateSpanId(Object, SpanId.GetValue());
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
			   TEXT("USpatialEventTracerUserInterface::GetSpatialNetDriver - World is null."));
		return nullptr;
	}

	USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(World->GetNetDriver());
	return NetDriver;
}
