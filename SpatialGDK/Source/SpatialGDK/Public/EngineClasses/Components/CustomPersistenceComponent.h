// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "CustomPersistenceComponent.generated.h"

/*
 Offers a configurable means of persisting variables.
 This component should be attached to an actor.
 */
UCLASS(ClassGroup = (SpatialGDK), Meta = (BlueprintSpawnableComponent))
class SPATIALGDK_API UCustomPersistenceComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()
};
