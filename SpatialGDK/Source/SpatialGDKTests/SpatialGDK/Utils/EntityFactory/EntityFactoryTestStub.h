// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "EntityFactoryTestStub.generated.h"

UCLASS(SpatialType)
class APersistentByDefaultSpatialActor : public AActor
{
	GENERATED_BODY()
};

UCLASS(SpatialType = Persistent)
class AExplicitPersistentSpatialActor : public AActor
{
	GENERATED_BODY()
};

UCLASS(SpatialType = NotPersistent)
class ANonPersistentSpatialActor : public AActor
{
	GENERATED_BODY()
};

UCLASS()
class AActorSubclassOfPersistentByDefaultSpatialActor : public APersistentByDefaultSpatialActor
{
	GENERATED_BODY()
};

UCLASS()
class AActorSubclassOfExplicitPersistentSpatialActor : public AExplicitPersistentSpatialActor
{
	GENERATED_BODY()
};

UCLASS()
class AActorSubclassOfNonPersistentSpatialActor : public ANonPersistentSpatialActor
{
	GENERATED_BODY()
};

UCLASS(SpatialType = NotPersistent)
class ANonPersistentActorSubclassOfPersistentByDefaultSpatialActor : public APersistentByDefaultSpatialActor
{
	GENERATED_BODY()
};

UCLASS(SpatialType = NotPersistent)
class ANonPersistentActorSubclassOfExpicitPersistentSpatialActor : public AExplicitPersistentSpatialActor
{
	GENERATED_BODY()
};

UCLASS(SpatialType = Persistent)
class APersistentActorSubclassOfNonPersistentSpatialActor : public ANonPersistentSpatialActor
{
	GENERATED_BODY()
};

UCLASS(SpatialType = Persistent)
class APersistentActorSubclassOfActorSubclassOfNonPersistentSpatialActor : public AActorSubclassOfNonPersistentSpatialActor
{
	GENERATED_BODY()
};

UCLASS(SpatialType = NotPersistent)
class ANonPersistentActorSubclassOfActorSubclassOfPersistentByDefaultSpatialActor : public AActorSubclassOfPersistentByDefaultSpatialActor
{
	GENERATED_BODY()
};

UCLASS(SpatialType = NotPersistent)
class ANonPersistentActorSubclassOfActorSubclassOfExplicitPersistentSpatialActor : public AActorSubclassOfExplicitPersistentSpatialActor
{
	GENERATED_BODY()
};
