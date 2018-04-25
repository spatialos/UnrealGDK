// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "SpatialOsComponent.h"
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "BuiltInTypesComponentUpdate.h"
#include "BuiltInTypesAddComponentOp.h"
#include "ComponentId.h"
#include "SpatialOSViewTypes.h"
#include "SpatialOSWorkerTypes.h"
#include "TestSchema.h"

#include <string>
#include "improbable/standard_library.h"
#include "SpatialOSConversionFunctionLibrary.h"
#include "improbable/vector3.h"
#include "EntityId.h"


#include "BuiltInTypesComponent.generated.h"

class UCallbackDispatcher;
class UComponentUpdateOpWrapperBase;

USTRUCT()
struct FBuiltInTypesComponentSnapshot
{
	GENERATED_BODY()

	FBuiltInTypesComponentSnapshot()
	{
	}

	bool BoolProperty;
	int Uint32Property;
	int Int32Property;
	float FloatProperty;
	float DoubleProperty;
	FString StringProperty;
	FString BytesProperty;
	FVector CoordinatesProperty;
	FVector Vector3dProperty;
	FVector Vector3fProperty;
	FEntityId EntityIdProperty;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SPATIALOS_API UBuiltInTypesComponent : public USpatialOsComponent
{
    GENERATED_BODY()

public:
    UBuiltInTypesComponent();
	virtual ~UBuiltInTypesComponent() override = default;

    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "BuiltInTypesComponent")
    FComponentId GetComponentId();

    const static worker::ComponentId ComponentId;

    void Init(const TWeakPtr<SpatialOSConnection>& InConnection, const TWeakPtr<SpatialOSView>& InView, worker::EntityId InEntityId, UCallbackDispatcher* InCallbackDispatcher) override;

	void Disable(const worker::EntityId InEntityId, UCallbackDispatcher* CallbackDispatcher);

	void ApplyInitialState(const UAddComponentOpWrapperBase& Op) override;

	DEPRECATED(12.1, "Please use TriggerAutomaticComponentUpdate.")
	void ReplicateChanges(float DeltaSeconds) override;

	void TriggerAutomaticComponentUpdate(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "BuiltInTypesComponent")
	void TriggerManualComponentUpdate();

    UPROPERTY(BlueprintAssignable, Category = "BuiltInTypesComponent")
    FSpatialComponentUpdated OnBoolPropertyUpdate;

    UPROPERTY(BlueprintAssignable, Category = "BuiltInTypesComponent")
    FSpatialComponentUpdated OnUint32PropertyUpdate;

    UPROPERTY(BlueprintAssignable, Category = "BuiltInTypesComponent")
    FSpatialComponentUpdated OnInt32PropertyUpdate;

    UPROPERTY(BlueprintAssignable, Category = "BuiltInTypesComponent")
    FSpatialComponentUpdated OnFloatPropertyUpdate;

    UPROPERTY(BlueprintAssignable, Category = "BuiltInTypesComponent")
    FSpatialComponentUpdated OnDoublePropertyUpdate;

    UPROPERTY(BlueprintAssignable, Category = "BuiltInTypesComponent")
    FSpatialComponentUpdated OnStringPropertyUpdate;

    UPROPERTY(BlueprintAssignable, Category = "BuiltInTypesComponent")
    FSpatialComponentUpdated OnBytesPropertyUpdate;

    UPROPERTY(BlueprintAssignable, Category = "BuiltInTypesComponent")
    FSpatialComponentUpdated OnCoordinatesPropertyUpdate;

    UPROPERTY(BlueprintAssignable, Category = "BuiltInTypesComponent")
    FSpatialComponentUpdated OnVector3dPropertyUpdate;

    UPROPERTY(BlueprintAssignable, Category = "BuiltInTypesComponent")
    FSpatialComponentUpdated OnVector3fPropertyUpdate;

    UPROPERTY(BlueprintAssignable, Category = "BuiltInTypesComponent")
    FSpatialComponentUpdated OnEntityIdPropertyUpdate;



	DEPRECATED(12.1, "This function is deprecated, the component updates are now sent automatically. Use MaxUpdatesPerSecond property to control the update frequency, or set it to 0 and use TriggerManualComponentUpdate for more control.")
    UFUNCTION(BlueprintCallable, Category = "BuiltInTypesComponent", meta=(DeprecatedFunction, DeprecationMessage="This function is deprecated, the component updates are now sent automatically. Use MaxUpdatesPerSecond property to control the update frequency, or set it to 0 and use TriggerManualComponentUpdate for more control."))
    void SendComponentUpdate(UBuiltInTypesComponentUpdate* update);

    UPROPERTY(BlueprintAssignable, Category = "BuiltInTypesComponent")
    FSpatialComponentUpdated OnComponentUpdate;

	DEPRECATED(12.0, "This function is deprecated, access the BoolProperty property directly.")
    UFUNCTION(BlueprintPure, Category = "BuiltInTypesComponent", meta=(DeprecatedFunction, DeprecationMessage="This function is deprecated, access the BoolProperty property directly."))
    bool GetBoolProperty();
	DEPRECATED(12.0, "This function is deprecated, access the Uint32Property property directly.")
    UFUNCTION(BlueprintPure, Category = "BuiltInTypesComponent", meta=(DeprecatedFunction, DeprecationMessage="This function is deprecated, access the Uint32Property property directly."))
    int GetUint32Property();
	DEPRECATED(12.0, "This function is deprecated, access the Int32Property property directly.")
    UFUNCTION(BlueprintPure, Category = "BuiltInTypesComponent", meta=(DeprecatedFunction, DeprecationMessage="This function is deprecated, access the Int32Property property directly."))
    int GetInt32Property();
	DEPRECATED(12.0, "This function is deprecated, access the FloatProperty property directly.")
    UFUNCTION(BlueprintPure, Category = "BuiltInTypesComponent", meta=(DeprecatedFunction, DeprecationMessage="This function is deprecated, access the FloatProperty property directly."))
    float GetFloatProperty();
	DEPRECATED(12.0, "This function is deprecated, access the DoubleProperty property directly.")
    UFUNCTION(BlueprintPure, Category = "BuiltInTypesComponent", meta=(DeprecatedFunction, DeprecationMessage="This function is deprecated, access the DoubleProperty property directly."))
    float GetDoubleProperty();
	DEPRECATED(12.0, "This function is deprecated, access the StringProperty property directly.")
    UFUNCTION(BlueprintPure, Category = "BuiltInTypesComponent", meta=(DeprecatedFunction, DeprecationMessage="This function is deprecated, access the StringProperty property directly."))
    FString GetStringProperty();
	DEPRECATED(12.0, "This function is deprecated, access the BytesProperty property directly.")
    UFUNCTION(BlueprintPure, Category = "BuiltInTypesComponent", meta=(DeprecatedFunction, DeprecationMessage="This function is deprecated, access the BytesProperty property directly."))
    FString GetBytesProperty();
	DEPRECATED(12.0, "This function is deprecated, access the CoordinatesProperty property directly.")
    UFUNCTION(BlueprintPure, Category = "BuiltInTypesComponent", meta=(DeprecatedFunction, DeprecationMessage="This function is deprecated, access the CoordinatesProperty property directly."))
    FVector GetCoordinatesProperty();
	DEPRECATED(12.0, "This function is deprecated, access the Vector3dProperty property directly.")
    UFUNCTION(BlueprintPure, Category = "BuiltInTypesComponent", meta=(DeprecatedFunction, DeprecationMessage="This function is deprecated, access the Vector3dProperty property directly."))
    FVector GetVector3dProperty();
	DEPRECATED(12.0, "This function is deprecated, access the Vector3fProperty property directly.")
    UFUNCTION(BlueprintPure, Category = "BuiltInTypesComponent", meta=(DeprecatedFunction, DeprecationMessage="This function is deprecated, access the Vector3fProperty property directly."))
    FVector GetVector3fProperty();
	DEPRECATED(12.0, "This function is deprecated, access the EntityIdProperty property directly.")
    UFUNCTION(BlueprintPure, Category = "BuiltInTypesComponent", meta=(DeprecatedFunction, DeprecationMessage="This function is deprecated, access the EntityIdProperty property directly."))
    FEntityId GetEntityIdProperty();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool BoolProperty;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int Uint32Property;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int Int32Property;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FloatProperty;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DoubleProperty;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString StringProperty;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString BytesProperty;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector CoordinatesProperty;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Vector3dProperty;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Vector3fProperty;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FEntityId EntityIdProperty;

private:
	void GenerateSnapshot();

	UPROPERTY()
	UBuiltInTypesComponentUpdate* ComponentUpdater;


	FBuiltInTypesComponentSnapshot* Snapshot;

    void OnComponentUpdateDispatcherCallback(UComponentUpdateOpWrapperBase& Op);

	void OnAuthorityChangeDispatcherCallback(const worker::AuthorityChangeOp& Op) override;

	void ReplicateChangesInternal();
	void SendComponentUpdateInternal(UBuiltInTypesComponentUpdate* update);
    void ApplyComponentUpdate(const test::BuiltInTypes::Update& update);
	void NotifyUpdate(const test::BuiltInTypes::Update& update);

};
