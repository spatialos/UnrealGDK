// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestPossessionController.h"
#include "Engine/World.h"
#include "EngineClasses/Components/RemotePossessionComponent.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialNetDriverDebugContext.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "LoadBalancing/DebugLBStrategy.h"
#include "SpatialConstants.h"
#include "Utils/SpatialStatics.h"

DEFINE_LOG_CATEGORY(LogTestPossessionController);

int32 ATestPossessionController::OnPossessCalled = 0;

ATestPossessionController::ATestPossessionController()
	: BeforePossessionWorkerId(SpatialConstants::INVALID_VIRTUAL_WORKER_ID)
	, AfterPossessionWorkerId(SpatialConstants::INVALID_VIRTUAL_WORKER_ID)
{
	bReplicates = true;
}

void ATestPossessionController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (HasAuthority() && InPawn->HasAuthority())
	{
		++OnPossessCalled;
		AfterPossessionWorkerId = GetCurrentWorkerId();
		UE_LOG(LogTestPossessionController, Log, TEXT("%s OnPossess(%s) OnPossessCalled:%d"), *GetName(), *InPawn->GetName(),
			   OnPossessCalled);
	}
	else
	{
		UE_LOG(LogTestPossessionController, Error, TEXT("%s OnPossess(%s) OnPossessCalled:%d in different worker"), *GetName(),
			   *InPawn->GetName(), OnPossessCalled);
	}
}

void ATestPossessionController::OnUnPossess()
{
	Super::OnUnPossess();
	UE_LOG(LogTestPossessionController, Log, TEXT("%s OnUnPossess()"), *GetName());
}

void ATestPossessionController::RemotePossessOnServer(APawn* InPawn, bool bLockBefore)
{
	if (bLockBefore)
	{
		USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(GetNetDriver());
		if (NetDriver != nullptr && NetDriver->LockingPolicy)
		{
			NetDriver->LockingPolicy->AcquireLock(this, TEXT("TestLock"));
		}
	}
	URemotePossessionComponent* Component =
		NewObject<URemotePossessionComponent>(this, URemotePossessionComponent::StaticClass(), TEXT("CrossServer Possession"));
	Component->Target = InPawn;
	Component->RegisterComponent();
	BeforePossessionWorkerId = GetCurrentWorkerId();
}

void ATestPossessionController::RemotePossess_Implementation(APawn* InPawn)
{
	RemotePossessOnServer(InPawn, false);
}

void ATestPossessionController::ResetCalledCounter()
{
	OnPossessCalled = 0;
}

VirtualWorkerId ATestPossessionController::GetCurrentWorkerId()
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
