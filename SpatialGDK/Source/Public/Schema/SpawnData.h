// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace improbable
{

struct SpawnData : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::SPAWN_DATA_COMPONENT_ID;

	SpawnData() = default;

	SpawnData(AActor* Actor)
	{
		const USceneComponent* RootComponent = Actor->GetRootComponent();

		Rotation = RootComponent ? Actor->GetActorRotation() : FRotator::ZeroRotator;
		Scale = RootComponent ? Actor->GetActorScale() : FVector::OneVector;
		Velocity = RootComponent ? Actor->GetVelocity() : FVector::ZeroVector;
	}

	SpawnData(const Worker_ComponentData& Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		Rotation = GetRotatorFromSchema(ComponentObject, 1);
		Scale = GetVectorFromSchema(ComponentObject, 2);
		Velocity = GetVectorFromSchema(ComponentObject, 3);
	}

	Worker_ComponentData CreateSpawnDataData()
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData(ComponentId);
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		AddRotatorToSchema(ComponentObject, 1, Rotation);
		AddVectorToSchema(ComponentObject, 2, Scale);
		AddVectorToSchema(ComponentObject, 3, Velocity);

		return Data;
	}

	FRotator Rotation;
	FVector Scale;
	FVector Velocity;
};

}
