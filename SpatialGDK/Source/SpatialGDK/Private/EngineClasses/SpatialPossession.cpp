// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialPossession.h"

#include "EngineClasses/SpatialNetDriver.h"

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
