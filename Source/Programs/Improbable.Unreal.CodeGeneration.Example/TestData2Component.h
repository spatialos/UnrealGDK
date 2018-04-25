// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "SpatialOsComponent.h"
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "TestData2ComponentUpdate.h"
#include "TestData2AddComponentOp.h"
#include "ComponentId.h"
#include "SpatialOSViewTypes.h"
#include "SpatialOSWorkerTypes.h"
#include "TestSchema.h"

#include "RequestTestdata1CommandResponder.h"

#include "TextEvent.h"

#include "TestData2Component.generated.h"

class UCallbackDispatcher;
class UComponentUpdateOpWrapperBase;

USTRUCT()
struct FTestData2ComponentSnapshot
{
	GENERATED_BODY()

	FTestData2ComponentSnapshot()
	{
	}

	float DoubleProperty;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SPATIALOS_API UTestData2Component : public USpatialOsComponent
{
    GENERATED_BODY()

public:
    UTestData2Component();
	virtual ~UTestData2Component() override = default;

    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "TestData2Component")
    FComponentId GetComponentId();

    const static worker::ComponentId ComponentId;

    void Init(const TWeakPtr<SpatialOSConnection>& InConnection, const TWeakPtr<SpatialOSView>& InView, worker::EntityId InEntityId, UCallbackDispatcher* InCallbackDispatcher) override;

	void Disable(const worker::EntityId InEntityId, UCallbackDispatcher* CallbackDispatcher);

	void ApplyInitialState(const UAddComponentOpWrapperBase& Op) override;

	DEPRECATED(12.1, "Please use TriggerAutomaticComponentUpdate.")
	void ReplicateChanges(float DeltaSeconds) override;

	void TriggerAutomaticComponentUpdate(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "TestData2Component")
	void TriggerManualComponentUpdate();

    UPROPERTY(BlueprintAssignable, Category = "TestData2Component")
    FSpatialComponentUpdated OnDoublePropertyUpdate;

    UPROPERTY(BlueprintAssignable, Category = "TestData2Component")
    FTextEventDelegate OnTwoTextEvent;
	UFUNCTION(BlueprintCallable, Category = "TestData2Component")
    void TwoTextEvent(UTextEvent* Data);

    UPROPERTY(BlueprintAssignable, Category = "TestData2Component")
    FRequestTestdata1Command OnRequestTestdata1CommandRequest;

	DEPRECATED(12.1, "This function is deprecated, the component updates are now sent automatically. Use MaxUpdatesPerSecond property to control the update frequency, or set it to 0 and use TriggerManualComponentUpdate for more control.")
    UFUNCTION(BlueprintCallable, Category = "TestData2Component", meta=(DeprecatedFunction, DeprecationMessage="This function is deprecated, the component updates are now sent automatically. Use MaxUpdatesPerSecond property to control the update frequency, or set it to 0 and use TriggerManualComponentUpdate for more control."))
    void SendComponentUpdate(UTestData2ComponentUpdate* update);

    UPROPERTY(BlueprintAssignable, Category = "TestData2Component")
    FSpatialComponentUpdated OnComponentUpdate;

	DEPRECATED(12.0, "This function is deprecated, access the DoubleProperty property directly.")
    UFUNCTION(BlueprintPure, Category = "TestData2Component", meta=(DeprecatedFunction, DeprecationMessage="This function is deprecated, access the DoubleProperty property directly."))
    float GetDoubleProperty();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DoubleProperty;

private:
	void GenerateSnapshot();

	UPROPERTY()
	UTestData2ComponentUpdate* ComponentUpdater;

	UPROPERTY()
	UTextEvent* TwoTextEventWrapper;
	
	FTestData2ComponentSnapshot* Snapshot;

    void OnComponentUpdateDispatcherCallback(UComponentUpdateOpWrapperBase& Op);

	void OnAuthorityChangeDispatcherCallback(const worker::AuthorityChangeOp& Op) override;

	void ReplicateChangesInternal();
	void SendComponentUpdateInternal(UTestData2ComponentUpdate* update);
    void ApplyComponentUpdate(const test::TestData2::Update& update);
	void NotifyUpdate(const test::TestData2::Update& update);


    void OnRequestTestdata1CommandRequestDispatcherCallback(
        const worker::CommandRequestOp<test::TestData2::Commands::RequestTestdata1>& op);

};
