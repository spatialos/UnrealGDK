// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "CoreMinimal.h"

#include "SchemaGenObjectStub.generated.h"

UCLASS()
class USchemaGenObjectStub : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(Replicated)
	int IntValue;

	UPROPERTY(Replicated)
	bool BoolValue;
};

UCLASS(SpatialType)
class USpatialTypeObjectStub : public UObject
{
	GENERATED_BODY()
};

UCLASS()
class UChildOfSpatialTypeObjectStub : public USpatialTypeObjectStub
{
	GENERATED_BODY()
};

UCLASS(NotSpatialType)
class UNotSpatialTypeObjectStub : public UObject
{
	GENERATED_BODY()
};

UCLASS()
class UChildOfNotSpatialTypeObjectStub : public UNotSpatialTypeObjectStub
{
	GENERATED_BODY()
};

UCLASS()
class UNoSpatialFlagsObjectStub : public UObject
{
	GENERATED_BODY()
};

UCLASS()
class UChildOfNoSpatialFlagsObjectStub : public UNoSpatialFlagsObjectStub
{
	GENERATED_BODY()
};

UCLASS(SpatialType)
class ASpatialTypeActor : public AActor
{
	GENERATED_BODY()
};

UCLASS()
class ANonSpatialTypeActor : public AActor
{
	GENERATED_BODY()
};

UCLASS(SpatialType)
class USpatialTypeActorComponent : public UActorComponent
{
	GENERATED_BODY()
};

UCLASS(SpatialType)
class ASpatialTypeActorWithActorComponent : public AActor
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(Replicated)
	USpatialTypeActorComponent* SpatialActorComponent;
};

UCLASS(SpatialType)
class ASpatialTypeActorWithMultipleActorComponents : public AActor
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(Replicated)
	USpatialTypeActorComponent* FirstSpatialActorComponent;

	UPROPERTY(Replicated)
	USpatialTypeActorComponent* SecondSpatialActorComponent;
};

UCLASS(SpatialType)
class ASpatialTypeActorWithMultipleObjectComponents : public AActor
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(Replicated)
	USpatialTypeObjectStub* FirstSpatialObjectComponent;

	UPROPERTY(Replicated)
	USpatialTypeObjectStub* SecondSpatialObjectComponent;
};

UCLASS(SpatialType)
class ASpatialTypeActorWithSubobject : public AActor
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(Replicated)
	USpatialTypeObjectStub* SpatialActorSubobject;
};
