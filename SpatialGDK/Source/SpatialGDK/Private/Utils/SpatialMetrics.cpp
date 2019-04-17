// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SpatialMetrics.h"

#include "Engine/Engine.h"
#include "EngineGlobals.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "SpatialGDKSettings.h"

void USpatialMetrics::Init(USpatialNetDriver* InNetDriver)
{
	NetDriver = InNetDriver;
	TimeBetweenMetricsReports = GetDefault<USpatialGDKSettings>()->MetricsReportRate;
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
	WorkerLoad = CalculateLoad();

	improbable::GaugeMetric DynamicFPSGauge;
	DynamicFPSGauge.Key = TCHAR_TO_UTF8(*SpatialConstants::SPATIALOS_METRICS_DYNAMIC_FPS);
	DynamicFPSGauge.Value = AverageFPS;

	improbable::Metrics DynamicFPSMetrics;
	DynamicFPSMetrics.GaugeMetrics.Add(DynamicFPSGauge);
	DynamicFPSMetrics.Load = WorkerLoad;

	TimeSinceLastReport = NetDriver->Time;
	FramesSinceLastReport = 0;

	NetDriver->Connection->SendMetrics(DynamicFPSMetrics);
}

// Load defined as performance relative to target frame time or just frame time based on config value.
double USpatialMetrics::CalculateLoad()
{
	double AverageFrameTime = (NetDriver->Time - TimeSinceLastReport) / FramesSinceLastReport;

	if (GetDefault<USpatialGDKSettings>()->bUseFrameTimeAsLoad)
	{
		return AverageFrameTime;
	}

	double TargetFrameTime = 1.0f / NetDriver->NetServerMaxTickRate;

	return AverageFrameTime / TargetFrameTime;
}
