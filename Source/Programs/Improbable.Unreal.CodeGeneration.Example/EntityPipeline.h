// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "SpatialOSViewTypes.h"
#include "SpatialOSWorkerTypes.h"
#include "EntityPipelineBlock.h"
#include "ScopedViewCallbacks.h"
#include "TestData1AddComponentOp.h"
#include "TestData2AddComponentOp.h"
#include "BuiltInTypesAddComponentOp.h"
#include "TestFieldTypesAddComponentOp.h"
#include "TestEmptyAddComponentOp.h"
#include "TestCommandResponseTypesAddComponentOp.h"
#include "ComponentWithSimilarlyNamedPropertyAndEventAddComponentOp.h"
#include "EntityPipeline.generated.h"

UCLASS()
class SPATIALOS_API UEntityPipeline : public UObject
{
	GENERATED_BODY()

public:
	UEntityPipeline();

	/**
	* Initialise the UEntityPipeline. Calling Init() more than once results in an error.
	*/
	void Init(const TWeakPtr<SpatialOSView>& InView, UCallbackDispatcher* InCallbackDispatcher);

	/**
	* Deregister all callbacks. Init() may be called again after this method is called.
	*/
	void DeregisterAllCallbacks();

	void AddBlock(UEntityPipelineBlock* NewBlock);
	void ProcessOps(const TWeakPtr<SpatialOSView>& InView, const TWeakPtr<SpatialOSConnection>& InConnection, UWorld* World);

	void OnAddEntity(const worker::AddEntityOp& Op) { FirstBlock->AddEntity(Op); }
	void OnRemoveEntity(const worker::RemoveEntityOp& Op) { FirstBlock->RemoveEntity(Op); }
	void OnRemoveComponent(const worker::ComponentId ComponentId, const worker::RemoveComponentOp& Op) { FirstBlock->RemoveComponent(ComponentId, Op); }
	void OnAuthorityChange(const worker::ComponentId ComponentId, const worker::AuthorityChangeOp& Op) { FirstBlock->ChangeAuthority(ComponentId, Op); }

	void AddTestData1ComponentOp(const worker::AddComponentOp<test::TestData1>& Op);
	void RemoveTestData1ComponentOp(const worker::RemoveComponentOp& Op);
	void AddTestData2ComponentOp(const worker::AddComponentOp<test::TestData2>& Op);
	void RemoveTestData2ComponentOp(const worker::RemoveComponentOp& Op);
	void AddBuiltInTypesComponentOp(const worker::AddComponentOp<test::BuiltInTypes>& Op);
	void RemoveBuiltInTypesComponentOp(const worker::RemoveComponentOp& Op);
	void AddTestFieldTypesComponentOp(const worker::AddComponentOp<test::TestFieldTypes>& Op);
	void RemoveTestFieldTypesComponentOp(const worker::RemoveComponentOp& Op);
	void AddTestEmptyComponentOp(const worker::AddComponentOp<test::TestEmpty>& Op);
	void RemoveTestEmptyComponentOp(const worker::RemoveComponentOp& Op);
	void AddTestCommandResponseTypesComponentOp(const worker::AddComponentOp<test::TestCommandResponseTypes>& Op);
	void RemoveTestCommandResponseTypesComponentOp(const worker::RemoveComponentOp& Op);
	void AddComponentWithSimilarlyNamedPropertyAndEventComponentOp(const worker::AddComponentOp<test::ComponentWithSimilarlyNamedPropertyAndEvent>& Op);
	void RemoveComponentWithSimilarlyNamedPropertyAndEventComponentOp(const worker::RemoveComponentOp& Op);

private:
	UPROPERTY()
	UEntityPipelineBlock* FirstBlock;
	UPROPERTY()
	UEntityPipelineBlock* LastBlock;
	UPROPERTY()
	UCallbackDispatcher* CallbackDispatcher;

	bool bInitialised;
	improbable::unreal::callbacks::FScopedViewCallbacks Callbacks;
};
