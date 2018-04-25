// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TestSchema.h"

#include <string>
#include "improbable/standard_library.h"
#include "SpatialOSConversionFunctionLibrary.h"
#include "improbable/vector3.h"
#include "EntityId.h"
#include "BuiltInTypesData.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class SPATIALOS_API UBuiltInTypesData : public UObject
{
    GENERATED_BODY()

  public:
    UBuiltInTypesData();
    UBuiltInTypesData* Init(const test::BuiltInTypesData& underlying);

    UFUNCTION(BlueprintPure, Category = "BuiltInTypesData")
    bool GetBoolProperty();
	UFUNCTION(BlueprintCallable, Category = "BuiltInTypesData")
    UBuiltInTypesData* SetBoolProperty(bool bool_property);

    UFUNCTION(BlueprintPure, Category = "BuiltInTypesData")
    int GetUint32Property();
	UFUNCTION(BlueprintCallable, Category = "BuiltInTypesData")
    UBuiltInTypesData* SetUint32Property(int uint32_property);

    UFUNCTION(BlueprintPure, Category = "BuiltInTypesData")
    int GetInt32Property();
	UFUNCTION(BlueprintCallable, Category = "BuiltInTypesData")
    UBuiltInTypesData* SetInt32Property(int int32_property);

    UFUNCTION(BlueprintPure, Category = "BuiltInTypesData")
    float GetFloatProperty();
	UFUNCTION(BlueprintCallable, Category = "BuiltInTypesData")
    UBuiltInTypesData* SetFloatProperty(float float_property);

    UFUNCTION(BlueprintPure, Category = "BuiltInTypesData")
    float GetDoubleProperty();
	UFUNCTION(BlueprintCallable, Category = "BuiltInTypesData")
    UBuiltInTypesData* SetDoubleProperty(float double_property);

    UFUNCTION(BlueprintPure, Category = "BuiltInTypesData")
    FString GetStringProperty();
	UFUNCTION(BlueprintCallable, Category = "BuiltInTypesData")
    UBuiltInTypesData* SetStringProperty(FString string_property);

    UFUNCTION(BlueprintPure, Category = "BuiltInTypesData")
    FString GetBytesProperty();
	UFUNCTION(BlueprintCallable, Category = "BuiltInTypesData")
    UBuiltInTypesData* SetBytesProperty(FString bytes_property);

    UFUNCTION(BlueprintPure, Category = "BuiltInTypesData")
    FVector GetCoordinatesProperty();
	UFUNCTION(BlueprintCallable, Category = "BuiltInTypesData")
    UBuiltInTypesData* SetCoordinatesProperty(FVector coordinates_property);

    UFUNCTION(BlueprintPure, Category = "BuiltInTypesData")
    FVector GetVector3dProperty();
	UFUNCTION(BlueprintCallable, Category = "BuiltInTypesData")
    UBuiltInTypesData* SetVector3dProperty(FVector vector3d_property);

    UFUNCTION(BlueprintPure, Category = "BuiltInTypesData")
    FVector GetVector3fProperty();
	UFUNCTION(BlueprintCallable, Category = "BuiltInTypesData")
    UBuiltInTypesData* SetVector3fProperty(FVector vector3f_property);

    UFUNCTION(BlueprintPure, Category = "BuiltInTypesData")
    FEntityId GetEntityIdProperty();
	UFUNCTION(BlueprintCallable, Category = "BuiltInTypesData")
    UBuiltInTypesData* SetEntityIdProperty(FEntityId entity_id_property);


	test::BuiltInTypesData GetUnderlying();

  private:
    TUniquePtr<test::BuiltInTypesData> Underlying;
};

