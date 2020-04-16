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
    TUniquePtr<FEvent, FEventDeleter>       Event;
    float                                   WaitSeconds;
public:
    WorkerConnectionCoordinator(bool bCanWake, float InWaitSeconds)
    {
        if(bCanWake)
        {
            Event.Reset(FGenericPlatformProcess::GetSynchEventFromPool());
        }
        WaitSeconds = InWaitSeconds;
    }
    ~WorkerConnectionCoordinator()
    {
    }

    void Wait()
    {
        if(Event)
        {
            FTimespan WaitTime = FTimespan::FromSeconds(WaitSeconds); // Microseconds to allow for fractional ms
            Event->Wait(WaitTime);
        }
        else
        {
            FPlatformProcess::Sleep(WaitSeconds);
        }
    }
    void Wake()
    {
		if (Event)
        {
			Event->Trigger();
        }
    }
};
