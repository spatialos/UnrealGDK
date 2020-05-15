// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

/******
******* Note this is an internal only feature
*******/

#include "Utils/SpatialMetrics.h"
#include "Utils/PerfMetricsTypes.h"

struct ScopedCounter
{
	int64_t TicksStart;
	USpatialMetrics* MetricsObj;
	GDKMetric Metric; // Always a static string
	ScopedCounter(USpatialMetrics* InMetricsObj, GDKMetric InMetric)
		: TicksStart(FDateTime::Now().GetTicks()) // TODO: Perf of this?
		, MetricsObj(InMetricsObj)
		, Metric(InMetric)
	{
	}
	~ScopedCounter()
	{
		int64_t TicksEnd = FDateTime::Now().GetTicks();
		int64_t Duration = TicksEnd - TicksStart;
		double Milliseconds = FTimespan(Duration).GetTotalMilliseconds();
		MetricsObj->IncPerfMetric((int)Metric, Milliseconds); // MetricsObj needs to stay valid for the scope..
	}
};

inline const char* GetMetricName(GDKMetric Metric)
{
	// TODO: Use UENUM 
	switch (Metric)
	{
	case GDKMetric::NumActorsReplicated: return "GDK_ActorsReplicated";
	case GDKMetric::ActorReplicationTime: return "GDK_ActorReplicationTime";
	case GDKMetric::NetTick: return "GDK_NetTick";
	case GDKMetric::TickFlush: return "GDK_TickFlush";
	}
	return "Unknown";
}

#define GDK_CONCAT_(x,y) x##y
#define GDK_CONCAT(x,y) GDK_CONCAT_(x,y)

// A simple counter for number of hits at frame resolution
#define GDK_PERF_FRAME_COUNTER(MetricsObj, Metric) \
MetricsObj->IncPerfMetric((int)Metric, 1.0f);

// A simple counter recording scoped time at frame resolution
#define GDK_PERF_FRAME_SCOPED_TIME(MetricsObj, Metric) \
ScopedCounter GDK_CONCAT(sc_, __COUNTER__)(MetricsObj, Metric)
