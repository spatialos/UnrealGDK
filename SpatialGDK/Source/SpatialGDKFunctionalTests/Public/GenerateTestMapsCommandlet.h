// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#if WITH_EDITOR

#include "Commandlets/Commandlet.h"
#include "CoreMinimal.h"
#include "GenerateTestMapsCommandlet.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGenerateTestMapsCommandlet, Log, All);

/**
 * Generate test maps, pls.
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UGenerateTestMapsCommandlet : public UCommandlet
{
	GENERATED_BODY()

	UGenerateTestMapsCommandlet();

public:
	virtual int32 Main(const FString& CmdLineParams) override;
};
#endif // WITH_EDITOR
