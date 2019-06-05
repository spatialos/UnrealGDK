// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialWorker.h"

#include "SpatialConstants.h"

#include "Engine/World.h"


bool USpatialWorker::CanHaveAuthority(const AActor* Actor)
{
	if (Actor == nullptr)
	{
		return false;
	}

	if (UWorld* World = Actor->GetWorld())
	{
		FString WorkerType = World->GetGameInstance()->GetSpatialWorkerType();
		bool bUsesDefaultAuthority = Actor->GetClass()->WorkerAssociation.IsEmpty() && WorkerType.Equals(SpatialConstants::ServerWorkerType);
		bool bMatchesWorkerAssociation = Actor->GetClass()->WorkerAssociation.Equals(WorkerType);
		return bUsesDefaultAuthority || bMatchesWorkerAssociation;
	}
	return false;
}
