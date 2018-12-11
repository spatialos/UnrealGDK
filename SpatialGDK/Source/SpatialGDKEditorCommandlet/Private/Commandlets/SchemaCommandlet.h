// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Commandlets/Commandlet.h"

#include "SchemaCommandlet.generated.h"

UCLASS()
class USchemaCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	USchemaCommandlet();

public:
	virtual int32 Main(const FString& Params) override;
};
