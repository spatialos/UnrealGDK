// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TestSchema.h"

#include "WorkerEntityIdList.h"
#include "EntityId.h"
#include "StdStringToImprobableCoordinatesMap.h"
#include <string>
#include "improbable/standard_library.h"
#include "SpatialOSConversionFunctionLibrary.h"
#include "BoolOption.h"
#include "BuiltInTypesData.h"
#include "ListMapOptionUserTypeData.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class SPATIALOS_API UListMapOptionUserTypeData : public UObject
{
    GENERATED_BODY()

  public:
    UListMapOptionUserTypeData();
    UListMapOptionUserTypeData* Init(const test::ListMapOptionUserTypeData& underlying);

    UFUNCTION(BlueprintPure, Category = "ListMapOptionUserTypeData")
    UWorkerEntityIdList* GetListProperty();
	UFUNCTION(BlueprintCallable, Category = "ListMapOptionUserTypeData")
    UListMapOptionUserTypeData* SetListProperty(UWorkerEntityIdList* list_property);

    UFUNCTION(BlueprintPure, Category = "ListMapOptionUserTypeData")
    UStdStringToImprobableCoordinatesMap* GetMapProperty();
	UFUNCTION(BlueprintCallable, Category = "ListMapOptionUserTypeData")
    UListMapOptionUserTypeData* SetMapProperty(UStdStringToImprobableCoordinatesMap* map_property);

    UFUNCTION(BlueprintPure, Category = "ListMapOptionUserTypeData")
    UBoolOption* GetOptionProperty();
	UFUNCTION(BlueprintCallable, Category = "ListMapOptionUserTypeData")
    UListMapOptionUserTypeData* SetOptionProperty(UBoolOption* option_property);

    UFUNCTION(BlueprintPure, Category = "ListMapOptionUserTypeData")
    UBuiltInTypesData* GetUserTypeProperty();
	UFUNCTION(BlueprintCallable, Category = "ListMapOptionUserTypeData")
    UListMapOptionUserTypeData* SetUserTypeProperty(UBuiltInTypesData* user_type_property);


	test::ListMapOptionUserTypeData GetUnderlying();

  private:
    TUniquePtr<test::ListMapOptionUserTypeData> Underlying;
};

