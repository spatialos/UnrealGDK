// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "SpatialOsComponent.h"
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "TestData1ComponentUpdate.h"
#include "TestData1AddComponentOp.h"
#include "ComponentId.h"
#include "SpatialOSViewTypes.h"
#include "SpatialOSWorkerTypes.h"
#include "TestSchema.h"

#include <string>
#include "improbable/standard_library.h"
#include "SpatialOSConversionFunctionLibrary.h"
#include "RequestTestdata2CommandResponder.h"

#include "TextEvent.h"

#include "TestData1Component.generated.h"

class UCallbackDispatcher;
class UComponentUpdateOpWrapperBase;

USTRUCT()
struct FTestData1ComponentSnapshot
{
	GENERATED_BODY()

	FTestData1ComponentSnapshot()
	{
	}

	FString StringProperty;
	int Int32Property;
	FVector CoordinatesProperty;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SPATIALOS_API UTestData1Component : public USpatialOsComponent
{
    GENERATED_BODY()

public:
    UTestData1Component();
	virtual ~UTestData1Component() override = default;

    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "TestData1Component")
    FComponentId GetComponentId();

    const static worker::ComponentId ComponentId;

    void Init(const TWeakPtr<SpatialOSConnection>& InConnection, const TWeakPtr<SpatialOSView>& InView, worker::EntityId InEntityId, UCallbackDispatcher* InCallbackDispatcher) override;

	void Disable(const worker::EntityId InEntityId, UCallbackDispatcher* CallbackDispatcher);

	void ApplyInitialState(const UAddComponentOpWrapperBase& Op) override;

	DEPRECATED(12.1, "Please use TriggerAutomaticComponentUpdate.")
	void ReplicateChanges(float DeltaSeconds) override;

	void TriggerAutomaticComponentUpdate(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "TestData1Component")
	void TriggerManualComponentUpdate();

    UPROPERTY(BlueprintAssignable, Category = "TestData1Component")
    FSpatialComponentUpdated OnStringPropertyUpdate;

    UPROPERTY(BlueprintAssignable, Category = "TestData1Component")
    FSpatialComponentUpdated OnInt32PropertyUpdate;

    UPROPERTY(BlueprintAssignable, Category = "TestData1Component")
    FSpatialComponentUpdated OnCoordinatesPropertyUpdate;

    UPROPERTY(BlueprintAssignable, Category = "TestData1Component")
    FTextEventDelegate OnOneTextEvent;
	UFUNCTION(BlueprintCallable, Category = "TestData1Component")
    void OneTextEvent(UTextEvent* Data);

    UPROPERTY(BlueprintAssignable, Category = "TestData1Component")
    FRequestTestdata2Command OnRequestTestdata2CommandRequest;

	DEPRECATED(12.1, "This function is deprecated, the component updates are now sent automatically. Use MaxUpdatesPerSecond property to control the update frequency, or set it to 0 and use TriggerManualComponentUpdate for more control.")
    UFUNCTION(BlueprintCallable, Category = "TestData1Component", meta=(DeprecatedFunction, DeprecationMessage="This function is deprecated, the component updates are now sent automatically. Use MaxUpdatesPerSecond property to control the update frequency, or set it to 0 and use TriggerManualComponentUpdate for more control."))
    void SendComponentUpdate(UTestData1ComponentUpdate* update);

    UPROPERTY(BlueprintAssignable, Category = "TestData1Component")
    FSpatialComponentUpdated OnComponentUpdate;

	DEPRECATED(12.0, "This function is deprecated, access the StringProperty property directly.")
    UFUNCTION(BlueprintPure, Category = "TestData1Component", meta=(DeprecatedFunction, DeprecationMessage="This function is deprecated, access the StringProperty property directly."))
    FString GetStringProperty();
	DEPRECATED(12.0, "This function is deprecated, access the Int32Property property directly.")
    UFUNCTION(BlueprintPure, Category = "TestData1Component", meta=(DeprecatedFunction, DeprecationMessage="This function is deprecated, access the Int32Property property directly."))
    int GetInt32Property();
	DEPRECATED(12.0, "This function is deprecated, access the CoordinatesProperty property directly.")
    UFUNCTION(BlueprintPure, Category = "TestData1Component", meta=(DeprecatedFunction, DeprecationMessage="This function is deprecated, access the CoordinatesProperty property directly."))
    FVector GetCoordinatesProperty();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString StringProperty;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int Int32Property;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector CoordinatesProperty;

private:
	void GenerateSnapshot();

	UPROPERTY()
	UTestData1ComponentUpdate* ComponentUpdater;

	UPROPERTY()
	UTextEvent* OneTextEventWrapper;
	
	FTestData1ComponentSnapshot* Snapshot;

    void OnComponentUpdateDispatcherCallback(UComponentUpdateOpWrapperBase& Op);

	void OnAuthorityChangeDispatcherCallback(const worker::AuthorityChangeOp& Op) override;

	void ReplicateChangesInternal();
	void SendComponentUpdateInternal(UTestData1ComponentUpdate* update);
    void ApplyComponentUpdate(const test::TestData1::Update& update);
	void NotifyUpdate(const test::TestData1::Update& update);


    void OnRequestTestdata2CommandRequestDispatcherCallback(
        const worker::CommandRequestOp<test::TestData1::Commands::RequestTestdata2>& op);

};
