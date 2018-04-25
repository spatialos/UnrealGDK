// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TestSchema.h"

#include "TestEmptyData.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class SPATIALOS_API UTestEmptyData : public UObject
{
    GENERATED_BODY()

  public:
    UTestEmptyData();
    UTestEmptyData* Init(const test::TestEmptyData& underlying);


	test::TestEmptyData GetUnderlying();

  private:
    TUniquePtr<test::TestEmptyData> Underlying;
};

