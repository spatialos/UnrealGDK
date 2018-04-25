// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TestType1.h"
#include "TestType2.h"
#include "BuiltInTypesData.h"
#include "TestFieldTypesData.h"
#include "TestEmptyData.h"
#include "TestCommandResponseTypesData.h"
#include "ComponentWithSimilarlyNamedPropertyAndEventData.h"
#include "TestSchema.h"
#include "EntityTemplate.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class SPATIALOS_API UEntityTemplate : public UObject
{
    GENERATED_BODY()

  public:
    UEntityTemplate();
    UEntityTemplate* Init(const worker::Entity& underlying);

    UFUNCTION(BlueprintCallable, Category = "EntityTemplate")
    UEntityTemplate* AddTestData1Component(UTestType1* data);

    UFUNCTION(BlueprintCallable, Category = "EntityTemplate")
    UEntityTemplate* AddTestData2Component(UTestType2* data);

    UFUNCTION(BlueprintCallable, Category = "EntityTemplate")
    UEntityTemplate* AddBuiltInTypesComponent(UBuiltInTypesData* data);

    UFUNCTION(BlueprintCallable, Category = "EntityTemplate")
    UEntityTemplate* AddTestFieldTypesComponent(UTestFieldTypesData* data);

    UFUNCTION(BlueprintCallable, Category = "EntityTemplate")
    UEntityTemplate* AddTestEmptyComponent(UTestEmptyData* data);

    UFUNCTION(BlueprintCallable, Category = "EntityTemplate")
    UEntityTemplate* AddTestCommandResponseTypesComponent(UTestCommandResponseTypesData* data);

    UFUNCTION(BlueprintCallable, Category = "EntityTemplate")
    UEntityTemplate* AddComponentWithSimilarlyNamedPropertyAndEventComponent(UComponentWithSimilarlyNamedPropertyAndEventData* data);

    worker::Entity GetUnderlying();

  private:
    worker::Entity Underlying;
};
