// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SpatialMetricsDisplay.h"

#include "Debug/DebugDrawService.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Net/UnrealNetwork.h"
#include "Utils/SpatialMetrics.h"

#if USE_SERVER_PERF_COUNTERS
#include "Net/PerfCountersHelpers.h"
#endif

ASpatialMetricsDisplay::ASpatialMetricsDisplay(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	bReplicates = true;
	bAlwaysRelevant = true;

	NetUpdateFrequency = 1.f;

#if USE_SERVER_PERF_COUNTERS
	IPerfCountersModule& PerformanceModule = IPerfCountersModule::Get();
	if (PerformanceModule.GetPerformanceCounters() == nullptr)
	{
		PerformanceModule.CreatePerformanceCounters();
	}
#endif
}

void ASpatialMetricsDisplay::BeginPlay()
{
	Super::BeginPlay();

	WorkerStats.Reserve(PreallocatedWorkerCount);
	WorkerStatsLastUpdateTime.Reserve(PreallocatedWorkerCount);

	if (!GetWorld()->IsServer() && GetDefault<USpatialGDKSettings>()->bEnableMetricsDisplay)
	{
		ToggleStatDisplay();
	}
}

void ASpatialMetricsDisplay::Destroyed()
{
	Super::Destroyed();

	if (DrawDebugDelegateHandle.IsValid())
	{
		UDebugDrawService::Unregister(DrawDebugDelegateHandle);
	}
}

bool ASpatialMetricsDisplay::ServerUpdateWorkerStats_Validate(const float Time, const FWorkerStats& OneWorkerStats)
{
	return true;
}

void ASpatialMetricsDisplay::ServerUpdateWorkerStats_Implementation(const float Time, const FWorkerStats& OneWorkerStats)
{
	int32 StatsIndex = WorkerStats.Find(OneWorkerStats);

	if (StatsIndex == INDEX_NONE)
	{
		StatsIndex = WorkerStats.AddDefaulted();
	}

	WorkerStats[StatsIndex] = OneWorkerStats;

	float& WorkerUpdateTime = WorkerStatsLastUpdateTime.FindOrAdd(OneWorkerStats.WorkerName);
	WorkerUpdateTime = Time;
}

void ASpatialMetricsDisplay::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialMetricsDisplay, WorkerStats);
}

void ASpatialMetricsDisplay::DrawDebug(class UCanvas* Canvas, APlayerController* Controller)
{
	enum StatColumns
	{
		StatColumn_Worker,
		StatColumn_AverageFrameTime,
		StatColumn_MovementCorrections,
		StatColumn_Last
	};

	const uint32 StatDisplayStartX = 25;
	const uint32 StatDisplayStartY = 80;

	const FString StatColumnTitles[StatColumn_Last] = { TEXT("Worker"), TEXT("Frame"), TEXT("Movement Corrections") };
	const uint32 StatColumnOffsets[StatColumn_Last] = { 0, 160, 80 };
	const uint32 StatRowOffset = 20;

	const FString StatSectionTitle = TEXT("Spatial Metrics Display");

	if (GetWorld()->IsServer())
	{
		return;
	}

	FFontRenderInfo FontRenderInfo = Canvas->CreateFontRenderInfo(false, true);

	UFont* RenderFont = GEngine->GetSmallFont();

	uint32 DrawX = StatDisplayStartX;
	uint32 DrawY = StatDisplayStartY;

	Canvas->SetDrawColor(FColor::Green);
	Canvas->DrawText(RenderFont, StatSectionTitle, DrawX, DrawY, 1.0f, 1.0f, FontRenderInfo);
	DrawY += StatRowOffset;

	Canvas->SetDrawColor(FColor::White);
	for (int i = 0; i < StatColumn_Last; ++i)
	{
		DrawX += StatColumnOffsets[i];
		Canvas->DrawText(RenderFont, StatColumnTitles[i], DrawX, DrawY, 1.0f, 1.0f, FontRenderInfo);
	}

	DrawY += StatRowOffset;

	for (const FWorkerStats& OneWorkerStats : WorkerStats)
	{
		DrawX = StatDisplayStartX + StatColumnOffsets[StatColumn_Worker];
		Canvas->DrawText(RenderFont, FString::Printf(TEXT("%s"), *OneWorkerStats.WorkerName), DrawX, DrawY, 1.0f, 1.0f, FontRenderInfo);

		DrawX += StatColumnOffsets[StatColumn_AverageFrameTime];
		Canvas->DrawText(RenderFont, FString::Printf(TEXT("%.2f ms"), 1000.f / OneWorkerStats.AverageFPS), DrawX, DrawY, 1.0f, 1.0f, FontRenderInfo);

		DrawX += StatColumnOffsets[StatColumn_MovementCorrections];
		Canvas->DrawText(RenderFont, FString::Printf(TEXT("%.4f"), OneWorkerStats.ServerMovementCorrections), DrawX, DrawY, 1.0f, 1.0f, FontRenderInfo);

		DrawY += StatRowOffset;
	}
}

void ASpatialMetricsDisplay::ToggleStatDisplay()
{
	if (DrawDebugDelegateHandle.IsValid())
	{
		UDebugDrawService::Unregister(DrawDebugDelegateHandle);
		DrawDebugDelegateHandle.Reset();
	}
	else
	{
		DrawDebugDelegateHandle = UDebugDrawService::Register(TEXT("Game"), FDebugDrawDelegate::CreateUObject(this, &ASpatialMetricsDisplay::DrawDebug));
	}
}

void ASpatialMetricsDisplay::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!GetWorld()->IsServer() || !HasActorBegunPlay())
	{
		return;
	}

	USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver());

	if (SpatialNetDriver == nullptr ||
		SpatialNetDriver->Connection == nullptr ||
		SpatialNetDriver->SpatialMetrics == nullptr)
	{
		return;
	}

	// Cleanup stats entries for workers that have not reported stats for awhile
	if (Role == ROLE_Authority)
	{
		const float CurrentTime = SpatialNetDriver->Time;
		TArray<FWorkerStats> WorkerStatsToRemove;

		for (const FWorkerStats& OneWorkerStats : WorkerStats)
		{
			if (ShouldRemoveStats(SpatialNetDriver->Time, OneWorkerStats))
			{
				WorkerStatsToRemove.Add(OneWorkerStats);
			}
		}

		for (const FWorkerStats& OneWorkerStats : WorkerStatsToRemove)
		{
			WorkerStatsLastUpdateTime.Remove(OneWorkerStats.WorkerName);
			WorkerStats.Remove(OneWorkerStats);
		}
	}

	const USpatialMetrics& Metrics = *SpatialNetDriver->SpatialMetrics;

	FWorkerStats Stats{};
	Stats.WorkerName = SpatialNetDriver->Connection->GetWorkerId().Left(WorkerNameMaxLength).ToLower();
	Stats.AverageFPS = Metrics.GetAverageFPS();

#if USE_SERVER_PERF_COUNTERS
	float MovementCorrectionsPerSecond = 0.f;
	int32 NumServerMoveCorrections = 0;
	float WorldTime = GetWorld()->GetTimeSeconds();
	NumServerMoveCorrections = PerfCountersGet(TEXT("NumServerMoveCorrections"), NumServerMoveCorrections);
	MovementCorrectionRecord OldestRecord;
	if (MovementCorrectionRecords.Peek(OldestRecord))
	{
		const float WorldTimeDelta = WorldTime - OldestRecord.Time;
		const int32 CorrectionsDelta = NumServerMoveCorrections - OldestRecord.MovementCorrections;
		if (WorldTimeDelta > 0.f && CorrectionsDelta > 0)
		{
			MovementCorrectionsPerSecond = CorrectionsDelta / WorldTimeDelta;
		}

		// Store the most recent 30 seconds of game time worth of measurements
		if (WorldTimeDelta > 30.f)
		{
			MovementCorrectionRecords.Pop();
		}
	}
	Stats.ServerMovementCorrections = MovementCorrectionsPerSecond;

	// Don't store a measurement if time hasn't progressed
	if (DeltaSeconds > 0.f)
	{
		MovementCorrectionRecords.Enqueue({ NumServerMoveCorrections, WorldTime });
	}
#endif

	ServerUpdateWorkerStats(SpatialNetDriver->Time, Stats);
}

bool ASpatialMetricsDisplay::ShouldRemoveStats(const float CurrentTime, const FWorkerStats& OneWorkerStats) const
{
	const float* LastUpdateTime = WorkerStatsLastUpdateTime.Find(OneWorkerStats.WorkerName);

	if (LastUpdateTime == nullptr)
	{
		return true;
	}

	const float TimeSinceUpdate = CurrentTime - *LastUpdateTime;
	return TimeSinceUpdate > DropStatsIfNoUpdateForTime;
}
