// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "SpatialOsComponent.h"
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "TestEmptyComponentUpdate.h"
#include "TestEmptyAddComponentOp.h"
#include "ComponentId.h"
#include "SpatialOSViewTypes.h"
#include "SpatialOSWorkerTypes.h"
#include "TestSchema.h"



#include "TestEmptyComponent.generated.h"

class UCallbackDispatcher;
class UComponentUpdateOpWrapperBase;

USTRUCT()
struct FTestEmptyComponentSnapshot
{
	GENERATED_BODY()

	FTestEmptyComponentSnapshot()
	{
	}

};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SPATIALOS_API UTestEmptyComponent : public USpatialOsComponent
{
    GENERATED_BODY()

public:
    UTestEmptyComponent();
	virtual ~UTestEmptyComponent() override = default;

    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "TestEmptyComponent")
    FComponentId GetComponentId();

    const static worker::ComponentId ComponentId;

    void Init(const TWeakPtr<SpatialOSConnection>& InConnection, const TWeakPtr<SpatialOSView>& InView, worker::EntityId InEntityId, UCallbackDispatcher* InCallbackDispatcher) override;

	void Disable(const worker::EntityId InEntityId, UCallbackDispatcher* CallbackDispatcher);

	void ApplyInitialState(const UAddComponentOpWrapperBase& Op) override;

	DEPRECATED(12.1, "Please use TriggerAutomaticComponentUpdate.")
	void ReplicateChanges(float DeltaSeconds) override;

	void TriggerAutomaticComponentUpdate(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "TestEmptyComponent")
	void TriggerManualComponentUpdate();



	DEPRECATED(12.1, "This function is deprecated, the component updates are now sent automatically. Use MaxUpdatesPerSecond property to control the update frequency, or set it to 0 and use TriggerManualComponentUpdate for more control.")
    UFUNCTION(BlueprintCallable, Category = "TestEmptyComponent", meta=(DeprecatedFunction, DeprecationMessage="This function is deprecated, the component updates are now sent automatically. Use MaxUpdatesPerSecond property to control the update frequency, or set it to 0 and use TriggerManualComponentUpdate for more control."))
    void SendComponentUpdate(UTestEmptyComponentUpdate* update);

    UPROPERTY(BlueprintAssignable, Category = "TestEmptyComponent")
    FSpatialComponentUpdated OnComponentUpdate;



private:
	void GenerateSnapshot();

	UPROPERTY()
	UTestEmptyComponentUpdate* ComponentUpdater;


	FTestEmptyComponentSnapshot* Snapshot;

    void OnComponentUpdateDispatcherCallback(UComponentUpdateOpWrapperBase& Op);

	void OnAuthorityChangeDispatcherCallback(const worker::AuthorityChangeOp& Op) override;

	void ReplicateChangesInternal();
	void SendComponentUpdateInternal(UTestEmptyComponentUpdate* update);
    void ApplyComponentUpdate(const test::TestEmpty::Update& update);
	void NotifyUpdate(const test::TestEmpty::Update& update);

};
