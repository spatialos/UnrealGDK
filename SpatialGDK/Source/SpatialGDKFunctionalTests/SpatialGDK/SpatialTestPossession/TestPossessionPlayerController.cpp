// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestPossessionPlayerController.h"
#include "Engine/World.h"
#include "EngineClasses/Components/RemotePossessionComponent.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialNetDriverDebugContext.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "LoadBalancing/DebugLBStrategy.h"
#include "SpatialConstants.h"
#include "Utils/SpatialStatics.h"

DEFINE_LOG_CATEGORY(LogTestPossessionPlayerController);

int32 ATestPossessionPlayerController::OnPossessCalled = 0;

ATestPossessionPlayerController::ATestPossessionPlayerController()
	: BeforePossessionWorkerId(SpatialConstants::INVALID_VIRTUAL_WORKER_ID)
	, AfterPossessionWorkerId(SpatialConstants::INVALID_VIRTUAL_WORKER_ID)
{
}

void ATestPossessionPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (HasAuthority() && InPawn->HasAuthority())
	{
		++OnPossessCalled;
		AfterPossessionWorkerId = GetCurrentWorkerId();
		UE_LOG(LogTestPossessionPlayerController, Log, TEXT("%s OnPossess(%s) OnPossessCalled:%d"), *GetName(), *InPawn->GetName(),
			   OnPossessCalled);
	}
	else
	{
		UE_LOG(LogTestPossessionPlayerController, Error, TEXT("%s OnPossess(%s) OnPossessCalled:%d in different worker"), *GetName(),
			   *InPawn->GetName(), OnPossessCalled);
	}
}

void ATestPossessionPlayerController::RemotePossessOnServer(APawn* InPawn)
{
	URemotePossessionComponent* Component =
		NewObject<URemotePossessionComponent>(this, URemotePossessionComponent::StaticClass(), TEXT("CrossServer Possession"));
	Component->Target = InPawn;
	Component->RegisterComponent();
	BeforePossessionWorkerId = GetCurrentWorkerId();
}

void ATestPossessionPlayerController::ResetCalledCounter()
{
	OnPossessCalled = 0;
}

VirtualWorkerId ATestPossessionPlayerController::GetCurrentWorkerId()
{
	UAbstractLBStrategy* LBStrategy = nullptr;
	UWorld* World = GetWorld();
	USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(World->GetNetDriver());
	if (NetDriver->DebugCtx != nullptr)
	{
		LBStrategy = NetDriver->DebugCtx->DebugStrategy->GetWrappedStrategy();
	}
	else
	{
		LBStrategy = NetDriver->LoadBalanceStrategy;
	}
	if (LBStrategy != nullptr)
	{
		return LBStrategy->GetLocalVirtualWorkerId();
	}
	return SpatialConstants::INVALID_VIRTUAL_WORKER_ID;
}
