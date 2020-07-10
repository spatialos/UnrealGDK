// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Commandlets/CompileAllBlueprintsCommandlet.h"
#include "GenerateSchemaCommandlet.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGenerateSchemaCommandlet, Log, All);

struct FObjectListener;
/**
 * -- EXPERIMENTAL --
 * - Use CompileAllBlueprintsCommandlet to find all blueprints, skipping compile though.
 * - Use ObjectArrayListener to detect supported blueprints.
 * - Use FLinkerLoad to try and load level blueprints from inside maps without loading them...
 */
UCLASS()
class UGenerateSchemaCommandlet : public UCompileAllBlueprintsCommandlet
{
	GENERATED_BODY()

private:
	void InitCmdLine(const FString& Params);

	UPROPERTY()
	TArray<FString> MapPackagePaths;

public:
	UGenerateSchemaCommandlet();

	virtual int32 Main(const FString& CmdLineParams) override;

protected:
	virtual void CompileBlueprint(UBlueprint* Blueprint) override;

	void LoadLevelBlueprints();
};
