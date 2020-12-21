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
	AController* Controller = Cast<AController>(GetOwner());
	ensure(Controller);
	if (Target == nullptr)
	{
		UE_LOG(LogRemotePossessionComponent, Error, TEXT("Target is null"));
		return;
	}
	if (!Target->HasAuthority())
	{
		UE_LOG(LogRemotePossessionComponent, Verbose, TEXT("Worker is not authoritative over target: %s"), *Target->GetName());
		return;
	}
	else
	{
		if (EvaluatePossess())
		{
			UE_LOG(LogRemotePossessionComponent, Log, TEXT("Possess(%s)"), *Target->GetName());
			Controller->Possess(Target);
		}
		else
		{
			UE_LOG(LogRemotePossessionComponent, Log, TEXT("EvaluatePossess(%s) failed"), *Target->GetName());
		}
		MarkToDestroy();
	}
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

bool URemotePossessionComponent::EvaluatePossess()
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		return ReceiveEvaluatePossess();
	}
	else
	{
		if (Target->GetController() == nullptr)
			return true;
		return false;
	}
}

void URemotePossessionComponent::MarkToDestroy()
{
	PendingDestroy = true;
}
