// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Utils/SchemaUtils.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "EntityPool.generated.h"

struct EntityRange
{
	Worker_EntityId CurrentEntityId;
	Worker_EntityId LastEntityId;
	bool bExpired;
	uint32 EntityRangeId; // Used to identify an entity range when it has expired.
};

class USpatialReceiver;
class FTimerManager;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialEntityPool, Log, All)

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEntityPoolReadyEvent);

UCLASS()
class SPATIALGDK_API UEntityPool : public UObject
{
	GENERATED_BODY()

public:
	void Init(USpatialNetDriver* InNetDriver, FTimerManager* TimerManager);
	void ReserveEntityIDs(int32 EntitiesToReserve);
	Worker_EntityId GetNextEntityId();
	FEntityPoolReadyEvent& GetEntityPoolReadyDelegate();

	FORCEINLINE bool IsReady() const { return bIsReady; }

private:
	void OnEntityRangeExpired(uint32 ExpiringEntityRangeId);

	UPROPERTY()
	USpatialNetDriver* NetDriver;

	UPROPERTY()
	USpatialReceiver* Receiver;

	FTimerManager* TimerManager;
	TArray<EntityRange> ReservedEntityIDRanges;

	bool bIsReady;
	bool bIsAwaitingResponse;

	uint32 NextEntityRangeId;

	FEntityPoolReadyEvent EntityPoolReadyDelegate;
};
