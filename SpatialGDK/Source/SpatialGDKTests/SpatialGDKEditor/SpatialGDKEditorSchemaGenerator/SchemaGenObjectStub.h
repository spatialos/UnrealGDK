// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Utils/RPCContainer.h"

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

UCLASS(NotSpatialType)
class UNotSpatialTypeObjectStub : public UObject
{
	GENERATED_BODY()
};
