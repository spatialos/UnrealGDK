#pragma once

#include "CommanderTypes.h"
#include "EntityId.h"
#include "RequestId.h"
#include "SpatialOSWorkerTypes.h"
#include "UObject/NoExportTypes.h"
#include "EntityQueryCommandResult.generated.h"

/**
*
*/
UCLASS(abstract, BlueprintType)
class SPATIALGDK_API UEntityQueryCommandResultBase : public UObject
{
  GENERATED_BODY()

public:
  UEntityQueryCommandResultBase();

  UFUNCTION(BlueprintPure, Category = "SpatialOS Query Result")
  bool Success() const;

  UFUNCTION(BlueprintPure, Category = "SpatialOS Query Result")
  FString GetErrorMessage() const;

  UFUNCTION(BlueprintPure, Category = "SpatialOS Query Result")
  ECommandResponseCode GetErrorCode() const;

  UFUNCTION(BlueprintPure, Category = "SpatialOS Query Result")
  FRequestId GetRequestId() const;

protected:
  worker::EntityQueryResponseOp Underlying;
  FRequestId CachedRequestId;
};

/**
*
*/
UCLASS(BlueprintType)
class SPATIALGDK_API UEntityQueryCountCommandResult : public UEntityQueryCommandResultBase
{
  GENERATED_BODY()

public:
  UEntityQueryCountCommandResult();
  UEntityQueryCommandResultBase* Init(const worker::EntityQueryResponseOp& underlying);

  UFUNCTION(BlueprintPure, Category = "SpatialOS Query Result")
  int GetCount() const;
};

/**
*
*/
UCLASS(BlueprintType)
class SPATIALGDK_API UEntityQuerySnapshotCommandResult : public UEntityQueryCommandResultBase
{
  GENERATED_BODY()

public:
  UEntityQuerySnapshotCommandResult();
  UEntityQueryCommandResultBase* Init(const worker::EntityQueryResponseOp& underlying);

  UFUNCTION(BlueprintPure, Category = "SpatialOS Query Result")
  int GetSnapshotCount() const;

  UFUNCTION(BlueprintPure, Category = "SpatialOS Query Result")
  TArray<FEntityId> GetEntityIDs() const;
};
