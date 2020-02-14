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

// We need the optional IgnoredActor argument because Actor deletions are broadcast before
// SetOwner is called and ownership hierarchies are updated.
inline TArray<AActor*> GetOwnershipHierarchyPath(const AActor* Actor, const AActor* IgnoredActor = nullptr)
{
	check(Actor != nullptr);

	AActor* OwnerIterator = Actor->GetOwner();
	if (OwnerIterator == nullptr)
	{
		return TArray<AActor*>();
	}

	TArray<AActor*> OwnershipHierarchy {OwnerIterator};

	while (OwnerIterator->GetOwner() != nullptr && OwnerIterator->GetOwner() != IgnoredActor)
	{
		OwnerIterator = OwnerIterator->GetOwner();
		OwnershipHierarchy.Insert(OwnerIterator, 0);
	}

	return OwnershipHierarchy;
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
