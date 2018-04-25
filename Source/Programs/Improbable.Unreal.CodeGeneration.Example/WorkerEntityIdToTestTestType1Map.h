// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "improbable/collections.h"
#include "UObject/NoExportTypes.h"

#include "TestType1.h"

#include "EntityId.h"

#include "WorkerEntityIdToTestTestType1Map.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class SPATIALOS_API UWorkerEntityIdToTestTestType1Map : public UObject
{
    GENERATED_BODY()

  public:
    UWorkerEntityIdToTestTestType1Map();
    UWorkerEntityIdToTestTestType1Map* Init(const worker::Map<worker::EntityId, test::TestType1>& underlying);

    UFUNCTION(BlueprintCallable, Category = "WorkerEntityIdToTestTestType1Map")
    UWorkerEntityIdToTestTestType1Map* Emplace(FEntityId key, UTestType1* value);

    UFUNCTION(BlueprintCallable, Category = "WorkerEntityIdToTestTestType1Map")
    UWorkerEntityIdToTestTestType1Map* Remove(FEntityId key);

    UFUNCTION(BlueprintPure, Category = "WorkerEntityIdToTestTestType1Map")
    bool Contains(FEntityId key);

    UFUNCTION(BlueprintCallable, Category = "WorkerEntityIdToTestTestType1Map")
    UTestType1* Get(FEntityId key);

    UFUNCTION(BlueprintCallable, Category = "WorkerEntityIdToTestTestType1Map")
    UWorkerEntityIdToTestTestType1Map* Clear();

    UFUNCTION(BlueprintPure, Category = "WorkerEntityIdToTestTestType1Map")
    bool IsEmpty();

    worker::Map<worker::EntityId, test::TestType1> GetUnderlying();

	bool operator==(const worker::Map<worker::EntityId, test::TestType1>& OtherUnderlying) const;
	bool operator!=(const worker::Map<worker::EntityId, test::TestType1>& OtherUnderlying) const;

  private:
    worker::Map<worker::EntityId, test::TestType1> Underlying;
};
