// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "Schema/Component.h"
#include "Schema/StandardLibrary.h"
#include "Schema/UnrealMetadata.h"
#include "SpatialConstants.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialDispatcher.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialView, Log, All);

class USpatialNetDriver;
class USpatialReceiver;
class USpatialStaticComponentView;

UCLASS()
class SPATIALGDK_API USpatialDispatcher : public UObject
{
	GENERATED_BODY()

public:
	void Init(USpatialNetDriver* NetDriver);
	void ProcessOps(Worker_OpList* OpList);

private:
	UPROPERTY()
	USpatialNetDriver* NetDriver;

	UPROPERTY()
	USpatialReceiver* Receiver;

	UPROPERTY()
	USpatialStaticComponentView* StaticComponentView;
};
