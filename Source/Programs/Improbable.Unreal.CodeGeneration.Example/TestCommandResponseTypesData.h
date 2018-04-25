// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TestSchema.h"

#include "TestCommandResponseTypesData.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class SPATIALOS_API UTestCommandResponseTypesData : public UObject
{
    GENERATED_BODY()

  public:
    UTestCommandResponseTypesData();
    UTestCommandResponseTypesData* Init(const test::TestCommandResponseTypesData& underlying);


	test::TestCommandResponseTypesData GetUnderlying();

  private:
    TUniquePtr<test::TestCommandResponseTypesData> Underlying;
};

