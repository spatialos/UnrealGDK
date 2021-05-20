// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/Components/RemotePossessionComponent.h"

#include "Engine/World.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "LoadBalancing/OwnershipLockingPolicy.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(LogRemotePossessionComponent);

URemotePossessionComponent::URemotePossessionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bPendingDestroy(false)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

#if ENGINE_MINOR_VERSION <= 23
	bReplicates = true;
#else
	SetIsReplicatedByDefault(true);
#endif
}

void URemotePossessionComponent::BeginPlay()
{
	Super::BeginPlay();
	if (GetOwner()->HasAuthority())
	{
		if (Target == nullptr)
		{
			OnInvalidTarget();
			return;
		}
		if (Target->HasAuthority())
		{
			Possess();
			MarkToDestroy();
		}
	}
}

void URemotePossessionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, Target, COND_ServerOnly);
}

void URemotePossessionComponent::OnAuthorityGained()
{
	if (Target == nullptr)
	{
		OnInvalidTarget();
		return;
	}
	if (!Target->HasAuthority())
	{
		UE_LOG(LogRemotePossessionComponent, Verbose, TEXT("Worker is not authoritative over target: %s"), *Target->GetName());
	}
	else
	{
		Possess();
		DestroyComponent();
	}
}

void URemotePossessionComponent::Possess()
{
	if (EvaluatePossess())
	{
		AController* Controller = Cast<AController>(GetOwner());
		ensure(Controller);
		Controller->Possess(Target);
		UE_LOG(LogRemotePossessionComponent, Verbose, TEXT("Remote possession succesful on (%s)"), *Target->GetName());
	}
	else
	{
		UE_LOG(LogRemotePossessionComponent, Verbose, TEXT("EvaluatePossess(%s) failed"), *Target->GetName());
	}
}

void URemotePossessionComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bPendingDestroy)
	{
		UE_LOG(LogRemotePossessionComponent, Verbose, TEXT("Destroy RemotePossessionComponent"));
		DestroyComponent();
	}
}

bool URemotePossessionComponent::EvaluatePossess_Implementation()
{
	return true;
}

void URemotePossessionComponent::OnInvalidTarget_Implementation()
{
	UE_LOG(LogRemotePossessionComponent, Error, TEXT("Target is invalid for remote possession component on actor %s"),
		   *GetOwner()->GetName());
}

void URemotePossessionComponent::MarkToDestroy()
{
	bPendingDestroy = true;
}
