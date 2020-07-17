#include "Utils/SpatialDebuggerEditor.h"

#include "LoadBalancing/GridBasedLBStrategy.h"
#include "LoadBalancing/LayeredLBStrategy.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "SpatialCommonTypes.h"
#include "Utils/InspectionColors.h"
#include "Debug/DebugDrawService.h"
#include "Editor.h"

using namespace SpatialGDK;

ASpatialDebuggerEditor::ASpatialDebuggerEditor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.TickInterval = 1.f;

	bAlwaysRelevant = false;
	bNetLoadOnClient = false;
	bReplicates = false;

	NetUpdateFrequency = 1.f;

	bShowWorkerRegions = true;
}

void ASpatialDebuggerEditor::BeginPlay()
{
	// Destroy editor worker regions before game runs 
	DestroyWorkerRegions();
}

void ASpatialDebuggerEditor::Destroyed()
{
	DestroyWorkerRegions();
}

void ASpatialDebuggerEditor::SpatialToggleDebugger()
{
	DestroyWorkerRegions();

	if (bShowWorkerRegions)
	{
		InitialiseWorkerRegions();
		CreateWorkerRegions();
	}
	bShowWorkerRegions = !bShowWorkerRegions;

	// Redraw editor window to show changes
	GEditor->GetActiveViewport()->Invalidate();
}

void ASpatialDebuggerEditor::InitialiseWorkerRegions()
{
	ULayeredLBStrategy* LoadBalanceStrategy = NewObject<ULayeredLBStrategy>();
	LoadBalanceStrategy->Init(GetWorld());
	LoadBalanceStrategy->SetVirtualWorkerIds(1, LoadBalanceStrategy->GetMinimumRequiredWorkers());

	if (LoadBalanceStrategy)
	{
		const ULayeredLBStrategy* LayeredLBStrategy = Cast<ULayeredLBStrategy>(LoadBalanceStrategy);
		if (LayeredLBStrategy == nullptr)
		{
			UE_LOG(LogSpatialDebugger, Warning, TEXT("SpatialDebuggerEditor enabled but unable to get LayeredLBStrategy."));
			return;
		}

		if (const UGridBasedLBStrategy* GridBasedLBStrategy = Cast<UGridBasedLBStrategy>(LayeredLBStrategy->GetLBStrategyForVisualRendering()))
		{
			const UGridBasedLBStrategy::LBStrategyRegions LBStrategyRegions = GridBasedLBStrategy->GetLBStrategyRegions();

			WorkerRegions.SetNum(LBStrategyRegions.Num());
			for (int i = 0; i < LBStrategyRegions.Num(); i++)
			{
				const TPair<VirtualWorkerId, FBox2D>& LBStrategyRegion = LBStrategyRegions[i];
				FWorkerRegionInfo WorkerRegionInfo;
				// Generate our own unique worker name as we only need it to generate a unique colour
				const PhysicalWorkerName WorkerName = PhysicalWorkerName::Printf(TEXT("WorkerRegion%d"), i);
				WorkerRegionInfo.Color = GetColorForWorkerName(WorkerName);
				WorkerRegionInfo.Extents = LBStrategyRegion.Value;

				WorkerRegions[i] = WorkerRegionInfo;
			}
		}
	}
}
