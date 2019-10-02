// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/CookCommandlet.h"
#include "CookAndGenerateSchemaCommandlet.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCookAndGenerateSchemaCommandlet, Log, All);

struct FObjectListener;
/**
 * This Commandlet generates schema and performs a cook command.
 * It supports the same set of arguments as cook. It will only generate
 * schema for blueprints required by the cook.
 */
UCLASS()
class SPATIALGDKEDITORCOMMANDLET_API UCookAndGenerateSchemaCommandlet : public UCookCommandlet
{
	GENERATED_BODY()

	UCookAndGenerateSchemaCommandlet();

public:
	virtual int32 Main(const FString& CmdLineParams) override;
};
