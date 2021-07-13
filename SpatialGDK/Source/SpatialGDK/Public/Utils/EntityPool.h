// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Utils/SchemaUtils.h"

#include "Interop/ReserveEntityIdsHandler.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "EntityPool.generated.h"

struct EntityRange
{
	Worker_EntityId CurrentEntityId;
	Worker_EntityId LastEntityId;
};

class USpatialReceiver;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialEntityPool, Log, All)

DECLARE_MULTICAST_DELEGATE(FEntityPoolReadyEvent);

UCLASS()
class SPATIALGDK_API UEntityPool : public UObject
{
	GENERATED_BODY()

public:
	void Init(USpatialNetDriver& InNetDriver);
	void ReserveEntityIDs(uint32 EntitiesToReserve);
	Worker_EntityId GetNextEntityId();
	FEntityPoolReadyEvent& GetEntityPoolReadyDelegate();

	FORCEINLINE bool IsReady() const { return bIsReady; }

	void Advance();

private:
	UPROPERTY()
	USpatialNetDriver* NetDriver;

	TArray<EntityRange> ReservedEntityIDRanges;

	bool bIsReady = false;
	bool bIsAwaitingResponse = false;

	FEntityPoolReadyEvent EntityPoolReadyDelegate;

	SpatialGDK::FReserveEntityIdsHandler ReserveEntityIdsHandler;
};
