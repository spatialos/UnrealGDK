// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "improbable/collections.h"
#include "UObject/NoExportTypes.h"

#include "improbable/standard_library.h"
#include "SpatialOSConversionFunctionLibrary.h"

#include <string>

#include "StdStringToImprobableCoordinatesMap.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class SPATIALOS_API UStdStringToImprobableCoordinatesMap : public UObject
{
    GENERATED_BODY()

  public:
    UStdStringToImprobableCoordinatesMap();
    UStdStringToImprobableCoordinatesMap* Init(const worker::Map<std::string, improbable::Coordinates>& underlying);

    UFUNCTION(BlueprintCallable, Category = "StdStringToImprobableCoordinatesMap")
    UStdStringToImprobableCoordinatesMap* Emplace(const FString& key, const FVector& value);

    UFUNCTION(BlueprintCallable, Category = "StdStringToImprobableCoordinatesMap")
    UStdStringToImprobableCoordinatesMap* Remove(const FString& key);

    UFUNCTION(BlueprintPure, Category = "StdStringToImprobableCoordinatesMap")
    bool Contains(const FString& key);

    UFUNCTION(BlueprintCallable, Category = "StdStringToImprobableCoordinatesMap")
    FVector Get(const FString& key);

    UFUNCTION(BlueprintCallable, Category = "StdStringToImprobableCoordinatesMap")
    UStdStringToImprobableCoordinatesMap* Clear();

    UFUNCTION(BlueprintPure, Category = "StdStringToImprobableCoordinatesMap")
    bool IsEmpty();

    worker::Map<std::string, improbable::Coordinates> GetUnderlying();

	bool operator==(const worker::Map<std::string, improbable::Coordinates>& OtherUnderlying) const;
	bool operator!=(const worker::Map<std::string, improbable::Coordinates>& OtherUnderlying) const;

  private:
    worker::Map<std::string, improbable::Coordinates> Underlying;
};
