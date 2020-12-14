// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialPossession.h"

#include "Engine/World.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "LoadBalancing/LayeredLBStrategy.h"

DEFINE_LOG_CATEGORY(LogSpatialPossession);

void USpatialPossession::RemotePossess(AController* Controller, APawn* Pawn)
{
	ensure(Controller);
	// If we're calling RemotePossess on a null pointer return
	if (Pawn == nullptr)
	{
		return;
	}

	// If the pawn we want to possess is authoritative on this worker just possess it
	if (Pawn->HasAuthority())
	{
		Controller->Possess(Pawn);
	}
	else
	{
		USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(Controller->GetNetDriver());
		if (NetDriver->LockingPolicy && NetDriver->LockingPolicy->IsLocked(Controller))
		{
			Controller->OnPossessFailed(ERemotePossessFailure::ControllerLocked);
			return;
		}
		Controller->IntendedPawnToPossess = Pawn;
		Controller->IntendedPossessionCount = Pawn->PossessionCount;
		// We force unpossess because it appears even though we own this pawn it won't migrate (at least not right away)
		Controller->UnPossess();

		Controller->ForceNetUpdate();
	}
}

void USpatialPossession::PossessAfterMigration(AController& Controller)
{
	UWorld* World = Controller.GetWorld();
	USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(World->GetNetDriver());

	if (auto lbs = Cast<ULayeredLBStrategy>(NetDriver->LoadBalanceStrategy))
	{
		FName LocalLayer = lbs->GetLocalLayerName();
		auto sublbs = lbs->GetLBStrategyForLayer(LocalLayer);
		UGridBasedLBStrategy* GridStrategy = Cast<UGridBasedLBStrategy>(sublbs);
		UE_LOG(LogSpatialPossession, Log, TEXT("PossessAfterMigration Current WorkerId: %d"), GridStrategy->GetLocalVirtualWorkerId());
	}

	auto WorkerId = NetDriver->LoadBalanceStrategy->WhoShouldHaveAuthority(Controller);
	UE_LOG(LogSpatialPossession, Log, TEXT("PossessAfterMigration: %s, WorkerId: %d"), *Controller.GetName(), WorkerId);

	WorkerId = NetDriver->LoadBalanceStrategy->WhoShouldHaveAuthority(*Controller.IntendedPawnToPossess);
	UE_LOG(LogSpatialPossession, Log, TEXT("PossessAfterMigration: %s, WorkerId: %d"), *Controller.IntendedPawnToPossess->GetName(),
		   WorkerId);

	if (Controller.IntendedPawnToPossess->HasAuthority())
	{
		if (Controller.IntendedPossessionCount == Controller.IntendedPawnToPossess->PossessionCount)
		{
			Controller.Possess(Controller.IntendedPawnToPossess);
		}
		else
		{
			Controller.OnPossessFailed(ERemotePossessFailure::PossessionStateChanged);
		}
		Controller.IntendedPawnToPossess = nullptr;
	}
}
