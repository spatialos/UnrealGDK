// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialNetDriverAuthorityDebugger.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "EngineClasses/SpatialShadowActor.h"

DEFINE_LOG_CATEGORY(LogSpatialNetDriverAuthorityDebugger);

const TArray<FName> USpatialNetDriverAuthorityDebugger::SuppressedActors = { TEXT("GameStateBase"),
																			   TEXT("DefaultPawn"),
																			   TEXT("PlayerState"),
																			   TEXT("SpatialFunctionalTestFlowController"),
																			   TEXT("GameplayDebuggerCategoryReplicator"),
																			   TEXT("SpatialTestPropertyReplicationMultiworker") };


void USpatialNetDriverAuthorityDebugger::Init(USpatialNetDriver& InNetDriver)
{
	NetDriver = &InNetDriver;
}

void USpatialNetDriverAuthorityDebugger::CheckUnauthorisedDataChanges(const AActor* Actor)
{
	if (!NetDriver->IsServer())
	{
		return;
	}

	Worker_EntityId EntityId = NetDriver->PackageMap->GetEntityIdFromObject(Actor);

	USpatialShadowActor** SpatialShadowActor = SpatialShadowActors.Find(EntityId);

	if (SpatialShadowActor == nullptr)
	{
		return;
	}

	(*SpatialShadowActor)->CheckUnauthorisedDataChanges(EntityId, Actor);
}

void USpatialNetDriverAuthorityDebugger::AddSpatialShadowActor(const Worker_EntityId_Key EntityId)
{
	if (!NetDriver->IsServer())
	{
		return;
	}

	AActor* Actor = Cast<AActor>(NetDriver->PackageMap->GetObjectFromEntityId(EntityId));
	if (Actor == nullptr || !IsValid(Actor) || Actor->IsPendingKillOrUnreachable())
	{
		return;
	}

	if (SpatialShadowActors.Contains(EntityId))
	{
		UE_LOG(LogSpatialNetDriverAuthorityDebugger, Error,
			   TEXT("Should only be adding a SpatialShadowActor once for each entity, EntityID %i"),
			   EntityId);
	}
	else
	{
		USpatialShadowActor* SpatialShadowActor(NewObject<USpatialShadowActor>());
		SpatialShadowActor->Init(EntityId, Actor);
		SpatialShadowActors.Emplace(EntityId, SpatialShadowActor);
	}
}

void USpatialNetDriverAuthorityDebugger::RemoveSpatialShadowActor(const Worker_EntityId_Key EntityId)
{
	if (!NetDriver->IsServer())
	{
		return;
	}

	SpatialShadowActors.Remove(EntityId);
}

void USpatialNetDriverAuthorityDebugger::UpdateSpatialShadowActor(const Worker_EntityId_Key EntityId)
{
	if (!NetDriver->IsServer())
	{
		return;
	}

	USpatialShadowActor** SpatialShadowActor = SpatialShadowActors.Find(EntityId);

	if (SpatialShadowActor == nullptr)
	{
		// We can receive updates without receiving adds - in this case create the SpatialShadowActor
		AddSpatialShadowActor(EntityId);
		return;
	}

	AActor* Actor = Cast<AActor>(NetDriver->PackageMap->GetObjectFromEntityId(EntityId));
	if (!IsValid(Actor))
	{
		return;
	}

	(*SpatialShadowActor)->Update(EntityId, Actor);
}

bool USpatialNetDriverAuthorityDebugger::IsSuppressedActor(const AActor* InActor)
{
	return SuppressedActors.Contains(*InActor->GetClass()->GetName());
}
