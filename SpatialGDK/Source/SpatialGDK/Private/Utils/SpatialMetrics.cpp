// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SpatialMetrics.h"

#include "Engine/Engine.h"
#include "EngineGlobals.h"
#include "GameFramework/PlayerController.h"

#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "SpatialGDKSettings.h"
#include "Utils/SchemaUtils.h"

DEFINE_LOG_CATEGORY(LogSpatialMetrics);

void USpatialMetrics::Init(USpatialNetDriver* InNetDriver)
{
	NetDriver = InNetDriver;
	TimeBetweenMetricsReports = GetDefault<USpatialGDKSettings>()->MetricsReportRate;
	FramesSinceLastReport = 0;
	TimeOfLastReport = 0.0f;

	bRPCTrackingEnabled = false;
	RPCTrackingStartTime = 0.0f;
}

void USpatialMetrics::TickMetrics()
{
	FramesSinceLastReport++;

	TimeSinceLastReport = NetDriver->Time - TimeOfLastReport;

	// Check that there has been a sufficient amount of time since the last report.
	if (TimeSinceLastReport > 0.f && TimeSinceLastReport < TimeBetweenMetricsReports)
	{
		return;
	}

	AverageFPS = FramesSinceLastReport / TimeSinceLastReport;
	WorkerLoad = CalculateLoad();

	SpatialGDK::GaugeMetric DynamicFPSGauge;
	DynamicFPSGauge.Key = TCHAR_TO_UTF8(*SpatialConstants::SPATIALOS_METRICS_DYNAMIC_FPS);
	DynamicFPSGauge.Value = AverageFPS;

	SpatialGDK::SpatialMetrics DynamicFPSMetrics;
	DynamicFPSMetrics.GaugeMetrics.Add(DynamicFPSGauge);
	DynamicFPSMetrics.Load = WorkerLoad;

	TimeOfLastReport = NetDriver->Time;
	FramesSinceLastReport = 0;

	NetDriver->Connection->SendMetrics(DynamicFPSMetrics);
}

// Load defined as performance relative to target frame time or just frame time based on config value.
double USpatialMetrics::CalculateLoad() const
{
	float AverageFrameTime = TimeSinceLastReport / FramesSinceLastReport;

	if (GetDefault<USpatialGDKSettings>()->bUseFrameTimeAsLoad)
	{
		return AverageFrameTime;
	}

	float TargetFrameTime = 1.0f / NetDriver->NetServerMaxTickRate;

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
	if (!NetDriver->IsServer())
	{
		FUnrealObjectRef PCObjectRef = NetDriver->PackageMap->GetUnrealObjectRefFromObject(Cast<APlayerController>(NetDriver->GetSpatialOSNetConnection()->OwningActor));
		Worker_EntityId ControllerEntityId = PCObjectRef.Entity;

		if (ControllerEntityId != SpatialConstants::INVALID_ENTITY_ID)
		{
			Worker_CommandRequest Request = {};
			Request.component_id = SpatialConstants::DEBUG_METRICS_COMPONENT_ID;
			Request.command_index = SpatialConstants::DEBUG_METRICS_START_RPC_METRICS_ID;
			Request.schema_type = Schema_CreateCommandRequest();
			NetDriver->Connection->SendCommandRequest(ControllerEntityId, &Request, SpatialConstants::DEBUG_METRICS_START_RPC_METRICS_ID);
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
		UE_LOG(LogSpatialMetrics, Log, TEXT("Recently sent RPCs - %s:"), NetDriver->IsServer() ? TEXT("Server") : TEXT("Client"));
		UE_LOG(LogSpatialMetrics, Log, TEXT("RPC Type           | %s | # of calls |  Calls/sec | Total payload | Avg. payload | Payload/sec"), *FString(TEXT("RPC Name")).RightPad(MaxRPCNameLen));

		FString SeparatorLine = FString::Printf(TEXT("-------------------+-%s-+------------+------------+---------------+--------------+------------"), *FString::ChrN(MaxRPCNameLen, '-'));

		ESchemaComponentType PrevType = SCHEMA_Invalid;
		for (RPCStat& Stat : RecentRPCArray)
		{
			FString RPCTypeField;
			if (Stat.Type != PrevType)
			{
				RPCTypeField = RPCSchemaTypeToString(Stat.Type);
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
	if (!NetDriver->IsServer())
	{
		FUnrealObjectRef PCObjectRef = NetDriver->PackageMap->GetUnrealObjectRefFromObject(Cast<APlayerController>(NetDriver->GetSpatialOSNetConnection()->OwningActor));
		Worker_EntityId ControllerEntityId = PCObjectRef.Entity;

		if (ControllerEntityId != SpatialConstants::INVALID_ENTITY_ID)
		{
			Worker_CommandRequest Request = {};
			Request.component_id = SpatialConstants::DEBUG_METRICS_COMPONENT_ID;
			Request.command_index = SpatialConstants::DEBUG_METRICS_STOP_RPC_METRICS_ID;
			Request.schema_type = Schema_CreateCommandRequest();
			NetDriver->Connection->SendCommandRequest(ControllerEntityId, &Request, SpatialConstants::DEBUG_METRICS_STOP_RPC_METRICS_ID);
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
	if (!NetDriver->IsServer())
	{
		FUnrealObjectRef PCObjectRef = NetDriver->PackageMap->GetUnrealObjectRefFromObject(Cast<APlayerController>(NetDriver->GetSpatialOSNetConnection()->OwningActor));
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

			NetDriver->Connection->SendCommandRequest(ControllerEntityId, &Request, SpatialConstants::DEBUG_METRICS_MODIFY_SETTINGS_ID);
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

void USpatialMetrics::TrackSentRPC(UFunction* Function, ESchemaComponentType RPCType, int PayloadSize)
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
