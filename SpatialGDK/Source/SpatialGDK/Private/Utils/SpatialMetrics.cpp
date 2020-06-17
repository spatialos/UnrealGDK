// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SpatialMetrics.h"

#include "Engine/Engine.h"
#include "EngineGlobals.h"

#include "Interop/Connection/SpatialWorkerConnection.h"
#include "SpatialGDKSettings.h"
#include "Utils/SchemaUtils.h"
#include "Utils/PerfMetrics.h"

DEFINE_LOG_CATEGORY(LogSpatialMetrics);

USpatialMetrics::WorkerMetricsDelegate USpatialMetrics::WorkerMetricsRecieved;

void USpatialMetrics::Init(USpatialWorkerConnection* InConnection, float InNetServerMaxTickRate, bool bInIsServer)
{
	Connection = InConnection;
	bIsServer = bInIsServer;
	NetServerMaxTickRate = InNetServerMaxTickRate;

	TimeBetweenMetricsReports = GetDefault<USpatialGDKSettings>()->MetricsReportRate;
	FramesSinceLastReport = 0;
	TimeOfLastReport = 0.0f;

	WorkerLoad = 0.0;

	bRPCTrackingEnabled = false;
	RPCTrackingStartTime = 0.0f;

	PerformanceCounters.SetNum((int32)GDKMetric::Count);
	ResetPerfMetrics();
	
	UserSuppliedMetric Delegate;
	Delegate.BindUObject(this, &USpatialMetrics::GetAverageFPS);
	SetCustomMetric(SpatialConstants::SPATIALOS_METRICS_DYNAMIC_FPS, Delegate);
}

void USpatialMetrics::TickMetrics(float NetDriverTime)
{
	FramesSinceLastReport++;

	TimeSinceLastReport = NetDriverTime - TimeOfLastReport;

	// Check that there has been a sufficient amount of time since the last report.
	if (TimeSinceLastReport > 0.f && TimeSinceLastReport < TimeBetweenMetricsReports)
	{
		return;
	}

	AverageFPS = FramesSinceLastReport / TimeSinceLastReport;
	if (WorkerLoadDelegate.IsBound())
	{
		WorkerLoad = WorkerLoadDelegate.Execute();
	}
	else
	{
		WorkerLoad = CalculateLoad();
	}

	SpatialGDK::SpatialMetrics Metrics;
	Metrics.Load = WorkerLoad;
	
	// User supplied metrics
	TArray<FString> UnboundMetrics;
	for (const TPair<FString, UserSuppliedMetric>& Gauge : UserSuppliedMetrics)
	{
		if (Gauge.Value.IsBound())
		{
			SpatialGDK::GaugeMetric Metric;

			Metric.Key = TCHAR_TO_UTF8(*Gauge.Key);
			Metric.Value = Gauge.Value.Execute();
			Metrics.GaugeMetrics.Add(Metric);
		}
		else
		{
			UnboundMetrics.Add(Gauge.Key);
		}
	}
	for (const FString& KeyToRemove : UnboundMetrics)
	{
		UserSuppliedMetrics.Remove(KeyToRemove);
	}

	TimeOfLastReport = NetDriverTime;
	FramesSinceLastReport = 0;

	// GDK metrics. Merge into UserMetrics
	if (bIsServer)
	{
		for (int i = 0; i < (size_t)GDKMetric::Count; i++)
		{
			SpatialGDK::GaugeMetric Guage;
			Guage.Key = GetMetricName(GDKMetric(i));
			Guage.Value = PerformanceCounters[i];
			Metrics.GaugeMetrics.Add(Guage);
		}
	}
	Connection->SendMetrics(Metrics);
	ResetPerfMetrics();
}

// Load defined as performance relative to target frame time or just frame time based on config value.
double USpatialMetrics::CalculateLoad() const
{
	float AverageFrameTime = TimeSinceLastReport / FramesSinceLastReport;

	if (GetDefault<USpatialGDKSettings>()->bUseFrameTimeAsLoad)
	{
		return AverageFrameTime;
	}

	float TargetFrameTime = 1.0f / NetServerMaxTickRate;

	return AverageFrameTime / TargetFrameTime;
}

void USpatialMetrics::SpatialStartRPCMetrics()
{
	if (bRPCTrackingEnabled)
	{
		UE_LOG(LogSpatialMetrics, Log, TEXT("Already recording RPC metrics"));
		return;
	}

	UE_LOG(LogSpatialMetrics, Log, TEXT("Recording RPC metrics"));

	bRPCTrackingEnabled = true;
	RPCTrackingStartTime = FPlatformTime::Seconds();

	// If RPC tracking is activated on a client, send a command to the server to start tracking.
	if (!bIsServer && ControllerRefProvider.IsBound())
	{
		FUnrealObjectRef PCObjectRef = ControllerRefProvider.Execute();
		Worker_EntityId ControllerEntityId = PCObjectRef.Entity;

		if (ControllerEntityId != SpatialConstants::INVALID_ENTITY_ID)
		{
			Worker_CommandRequest Request = {};
			Request.component_id = SpatialConstants::DEBUG_METRICS_COMPONENT_ID;
			Request.command_index = SpatialConstants::DEBUG_METRICS_START_RPC_METRICS_ID;
			Request.schema_type = Schema_CreateCommandRequest();
			Connection->SendCommandRequest(ControllerEntityId, &Request, SpatialConstants::DEBUG_METRICS_START_RPC_METRICS_ID);
		}
		else
		{
			UE_LOG(LogSpatialMetrics, Warning, TEXT("SpatialStartRPCMetrics: Could not resolve local PlayerController entity! RPC metrics will not start on the server."));
		}
	}
}

void USpatialMetrics::OnStartRPCMetricsCommand()
{
	SpatialStartRPCMetrics();
}

void USpatialMetrics::SpatialStopRPCMetrics()
{
	if (!bRPCTrackingEnabled)
	{
		UE_LOG(LogSpatialMetrics, Log, TEXT("Could not stop recording RPC metrics. RPC metrics not yet started."));
		return;
	}

	// Display recorded sent RPCs.
	const double TrackRPCInterval = FPlatformTime::Seconds() - RPCTrackingStartTime;
	UE_LOG(LogSpatialMetrics, Log, TEXT("Recorded %d unique RPCs over the last %.3f seconds:"), RecentRPCs.Num(), TrackRPCInterval);

	if (RecentRPCs.Num() > 0)
	{
		// NICELY log sent RPCs.
		TArray<RPCStat> RecentRPCArray;
		RecentRPCs.GenerateValueArray(RecentRPCArray);

		// Show the most frequently called RPCs at the top.
		RecentRPCArray.Sort([](const RPCStat& A, const RPCStat& B)
		{
			if (A.Type != B.Type)
			{
				return static_cast<int>(A.Type) < static_cast<int>(B.Type);
			}
			return A.Calls > B.Calls;
		});

		int MaxRPCNameLen = 0;
		for (RPCStat& Stat : RecentRPCArray)
		{
			MaxRPCNameLen = FMath::Max(MaxRPCNameLen, Stat.Name.Len());
		}

		int TotalCalls = 0;
		int TotalPayload = 0;

		UE_LOG(LogSpatialMetrics, Log, TEXT("---------------------------"));
		UE_LOG(LogSpatialMetrics, Log, TEXT("Recently sent RPCs - %s:"), bIsServer ? TEXT("Server") : TEXT("Client"));
		UE_LOG(LogSpatialMetrics, Log, TEXT("RPC Type           | %s | # of calls |  Calls/sec | Total payload | Avg. payload | Payload/sec"), *FString(TEXT("RPC Name")).RightPad(MaxRPCNameLen));

		FString SeparatorLine = FString::Printf(TEXT("-------------------+-%s-+------------+------------+---------------+--------------+------------"), *FString::ChrN(MaxRPCNameLen, '-'));

		ERPCType PrevType = ERPCType::Invalid;
		for (RPCStat& Stat : RecentRPCArray)
		{
			FString RPCTypeField;
			if (Stat.Type != PrevType)
			{
				RPCTypeField = SpatialConstants::RPCTypeToString(Stat.Type);
				PrevType = Stat.Type;
				UE_LOG(LogSpatialMetrics, Log, TEXT("%s"), *SeparatorLine);
			}
			UE_LOG(LogSpatialMetrics, Log, TEXT("%s | %s | %10d | %10.4f | %13d | %12.4f | %11.4f"), *RPCTypeField.RightPad(18), *Stat.Name.RightPad(MaxRPCNameLen), Stat.Calls, Stat.Calls / TrackRPCInterval, Stat.TotalPayload, (float)Stat.TotalPayload / Stat.Calls, Stat.TotalPayload / TrackRPCInterval);
			TotalCalls += Stat.Calls;
			TotalPayload += Stat.TotalPayload;
		}
		UE_LOG(LogSpatialMetrics, Log, TEXT("%s"), *SeparatorLine);
		UE_LOG(LogSpatialMetrics, Log, TEXT("Total              | %s | %10d | %10.4f | %13d | %12.4f | %11.4f"), *FString::ChrN(MaxRPCNameLen, ' '), TotalCalls, TotalCalls / TrackRPCInterval, TotalPayload, (float)TotalPayload / TotalCalls, TotalPayload / TrackRPCInterval);

		RecentRPCs.Empty();
	}

	bRPCTrackingEnabled = false;

	// If RPC tracking is stopped on a client, send a command to the server to stop tracking.
	if (!bIsServer && ControllerRefProvider.IsBound())
	{
		FUnrealObjectRef PCObjectRef = ControllerRefProvider.Execute();
		Worker_EntityId ControllerEntityId = PCObjectRef.Entity;

		if (ControllerEntityId != SpatialConstants::INVALID_ENTITY_ID)
		{
			Worker_CommandRequest Request = {};
			Request.component_id = SpatialConstants::DEBUG_METRICS_COMPONENT_ID;
			Request.command_index = SpatialConstants::DEBUG_METRICS_STOP_RPC_METRICS_ID;
			Request.schema_type = Schema_CreateCommandRequest();
			Connection->SendCommandRequest(ControllerEntityId, &Request, SpatialConstants::DEBUG_METRICS_STOP_RPC_METRICS_ID);
		}
		else
		{
			UE_LOG(LogSpatialMetrics, Warning, TEXT("SpatialStopRPCMetrics: Could not resolve local PlayerController entity! RPC metrics will not stop on the server."));
		}
	}
}

void USpatialMetrics::OnStopRPCMetricsCommand()
{
	SpatialStopRPCMetrics();
}

void USpatialMetrics::SpatialModifySetting(const FString& Name, float Value)
{
	if (!bIsServer && ControllerRefProvider.IsBound())
	{
		FUnrealObjectRef PCObjectRef = ControllerRefProvider.Execute();
		Worker_EntityId ControllerEntityId = PCObjectRef.Entity;

		if (ControllerEntityId != SpatialConstants::INVALID_ENTITY_ID)
		{
			Worker_CommandRequest Request = {};
			Request.component_id = SpatialConstants::DEBUG_METRICS_COMPONENT_ID;
			Request.command_index = SpatialConstants::DEBUG_METRICS_MODIFY_SETTINGS_ID;
			Request.schema_type = Schema_CreateCommandRequest();
			
			Schema_Object* RequestObject = Schema_GetCommandRequestObject(Request.schema_type);
			SpatialGDK::AddStringToSchema(RequestObject, SpatialConstants::MODIFY_SETTING_PAYLOAD_NAME_ID, Name);
			Schema_AddFloat(RequestObject, SpatialConstants::MODIFY_SETTING_PAYLOAD_VALUE_ID, Value);

			Connection->SendCommandRequest(ControllerEntityId, &Request, SpatialConstants::DEBUG_METRICS_MODIFY_SETTINGS_ID);
		}
		else
		{
			UE_LOG(LogSpatialMetrics, Warning, TEXT("SpatialModifySetting: Could not resolve local PlayerController entity! Setting will not be sent to server."));
		}
	}
	else
	{
		bool bKnownSetting = true;
		if (Name == TEXT("ActorReplicationRateLimit"))
		{
			GetMutableDefault<USpatialGDKSettings>()->ActorReplicationRateLimit = static_cast<uint32>(Value);
		}
		else if (Name == TEXT("EntityCreationRateLimit"))
		{
			GetMutableDefault<USpatialGDKSettings>()->EntityCreationRateLimit = static_cast<uint32>(Value);
		}
		else if (Name == TEXT("PositionUpdateFrequency"))
		{
			GetMutableDefault<USpatialGDKSettings>()->PositionUpdateFrequency = Value;
		}
		else if (Name == TEXT("PositionDistanceThreshold"))
		{
			GetMutableDefault<USpatialGDKSettings>()->PositionDistanceThreshold = Value;
		}
		else
		{
			bKnownSetting = false;
		}

		if (bKnownSetting)
		{
			UE_LOG(LogSpatialMetrics, Log, TEXT("SpatialModifySetting: Spatial GDK setting %s set to %f"), *Name, Value);
		}
		else
		{
			UE_LOG(LogSpatialMetrics, Warning, TEXT("SpatialModifySetting: Invalid setting %s"), *Name);
		}
	}
}

void USpatialMetrics::OnModifySettingCommand(Schema_Object* CommandPayload)
{
	FString Name = SpatialGDK::GetStringFromSchema(CommandPayload, SpatialConstants::MODIFY_SETTING_PAYLOAD_NAME_ID);
	float Value = Schema_GetFloat(CommandPayload, SpatialConstants::MODIFY_SETTING_PAYLOAD_VALUE_ID);

	SpatialModifySetting(Name, Value);
}

void USpatialMetrics::TrackSentRPC(UFunction* Function, ERPCType RPCType, int PayloadSize)
{
	if (!bRPCTrackingEnabled)
	{
		return;
	}

	FString FunctionName = FString::Printf(TEXT("%s::%s"), *Function->GetOuter()->GetName(), *Function->GetName());

	if (RecentRPCs.Find(FunctionName) == nullptr)
	{
		RPCStat Stat;
		Stat.Name = FunctionName;
		Stat.Type = RPCType;
		Stat.Calls = 0;
		Stat.TotalPayload = 0;

		RecentRPCs.Add(FunctionName, Stat);
	}

	RPCStat& Stat = RecentRPCs[FunctionName];
	Stat.Calls++;
	Stat.TotalPayload += PayloadSize;
}

void USpatialMetrics::HandleWorkerMetrics(Worker_Op* Op)
{
	if (WorkerMetricsRecieved.IsBound())
	{
		int32 NumMetrics = Op->op.metrics.metrics.gauge_metric_count;

		if (NumMetrics > 0)
		{
			// Construct a map to store all the metrics and pass it to the users delegate
			TMap<FString, double> WorkerMetrics;
			WorkerMetrics.Reserve(NumMetrics);

			for (int32 i = 0; i < NumMetrics; i++)
			{
				WorkerMetrics.Add(Op->op.metrics.metrics.gauge_metrics[i].key, Op->op.metrics.metrics.gauge_metrics[i].value);
			}

			WorkerMetricsRecieved.Broadcast(WorkerMetrics);
		}
	}
}

void USpatialMetrics::SetCustomMetric(const FString& Metric, const UserSuppliedMetric& Delegate)
{
	UE_LOG(LogSpatialMetrics, Log, TEXT("USpatialMetrics: Adding custom metric %s (%s)"), *Metric, Delegate.GetUObject() ? *GetNameSafe(Delegate.GetUObject()) : TEXT("Not attached to UObject"));
	if (UserSuppliedMetric* ExistingMetric = UserSuppliedMetrics.Find(Metric))
	{
		*ExistingMetric = Delegate;
	}
	else
	{
		UserSuppliedMetrics.Add(Metric, Delegate);
	}
}

void USpatialMetrics::RemoveCustomMetric(const FString& Metric)
{
	if (UserSuppliedMetric* ExistingMetric = UserSuppliedMetrics.Find(Metric))
	{
		UE_LOG(LogSpatialMetrics, Log, TEXT("USpatialMetrics: Removing custom metric %s (%s)"), *Metric, ExistingMetric->GetUObject() ? *GetNameSafe(ExistingMetric->GetUObject()) : TEXT("Not attached to UObject"));
		UserSuppliedMetrics.Remove(Metric);
	}
}

void USpatialMetrics::ResetPerfMetrics()
{
	for (int i = 0; i < (int)GDKMetric::Count; i++)
	{
		PerformanceCounters[i] = 0.0f;
	}
}