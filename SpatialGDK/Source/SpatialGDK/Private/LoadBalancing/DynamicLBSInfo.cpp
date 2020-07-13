#include "LoadBalancing/DynamicLBSInfo.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "LoadBalancing/GridBasedLBStrategy.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(LogDynamicLBStrategy);

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
	DOREPLIFETIME_CONDITION(ADynamicLBSInfo, ActorCounters, COND_SimulatedOnly);
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
			ActorCounters.SetNum(LBStrategyRegions.Num());

			for (int i = 0; i < LBStrategyRegions.Num(); i++)
			{
				const TPair<VirtualWorkerId, FBox2D>& LBStrategyRegion = LBStrategyRegions[i];
				DynamicWorkerCells[i] = FBox2D(LBStrategyRegion.Value.Min, LBStrategyRegion.Value.Max);

				//ActorCounters[i] = 0;
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

void ADynamicLBSInfo::OnRep_ActorCounters()
{
	UE_LOG(LogTemp, Warning, TEXT("[DynamicLBSInfo] actor counters changed!!!"));
}

uint32 ADynamicLBSInfo::GetActorCounter(const VirtualWorkerId TargetWorkerId)
{
	return ActorCounters[TargetWorkerId - 1];
}

void ADynamicLBSInfo::IncreseActorCounter(const VirtualWorkerId TargetWorkerId)
{
	uint32 ActorCounter;
	if (TargetWorkerId > (uint32)ActorCounters.Num())
	{
		ActorCounters.SetNum(TargetWorkerId);
		ActorCounter = 1;
	}
	else
	{
		ActorCounter = GetActorCounter(TargetWorkerId);
		ActorCounter++;
	}
	ActorCounters[TargetWorkerId - 1] = ActorCounter;
	UE_LOG(LogDynamicLBStrategy, Log, TEXT("VirtualWorker[%d] increased actor counter to: %d"), TargetWorkerId, ActorCounter);
}

void ADynamicLBSInfo::DecreaseActorCounter(const VirtualWorkerId TargetWorkerId)
{
	uint32 ActorCounter;
	if (TargetWorkerId > (uint32)ActorCounters.Num())
	{
		ActorCounters.SetNum(TargetWorkerId);
		ActorCounter = 0;
	}
	else
	{
		ActorCounter = GetActorCounter(TargetWorkerId);
		if (ActorCounter > 0)
		{
			ActorCounter--;
		}
	}
	ActorCounters[TargetWorkerId - 1] = ActorCounter;
	UE_LOG(LogDynamicLBStrategy, Log, TEXT("VirtualWorker[%d] decreased actor counter to: %d"), TargetWorkerId, ActorCounter);
}
