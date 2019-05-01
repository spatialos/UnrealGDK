// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{

struct SpawnData : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::SPAWN_DATA_COMPONENT_ID;

	SpawnData() = default;

	SpawnData(AActor* Actor)
	{
		const USceneComponent* RootComponent = Actor->GetRootComponent();

		Location = RootComponent ? FRepMovement::RebaseOntoZeroOrigin(Actor->GetActorLocation(), Actor) : FVector::ZeroVector;
		Rotation = RootComponent ? Actor->GetActorRotation() : FRotator::ZeroRotator;
		Scale = RootComponent ? Actor->GetActorScale() : FVector::OneVector;
		Velocity = RootComponent ? Actor->GetVelocity() : FVector::ZeroVector;
	}

	SpawnData(const Worker_ComponentData& Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		Location = GetVectorFromSchema(ComponentObject, 1);
		Rotation = GetRotatorFromSchema(ComponentObject, 2);
		Scale = GetVectorFromSchema(ComponentObject, 3);
		Velocity = GetVectorFromSchema(ComponentObject, 4);
	}

	Worker_ComponentData CreateSpawnDataData()
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData(ComponentId);
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		AddVectorToSchema(ComponentObject, 1, Location);
		AddRotatorToSchema(ComponentObject, 2, Rotation);
		AddVectorToSchema(ComponentObject, 3, Scale);
		AddVectorToSchema(ComponentObject, 4, Velocity);

		return Data;
	}

	FVector Location;
	FRotator Rotation;
	FVector Scale;
	FVector Velocity;
};

} // namespace SpatialGDK
