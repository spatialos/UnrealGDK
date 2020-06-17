// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

/******
******* Note this is an internal only feature
*******/

#include "Utils/SpatialMetrics.h"

struct ScopedCounter
{
	int64_t TicksStart;
	USpatialMetrics* MetricsObj;
	const FName& Metric; // Always a static string
	ScopedCounter(USpatialMetrics* InMetricsObj, const FName& MetricName)
		: TicksStart(FDateTime::Now().GetTicks()) // TODO: Perf of this?
		, MetricsObj(InMetricsObj)
		, Metric(MetricName)
	{
	}
	~ScopedCounter()
	{
		int64_t TicksEnd = FDateTime::Now().GetTicks();
		int64_t Duration = TicksEnd - TicksStart;
		double Milliseconds = FTimespan(Duration).GetTotalMilliseconds();
		MetricsObj->IncPerfMetric(Metric, Milliseconds); // MetricsObj needs to stay valid for the scope..
	}
};

#define GDK_CONCAT_(x,y) x##y
#define GDK_CONCAT(x,y) GDK_CONCAT_(x,y)

#if GDK_PERF_METRICS_ENABLED
// A simple counter for number of hits at frame resolution
#define GDK_PERF_FRAME_COUNTER(MetricsObj, Metric) \
if(MetricsObj) { MetricsObj->IncPerfMetric(Metric, 1.0f); }

// A simple counter recording scoped time at frame resolution
#define GDK_PERF_FRAME_SCOPED_TIME(MetricsObj, Metric) \
if(MetricsObj) { ScopedCounter GDK_CONCAT(sc_, __COUNTER__)(MetricsObj, Metric); }
#else
#define GDK_PERF_FRAME_COUNTER(...)
#define GDK_PERF_FRAME_SCOPED_TIME(...)
#endif
