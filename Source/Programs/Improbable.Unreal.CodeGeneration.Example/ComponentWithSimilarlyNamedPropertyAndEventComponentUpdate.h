// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ComponentUpdateOpWrapperBase.h"
#include "TestSchema.h"

#include "TestType1.h"

#include "ComponentWithSimilarlyNamedPropertyAndEventComponentUpdate.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class SPATIALOS_API UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate : public UComponentUpdateOpWrapperBase
{
    GENERATED_BODY()

  public:
    UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate();
    
	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
	UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate* Init(const test::ComponentWithSimilarlyNamedPropertyAndEvent::Update& underlying);
	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
	UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate* Reset();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "ComponentWithSimilarlyNamedPropertyAndEventComponentUpdate")
    bool HasMyValue();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "ComponentWithSimilarlyNamedPropertyAndEventComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    int GetMyValue();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintCallable, Category = "ComponentWithSimilarlyNamedPropertyAndEventComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate* SetMyValue(int newValue);

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintCallable, Category = "ComponentWithSimilarlyNamedPropertyAndEventComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate* AddMyValueUpdate(UTestType1* event);

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    const test::ComponentWithSimilarlyNamedPropertyAndEvent::Update GetUnderlying();

  private:
    friend class UComponentWithSimilarlyNamedPropertyAndEventComponent;
	friend class UCallbackDispatcher;

	UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate* InitInternal(const test::ComponentWithSimilarlyNamedPropertyAndEvent::Update& underlying);
	UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate* ResetInternal();

    UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate* SetMyValueInternal(int newValue);

    UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate* AddMyValueUpdateInternal(UTestType1* event);

    const test::ComponentWithSimilarlyNamedPropertyAndEvent::Update GetUnderlyingInternal();

	UPROPERTY()
	int MyValue;

    test::ComponentWithSimilarlyNamedPropertyAndEvent::Update Underlying;
	static test::ComponentWithSimilarlyNamedPropertyAndEvent::Update DefaultUnderlying;
};
