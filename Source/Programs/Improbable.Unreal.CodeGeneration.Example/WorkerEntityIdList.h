// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "improbable/collections.h"
#include "UObject/NoExportTypes.h"

#include "EntityId.h"

#include "WorkerEntityIdList.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class SPATIALOS_API UWorkerEntityIdList : public UObject
{
    GENERATED_BODY()

  public:
    UWorkerEntityIdList();
    UWorkerEntityIdList* Init(const worker::List<worker::EntityId>& underlying);

    UFUNCTION(BlueprintCallable, Category = "WorkerEntityIdList")
    UWorkerEntityIdList* Add(FEntityId value);

    UFUNCTION(BlueprintPure, Category = "WorkerEntityIdList")
    FEntityId Get(int pos);

    UFUNCTION(BlueprintCallable, Category = "WorkerEntityIdList")
    UWorkerEntityIdList* Clear();

    UFUNCTION(BlueprintPure, Category = "WorkerEntityIdList")
    bool IsEmpty();

	UFUNCTION(BlueprintPure, Category = "WorkerEntityIdList")
    int Size();

	bool operator==(const worker::List<worker::EntityId>& OtherUnderlying) const;
	bool operator!=(const worker::List<worker::EntityId>& OtherUnderlying) const;	

    worker::List<worker::EntityId> GetUnderlying();

  private:
    worker::List<worker::EntityId> Underlying;
};
