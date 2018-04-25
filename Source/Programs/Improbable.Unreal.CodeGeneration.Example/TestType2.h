// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TestSchema.h"

#include "TestType2.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class SPATIALOS_API UTestType2 : public UObject
{
    GENERATED_BODY()

  public:
    UTestType2();
    UTestType2* Init(const test::TestType2& underlying);

    UFUNCTION(BlueprintPure, Category = "TestType2")
    float GetDoubleProperty();
	UFUNCTION(BlueprintCallable, Category = "TestType2")
    UTestType2* SetDoubleProperty(float double_property);


	test::TestType2 GetUnderlying();

  private:
    TUniquePtr<test::TestType2> Underlying;
};

