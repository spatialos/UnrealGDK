// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "UObject/Class.h"
#include "SpatialOSViewTypes.h"
#include "SpatialOSWorkerTypes.h"
#include "ComponentIdentifier.h"
#include "ComponentUpdateQueue.h"
#include "TestSchema.h"
#include "ScopedViewCallbacks.h"
#include "SpatialOS.h"
#include "CallbackDispatcher.generated.h"

class UComponentUpdateOpWrapperBase;

DECLARE_EVENT_TwoParams( UCallbackDispatcher, FAuthorityChangeOpReceivedEvent, worker::ComponentId, const worker::AuthorityChangeOp&);

UCLASS()
class SPATIALOS_API UCallbackDispatcher : public UObject
{
	GENERATED_BODY()

public:
	UCallbackDispatcher();

	/**
	* Initialise the UCallbackDispatcher. Calling Init() more than once results in an error.
	*/
	void Init(const TWeakPtr<SpatialOSView>& InView);

	/**
	* Reset the UCallbackDispatcher to its initial state. Init() may be called again after this method is called.
	*/
	void Reset();

	void AddComponentUpdateCallback(worker::EntityId EntityId, worker::ComponentId ComponentId, TFunction<void(UComponentUpdateOpWrapperBase&)> Callback);
	void AddAuthorityChangeCallback(worker::EntityId EntityId, worker::ComponentId ComponentId, TFunction<void(const worker::AuthorityChangeOp&)> Callback);

	void RemoveComponentUpdateCallback(worker::EntityId EntityId, worker::ComponentId ComponentId);
	void RemoveAuthorityChangeCallback(worker::EntityId EntityId, worker::ComponentId ComponentId);

	FAuthorityChangeOpReceivedEvent OnAuthorityChangeOpReceived;

	void OnTestData1ComponentUpdateOp(const worker::ComponentUpdateOp<test::TestData1>& Op);
	void OnTestData1AuthorityChangeOp(const worker::AuthorityChangeOp& Op);

	void OnTestData2ComponentUpdateOp(const worker::ComponentUpdateOp<test::TestData2>& Op);
	void OnTestData2AuthorityChangeOp(const worker::AuthorityChangeOp& Op);

	void OnBuiltInTypesComponentUpdateOp(const worker::ComponentUpdateOp<test::BuiltInTypes>& Op);
	void OnBuiltInTypesAuthorityChangeOp(const worker::AuthorityChangeOp& Op);

	void OnTestFieldTypesComponentUpdateOp(const worker::ComponentUpdateOp<test::TestFieldTypes>& Op);
	void OnTestFieldTypesAuthorityChangeOp(const worker::AuthorityChangeOp& Op);

	void OnTestEmptyComponentUpdateOp(const worker::ComponentUpdateOp<test::TestEmpty>& Op);
	void OnTestEmptyAuthorityChangeOp(const worker::AuthorityChangeOp& Op);

	void OnTestCommandResponseTypesComponentUpdateOp(const worker::ComponentUpdateOp<test::TestCommandResponseTypes>& Op);
	void OnTestCommandResponseTypesAuthorityChangeOp(const worker::AuthorityChangeOp& Op);

	void OnComponentWithSimilarlyNamedPropertyAndEventComponentUpdateOp(const worker::ComponentUpdateOp<test::ComponentWithSimilarlyNamedPropertyAndEvent>& Op);
	void OnComponentWithSimilarlyNamedPropertyAndEventAuthorityChangeOp(const worker::AuthorityChangeOp& Op);


private:
	TMap<FComponentIdentifier, TFunction<void(UComponentUpdateOpWrapperBase&)>> ComponentUpdateCallbacks;
	TMap<FComponentIdentifier, TFunction<void(const worker::AuthorityChangeOp&)>> AuthorityChangeCallbacks;
	
	UPROPERTY()
	TMap<FComponentIdentifier, FComponentUpdateQueue> QueuedUpdates;
	
	void HandleUpdate(UComponentUpdateOpWrapperBase* Update);
	void EnqueueUpdate(const FComponentIdentifier& Id, UComponentUpdateOpWrapperBase* Update);
	void DispatchQueuedUpdates(const FComponentIdentifier& Id);

	improbable::unreal::callbacks::FScopedViewCallbacks Callbacks;
	bool bInitialised;
};