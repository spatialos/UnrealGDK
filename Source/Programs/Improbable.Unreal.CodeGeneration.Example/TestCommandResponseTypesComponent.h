// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "SpatialOsComponent.h"
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "TestCommandResponseTypesComponentUpdate.h"
#include "TestCommandResponseTypesAddComponentOp.h"
#include "ComponentId.h"
#include "SpatialOSViewTypes.h"
#include "SpatialOSWorkerTypes.h"
#include "TestSchema.h"

#include "TestCommandUserTypeCommandResponder.h"
#include "TestCommandCoordinatesCommandResponder.h"
#include "TestCommandVector3dCommandResponder.h"
#include "TestCommandVector3fCommandResponder.h"


#include "TestCommandResponseTypesComponent.generated.h"

class UCallbackDispatcher;
class UComponentUpdateOpWrapperBase;

USTRUCT()
struct FTestCommandResponseTypesComponentSnapshot
{
	GENERATED_BODY()

	FTestCommandResponseTypesComponentSnapshot()
	{
	}

};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SPATIALOS_API UTestCommandResponseTypesComponent : public USpatialOsComponent
{
    GENERATED_BODY()

public:
    UTestCommandResponseTypesComponent();
	virtual ~UTestCommandResponseTypesComponent() override = default;

    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "TestCommandResponseTypesComponent")
    FComponentId GetComponentId();

    const static worker::ComponentId ComponentId;

    void Init(const TWeakPtr<SpatialOSConnection>& InConnection, const TWeakPtr<SpatialOSView>& InView, worker::EntityId InEntityId, UCallbackDispatcher* InCallbackDispatcher) override;

	void Disable(const worker::EntityId InEntityId, UCallbackDispatcher* CallbackDispatcher);

	void ApplyInitialState(const UAddComponentOpWrapperBase& Op) override;

	DEPRECATED(12.1, "Please use TriggerAutomaticComponentUpdate.")
	void ReplicateChanges(float DeltaSeconds) override;

	void TriggerAutomaticComponentUpdate(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "TestCommandResponseTypesComponent")
	void TriggerManualComponentUpdate();


    UPROPERTY(BlueprintAssignable, Category = "TestCommandResponseTypesComponent")
    FTestCommandUserTypeCommand OnTestCommandUserTypeCommandRequest;
    UPROPERTY(BlueprintAssignable, Category = "TestCommandResponseTypesComponent")
    FTestCommandCoordinatesCommand OnTestCommandCoordinatesCommandRequest;
    UPROPERTY(BlueprintAssignable, Category = "TestCommandResponseTypesComponent")
    FTestCommandVector3dCommand OnTestCommandVector3dCommandRequest;
    UPROPERTY(BlueprintAssignable, Category = "TestCommandResponseTypesComponent")
    FTestCommandVector3fCommand OnTestCommandVector3fCommandRequest;

	DEPRECATED(12.1, "This function is deprecated, the component updates are now sent automatically. Use MaxUpdatesPerSecond property to control the update frequency, or set it to 0 and use TriggerManualComponentUpdate for more control.")
    UFUNCTION(BlueprintCallable, Category = "TestCommandResponseTypesComponent", meta=(DeprecatedFunction, DeprecationMessage="This function is deprecated, the component updates are now sent automatically. Use MaxUpdatesPerSecond property to control the update frequency, or set it to 0 and use TriggerManualComponentUpdate for more control."))
    void SendComponentUpdate(UTestCommandResponseTypesComponentUpdate* update);

    UPROPERTY(BlueprintAssignable, Category = "TestCommandResponseTypesComponent")
    FSpatialComponentUpdated OnComponentUpdate;



private:
	void GenerateSnapshot();

	UPROPERTY()
	UTestCommandResponseTypesComponentUpdate* ComponentUpdater;


	FTestCommandResponseTypesComponentSnapshot* Snapshot;

    void OnComponentUpdateDispatcherCallback(UComponentUpdateOpWrapperBase& Op);

	void OnAuthorityChangeDispatcherCallback(const worker::AuthorityChangeOp& Op) override;

	void ReplicateChangesInternal();
	void SendComponentUpdateInternal(UTestCommandResponseTypesComponentUpdate* update);
    void ApplyComponentUpdate(const test::TestCommandResponseTypes::Update& update);
	void NotifyUpdate(const test::TestCommandResponseTypes::Update& update);


    void OnTestCommandUserTypeCommandRequestDispatcherCallback(
        const worker::CommandRequestOp<test::TestCommandResponseTypes::Commands::TestCommandUserType>& op);


    void OnTestCommandCoordinatesCommandRequestDispatcherCallback(
        const worker::CommandRequestOp<test::TestCommandResponseTypes::Commands::TestCommandCoordinates>& op);


    void OnTestCommandVector3dCommandRequestDispatcherCallback(
        const worker::CommandRequestOp<test::TestCommandResponseTypes::Commands::TestCommandVector3d>& op);


    void OnTestCommandVector3fCommandRequestDispatcherCallback(
        const worker::CommandRequestOp<test::TestCommandResponseTypes::Commands::TestCommandVector3f>& op);

};
