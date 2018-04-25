// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "improbable/collections.h"
#include "UObject/NoExportTypes.h"

#include "TestType2.h"

#include "TestTestType2Option.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class SPATIALOS_API UTestTestType2Option : public UObject
{
    GENERATED_BODY()

  public:
    UTestTestType2Option();
    UTestTestType2Option* Init(const worker::Option<test::TestType2>& underlying);

    UFUNCTION(BlueprintCallable, Category = "TestTestType2Option")
    UTestTestType2Option* SetValue(UTestType2* value);

    UFUNCTION(BlueprintPure, Category = "TestTestType2Option")
    bool HasValue();

	UFUNCTION(BlueprintPure, Category = "TestTestType2Option")
	UTestType2* GetValue();

	UFUNCTION(BlueprintCallable, Category = "TestTestType2Option")
	void Clear();

    worker::Option<test::TestType2> GetUnderlying();

	bool operator==(const worker::Option<test::TestType2>& OtherUnderlying) const;
	bool operator!=(const worker::Option<test::TestType2>& OtherUnderlying) const;	

  private:
    worker::Option<test::TestType2> Underlying;
};
