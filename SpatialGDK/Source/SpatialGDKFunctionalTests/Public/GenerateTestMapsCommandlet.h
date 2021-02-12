// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Commandlets/Commandlet.h"
#include "CoreMinimal.h"
#include "GenerateTestMapsCommandlet.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGenerateTestMapsCommandlet, Log, All);

/**
 * This commandlet is used to generate maps used for testing automatically.
 * Highly specific for internal GDK-usage.
 * See the GeneratedTestMap class if you want to define your own map to be used in testing.
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UGenerateTestMapsCommandlet : public UCommandlet
{
	GENERATED_BODY()

	UGenerateTestMapsCommandlet();

public:
	virtual int32 Main(const FString& CmdLineParams) override;
};
