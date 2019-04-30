// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialMetricsDisplay.h"

#include "DebugRenderSceneProxy.h"
#include "Debug/DebugDrawService.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Net/UnrealNetwork.h"
#include "Utils/SpatialMetrics.h"

ASpatialMetricsDisplay::ASpatialMetricsDisplay(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	bReplicates = true;
	bAlwaysRelevant = true;

	NetUpdateFrequency = 1;
}

void ASpatialMetricsDisplay::BeginPlay()
{
	Super::BeginPlay();

	WorkerStats.Reserve(PreallocatedWorkerCount);
	WorkerStatsLastUpdateTime.Reserve(PreallocatedWorkerCount);
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
		StatColumn_AverageFPS,
		StatColumn_WorkerLoad,
		StatColumn_Last
	};

	const uint32 StatDisplayStartX = 25;
	const uint32 StatDisplayStartY = 80;

	const FString StatColumnTitles[StatColumn_Last] = { TEXT("Worker"), TEXT("FPS"), TEXT("Load") };
	const uint32 StatColumnOffsets[StatColumn_Last] = { 0, 160, 80 };
	const uint32 StatRowOffset = 20;

	const FString StatSectionTitle = TEXT("Spatial Metrics Display");

	if (GetWorld()->IsServer())
		return;

	FFontRenderInfo FontRenderInfo = Canvas->CreateFontRenderInfo(false, true);

	UFont* RenderFont = GEngine->GetSmallFont();

	uint32 x = StatDisplayStartX;
	uint32 y = StatDisplayStartY;

	Canvas->SetDrawColor(FColor::Green);
	Canvas->DrawText(RenderFont, StatSectionTitle, x, y, 1.0f, 1.0f, FontRenderInfo);
	y += StatRowOffset;

	Canvas->SetDrawColor(FColor::White);
	for (int i = 0; i < StatColumn_Last; ++i)
	{
		x += StatColumnOffsets[i];
		Canvas->DrawText(RenderFont, StatColumnTitles[i], x, y, 1.0f, 1.0f, FontRenderInfo);
	}

	x = StatColumnOffsets[StatColumn_Worker];
	y += StatRowOffset;

	for (const FWorkerStats& OneWorkerStats : WorkerStats)
	{
		Canvas->DrawText(RenderFont, FString::Printf(TEXT("%s"), *OneWorkerStats.WorkerName), x, y, 1.0f, 1.0f, FontRenderInfo);
		x += StatColumnOffsets[StatColumn_AverageFPS];

		Canvas->DrawText(RenderFont, FString::Printf(TEXT("%.2f"), OneWorkerStats.AverageFPS), x, y, 1.0f, 1.0f, FontRenderInfo);
		x += StatColumnOffsets[StatColumn_WorkerLoad];

		Canvas->DrawText(RenderFont, FString::Printf(TEXT("%.2f"), OneWorkerStats.WorkerLoad), x, y, 1.0f, 1.0f, FontRenderInfo);
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

	if (!GetWorld()->IsServer())
		return;

	USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver());

	if (!SpatialNetDriver ||
		!SpatialNetDriver->SpatialMetrics)
		return;

	// Cleanup stats entries for workers that have not reported stats for awhile
	if (Role == ROLE_Authority)
	{
		const float CurrentTime = SpatialNetDriver->Time;
		TArray<FWorkerStats> WorkerStatsToRemove;

		for (const FWorkerStats& OneWorkerStats : WorkerStats)
		{
			const float TimeSinceUpdate = CurrentTime - WorkerStatsLastUpdateTime[OneWorkerStats.WorkerName];

			if (TimeSinceUpdate > DropStatsIfNoUpdateForTime)
			{
				WorkerStatsToRemove.Add(OneWorkerStats);
			}
		}

		for (const FWorkerStats& OneWorkerStats : WorkerStatsToRemove)
		{
			WorkerStats.Remove(OneWorkerStats);
		}
	}

	const USpatialMetrics& Metrics = *SpatialNetDriver->SpatialMetrics;

	FWorkerStats Stats;
	Stats.WorkerName = SpatialNetDriver->Connection->GetWorkerId().Left(WorkerNameMaxLength).ToLower();
	Stats.AverageFPS = Metrics.GetAverageFPS();
	Stats.WorkerLoad = Metrics.GetWorkerLoad();

	ServerUpdateWorkerStats(SpatialNetDriver->Time, Stats);
}
