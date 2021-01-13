// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestPossessionPlayerController.h"
#include "Engine/World.h"
#include "EngineClasses/Components/RemotePossessionComponent.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Utils/SpatialStatics.h"

DEFINE_LOG_CATEGORY(LogTestPossessionPlayerController);

int32 ATestPossessionPlayerController::OnPossessCalled = 0;

ATestPossessionPlayerController::ATestPossessionPlayerController() {}

void ATestPossessionPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	++OnPossessCalled;
	UE_LOG(LogTestPossessionPlayerController, Log, TEXT("%s OnPossess(%s) OnPossessCalled:%d"), *GetName(), *InPawn->GetName(),
		   OnPossessCalled);
}

void ATestPossessionPlayerController::OnUnPossess()
{
	Super::OnUnPossess();
	UE_LOG(LogTestPossessionPlayerController, Log, TEXT("%s OnUnPossess()"), *GetName());
}

void ATestPossessionPlayerController::RemotePossessOnClient_Implementation(APawn* InPawn, bool bLockBefore)
{
	UE_LOG(LogTestPossessionPlayerController, Log, TEXT("%s RemotePossessOnClient_Implementation:%s"), *GetName(), *InPawn->GetName());
	if (bLockBefore)
	{
		USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(GetNetDriver());
		if (NetDriver != nullptr && NetDriver->LockingPolicy)
		{
			NetDriver->LockingPolicy->AcquireLock(this, TEXT("TestLock"));
		}
	}
	RemotePossessOnServer(InPawn);
}

void ATestPossessionPlayerController::RemotePossessOnServer(APawn* InPawn)
{
	URemotePossessionComponent* Component =
		NewObject<URemotePossessionComponent>(this, URemotePossessionComponent::StaticClass(), TEXT("CrossServer Possession"));
	Component->Target = InPawn;
	Component->RegisterComponent();
}

void ATestPossessionPlayerController::ResetCalledCounter()
{
	OnPossessCalled = 0;
}
