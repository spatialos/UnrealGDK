// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "improbable/c_worker.h"
#include "improbable/c_schema.h"

#include "DTBManager.generated.h"

UCLASS()
class SPATIALGDK_API UDTBManager : public UObject
{
	GENERATED_BODY()

public:
	UDTBManager();

	void InitClient();
	void InitServer();

	void Tick();

	TFunction<void()> OnSpawnRequest;

	Worker_Connection* Connection;
};
