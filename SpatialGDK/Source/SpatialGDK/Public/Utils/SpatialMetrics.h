// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "NoExportTypes.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialMetrics.generated.h"

class USpatialNetDriver;
class USpatialWorkerConnection;

UCLASS()
class USpatialMetrics : public UObject
{
	GENERATED_BODY()

public:
	void Init(USpatialNetDriver* InNetDriver);

	void TickMetrics();

	double CalculateLoad();

private:
	UPROPERTY()
	USpatialNetDriver* NetDriver;

	float TimeSinceLastReport;

	float TimeBetweenMetricsReports;
	int FramesSinceLastReport;

	double AverageFPS;
	double WorkerLoad;
};

