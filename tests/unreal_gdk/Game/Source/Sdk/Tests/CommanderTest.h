// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Tests/SpatialOSTest.h"
#include "CommanderTest.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSdkTest, Log, All);

UCLASS()
class SDK_API UCommanderTest : public USpatialOSTest
{
	GENERATED_BODY()
public:
	UFUNCTION()
	void TestSendReserveEntityIdCommand() const;

	UFUNCTION()
	void TestSendRequestTestdata1() const;
};
