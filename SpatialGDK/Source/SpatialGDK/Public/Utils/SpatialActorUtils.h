// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Algo/Copy.h"
#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialNetDriver.h"

#include "Components/SceneComponent.h"
#include "Containers/UnrealString.h"
#include "Engine/EngineTypes.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/PlayerController.h"
#include "Math/Vector.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebuggerCategoryReplicator.h"
#endif

namespace SpatialGDK
{
inline AActor* GetTopmostReplicatedOwner(const AActor* Actor)
{
	if (!ensureAlwaysMsgf(Actor != nullptr, TEXT("Called GetTopmostReplicatedOwner for nullptr Actor")))
	{
		return nullptr;
	}

	AActor* Owner = Actor->GetOwner();
	if (Owner == nullptr || Owner->IsPendingKillPending() || !Owner->GetIsReplicated())
	{
		return nullptr;
	}

	while (Owner->GetOwner() != nullptr && !Owner->GetOwner()->IsPendingKillPending() && Owner->GetIsReplicated())
	{
		Owner = Owner->GetOwner();
	}

	return Owner;
}

inline AActor* GetReplicatedHierarchyRoot(const AActor* Actor)
{
	AActor* TopmostOwner = GetTopmostReplicatedOwner(Actor);
	return TopmostOwner != nullptr ? TopmostOwner : const_cast<AActor*>(Actor);
}

// Effectively, if this Actor is in a player hierarchy, get the PlayerController entity ID.
inline Worker_PartitionId GetConnectionOwningPartitionId(const AActor* Actor)
{
	if (const USpatialNetConnection* NetConnection = Cast<USpatialNetConnection>(Actor->GetNetConnection()))
	{
		return NetConnection->GetPlayerControllerEntityId();
	}

	return SpatialConstants::INVALID_ENTITY_ID;
}

inline Worker_EntityId GetConnectionOwningClientSystemEntityId(const APlayerController* PC)
{
	const USpatialNetConnection* NetConnection = Cast<USpatialNetConnection>(PC->GetNetConnection());
	checkf(NetConnection != nullptr, TEXT("PlayerController did not have NetConnection when trying to find client system entity ID."));

	return NetConnection->ConnectionClientWorkerSystemEntityId;
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
#if WITH_GAMEPLAY_DEBUGGER
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

inline bool IsDynamicSubObject(const USpatialNetDriver& NetDriver, const AActor& Actor, const ObjectOffset SubObjectOffset)
{
	const FClassInfo& ActorClassInfo = NetDriver.ClassInfoManager->GetOrCreateClassInfoByClass(Actor.GetClass());
	return !ActorClassInfo.SubobjectInfo.Contains(SubObjectOffset);
}

using FSubobjectToOffsetMap = TMap<UObject*, ObjectOffset>;
static FSubobjectToOffsetMap CreateStaticOffsetMapFromActor(AActor& Actor, const FClassInfo& ActorInfo)
{

	FSubobjectToOffsetMap SubobjectNameToOffset;

	for (auto& SubobjectOffsetToInfoPair : ActorInfo.SubobjectInfo)
	{
		UObject* Subobject = StaticFindObjectFast(UObject::StaticClass(), &Actor, SubobjectOffsetToInfoPair.Value->SubobjectName);
		const ObjectOffset Offset = SubobjectOffsetToInfoPair.Key;

		if (Subobject != nullptr && Subobject->IsPendingKill() == false && Subobject->IsSupportedForNetworking())
		{
			SubobjectNameToOffset.Add(Subobject, Offset);
		}
	}

	return SubobjectNameToOffset;
}

static FSubobjectToOffsetMap CreateOffsetMapFromActor(USpatialPackageMapClient& PackageMap, AActor& Actor, const FClassInfo& ActorInfo)
{

	FSubobjectToOffsetMap SubobjectNameToOffset = CreateStaticOffsetMapFromActor(Actor, ActorInfo);

	if (Actor.GetInstanceComponents().Num() > 0)
	{
		// Process components attached to this object; this allows us to join up
		// server- and client-side components added in the level.
		TArray<UActorComponent*> ActorInstanceComponents;

		// In non-editor builds, editor-only components can be allocated a slot in the array, but left as nullptrs.
		Algo::CopyIf(Actor.GetInstanceComponents(), ActorInstanceComponents, [](UActorComponent* Component) -> bool {
			return IsValid(Component);
		});
		// These need to be ordered in case there are more than one component of the same type, or
		// we may end up with wrong component instances having associations between them.
		ActorInstanceComponents.Sort([](const UActorComponent& Lhs, const UActorComponent& Rhs) -> bool {
			return Lhs.GetName().Compare(Rhs.GetName()) < 0;
		});

		for (UActorComponent* DynamicComponent : ActorInstanceComponents)
		{
			if (!DynamicComponent->IsSupportedForNetworking())
			{
				continue;
			}

			const FClassInfo* DynamicComponentClassInfo = PackageMap.TryResolveNewDynamicSubobjectAndGetClassInfo(DynamicComponent);

			if (DynamicComponentClassInfo != nullptr)
			{
				SubobjectNameToOffset.Add(DynamicComponent, DynamicComponentClassInfo->SchemaComponents[SCHEMA_Data]);
			}
		}
	}

	return SubobjectNameToOffset;
}
} // namespace SpatialGDK
