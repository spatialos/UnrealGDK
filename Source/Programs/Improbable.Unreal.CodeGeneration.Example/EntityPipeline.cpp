// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#include "EntityPipeline.h"
#include "EntityPipelineBlock.h"
#include "CoreMinimal.h"

#include "SpatialOSViewTypes.h"
#include "SpatialOSWorkerTypes.h"
#include "CallbackDispatcher.h"
#include "TestData1AddComponentOp.h"
#include "TestData2AddComponentOp.h"
#include "BuiltInTypesAddComponentOp.h"
#include "TestFieldTypesAddComponentOp.h"
#include "TestEmptyAddComponentOp.h"
#include "TestCommandResponseTypesAddComponentOp.h"
#include "ComponentWithSimilarlyNamedPropertyAndEventAddComponentOp.h"

/**
*
*/

DECLARE_LOG_CATEGORY_EXTERN(LogEntityPipeline, Log, All);
DEFINE_LOG_CATEGORY(LogEntityPipeline);

UEntityPipeline::UEntityPipeline()
: FirstBlock(nullptr)
, LastBlock(nullptr)
, CallbackDispatcher(nullptr)
, bInitialised(false)
{
}

void UEntityPipeline::Init(const TWeakPtr<SpatialOSView>& InView, UCallbackDispatcher* InCallbackDispatcher)
{
	checkf(!bInitialised, TEXT("Attempting to call Init more than once!"));
	checkf(FirstBlock, TEXT("Trying to bind callbacks but no blocks have been added!"));

	Callbacks.Init(InView);
	CallbackDispatcher = InCallbackDispatcher;
	CallbackDispatcher->OnAuthorityChangeOpReceived.AddUObject(this, &UEntityPipeline::OnAuthorityChange);

	auto LockedView = InView.Pin();
	if (LockedView.IsValid())
	{
		Callbacks.Add(LockedView->OnAddEntity(
			std::bind(&UEntityPipeline::OnAddEntity, this, std::placeholders::_1)));

		Callbacks.Add(LockedView->OnRemoveEntity(
			std::bind(&UEntityPipeline::OnRemoveEntity, this, std::placeholders::_1)));

		Callbacks.Add(LockedView->OnAddComponent<test::TestData1>(
			std::bind(&UEntityPipeline::AddTestData1ComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAddComponent<test::TestData2>(
			std::bind(&UEntityPipeline::AddTestData2ComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAddComponent<test::BuiltInTypes>(
			std::bind(&UEntityPipeline::AddBuiltInTypesComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAddComponent<test::TestFieldTypes>(
			std::bind(&UEntityPipeline::AddTestFieldTypesComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAddComponent<test::TestEmpty>(
			std::bind(&UEntityPipeline::AddTestEmptyComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAddComponent<test::TestCommandResponseTypes>(
			std::bind(&UEntityPipeline::AddTestCommandResponseTypesComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAddComponent<test::ComponentWithSimilarlyNamedPropertyAndEvent>(
			std::bind(&UEntityPipeline::AddComponentWithSimilarlyNamedPropertyAndEventComponentOp, this, std::placeholders::_1)));

		Callbacks.Add(LockedView->OnRemoveComponent<test::TestData1>(
			std::bind(&UEntityPipeline::RemoveTestData1ComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnRemoveComponent<test::TestData2>(
			std::bind(&UEntityPipeline::RemoveTestData2ComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnRemoveComponent<test::BuiltInTypes>(
			std::bind(&UEntityPipeline::RemoveBuiltInTypesComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnRemoveComponent<test::TestFieldTypes>(
			std::bind(&UEntityPipeline::RemoveTestFieldTypesComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnRemoveComponent<test::TestEmpty>(
			std::bind(&UEntityPipeline::RemoveTestEmptyComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnRemoveComponent<test::TestCommandResponseTypes>(
			std::bind(&UEntityPipeline::RemoveTestCommandResponseTypesComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnRemoveComponent<test::ComponentWithSimilarlyNamedPropertyAndEvent>(
			std::bind(&UEntityPipeline::RemoveComponentWithSimilarlyNamedPropertyAndEventComponentOp, this, std::placeholders::_1)));

	}
	bInitialised = true;
}

void UEntityPipeline::DeregisterAllCallbacks()
{
	if (CallbackDispatcher != nullptr)
	{
		CallbackDispatcher->Reset();
		CallbackDispatcher = nullptr;
	}
	Callbacks.Reset();
	bInitialised = false;
}

void UEntityPipeline::AddBlock(UEntityPipelineBlock* NewBlock)
{
    checkf(!bInitialised, TEXT("Cannot add blocks after the pipeline has started"));

    if(FirstBlock == nullptr)
	{
		FirstBlock = NewBlock;
	}
	if(LastBlock != nullptr)
	{
		LastBlock->NextBlock = NewBlock;
	}
	LastBlock = NewBlock;
}

void UEntityPipeline::ProcessOps(const TWeakPtr<SpatialOSView>& InView, const TWeakPtr<SpatialOSConnection>& InConnection, UWorld* World)
{
	auto Block = FirstBlock;

	while (Block != nullptr)
	{
		Block->ProcessOps(InView, InConnection, World, CallbackDispatcher);
		Block = Block->NextBlock;
	}
}

void UEntityPipeline::AddTestData1ComponentOp(const worker::AddComponentOp<test::TestData1>& Op) {
	UTestData1AddComponentOp* NewOp = NewObject<UTestData1AddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = test::TestData1::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemoveTestData1ComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = test::TestData1::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
void UEntityPipeline::AddTestData2ComponentOp(const worker::AddComponentOp<test::TestData2>& Op) {
	UTestData2AddComponentOp* NewOp = NewObject<UTestData2AddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = test::TestData2::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemoveTestData2ComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = test::TestData2::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
void UEntityPipeline::AddBuiltInTypesComponentOp(const worker::AddComponentOp<test::BuiltInTypes>& Op) {
	UBuiltInTypesAddComponentOp* NewOp = NewObject<UBuiltInTypesAddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = test::BuiltInTypes::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemoveBuiltInTypesComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = test::BuiltInTypes::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
void UEntityPipeline::AddTestFieldTypesComponentOp(const worker::AddComponentOp<test::TestFieldTypes>& Op) {
	UTestFieldTypesAddComponentOp* NewOp = NewObject<UTestFieldTypesAddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = test::TestFieldTypes::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemoveTestFieldTypesComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = test::TestFieldTypes::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
void UEntityPipeline::AddTestEmptyComponentOp(const worker::AddComponentOp<test::TestEmpty>& Op) {
	UTestEmptyAddComponentOp* NewOp = NewObject<UTestEmptyAddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = test::TestEmpty::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemoveTestEmptyComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = test::TestEmpty::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
void UEntityPipeline::AddTestCommandResponseTypesComponentOp(const worker::AddComponentOp<test::TestCommandResponseTypes>& Op) {
	UTestCommandResponseTypesAddComponentOp* NewOp = NewObject<UTestCommandResponseTypesAddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = test::TestCommandResponseTypes::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemoveTestCommandResponseTypesComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = test::TestCommandResponseTypes::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
void UEntityPipeline::AddComponentWithSimilarlyNamedPropertyAndEventComponentOp(const worker::AddComponentOp<test::ComponentWithSimilarlyNamedPropertyAndEvent>& Op) {
	UComponentWithSimilarlyNamedPropertyAndEventAddComponentOp* NewOp = NewObject<UComponentWithSimilarlyNamedPropertyAndEventAddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = test::ComponentWithSimilarlyNamedPropertyAndEvent::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemoveComponentWithSimilarlyNamedPropertyAndEventComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = test::ComponentWithSimilarlyNamedPropertyAndEvent::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
