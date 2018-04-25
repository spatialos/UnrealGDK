// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ComponentUpdateOpWrapperBase.h"
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

#include "TestFieldTypesComponentUpdate.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class SPATIALOS_API UTestFieldTypesComponentUpdate : public UComponentUpdateOpWrapperBase
{
    GENERATED_BODY()

  public:
    UTestFieldTypesComponentUpdate();
    
	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
	UTestFieldTypesComponentUpdate* Init(const test::TestFieldTypes::Update& underlying);
	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
	UTestFieldTypesComponentUpdate* Reset();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "TestFieldTypesComponentUpdate")
    bool HasBuiltInProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "TestFieldTypesComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    int GetBuiltInProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintCallable, Category = "TestFieldTypesComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    UTestFieldTypesComponentUpdate* SetBuiltInProperty(int newValue);

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "TestFieldTypesComponentUpdate")
    bool HasEnumProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "TestFieldTypesComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    ETestEnum GetEnumProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintCallable, Category = "TestFieldTypesComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    UTestFieldTypesComponentUpdate* SetEnumProperty(ETestEnum newValue);

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "TestFieldTypesComponentUpdate")
    bool HasListProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "TestFieldTypesComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    UStdStringList* GetListProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintCallable, Category = "TestFieldTypesComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    UTestFieldTypesComponentUpdate* SetListProperty(UStdStringList* newValue);

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "TestFieldTypesComponentUpdate")
    bool HasMapProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "TestFieldTypesComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    UWorkerEntityIdToTestTestType1Map* GetMapProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintCallable, Category = "TestFieldTypesComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    UTestFieldTypesComponentUpdate* SetMapProperty(UWorkerEntityIdToTestTestType1Map* newValue);

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "TestFieldTypesComponentUpdate")
    bool HasOptionProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "TestFieldTypesComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    UTestTestType2Option* GetOptionProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintCallable, Category = "TestFieldTypesComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    UTestFieldTypesComponentUpdate* SetOptionProperty(UTestTestType2Option* newValue);

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "TestFieldTypesComponentUpdate")
    bool HasUserTypeProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "TestFieldTypesComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    UListMapOptionUserTypeData* GetUserTypeProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintCallable, Category = "TestFieldTypesComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    UTestFieldTypesComponentUpdate* SetUserTypeProperty(UListMapOptionUserTypeData* newValue);

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    const test::TestFieldTypes::Update GetUnderlying();

  private:
    friend class UTestFieldTypesComponent;
	friend class UCallbackDispatcher;

	UTestFieldTypesComponentUpdate* InitInternal(const test::TestFieldTypes::Update& underlying);
	UTestFieldTypesComponentUpdate* ResetInternal();

    UTestFieldTypesComponentUpdate* SetBuiltInPropertyInternal(int newValue);

    UTestFieldTypesComponentUpdate* SetEnumPropertyInternal(ETestEnum newValue);

    UTestFieldTypesComponentUpdate* SetListPropertyInternal(UStdStringList* newValue);

    UTestFieldTypesComponentUpdate* SetMapPropertyInternal(UWorkerEntityIdToTestTestType1Map* newValue);

    UTestFieldTypesComponentUpdate* SetOptionPropertyInternal(UTestTestType2Option* newValue);

    UTestFieldTypesComponentUpdate* SetUserTypePropertyInternal(UListMapOptionUserTypeData* newValue);

    const test::TestFieldTypes::Update GetUnderlyingInternal();

	UPROPERTY()
	int BuiltInProperty;
	UPROPERTY()
	ETestEnum EnumProperty;
	UPROPERTY()
	UStdStringList* ListProperty;
	UPROPERTY()
	UWorkerEntityIdToTestTestType1Map* MapProperty;
	UPROPERTY()
	UTestTestType2Option* OptionProperty;
	UPROPERTY()
	UListMapOptionUserTypeData* UserTypeProperty;

    test::TestFieldTypes::Update Underlying;
	static test::TestFieldTypes::Update DefaultUnderlying;
};
