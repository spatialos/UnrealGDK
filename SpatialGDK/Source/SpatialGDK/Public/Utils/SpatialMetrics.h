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
	void OnStartRPCMetricsCommand();

	UFUNCTION(Exec)
	void SpatialStopRPCMetrics();
	void OnStopRPCMetricsCommand();

	UFUNCTION(Exec)
	void SpatialModifySetting(const FString& Name, float Value);
	void OnModifySettingCommand(Schema_Object* CommandPayload);

	void TrackSentRPC(UFunction* Function, ESchemaComponentType RPCType, int PayloadSize);

	void HandleWorkerMetrics(Worker_Op* Op);

	// The user can bind their own delegate to handle worker metrics.
	typedef TMap<FString, double> WorkerMetrics;
	DECLARE_MULTICAST_DELEGATE_OneParam(WorkerMetricsDelegate, WorkerMetrics)
	WorkerMetricsDelegate WorkerMetricsRecieved;

private:
	UPROPERTY()
	USpatialNetDriver* NetDriver;

	float TimeOfLastReport;
	float TimeSinceLastReport;
	float TimeBetweenMetricsReports;
	int32 FramesSinceLastReport;

	double AverageFPS;
	double WorkerLoad;

	// RPC tracking is activated with "SpatialStartRPCMetrics" and stopped with "SpatialStopRPCMetrics"
	// console command. It will record every sent RPC as well as the size of its payload, and then display
	// tracked data upon stopping. Calling these console commands on the client will also start/stop RPC
	// tracking on the server.
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

