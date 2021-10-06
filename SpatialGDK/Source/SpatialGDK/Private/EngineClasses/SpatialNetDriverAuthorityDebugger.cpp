// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialNetDriverAuthorityDebugger.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "EngineClasses/SpatialShadowActor.h"

DEFINE_LOG_CATEGORY(LogSpatialNetDriverAuthorityDebugger);

const TArray<FName> USpatialNetDriverAuthorityDebugger::SuppressedActors = {
	TEXT("SpatialFunctionalTestFlowController"),	// OwningTest
	TEXT("LockingPlayerController_C"),				// Multiple
	TEXT("TestPossessionPlayerController"),			// Multiple
	TEXT("GameplayDebuggerCategoryReplicator"),		// CurrentServerWorkerId
	TEXT("PlayerController"),						// Multiple
	TEXT("Character"),								// Multiple
	TEXT("ReplicatedStartupActorPlayerController"), // Multiple
	TEXT("TestMovementCharacter"),					// Multiple
	TEXT("SpatialTestRepNotifyActor"),				// Multiple
	TEXT("SpatialTestSingleServerDynamicComponents"),
	TEXT("PlayerDisconnectController"), // Multiple
	TEXT("AlwaysInterestedTest"),		// OtherInterestedInThisReplicatedActor
	TEXT("CubeWithReferences")			// Multiple
};
const TArray<FName> USpatialNetDriverAuthorityDebugger::SuppressedProperties = {
	TEXT("Controller"),					// Multiple - BP_EventTracerCharacter_C, TestPawnBase_RepGraphAlwaysReplicate, TestPossessionPawn
	TEXT("ReplicatedWorldTimeSeconds"), // Multiple - GameStateBase, SpatialAuthorityTestGameState
	TEXT("Owner"),						// CrossServerAndClientOrchestrationFlowController
	TEXT("CurrentStepIndex"),			// Multiple
	TEXT("bActorEnableCollision"),		// SpatialWorldSettings
	TEXT("PlayerState")					// Multiple - BP_EventTracerCharacter_C, TestPawnBase_RepGraphAlwaysReplicate, DefaultPawn
};

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
