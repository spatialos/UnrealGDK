// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "improbable/collections.h"
#include "UObject/NoExportTypes.h"


#include "BoolOption.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class SPATIALOS_API UBoolOption : public UObject
{
    GENERATED_BODY()

  public:
    UBoolOption();
    UBoolOption* Init(const worker::Option<bool>& underlying);

    UFUNCTION(BlueprintCallable, Category = "BoolOption")
    UBoolOption* SetValue(bool value);

    UFUNCTION(BlueprintPure, Category = "BoolOption")
    bool HasValue();

	UFUNCTION(BlueprintPure, Category = "BoolOption")
	bool GetValue();

	UFUNCTION(BlueprintCallable, Category = "BoolOption")
	void Clear();

    worker::Option<bool> GetUnderlying();

	bool operator==(const worker::Option<bool>& OtherUnderlying) const;
	bool operator!=(const worker::Option<bool>& OtherUnderlying) const;	

  private:
    worker::Option<bool> Underlying;
};
