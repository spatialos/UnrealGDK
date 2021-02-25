// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "EntityFactoryTestStub.generated.h"

UCLASS(SpatialType, notplaceable, NotBlueprintable)
class APersistentByDefaultSpatialActor : public AActor
{
	GENERATED_BODY()
};

UCLASS(SpatialType = Persistent, notplaceable, NotBlueprintable)
class AExplicitPersistentSpatialActor : public AActor
{
	GENERATED_BODY()
};

UCLASS(SpatialType = NotPersistent, notplaceable, NotBlueprintable)
class ANonPersistentSpatialActor : public AActor
{
	GENERATED_BODY()
};

UCLASS(notplaceable, NotBlueprintable)
class AActorSubclassOfPersistentByDefaultSpatialActor : public APersistentByDefaultSpatialActor
{
	GENERATED_BODY()
};

UCLASS(notplaceable, NotBlueprintable)
class AActorSubclassOfExplicitPersistentSpatialActor : public AExplicitPersistentSpatialActor
{
	GENERATED_BODY()
};

UCLASS(notplaceable, NotBlueprintable)
class AActorSubclassOfNonPersistentSpatialActor : public ANonPersistentSpatialActor
{
	GENERATED_BODY()
};

UCLASS(SpatialType = NotPersistent, notplaceable, NotBlueprintable)
class ANonPersistentActorSubclassOfPersistentByDefaultSpatialActor : public APersistentByDefaultSpatialActor
{
	GENERATED_BODY()
};

UCLASS(SpatialType = NotPersistent, notplaceable, NotBlueprintable)
class ANonPersistentActorSubclassOfExpicitPersistentSpatialActor : public AExplicitPersistentSpatialActor
{
	GENERATED_BODY()
};

UCLASS(SpatialType = Persistent, notplaceable, NotBlueprintable)
class APersistentActorSubclassOfNonPersistentSpatialActor : public ANonPersistentSpatialActor
{
	GENERATED_BODY()
};

UCLASS(SpatialType = Persistent, notplaceable, NotBlueprintable)
class APersistentActorSubclassOfActorSubclassOfNonPersistentSpatialActor : public AActorSubclassOfNonPersistentSpatialActor
{
	GENERATED_BODY()
};

UCLASS(SpatialType = NotPersistent, notplaceable, NotBlueprintable)
class ANonPersistentActorSubclassOfActorSubclassOfPersistentByDefaultSpatialActor : public AActorSubclassOfPersistentByDefaultSpatialActor
{
	GENERATED_BODY()
};

UCLASS(SpatialType = NotPersistent, notplaceable, NotBlueprintable)
class ANonPersistentActorSubclassOfActorSubclassOfExplicitPersistentSpatialActor : public AActorSubclassOfExplicitPersistentSpatialActor
{
	GENERATED_BODY()
};
