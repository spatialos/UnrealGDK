// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TestSchema.h"

#include <string>
#include "improbable/standard_library.h"
#include "SpatialOSConversionFunctionLibrary.h"
#include "TestType1.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class SPATIALOS_API UTestType1 : public UObject
{
    GENERATED_BODY()

  public:
    UTestType1();
    UTestType1* Init(const test::TestType1& underlying);

    UFUNCTION(BlueprintPure, Category = "TestType1")
    FString GetStringProperty();
	UFUNCTION(BlueprintCallable, Category = "TestType1")
    UTestType1* SetStringProperty(FString string_property);

    UFUNCTION(BlueprintPure, Category = "TestType1")
    int GetInt32Property();
	UFUNCTION(BlueprintCallable, Category = "TestType1")
    UTestType1* SetInt32Property(int int32_property);

    UFUNCTION(BlueprintPure, Category = "TestType1")
    FVector GetCoordinatesProperty();
	UFUNCTION(BlueprintCallable, Category = "TestType1")
    UTestType1* SetCoordinatesProperty(FVector coordinates_property);


	test::TestType1 GetUnderlying();

  private:
    TUniquePtr<test::TestType1> Underlying;
};

 
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTestType1Delegate, UTestType1*, newEvent);
