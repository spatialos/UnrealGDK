// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/Components/RemotePossessionComponent.h"
#include "LoadBalancing/AbstractLBStrategy.h"

#include "Engine/World.h"

DEFINE_LOG_CATEGORY(LogRemotePossessionComponent);

URemotePossessionComponent::URemotePossessionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, PendingDestroy(false)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

#if ENGINE_MINOR_VERSION <= 23
	bReplicates = true;
#else
	SetIsReplicatedByDefault(true);
#endif
}

void URemotePossessionComponent::OnAuthorityGained()
{
	Possess();
}

void URemotePossessionComponent::Possess()
{
	UE_LOG(LogRemotePossessionComponent, Log, TEXT("OnAuthorityGained"));
	AController* Controller = Cast<AController>(GetOwner());
	if (ensure(Controller != nullptr))
	{
		if (Target == nullptr)
		{
			UE_LOG(LogRemotePossessionComponent, Log, TEXT("Target is null"));
			return;
		}
		if (!Target->HasAuthority())
		{
			UE_LOG(LogRemotePossessionComponent, Log, TEXT("Target is hasn't authority"));
			return;
		}
		else
		{
			if (EvaluatePossess(Controller, Target->GetController()))
			{
				UE_LOG(LogRemotePossessionComponent, Log, TEXT("Possess(%s)"), *Target->GetName());
				Controller->Possess(Target);
			}
			else
			{
				UE_LOG(LogRemotePossessionComponent, Log, TEXT("EvaluatePossess(%s) failed"), *Target->GetName());
			}
			PendingDestroy = true;
		}
	}
}

bool URemotePossessionComponent::EvaluateMigration(UAbstractLBStrategy* LBStrategy, VirtualWorkerId& WorkerId)
{
	if (true == PendingDestroy)
		return false;
	if (Target != nullptr)
	{
		UE_LOG(LogRemotePossessionComponent, Log, TEXT("Component->Target is:%s"), *Target->GetName());
		VirtualWorkerId ActorAuthVirtualWorkerId = LBStrategy->WhoShouldHaveAuthority(*GetOwner());
		VirtualWorkerId TargetVirtualWorkerId = LBStrategy->WhoShouldHaveAuthority(*Target);

		if (TargetVirtualWorkerId == SpatialConstants::INVALID_VIRTUAL_WORKER_ID)
		{
			UE_LOG(LogRemotePossessionComponent, Error, TEXT("Load Balancing Strategy returned invalid virtual worker for actor %s"),
				   **GetOwner()->GetName());
		}
		else if (ActorAuthVirtualWorkerId != TargetVirtualWorkerId)
		{
			UE_LOG(LogRemotePossessionComponent, Log, TEXT("Migrate actor:%s to worker:%d"), *GetOwner()->GetName(), TargetVirtualWorkerId);
			WorkerId = TargetVirtualWorkerId;
			return true;
		}
		else
		{
			Possess();
			UE_LOG(LogRemotePossessionComponent, Log, TEXT("Actor:%s and Target:%s are in same worker:%d"), *GetOwner()->GetName(),
				   *Target->GetName(), TargetVirtualWorkerId);
		}
	}
	else
	{
		UE_LOG(LogRemotePossessionComponent, Log, TEXT("Target is:null"));
	}
	return false;
}

void URemotePossessionComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (PendingDestroy)
	{
		UE_LOG(LogRemotePossessionComponent, Log, TEXT("DestroyComponent"));
		DestroyComponent();
	}
}

bool URemotePossessionComponent::EvaluatePossess(AController* CurrentController, AController* WantController)
{
	if (WantController == nullptr)
		return true;
	return false;
}
