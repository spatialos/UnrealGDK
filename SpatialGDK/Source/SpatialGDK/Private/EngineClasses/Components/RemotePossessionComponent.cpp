// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/Components/RemotePossessionComponent.h"
#include "LoadBalancing/AbstractLBStrategy.h"

#include "Engine/World.h"

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
	if (Target == nullptr)
	{
		OnInvalidTarget();
		return;
	}
	if (Target->HasAuthority())
	{
		Possess();
	}
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
	MarkToDestroy();
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
