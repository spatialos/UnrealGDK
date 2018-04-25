// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "SpatialOsComponent.h"
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "TestFieldTypesComponentUpdate.h"
#include "TestFieldTypesAddComponentOp.h"
#include "ComponentId.h"
#include "SpatialOSViewTypes.h"
#include "SpatialOSWorkerTypes.h"
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


#include "TestFieldTypesComponent.generated.h"

class UCallbackDispatcher;
class UComponentUpdateOpWrapperBase;

USTRUCT()
struct FTestFieldTypesComponentSnapshot
{
	GENERATED_BODY()

	FTestFieldTypesComponentSnapshot()
	: UserTypeProperty(test::ListMapOptionUserTypeData::Create())
	{
	}

	int BuiltInProperty;
	ETestEnum EnumProperty;
	worker::List<std::string> ListProperty;
	worker::Map<worker::EntityId, test::TestType1> MapProperty;
	worker::Option<test::TestType2> OptionProperty;
	test::ListMapOptionUserTypeData UserTypeProperty;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SPATIALOS_API UTestFieldTypesComponent : public USpatialOsComponent
{
    GENERATED_BODY()

public:
    UTestFieldTypesComponent();
	virtual ~UTestFieldTypesComponent() override = default;

    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "TestFieldTypesComponent")
    FComponentId GetComponentId();

    const static worker::ComponentId ComponentId;

    void Init(const TWeakPtr<SpatialOSConnection>& InConnection, const TWeakPtr<SpatialOSView>& InView, worker::EntityId InEntityId, UCallbackDispatcher* InCallbackDispatcher) override;

	void Disable(const worker::EntityId InEntityId, UCallbackDispatcher* CallbackDispatcher);

	void ApplyInitialState(const UAddComponentOpWrapperBase& Op) override;

	DEPRECATED(12.1, "Please use TriggerAutomaticComponentUpdate.")
	void ReplicateChanges(float DeltaSeconds) override;

	void TriggerAutomaticComponentUpdate(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "TestFieldTypesComponent")
	void TriggerManualComponentUpdate();

    UPROPERTY(BlueprintAssignable, Category = "TestFieldTypesComponent")
    FSpatialComponentUpdated OnBuiltInPropertyUpdate;

    UPROPERTY(BlueprintAssignable, Category = "TestFieldTypesComponent")
    FSpatialComponentUpdated OnEnumPropertyUpdate;

    UPROPERTY(BlueprintAssignable, Category = "TestFieldTypesComponent")
    FSpatialComponentUpdated OnListPropertyUpdate;

    UPROPERTY(BlueprintAssignable, Category = "TestFieldTypesComponent")
    FSpatialComponentUpdated OnMapPropertyUpdate;

    UPROPERTY(BlueprintAssignable, Category = "TestFieldTypesComponent")
    FSpatialComponentUpdated OnOptionPropertyUpdate;

    UPROPERTY(BlueprintAssignable, Category = "TestFieldTypesComponent")
    FSpatialComponentUpdated OnUserTypePropertyUpdate;



	DEPRECATED(12.1, "This function is deprecated, the component updates are now sent automatically. Use MaxUpdatesPerSecond property to control the update frequency, or set it to 0 and use TriggerManualComponentUpdate for more control.")
    UFUNCTION(BlueprintCallable, Category = "TestFieldTypesComponent", meta=(DeprecatedFunction, DeprecationMessage="This function is deprecated, the component updates are now sent automatically. Use MaxUpdatesPerSecond property to control the update frequency, or set it to 0 and use TriggerManualComponentUpdate for more control."))
    void SendComponentUpdate(UTestFieldTypesComponentUpdate* update);

    UPROPERTY(BlueprintAssignable, Category = "TestFieldTypesComponent")
    FSpatialComponentUpdated OnComponentUpdate;

	DEPRECATED(12.0, "This function is deprecated, access the BuiltInProperty property directly.")
    UFUNCTION(BlueprintPure, Category = "TestFieldTypesComponent", meta=(DeprecatedFunction, DeprecationMessage="This function is deprecated, access the BuiltInProperty property directly."))
    int GetBuiltInProperty();
	DEPRECATED(12.0, "This function is deprecated, access the EnumProperty property directly.")
    UFUNCTION(BlueprintPure, Category = "TestFieldTypesComponent", meta=(DeprecatedFunction, DeprecationMessage="This function is deprecated, access the EnumProperty property directly."))
    ETestEnum GetEnumProperty();
	DEPRECATED(12.0, "This function is deprecated, access the ListProperty property directly.")
    UFUNCTION(BlueprintPure, Category = "TestFieldTypesComponent", meta=(DeprecatedFunction, DeprecationMessage="This function is deprecated, access the ListProperty property directly."))
    UStdStringList* GetListProperty();
	DEPRECATED(12.0, "This function is deprecated, access the MapProperty property directly.")
    UFUNCTION(BlueprintPure, Category = "TestFieldTypesComponent", meta=(DeprecatedFunction, DeprecationMessage="This function is deprecated, access the MapProperty property directly."))
    UWorkerEntityIdToTestTestType1Map* GetMapProperty();
	DEPRECATED(12.0, "This function is deprecated, access the OptionProperty property directly.")
    UFUNCTION(BlueprintPure, Category = "TestFieldTypesComponent", meta=(DeprecatedFunction, DeprecationMessage="This function is deprecated, access the OptionProperty property directly."))
    UTestTestType2Option* GetOptionProperty();
	DEPRECATED(12.0, "This function is deprecated, access the UserTypeProperty property directly.")
    UFUNCTION(BlueprintPure, Category = "TestFieldTypesComponent", meta=(DeprecatedFunction, DeprecationMessage="This function is deprecated, access the UserTypeProperty property directly."))
    UListMapOptionUserTypeData* GetUserTypeProperty();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int BuiltInProperty;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETestEnum EnumProperty;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStdStringList* ListProperty;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UWorkerEntityIdToTestTestType1Map* MapProperty;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTestTestType2Option* OptionProperty;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UListMapOptionUserTypeData* UserTypeProperty;

private:
	void GenerateSnapshot();

	UPROPERTY()
	UTestFieldTypesComponentUpdate* ComponentUpdater;


	FTestFieldTypesComponentSnapshot* Snapshot;

    void OnComponentUpdateDispatcherCallback(UComponentUpdateOpWrapperBase& Op);

	void OnAuthorityChangeDispatcherCallback(const worker::AuthorityChangeOp& Op) override;

	void ReplicateChangesInternal();
	void SendComponentUpdateInternal(UTestFieldTypesComponentUpdate* update);
    void ApplyComponentUpdate(const test::TestFieldTypes::Update& update);
	void NotifyUpdate(const test::TestFieldTypes::Update& update);

};
