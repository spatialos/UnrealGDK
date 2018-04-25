// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "SpatialOsComponent.h"
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "ComponentWithSimilarlyNamedPropertyAndEventComponentUpdate.h"
#include "ComponentWithSimilarlyNamedPropertyAndEventAddComponentOp.h"
#include "ComponentId.h"
#include "SpatialOSViewTypes.h"
#include "SpatialOSWorkerTypes.h"
#include "TestSchema.h"


#include "TestType1.h"

#include "ComponentWithSimilarlyNamedPropertyAndEventComponent.generated.h"

class UCallbackDispatcher;
class UComponentUpdateOpWrapperBase;

USTRUCT()
struct FComponentWithSimilarlyNamedPropertyAndEventComponentSnapshot
{
	GENERATED_BODY()

	FComponentWithSimilarlyNamedPropertyAndEventComponentSnapshot()
	{
	}

	int MyValue;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SPATIALOS_API UComponentWithSimilarlyNamedPropertyAndEventComponent : public USpatialOsComponent
{
    GENERATED_BODY()

public:
    UComponentWithSimilarlyNamedPropertyAndEventComponent();
	virtual ~UComponentWithSimilarlyNamedPropertyAndEventComponent() override = default;

    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "ComponentWithSimilarlyNamedPropertyAndEventComponent")
    FComponentId GetComponentId();

    const static worker::ComponentId ComponentId;

    void Init(const TWeakPtr<SpatialOSConnection>& InConnection, const TWeakPtr<SpatialOSView>& InView, worker::EntityId InEntityId, UCallbackDispatcher* InCallbackDispatcher) override;

	void Disable(const worker::EntityId InEntityId, UCallbackDispatcher* CallbackDispatcher);

	void ApplyInitialState(const UAddComponentOpWrapperBase& Op) override;

	DEPRECATED(12.1, "Please use TriggerAutomaticComponentUpdate.")
	void ReplicateChanges(float DeltaSeconds) override;

	void TriggerAutomaticComponentUpdate(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "ComponentWithSimilarlyNamedPropertyAndEventComponent")
	void TriggerManualComponentUpdate();

    UPROPERTY(BlueprintAssignable, Category = "ComponentWithSimilarlyNamedPropertyAndEventComponent")
    FSpatialComponentUpdated OnMyValueUpdate;

    UPROPERTY(BlueprintAssignable, Category = "ComponentWithSimilarlyNamedPropertyAndEventComponent")
    FTestType1Delegate OnMyValueUpdate;
	UFUNCTION(BlueprintCallable, Category = "ComponentWithSimilarlyNamedPropertyAndEventComponent")
    void MyValueUpdate(UTestType1* Data);


	DEPRECATED(12.1, "This function is deprecated, the component updates are now sent automatically. Use MaxUpdatesPerSecond property to control the update frequency, or set it to 0 and use TriggerManualComponentUpdate for more control.")
    UFUNCTION(BlueprintCallable, Category = "ComponentWithSimilarlyNamedPropertyAndEventComponent", meta=(DeprecatedFunction, DeprecationMessage="This function is deprecated, the component updates are now sent automatically. Use MaxUpdatesPerSecond property to control the update frequency, or set it to 0 and use TriggerManualComponentUpdate for more control."))
    void SendComponentUpdate(UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate* update);

    UPROPERTY(BlueprintAssignable, Category = "ComponentWithSimilarlyNamedPropertyAndEventComponent")
    FSpatialComponentUpdated OnComponentUpdate;

	DEPRECATED(12.0, "This function is deprecated, access the MyValue property directly.")
    UFUNCTION(BlueprintPure, Category = "ComponentWithSimilarlyNamedPropertyAndEventComponent", meta=(DeprecatedFunction, DeprecationMessage="This function is deprecated, access the MyValue property directly."))
    int GetMyValue();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int MyValue;

private:
	void GenerateSnapshot();

	UPROPERTY()
	UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate* ComponentUpdater;

	UPROPERTY()
	UTestType1* MyValueUpdateWrapper;
	
	FComponentWithSimilarlyNamedPropertyAndEventComponentSnapshot* Snapshot;

    void OnComponentUpdateDispatcherCallback(UComponentUpdateOpWrapperBase& Op);

	void OnAuthorityChangeDispatcherCallback(const worker::AuthorityChangeOp& Op) override;

	void ReplicateChangesInternal();
	void SendComponentUpdateInternal(UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate* update);
    void ApplyComponentUpdate(const test::ComponentWithSimilarlyNamedPropertyAndEvent::Update& update);
	void NotifyUpdate(const test::ComponentWithSimilarlyNamedPropertyAndEvent::Update& update);

};
