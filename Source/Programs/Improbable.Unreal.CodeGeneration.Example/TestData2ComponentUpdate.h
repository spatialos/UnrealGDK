// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ComponentUpdateOpWrapperBase.h"
#include "TestSchema.h"

#include "TextEvent.h"

#include "TestData2ComponentUpdate.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class SPATIALOS_API UTestData2ComponentUpdate : public UComponentUpdateOpWrapperBase
{
    GENERATED_BODY()

  public:
    UTestData2ComponentUpdate();
    
	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
	UTestData2ComponentUpdate* Init(const test::TestData2::Update& underlying);
	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
	UTestData2ComponentUpdate* Reset();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "TestData2ComponentUpdate")
    bool HasDoubleProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "TestData2ComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    float GetDoubleProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintCallable, Category = "TestData2ComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    UTestData2ComponentUpdate* SetDoubleProperty(float newValue);

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintCallable, Category = "TestData2ComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    UTestData2ComponentUpdate* AddTwoTextEvent(UTextEvent* event);

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    const test::TestData2::Update GetUnderlying();

  private:
    friend class UTestData2Component;
	friend class UCallbackDispatcher;

	UTestData2ComponentUpdate* InitInternal(const test::TestData2::Update& underlying);
	UTestData2ComponentUpdate* ResetInternal();

    UTestData2ComponentUpdate* SetDoublePropertyInternal(float newValue);

    UTestData2ComponentUpdate* AddTwoTextEventInternal(UTextEvent* event);

    const test::TestData2::Update GetUnderlyingInternal();

	UPROPERTY()
	float DoubleProperty;

    test::TestData2::Update Underlying;
	static test::TestData2::Update DefaultUnderlying;
};
