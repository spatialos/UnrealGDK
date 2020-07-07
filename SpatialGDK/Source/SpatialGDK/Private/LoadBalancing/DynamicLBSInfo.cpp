#include "LoadBalancing/DynamicLBSInfo.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "LoadBalancing/GridBasedLBStrategy.h"
#include "Net/UnrealNetwork.h"

using namespace SpatialGDK;

ADynamicLBSInfo::ADynamicLBSInfo(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	bAlwaysRelevant = true;
	bNetLoadOnClient = false;
	bReplicates = true;

	NetUpdateFrequency = 30.f;
}

void ADynamicLBSInfo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ADynamicLBSInfo, DynamicWorkerCells, COND_SimulatedOnly);
}

void ADynamicLBSInfo::BeginPlay()
{
	//UE_LOG(LogTemp, Warning, TEXT("%s [DynamicLBSInfo] BeginPlay(), HasAuthority = %s"), *FDateTime::Now().ToString(), HasAuthority() ? "true" : "false");

	NetDriver = Cast<USpatialNetDriver>(GetNetDriver());

	if (HasAuthority() && NetDriver->LoadBalanceStrategy)
	{
		if (const UGridBasedLBStrategy* GridBasedLBStrategy = Cast<UGridBasedLBStrategy>(NetDriver->LoadBalanceStrategy))
		{
			const UGridBasedLBStrategy::LBStrategyRegions LBStrategyRegions = GridBasedLBStrategy->GetLBStrategyRegions();
			DynamicWorkerCells.SetNum(LBStrategyRegions.Num());
			for (int i = 0; i < LBStrategyRegions.Num(); i++)
			{
				const TPair<VirtualWorkerId, FBox2D>& LBStrategyRegion = LBStrategyRegions[i];
				DynamicWorkerCells[i] = FBox2D(LBStrategyRegion.Value.Min, LBStrategyRegion.Value.Max);
			}
		}
	}
}

void ADynamicLBSInfo::OnAuthorityGained()
{
	//UE_LOG(LogTemp, Warning, TEXT("%s [DynamicLBSInfo] OnAuthorityGained()"), *FDateTime::Now().ToString());
}

void ADynamicLBSInfo::OnRep_DynamicWorkerCells()
{
	//UE_LOG(LogTemp, Warning, TEXT("[DynamicLBSInfo] worker cells changed!!!"));
}
