// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TestSchema.h"

#include "ComponentWithSimilarlyNamedPropertyAndEventData.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class SPATIALOS_API UComponentWithSimilarlyNamedPropertyAndEventData : public UObject
{
    GENERATED_BODY()

  public:
    UComponentWithSimilarlyNamedPropertyAndEventData();
    UComponentWithSimilarlyNamedPropertyAndEventData* Init(const test::ComponentWithSimilarlyNamedPropertyAndEventData& underlying);

    UFUNCTION(BlueprintPure, Category = "ComponentWithSimilarlyNamedPropertyAndEventData")
    int GetMyValue();
	UFUNCTION(BlueprintCallable, Category = "ComponentWithSimilarlyNamedPropertyAndEventData")
    UComponentWithSimilarlyNamedPropertyAndEventData* SetMyValue(int my_value);


	test::ComponentWithSimilarlyNamedPropertyAndEventData GetUnderlying();

  private:
    TUniquePtr<test::ComponentWithSimilarlyNamedPropertyAndEventData> Underlying;
};

