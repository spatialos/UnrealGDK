// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "HAL/Platform.h"
#include "Interop/Connection/SpatialGDKSpanId.h"
#include "Misc/Optional.h"
#include "Templates/UniquePtr.h"
#include "Templates/UnrealTemplate.h"
#include "UObject/NameTypes.h"
#include "Utils/SpatialLatencyTracer.h"

#include <string>

#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{

	/** Parameters for a gauge metric. */
struct GaugeMetric
{
	/* The name of the metric. */
	std::string Key;
	/* The current value of the metric. */
	double Value;
};

/* Parameters for a histogram metric bucket. */
struct HistogramMetricBucket
{
	/* The upper bound. */
	double UpperBound;
	/* The number of observations that were less than or equal to the upper bound. */
	uint32 Samples;
};

/* Parameters for a histogram metric. */
struct HistogramMetric
{
	/* The name of the metric. */
	std::string Key;
	/* The sum of all observations. */
	double Sum;
	/* Array of buckets. */
	TArray<HistogramMetricBucket> Buckets;
};

/** Parameters for sending metrics to SpatialOS. */
struct SpatialMetrics
{
	/** The load value of this worker. If NULL, do not report load. */
	TOptional<double> Load;
	/** Array of gauge metrics. */
	TArray<GaugeMetric> GaugeMetrics;
	/** Array of histogram metrics. */
	TArray<HistogramMetric> HistogramMetrics;

	void SendToConnection(Worker_Connection* Connection);
};


} // namespace SpatialGDK
