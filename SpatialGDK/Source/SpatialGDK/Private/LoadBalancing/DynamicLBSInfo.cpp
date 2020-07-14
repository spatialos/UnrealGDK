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

void ADynamicLBSInfo::Init(const TArray<FBox2D> WorkerCells)
{
	NetDriver = Cast<USpatialNetDriver>(GetNetDriver());
	if (HasAuthority())
	{
		DynamicWorkerCells.SetNum(WorkerCells.Num());
		ActorCounters.SetNum(WorkerCells.Num());
		for (int i = 0; i < WorkerCells.Num(); i++)
		{
			DynamicWorkerCells[i] = FBox2D(WorkerCells[i].Min, WorkerCells[i].Max);
			ActorCounters[i] = 0;
		}
	}
}

void ADynamicLBSInfo::OnAuthorityGained()
{
	//UE_LOG(LogDynamicLBStrategy, Warning, TEXT("%s [DynamicLBSInfo] OnAuthorityGained()"), *FDateTime::Now().ToString());
}

void ADynamicLBSInfo::OnRep_DynamicWorkerCells()
{
	//UE_LOG(LogDynamicLBStrategy, Warning, TEXT("[DynamicLBSInfo] VirtualWorker[%d] received worker cells change!!!"), NetDriver->VirtualWorkerTranslator->GetLocalVirtualWorkerId());
}

void ADynamicLBSInfo::OnRep_ActorCounters()
{
	//UE_LOG(LogDynamicLBStrategy, Warning, TEXT("[DynamicLBSInfo] VirtualWorker[%d] received actor counters change!!!"), NetDriver->VirtualWorkerTranslator->GetLocalVirtualWorkerId());
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
