// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

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
	void Init(USpatialNetDriver* InNetDriver, double InTargetFPS);

	void TickMetrics();

	double CalculateLoad(double TargetFPS, double CalculatedFPS);

private:

	UPROPERTY()
	USpatialNetDriver* NetDriver;

	float TimeSinceLastReport;

	int TimeBetweenMetricsReports;
	int FramesSinceLastReport;

	double AverageFPS;
	double WorkerLoad;

};

