// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineClasses/SpatialNetConnection.h"

#include "Components/SceneComponent.h"
#include "Containers/UnrealString.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/PlayerController.h"
#include "Math/Vector.h"

#if WITH_UNREAL_DEVELOPER_TOOLS || (!UE_BUILD_SHIPPING && !UE_BUILD_TEST)
#include "GameplayDebuggerCategoryReplicator.h"
#endif

namespace SpatialGDK
{
inline AActor* GetOutermostReplicatedOwner(const AActor* Actor)
{
	check(Actor != nullptr);

	AActor* Itr = const_cast<AActor*>(Actor);

	AActor* Owner = Actor->GetOwner();
	while (Owner != nullptr && !Owner->IsPendingKillPending() && Owner->GetIsReplicated())
	{
		Itr = Owner;
		Owner = Owner->GetOwner();
	}

	return Itr == Actor ? nullptr : Itr;
}

inline AActor* GetReplicatedHierarchyRoot(const AActor* Actor)
{
	AActor* TopmostOwner = GetOutermostReplicatedOwner(Actor);
	return TopmostOwner != nullptr ? TopmostOwner : const_cast<AActor*>(Actor);
}

inline FString GetConnectionOwningWorkerId(const AActor* Actor)
{
	if (const USpatialNetConnection* NetConnection = Cast<USpatialNetConnection>(Actor->GetNetConnection()))
	{
		return NetConnection->ConnectionOwningWorkerId;
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

inline bool DoesActorClassIgnoreVisibilityCheck(AActor* InActor)
{
	if (InActor->IsA(APlayerController::StaticClass()) || InActor->IsA(AGameModeBase::StaticClass())
#if WITH_UNREAL_DEVELOPER_TOOLS || (!UE_BUILD_SHIPPING && !UE_BUILD_TEST)
		|| InActor->IsA(AGameplayDebuggerCategoryReplicator::StaticClass())
#endif
	)

	{
		return true;
	}

	return false;
}

inline bool ShouldActorHaveVisibleComponent(AActor* InActor)
{
	if (InActor->bAlwaysRelevant || !InActor->IsHidden() || DoesActorClassIgnoreVisibilityCheck(InActor))
	{
		return true;
	}

	return false;
}

} // namespace SpatialGDK
