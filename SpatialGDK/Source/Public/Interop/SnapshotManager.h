// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "Utils/SchemaUtils.h"

#include "EngineClasses/SpatialNetDriver.h" // TODO: Remove this.

#include <improbable/c_schema.h>
#include <improbable/c_worker.h>

#include "SnapshotManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSnapshotManager, Log, All)

UCLASS()
class SPATIALGDK_API USnapshotManager : public UObject
{
	GENERATED_BODY()

public:
	void Init(USpatialNetDriver* InNetDriver, UGlobalStateManager* InGlobalStateManager);

	void WorldWipe(const USpatialNetDriver::ServerTravelDelegate& Delegate);
	void DeleteEntities(const Worker_EntityQueryResponseOp& Op);
	void LoadSnapshot(FString SnapshotName);

private:
	UPROPERTY()
	USpatialNetDriver* NetDriver;

	UPROPERTY()
	UGlobalStateManager* GlobalStateManager;

	UPROPERTY()
	USpatialReceiver* Receiver;
};
