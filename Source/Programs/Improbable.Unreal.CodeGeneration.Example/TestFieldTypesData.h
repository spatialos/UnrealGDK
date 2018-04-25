// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TestSchema.h"

#include "TestEnum.h"
#include "StdStringList.h"
#include <string>
#include "WorkerEntityIdToTestTestType1Map.h"
#include "EntityId.h"
#include "TestType1.h"
#include "TestTestType2Option.h"
#include "TestType2.h"
#include "ListMapOptionUserTypeData.h"
#include "TestFieldTypesData.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class SPATIALOS_API UTestFieldTypesData : public UObject
{
    GENERATED_BODY()

  public:
    UTestFieldTypesData();
    UTestFieldTypesData* Init(const test::TestFieldTypesData& underlying);

    UFUNCTION(BlueprintPure, Category = "TestFieldTypesData")
    int GetBuiltInProperty();
	UFUNCTION(BlueprintCallable, Category = "TestFieldTypesData")
    UTestFieldTypesData* SetBuiltInProperty(int built_in_property);

    UFUNCTION(BlueprintPure, Category = "TestFieldTypesData")
    ETestEnum GetEnumProperty();
	UFUNCTION(BlueprintCallable, Category = "TestFieldTypesData")
    UTestFieldTypesData* SetEnumProperty(ETestEnum enum_property);

    UFUNCTION(BlueprintPure, Category = "TestFieldTypesData")
    UStdStringList* GetListProperty();
	UFUNCTION(BlueprintCallable, Category = "TestFieldTypesData")
    UTestFieldTypesData* SetListProperty(UStdStringList* list_property);

    UFUNCTION(BlueprintPure, Category = "TestFieldTypesData")
    UWorkerEntityIdToTestTestType1Map* GetMapProperty();
	UFUNCTION(BlueprintCallable, Category = "TestFieldTypesData")
    UTestFieldTypesData* SetMapProperty(UWorkerEntityIdToTestTestType1Map* map_property);

    UFUNCTION(BlueprintPure, Category = "TestFieldTypesData")
    UTestTestType2Option* GetOptionProperty();
	UFUNCTION(BlueprintCallable, Category = "TestFieldTypesData")
    UTestFieldTypesData* SetOptionProperty(UTestTestType2Option* option_property);

    UFUNCTION(BlueprintPure, Category = "TestFieldTypesData")
    UListMapOptionUserTypeData* GetUserTypeProperty();
	UFUNCTION(BlueprintCallable, Category = "TestFieldTypesData")
    UTestFieldTypesData* SetUserTypeProperty(UListMapOptionUserTypeData* user_type_property);


	test::TestFieldTypesData GetUnderlying();

  private:
    TUniquePtr<test::TestFieldTypesData> Underlying;
};

