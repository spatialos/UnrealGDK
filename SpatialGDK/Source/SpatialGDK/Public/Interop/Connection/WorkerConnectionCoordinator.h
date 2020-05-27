// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "HAL/Event.h"

struct FEventDeleter
{
	void operator()(FEvent* Event) const
	{
		FPlatformProcess::ReturnSynchEventToPool(Event);
	}
};

/**
* The reason this exists is because FEvent::Wait(Time) is not equivilant for 
* FPlatformProcess::Sleep and has overhead which impacts latency.
*/
class WorkerConnectionCoordinator
{
	TUniquePtr<FEvent, FEventDeleter> Event;
	int32 WaitTimeMS;
public:
	WorkerConnectionCoordinator(bool bCanWake, int32 InWaitMs)
		: Event(bCanWake ? FGenericPlatformProcess::GetSynchEventFromPool() : nullptr)
		, WaitTimeMS(InWaitMs)
	{
		
	}
	~WorkerConnectionCoordinator() = default;

	void Wait()
	{
		if (Event.IsValid())
		{
			Event->Wait(WaitTimeMS);
		}
		else
		{
			FPlatformProcess::Sleep(WaitTimeMS*0.001f);
		}
	}
	
	void Wake()
	{
		if (Event.IsValid())
		{
			Event->Trigger();
		}
	}
};
