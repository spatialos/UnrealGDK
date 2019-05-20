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
	void Init(USpatialNetDriver* InNetDriver);

	void TickMetrics();

	double CalculateLoad() const;

	double GetAverageFPS() const { return AverageFPS; }
	double GetWorkerLoad() const { return WorkerLoad; }

private:
	UPROPERTY()
	USpatialNetDriver* NetDriver;

	float TimeOfLastReport;
	float TimeSinceLastReport;
	float TimeBetweenMetricsReports;
	int32 FramesSinceLastReport;

	double AverageFPS;
	double WorkerLoad;
};

