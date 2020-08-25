// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "SpatialConstants.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialMetrics.generated.h"

class USpatialWorkerConnection;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialMetrics, Log, All);

DECLARE_DELEGATE_RetVal(double, UserSuppliedMetric);

UCLASS()
class SPATIALGDK_API USpatialMetrics : public UObject
{
	GENERATED_BODY()

public:
	void Init(USpatialWorkerConnection* Connection, float MaxServerTickRate, bool bIsServer);

	void TickMetrics(float NetDriverTime);

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

	void TrackSentRPC(UFunction* Function, ERPCType RPCType, int PayloadSize);

	void HandleWorkerMetrics(Worker_Op* Op);

	// The user can bind their own delegate to handle worker metrics.
	typedef TMap<FString, double> WorkerGaugeMetric;
	struct WorkerHistogramValues
	{
		TArray<TPair<double, uint32>> Buckets; // upper-bound, count
		double Sum;
	};
	typedef TMap<FString, WorkerHistogramValues> WorkerHistogramMetrics;
	DECLARE_MULTICAST_DELEGATE_TwoParams(WorkerMetricsDelegate, const WorkerGaugeMetric&, const WorkerHistogramMetrics&);
	WorkerMetricsDelegate WorkerMetricsUpdated;

	// Delegate used to poll for the current player controller's reference
	DECLARE_DELEGATE_RetVal(FUnrealObjectRef, FControllerRefProviderDelegate);
	FControllerRefProviderDelegate ControllerRefProvider;

	void SetWorkerLoadDelegate(const UserSuppliedMetric& Delegate) { WorkerLoadDelegate = Delegate; }
	void SetCustomMetric(const FString& Metric, const UserSuppliedMetric& Delegate);
	void RemoveCustomMetric(const FString& Metric);

private:
	// Worker SDK metrics
	WorkerGaugeMetric WorkerSDKGaugeMetrics;
	WorkerHistogramMetrics WorkerSDKHistogramMetrics;

	UPROPERTY()
	USpatialWorkerConnection* Connection;

	bool bIsServer;
	float NetServerMaxTickRate;

	float TimeOfLastReport;
	float TimeSinceLastReport;
	float TimeBetweenMetricsReports;
	int32 FramesSinceLastReport;

	double AverageFPS;
	double WorkerLoad;
	UserSuppliedMetric WorkerLoadDelegate;

	TMap<FString, UserSuppliedMetric> UserSuppliedMetrics;

	// RPC tracking is activated with "SpatialStartRPCMetrics" and stopped with "SpatialStopRPCMetrics"
	// console command. It will record every sent RPC as well as the size of its payload, and then display
	// tracked data upon stopping. Calling these console commands on the client will also start/stop RPC
	// tracking on the server.
	struct RPCStat
	{
		ERPCType Type;
		FString Name;
		int Calls;
		int TotalPayload;
	};
	TMap<FString, RPCStat> RecentRPCs;
	bool bRPCTrackingEnabled;
	float RPCTrackingStartTime;
};
