// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ComponentUpdateOpWrapperBase.h"
#include "TestSchema.h"

#include <string>
#include "improbable/standard_library.h"
#include "SpatialOSConversionFunctionLibrary.h"
#include "TextEvent.h"

#include "TestData1ComponentUpdate.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class SPATIALOS_API UTestData1ComponentUpdate : public UComponentUpdateOpWrapperBase
{
    GENERATED_BODY()

  public:
    UTestData1ComponentUpdate();
    
	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
	UTestData1ComponentUpdate* Init(const test::TestData1::Update& underlying);
	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
	UTestData1ComponentUpdate* Reset();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "TestData1ComponentUpdate")
    bool HasStringProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "TestData1ComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    FString GetStringProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintCallable, Category = "TestData1ComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    UTestData1ComponentUpdate* SetStringProperty(FString newValue);

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "TestData1ComponentUpdate")
    bool HasInt32Property();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "TestData1ComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    int GetInt32Property();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintCallable, Category = "TestData1ComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    UTestData1ComponentUpdate* SetInt32Property(int newValue);

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "TestData1ComponentUpdate")
    bool HasCoordinatesProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "TestData1ComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    FVector GetCoordinatesProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintCallable, Category = "TestData1ComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    UTestData1ComponentUpdate* SetCoordinatesProperty(FVector newValue);

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintCallable, Category = "TestData1ComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    UTestData1ComponentUpdate* AddOneTextEvent(UTextEvent* event);

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    const test::TestData1::Update GetUnderlying();

  private:
    friend class UTestData1Component;
	friend class UCallbackDispatcher;

	UTestData1ComponentUpdate* InitInternal(const test::TestData1::Update& underlying);
	UTestData1ComponentUpdate* ResetInternal();

    UTestData1ComponentUpdate* SetStringPropertyInternal(FString newValue);

    UTestData1ComponentUpdate* SetInt32PropertyInternal(int newValue);

    UTestData1ComponentUpdate* SetCoordinatesPropertyInternal(FVector newValue);

    UTestData1ComponentUpdate* AddOneTextEventInternal(UTextEvent* event);

    const test::TestData1::Update GetUnderlyingInternal();

	UPROPERTY()
	FString StringProperty;
	UPROPERTY()
	int Int32Property;
	UPROPERTY()
	FVector CoordinatesProperty;

    test::TestData1::Update Underlying;
	static test::TestData1::Update DefaultUnderlying;
};
