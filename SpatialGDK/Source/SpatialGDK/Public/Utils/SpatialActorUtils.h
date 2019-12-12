// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "Components/SceneComponent.h"
#include "Engine/EngineTypes.h"
#include "EngineClasses/SpatialNetConnection.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "SpatialCommonTypes.h"

class AActor;

namespace SpatialGDK
{

struct SPATIALGDK_API SpatialActorUtils
{
public:
	static const WorkerRequirementSet GetAuthoritativeWorkerRequirementSet(const AActor& Actor);
	static const WorkerRequirementSet GetAnyServerRequirementSet();
	static const WorkerRequirementSet GetAnyServerOrClientRequirementSet(const AActor& Actor);
	static const WorkerRequirementSet GetAnyServerOrOwningClientRequirementSet(const AActor& Actor);
	static const WorkerRequirementSet GetOwningClientOnlyRequirementSet(const AActor& Actor);
	static const FString GetOwnerWorkerAttribute(const AActor* Actor);

private:
	static const TSet<FName> GetServerWorkerTypes();
};

inline FVector GetActorSpatialPosition(const AActor* InActor)
{
	FVector Location = FVector::ZeroVector;

	// If the Actor is a Controller, use its Pawn's position,
	// Otherwise if the Actor has an Owner, use its position.
	// Otherwise if the Actor has a well defined location then use that
	// Otherwise use the origin
	const AController* Controller = Cast<AController>(InActor);
	if (Controller != nullptr && Controller->GetPawn() != nullptr)
	{
		USceneComponent* PawnRootComponent = Controller->GetPawn()->GetRootComponent();
		Location = PawnRootComponent ? PawnRootComponent->GetComponentLocation() : FVector::ZeroVector;
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
