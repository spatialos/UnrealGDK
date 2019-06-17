// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "SpatialConstants.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialMetrics.generated.h"

class USpatialNetDriver;
class USpatialWorkerConnection;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialMetrics, Log, All);

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

	UFUNCTION(Exec)
	void SpatialStartRPCMetrics();
	void OnStartRPCMetricsCommand(Worker_CommandRequestOp& Op);

	UFUNCTION(Exec)
	void SpatialStopRPCMetrics();
	void OnStopRPCMetricsCommand(Worker_CommandRequestOp& Op);

	void TrackSentRPC(UFunction* Function, ESchemaComponentType RPCType, int PayloadSize);

private:
	UPROPERTY()
	USpatialNetDriver* NetDriver;

	float TimeOfLastReport;
	float TimeSinceLastReport;
	float TimeBetweenMetricsReports;
	int32 FramesSinceLastReport;

	double AverageFPS;
	double WorkerLoad;

	// TODO: Comment on how this is used.
	struct RPCStat
	{
		ESchemaComponentType Type;
		FString Name;
		int Calls;
		int TotalPayload;
	};
	TMap<FString, RPCStat> RecentRPCs;
	bool bRPCTrackingEnabled;
	float RPCTrackingStartTime;
};

