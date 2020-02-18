// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineClasses/SpatialNetConnection.h"

#include "Components/SceneComponent.h"
#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "Math/Vector.h"

namespace SpatialGDK
{

// We need the optional ActorToStopIteratingAt argument because for ownership-based Actor sets we want to find a hierarchy
// assuming the future deletion of a given Actor. This is because Actor deletions are broadcast before SetOwner is called
// which then updates the ownership hierarchy.
inline AActor* GetHierarchyRoot(const AActor* Actor, const AActor* ActorToStopIteratingAt = nullptr)
{
	check(Actor != nullptr);

	AActor* Owner = Actor->GetOwner();
	if (Owner == ActorToStopIteratingAt)
	{
		return nullptr;
	}

	while (Owner->GetOwner() != nullptr && Owner->GetOwner() != ActorToStopIteratingAt)
	{
		Owner = Owner->GetOwner();
	}

	return Owner;
}

inline FString GetOwnerWorkerAttribute(AActor* Actor)
{
	if (const USpatialNetConnection* NetConnection = Cast<USpatialNetConnection>(Actor->GetNetConnection()))
	{
		return NetConnection->WorkerAttribute;
	}

	return FString();
}

inline FVector GetActorSpatialPosition(const AActor* InActor)
{
	FVector Location = FVector::ZeroVector;

	// If the Actor is a Controller, use its Pawn's position,
	// Otherwise if the Actor is a PlayerController, use its last spectator sync location, otherwise its focal point
	// Otherwise if the Actor has an Owner, use its position.
	// Otherwise if the Actor has a well defined location then use that
	// Otherwise use the origin
	const AController* Controller = Cast<AController>(InActor);
	if (Controller != nullptr && Controller->GetPawn() != nullptr)
	{
		USceneComponent* PawnRootComponent = Controller->GetPawn()->GetRootComponent();
		Location = PawnRootComponent ? PawnRootComponent->GetComponentLocation() : FVector::ZeroVector;
	}
	else if (const APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (PlayerController->IsInState(NAME_Spectating))
		{
			Location = PlayerController->LastSpectatorSyncLocation;
		}
		else
		{
			Location = PlayerController->GetFocalLocation();
		}
	}
	else if (InActor->GetOwner() != nullptr && InActor->GetOwner()->GetIsReplicated())
	{
		return GetActorSpatialPosition(InActor->GetOwner());
	}
	else if (USceneComponent* RootComponent = InActor->GetRootComponent())
	{
		Location = RootComponent->GetComponentLocation();
	}

	// Rebase location onto zero origin so actor is positioned correctly in SpatialOS.
	return FRepMovement::RebaseOntoZeroOrigin(Location, InActor);
}

} // namespace SpatialGDK
