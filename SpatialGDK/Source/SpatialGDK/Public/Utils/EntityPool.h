#pragma once

#include "CoreMinimal.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Utils/SchemaUtils.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "EntityPool.generated.h"

struct EntityRange {
	Worker_EntityId CurrentEntityId;
	Worker_EntityId LastEntityId;
};

class USpatialReceiver;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialEntityPool, Log, All)

UCLASS()
class SPATIALGDK_API UEntityPool : public UObject
{
	GENERATED_BODY()

public:
	void Init(USpatialNetDriver* InNetDriver);
	void ReserveEntityIDs(int32 EntitiesToReserve);
	Worker_EntityId Pop();
	bool IsReady();

private:
	UPROPERTY()
	USpatialNetDriver* NetDriver;

	UPROPERTY()
	USpatialReceiver* Receiver;

	TArray<EntityRange> ReservedIDs;

	bool bIsReady;
	bool bIsAwaitingResponse;
};
