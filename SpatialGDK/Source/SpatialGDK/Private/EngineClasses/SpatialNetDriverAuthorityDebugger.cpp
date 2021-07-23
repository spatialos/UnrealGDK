// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialNetDriverAuthorityDebugger.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "EngineClasses/SpatialShadowActor.h"

DEFINE_LOG_CATEGORY(LogSpatialNetDriverAuthorityDebugger);

const TArray<FName> USpatialNetDriverAuthorityDebugger::SuppressedActors = { TEXT("GameStateBase"), TEXT("DefaultPawn"),
																			 TEXT("PlayerState"),
																			 TEXT("SpatialFunctionalTestFlowController"),
																			 TEXT("LockingPlayerController_C") };

const TArray<FName> USpatialNetDriverAuthorityDebugger::SuppressedProperties = { TEXT("Role"),
																				 TEXT("RemoteRole"),
																				 TEXT("Owner"),
																				 TEXT("OwnerPC"),
																				 TEXT("CurrentStepIndex"),
																				 TEXT("bPreparedTest"),
																				 TEXT("bFinishedTest"),
																				 TEXT("bReadyToSpawnServerControllers") };

void USpatialNetDriverAuthorityDebugger::Init(USpatialNetDriver& InNetDriver)
{
	NetDriver = &InNetDriver;
}

void USpatialNetDriverAuthorityDebugger::CheckUnauthorisedDataChanges()
{
	for (auto It = SpatialShadowActors.CreateIterator(); It; ++It)
	{
		It.Value()->CheckUnauthorisedDataChanges();
	}
}

void USpatialNetDriverAuthorityDebugger::AddSpatialShadowActor(const Worker_EntityId_Key EntityId)
{
	AActor* Actor = Cast<AActor>(NetDriver->PackageMap->GetObjectFromEntityId(EntityId));
	if (!IsValid(Actor) || Actor->IsPendingKillOrUnreachable())
	{
		return;
	}

	if (SpatialShadowActors.Contains(EntityId))
	{
		UE_LOG(LogSpatialNetDriverAuthorityDebugger, Error,
			   TEXT("Should only be adding a SpatialShadowActor once for each entity, EntityID %i"), EntityId);
	}
	else
	{
		USpatialShadowActor* SpatialShadowActor(NewObject<USpatialShadowActor>());
		SpatialShadowActor->Init(*Actor);
		SpatialShadowActors.Emplace(EntityId, SpatialShadowActor);
	}
}

void USpatialNetDriverAuthorityDebugger::RemoveSpatialShadowActor(const Worker_EntityId_Key EntityId)
{
	SpatialShadowActors.Remove(EntityId);
}

void USpatialNetDriverAuthorityDebugger::UpdateSpatialShadowActor(const Worker_EntityId_Key EntityId)
{
	USpatialShadowActor** SpatialShadowActor = SpatialShadowActors.Find(EntityId);

	if (SpatialShadowActor == nullptr)
	{
		// We can receive updates without receiving adds - in this case create the SpatialShadowActor
		AddSpatialShadowActor(EntityId);
		return;
	}

	(*SpatialShadowActor)->Update();
}

bool USpatialNetDriverAuthorityDebugger::IsSuppressedActor(const AActor& InActor)
{
	return SuppressedActors.Contains(InActor.GetClass()->GetFName());
}

bool USpatialNetDriverAuthorityDebugger::IsSuppressedProperty(const FProperty& InProperty)
{
	return SuppressedProperties.Contains(InProperty.GetFName());
}
