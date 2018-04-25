// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "improbable/collections.h"
#include "UObject/NoExportTypes.h"

#include <string>

#include "StdStringList.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class SPATIALOS_API UStdStringList : public UObject
{
    GENERATED_BODY()

  public:
    UStdStringList();
    UStdStringList* Init(const worker::List<std::string>& underlying);

    UFUNCTION(BlueprintCallable, Category = "StdStringList")
    UStdStringList* Add(const FString& value);

    UFUNCTION(BlueprintPure, Category = "StdStringList")
    FString Get(int pos);

    UFUNCTION(BlueprintCallable, Category = "StdStringList")
    UStdStringList* Clear();

    UFUNCTION(BlueprintPure, Category = "StdStringList")
    bool IsEmpty();

	UFUNCTION(BlueprintPure, Category = "StdStringList")
    int Size();

	bool operator==(const worker::List<std::string>& OtherUnderlying) const;
	bool operator!=(const worker::List<std::string>& OtherUnderlying) const;	

    worker::List<std::string> GetUnderlying();

  private:
    worker::List<std::string> Underlying;
};
