// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Commandlets/CookCommandlet.h"
#include "CoreMinimal.h"
#include "CookAndGenerateSchemaCommandlet.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCookAndGenerateSchemaCommandlet, Log, All);

struct FObjectListener;
/**
 * -- EXPERIMENTAL --
 * This Commandlet generates schema and performs a cook command.
 * It supports the same set of arguments as cook. It will only generate
 * schema for blueprints required by the cook.
 *
 * Usage:
 * Engine\Binaries\Win64\UE4Editor-Cmd.exe <PathToGame>.uproject -run=CookAndGenerateSchema -targetplatform=LinuxServer -SkipShaderCompile
 * <...Native Cook Params>
 *
 * Known Issues:
 * - SchemaDatabase.uasset will need to be cooked again after running this Commandlet, potentially with [-iterate] flag.
 * - [-iterate] flag will result in schema only generated for dirty packages, you maintain previous .schema files if you want to use this
 * flag.
 *
 * Recommended Workflow:
 *  1. Run CookAndGenerateSchema for a LinuxServer platform with [-SkipShaderCompile] for needed maps WITHOUT [-iterate]
 *  2. Run GenerateSchemaAndSnapshots with [-SkipSchema] for needed maps.
 *  2. BuildCookRun for the same platform WITH [-iterate]
 *  3. BuildCookRun other platforms
 */
UCLASS()
class SPATIALGDKEDITORCOMMANDLET_API UCookAndGenerateSchemaCommandlet : public UCookCommandlet
{
	GENERATED_BODY()

	UCookAndGenerateSchemaCommandlet();

public:
	virtual int32 Main(const FString& CmdLineParams) override;
};
