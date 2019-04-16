// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SpatialMetrics.h"

#include <EngineGlobals.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>

#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/Connection/SpatialWorkerConnection.h"


void USpatialMetrics::Init(USpatialNetDriver* InNetDriver)
{
	NetDriver = InNetDriver;
	TimeBetweenMetricsReports = 2.0;
	FramesSinceLastReport = 0;
}

void USpatialMetrics::TickMetrics()
{
	FramesSinceLastReport++;

	// Should we report metrics?
	// Check that there has been a sufficient amount of time since the last report.
	if ((NetDriver->Time - TimeSinceLastReport) < TimeBetweenMetricsReports)
	{
		return;
	}

	AverageFPS = FramesSinceLastReport / (NetDriver->Time - TimeSinceLastReport);
	WorkerLoad = CalculateLoad(NetDriver->NetServerMaxTickRate, AverageFPS);

	improbable::metrics::GaugeMetric DynamicFPSGauge;
	DynamicFPSGauge.Key = TCHAR_TO_UTF8(*SpatialConstants::SPATIALOS_METRICS_DYNAMIC_FPS);
	DynamicFPSGauge.Value = AverageFPS;

	improbable::metrics::Metrics DynamicFPSMetrics;
	DynamicFPSMetrics.GaugeMetrics.Add(DynamicFPSGauge);
	DynamicFPSMetrics.Load = WorkerLoad;

	TimeSinceLastReport = NetDriver->Time;
	FramesSinceLastReport = 0;

	NetDriver->Connection->SendMetrics(DynamicFPSMetrics);
}

// Load defined as performance relative to target FPS.
// i.e. a load of 0.5 means that the worker is hitting the target FPS
// but achieving less than half the target FPS takes load above 1.0
double USpatialMetrics::CalculateLoad(double TargetFPS, double CalculatedFPS)
{
	return FMath::Max(0.0, 0.5 * (TargetFPS / CalculatedFPS));
}
