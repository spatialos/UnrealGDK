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
#include "improbable/vector3.h"
#include "EntityId.h"

#include "BuiltInTypesComponentUpdate.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class SPATIALOS_API UBuiltInTypesComponentUpdate : public UComponentUpdateOpWrapperBase
{
    GENERATED_BODY()

  public:
    UBuiltInTypesComponentUpdate();
    
	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
	UBuiltInTypesComponentUpdate* Init(const test::BuiltInTypes::Update& underlying);
	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
	UBuiltInTypesComponentUpdate* Reset();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "BuiltInTypesComponentUpdate")
    bool HasBoolProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "BuiltInTypesComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    bool GetBoolProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintCallable, Category = "BuiltInTypesComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    UBuiltInTypesComponentUpdate* SetBoolProperty(bool newValue);

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "BuiltInTypesComponentUpdate")
    bool HasUint32Property();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "BuiltInTypesComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    int GetUint32Property();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintCallable, Category = "BuiltInTypesComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    UBuiltInTypesComponentUpdate* SetUint32Property(int newValue);

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "BuiltInTypesComponentUpdate")
    bool HasInt32Property();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "BuiltInTypesComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    int GetInt32Property();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintCallable, Category = "BuiltInTypesComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    UBuiltInTypesComponentUpdate* SetInt32Property(int newValue);

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "BuiltInTypesComponentUpdate")
    bool HasFloatProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "BuiltInTypesComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    float GetFloatProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintCallable, Category = "BuiltInTypesComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    UBuiltInTypesComponentUpdate* SetFloatProperty(float newValue);

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "BuiltInTypesComponentUpdate")
    bool HasDoubleProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "BuiltInTypesComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    float GetDoubleProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintCallable, Category = "BuiltInTypesComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    UBuiltInTypesComponentUpdate* SetDoubleProperty(float newValue);

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "BuiltInTypesComponentUpdate")
    bool HasStringProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "BuiltInTypesComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    FString GetStringProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintCallable, Category = "BuiltInTypesComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    UBuiltInTypesComponentUpdate* SetStringProperty(FString newValue);

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "BuiltInTypesComponentUpdate")
    bool HasBytesProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "BuiltInTypesComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    FString GetBytesProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintCallable, Category = "BuiltInTypesComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    UBuiltInTypesComponentUpdate* SetBytesProperty(FString newValue);

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "BuiltInTypesComponentUpdate")
    bool HasCoordinatesProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "BuiltInTypesComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    FVector GetCoordinatesProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintCallable, Category = "BuiltInTypesComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    UBuiltInTypesComponentUpdate* SetCoordinatesProperty(FVector newValue);

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "BuiltInTypesComponentUpdate")
    bool HasVector3dProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "BuiltInTypesComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    FVector GetVector3dProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintCallable, Category = "BuiltInTypesComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    UBuiltInTypesComponentUpdate* SetVector3dProperty(FVector newValue);

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "BuiltInTypesComponentUpdate")
    bool HasVector3fProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "BuiltInTypesComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    FVector GetVector3fProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintCallable, Category = "BuiltInTypesComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    UBuiltInTypesComponentUpdate* SetVector3fProperty(FVector newValue);

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "BuiltInTypesComponentUpdate")
    bool HasEntityIdProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintPure, Category = "BuiltInTypesComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    FEntityId GetEntityIdProperty();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    UFUNCTION(BlueprintCallable, Category = "BuiltInTypesComponentUpdate", meta=(DeprecatedFunction, DeprecationMessage="This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate."))
    UBuiltInTypesComponentUpdate* SetEntityIdProperty(FEntityId newValue);

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    const test::BuiltInTypes::Update GetUnderlying();

  private:
    friend class UBuiltInTypesComponent;
	friend class UCallbackDispatcher;

	UBuiltInTypesComponentUpdate* InitInternal(const test::BuiltInTypes::Update& underlying);
	UBuiltInTypesComponentUpdate* ResetInternal();

    UBuiltInTypesComponentUpdate* SetBoolPropertyInternal(bool newValue);

    UBuiltInTypesComponentUpdate* SetUint32PropertyInternal(int newValue);

    UBuiltInTypesComponentUpdate* SetInt32PropertyInternal(int newValue);

    UBuiltInTypesComponentUpdate* SetFloatPropertyInternal(float newValue);

    UBuiltInTypesComponentUpdate* SetDoublePropertyInternal(float newValue);

    UBuiltInTypesComponentUpdate* SetStringPropertyInternal(FString newValue);

    UBuiltInTypesComponentUpdate* SetBytesPropertyInternal(FString newValue);

    UBuiltInTypesComponentUpdate* SetCoordinatesPropertyInternal(FVector newValue);

    UBuiltInTypesComponentUpdate* SetVector3dPropertyInternal(FVector newValue);

    UBuiltInTypesComponentUpdate* SetVector3fPropertyInternal(FVector newValue);

    UBuiltInTypesComponentUpdate* SetEntityIdPropertyInternal(FEntityId newValue);

    const test::BuiltInTypes::Update GetUnderlyingInternal();

	UPROPERTY()
	bool BoolProperty;
	UPROPERTY()
	int Uint32Property;
	UPROPERTY()
	int Int32Property;
	UPROPERTY()
	float FloatProperty;
	UPROPERTY()
	float DoubleProperty;
	UPROPERTY()
	FString StringProperty;
	UPROPERTY()
	FString BytesProperty;
	UPROPERTY()
	FVector CoordinatesProperty;
	UPROPERTY()
	FVector Vector3dProperty;
	UPROPERTY()
	FVector Vector3fProperty;
	UPROPERTY()
	FEntityId EntityIdProperty;

    test::BuiltInTypes::Update Underlying;
	static test::BuiltInTypes::Update DefaultUnderlying;
};
