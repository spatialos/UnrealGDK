// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EventSamplingProbabilityOverrideSettings.h"

FName UEventSamplingProbabilityOverrideSettings::OverridenEventType = FName("user.send_rpc");

UEventSamplingProbabilityOverrideSettings::UEventSamplingProbabilityOverrideSettings()
{
	EventSamplingModeOverrides.Add(OverridenEventType, 1.0f);
	SamplingProbability = 0.0;
	GDKEventPreFilter = "true";
	GDKEventPostFilter = "true";
	RuntimeEventPreFilter = "true";
	RuntimeEventPostFilter = "true";
}
