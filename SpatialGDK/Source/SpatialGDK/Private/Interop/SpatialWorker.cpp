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
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			FString WorkerType = GameInstance->GetSpatialWorkerType();
			bool bUsesDefaultAuthority = Actor->GetClass()->WorkerAssociation.IsEmpty() && WorkerType.Equals(SpatialConstants::ServerWorkerType);
			bool bMatchesWorkerAssociation = Actor->GetClass()->WorkerAssociation.Equals(WorkerType);
			return bUsesDefaultAuthority || bMatchesWorkerAssociation;
		}
	}
	return false;
}

bool USpatialWorker::CanHaveAuthorityForClass(const UWorld* World, const UClass* Class)
{
	if (World == nullptr || Class == nullptr)
	{
		return false;
	}

	if (UGameInstance* GameInstance = World->GetGameInstance())
	{
		FString WorkerType = GameInstance->GetSpatialWorkerType();
		bool bUsesDefaultAuthority = Class->WorkerAssociation.IsEmpty() && WorkerType.Equals(SpatialConstants::ServerWorkerType);
		bool bMatchesWorkerAssociation = Class->WorkerAssociation.Equals(WorkerType);
		return bUsesDefaultAuthority || bMatchesWorkerAssociation;
	}
	return false;
}
