// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/LightweightActorSystem.h"

#include "SpatialView/SubView.h"
#include "../../Public/LightweightActor.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"

DEFINE_LOG_CATEGORY(LogLightweightActorSystem);

DECLARE_CYCLE_STAT(TEXT("Lightweight Actor System ReceiveActor"), STAT_LightweightActorSystemReceiveActor, STATGROUP_SpatialNet);

namespace SpatialGDK
{
LightweightActorSystem::LightweightActorSystem(const FSubView& InLightweightActorSubView, USpatialNetDriver* InNetDriver)
	: LightweightActorSubView(&InLightweightActorSubView)
	, NetDriver(InNetDriver)
{
}

void LightweightActorSystem::PopulateDataStore(Worker_EntityId EntityId)
{
	ActorData& Components = LightweightActorDataStore.Emplace(EntityId, ActorData{});
	for (const ComponentData& Data : LightweightActorSubView->GetView()[EntityId].Components)
	{
		switch (Data.GetComponentId())
		{
		case SpatialConstants::SPAWN_DATA_COMPONENT_ID:
			Components.Spawn = SpawnData(Data.GetUnderlying());
			break;
		case SpatialConstants::UNREAL_METADATA_COMPONENT_ID:
			Components.Metadata = UnrealMetadata(Data.GetUnderlying());
			break;
		default:
			break;
		}
	}
}

void LightweightActorSystem::EntityAdded(Worker_EntityId EntityId)
{
	SCOPE_CYCLE_COUNTER(STAT_LightweightActorSystemReceiveActor);

	// TODO: Startup actors? Unique actors? Dormancy?
	ActorData& Components = LightweightActorDataStore[EntityId];
	UClass* ActorClass = ALightweightActor::StaticClass();

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.bRemoteOwned = true;
	SpawnInfo.bNoFail = true;

	FVector SpawnLocation = FRepMovement::RebaseOntoLocalOrigin(Components.Spawn.Location, NetDriver->GetWorld()->OriginLocation);

	AActor* NewActor =
		NetDriver->GetWorld()->SpawnActorAbsolute(ActorClass, FTransform(Components.Spawn.Rotation, SpawnLocation), SpawnInfo);
	check(NewActor);

	// Imitate the behavior in UPackageMapClient::SerializeNewActor.
	const float Epsilon = 0.001f;
	if (Components.Spawn.Velocity.Equals(FVector::ZeroVector, Epsilon))
	{
		NewActor->PostNetReceiveVelocity(Components.Spawn.Velocity);
	}
	if (!Components.Spawn.Scale.Equals(FVector::OneVector, Epsilon))
	{
		NewActor->SetActorScale3D(Components.Spawn.Scale);
	}

	// Don't have authority over Actor until SpatialOS delegates authority
	NewActor->Role = ROLE_SimulatedProxy;
	NewActor->RemoteRole = ROLE_Authority;



}

} // namespace SpatialGDK
