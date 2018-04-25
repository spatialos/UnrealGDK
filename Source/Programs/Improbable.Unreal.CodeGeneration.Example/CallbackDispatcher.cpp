// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 


#include "CallbackDispatcher.h"
#include "ComponentUpdateOpWrapperBase.h"
#include "TestData1ComponentUpdate.h"
#include "TestData2ComponentUpdate.h"
#include "BuiltInTypesComponentUpdate.h"
#include "TestFieldTypesComponentUpdate.h"
#include "TestEmptyComponentUpdate.h"
#include "TestCommandResponseTypesComponentUpdate.h"
#include "ComponentWithSimilarlyNamedPropertyAndEventComponentUpdate.h"
#include "ScopedViewCallbacks.h"
#include "SpatialOSViewTypes.h"


DECLARE_LOG_CATEGORY_EXTERN(LogCallbackDispatcher, Log, All);
DEFINE_LOG_CATEGORY(LogCallbackDispatcher);

UCallbackDispatcher::UCallbackDispatcher() : bInitialised(false)
{
}

void UCallbackDispatcher::Init(const TWeakPtr<SpatialOSView>& InView)
{
	checkf(!bInitialised, TEXT("Attempting to call Init more than once!"));
	Callbacks.Init(InView);

	auto LockedView = InView.Pin();
	if(LockedView.IsValid())
	{
		Callbacks.Add(LockedView->OnComponentUpdate<test::TestData1>(std::bind(
			&UCallbackDispatcher::OnTestData1ComponentUpdateOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAuthorityChange<test::TestData1>(std::bind(
			&UCallbackDispatcher::OnTestData1AuthorityChangeOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnComponentUpdate<test::TestData2>(std::bind(
			&UCallbackDispatcher::OnTestData2ComponentUpdateOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAuthorityChange<test::TestData2>(std::bind(
			&UCallbackDispatcher::OnTestData2AuthorityChangeOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnComponentUpdate<test::BuiltInTypes>(std::bind(
			&UCallbackDispatcher::OnBuiltInTypesComponentUpdateOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAuthorityChange<test::BuiltInTypes>(std::bind(
			&UCallbackDispatcher::OnBuiltInTypesAuthorityChangeOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnComponentUpdate<test::TestFieldTypes>(std::bind(
			&UCallbackDispatcher::OnTestFieldTypesComponentUpdateOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAuthorityChange<test::TestFieldTypes>(std::bind(
			&UCallbackDispatcher::OnTestFieldTypesAuthorityChangeOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnComponentUpdate<test::TestEmpty>(std::bind(
			&UCallbackDispatcher::OnTestEmptyComponentUpdateOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAuthorityChange<test::TestEmpty>(std::bind(
			&UCallbackDispatcher::OnTestEmptyAuthorityChangeOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnComponentUpdate<test::TestCommandResponseTypes>(std::bind(
			&UCallbackDispatcher::OnTestCommandResponseTypesComponentUpdateOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAuthorityChange<test::TestCommandResponseTypes>(std::bind(
			&UCallbackDispatcher::OnTestCommandResponseTypesAuthorityChangeOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnComponentUpdate<test::ComponentWithSimilarlyNamedPropertyAndEvent>(std::bind(
			&UCallbackDispatcher::OnComponentWithSimilarlyNamedPropertyAndEventComponentUpdateOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAuthorityChange<test::ComponentWithSimilarlyNamedPropertyAndEvent>(std::bind(
			&UCallbackDispatcher::OnComponentWithSimilarlyNamedPropertyAndEventAuthorityChangeOp, this, std::placeholders::_1)));
	}
	bInitialised = true;
}

void UCallbackDispatcher::Reset()
{
	ComponentUpdateCallbacks.Reset();
	AuthorityChangeCallbacks.Reset();
	QueuedUpdates.Reset();
	Callbacks.Reset();
	bInitialised = false;
}

void UCallbackDispatcher::AddComponentUpdateCallback(worker::EntityId EntityId, worker::ComponentId ComponentId, TFunction<void(UComponentUpdateOpWrapperBase&)> Callback)
{
	FComponentIdentifier Id{ EntityId, ComponentId };
	ComponentUpdateCallbacks.Emplace(Id, Callback);
	DispatchQueuedUpdates(Id);
}

void UCallbackDispatcher::AddAuthorityChangeCallback(worker::EntityId EntityId, worker::ComponentId ComponentId, TFunction<void(const worker::AuthorityChangeOp&)> Callback)
{
	AuthorityChangeCallbacks.Emplace(FComponentIdentifier{EntityId, ComponentId}, Callback);
}

void UCallbackDispatcher::RemoveComponentUpdateCallback(worker::EntityId EntityId, worker::ComponentId ComponentId)
{
	ComponentUpdateCallbacks.Remove(FComponentIdentifier{EntityId, ComponentId});
}

void UCallbackDispatcher::RemoveAuthorityChangeCallback(worker::EntityId EntityId, worker::ComponentId ComponentId)
{
	AuthorityChangeCallbacks.Remove(FComponentIdentifier{EntityId, ComponentId});
}

void UCallbackDispatcher::HandleUpdate(UComponentUpdateOpWrapperBase* Update)
{
	FComponentIdentifier Id{ Update->EntityId, Update->ComponentId };

	auto Callback = ComponentUpdateCallbacks.Find(Id);
	if (Callback != nullptr && (*Callback) != nullptr)
	{
		(*Callback)(*Update);
	}
	else
	{
		EnqueueUpdate(Id, Update);
	}
}

void UCallbackDispatcher::EnqueueUpdate(const FComponentIdentifier& Id, UComponentUpdateOpWrapperBase* Update)
{
	FComponentUpdateQueue* Queue = QueuedUpdates.Find(Id);
	if (!Queue)
	{
		Queue = &QueuedUpdates.Emplace(Id, FComponentUpdateQueue());
	}
	Queue->AddToQueue(Update);
}

void UCallbackDispatcher::DispatchQueuedUpdates(const FComponentIdentifier& Id)
{
	FComponentUpdateQueue* PendingUpdateQueue = QueuedUpdates.Find(Id);
	if (PendingUpdateQueue)
	{
		auto Callback = ComponentUpdateCallbacks.Find(Id);
		if (Callback != nullptr && (*Callback) != nullptr)
		{
			for (auto QueuedUpdate : PendingUpdateQueue->GetQueue())
			{
				(*Callback)(*QueuedUpdate);
			}
		}
		QueuedUpdates.Remove(Id);
	}
}

void UCallbackDispatcher::OnTestData1ComponentUpdateOp(const worker::ComponentUpdateOp<test::TestData1>& Op) {
	UTestData1ComponentUpdate* WrappedUpdate = NewObject<UTestData1ComponentUpdate>();
	if (WrappedUpdate)
	{
		WrappedUpdate->InitInternal(Op.Update);
		WrappedUpdate->ComponentId = test::TestData1::ComponentId;
		WrappedUpdate->EntityId = Op.EntityId;

		HandleUpdate(WrappedUpdate);
	}
}

void UCallbackDispatcher::OnTestData1AuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
	const worker::ComponentId ComponentId = test::TestData1::ComponentId;

	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
	if(Callback != nullptr && (*Callback) != nullptr)
	{
		(*Callback)(Op);
	}

	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
}

void UCallbackDispatcher::OnTestData2ComponentUpdateOp(const worker::ComponentUpdateOp<test::TestData2>& Op) {
	UTestData2ComponentUpdate* WrappedUpdate = NewObject<UTestData2ComponentUpdate>();
	if (WrappedUpdate)
	{
		WrappedUpdate->InitInternal(Op.Update);
		WrappedUpdate->ComponentId = test::TestData2::ComponentId;
		WrappedUpdate->EntityId = Op.EntityId;

		HandleUpdate(WrappedUpdate);
	}
}

void UCallbackDispatcher::OnTestData2AuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
	const worker::ComponentId ComponentId = test::TestData2::ComponentId;

	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
	if(Callback != nullptr && (*Callback) != nullptr)
	{
		(*Callback)(Op);
	}

	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
}

void UCallbackDispatcher::OnBuiltInTypesComponentUpdateOp(const worker::ComponentUpdateOp<test::BuiltInTypes>& Op) {
	UBuiltInTypesComponentUpdate* WrappedUpdate = NewObject<UBuiltInTypesComponentUpdate>();
	if (WrappedUpdate)
	{
		WrappedUpdate->InitInternal(Op.Update);
		WrappedUpdate->ComponentId = test::BuiltInTypes::ComponentId;
		WrappedUpdate->EntityId = Op.EntityId;

		HandleUpdate(WrappedUpdate);
	}
}

void UCallbackDispatcher::OnBuiltInTypesAuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
	const worker::ComponentId ComponentId = test::BuiltInTypes::ComponentId;

	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
	if(Callback != nullptr && (*Callback) != nullptr)
	{
		(*Callback)(Op);
	}

	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
}

void UCallbackDispatcher::OnTestFieldTypesComponentUpdateOp(const worker::ComponentUpdateOp<test::TestFieldTypes>& Op) {
	UTestFieldTypesComponentUpdate* WrappedUpdate = NewObject<UTestFieldTypesComponentUpdate>();
	if (WrappedUpdate)
	{
		WrappedUpdate->InitInternal(Op.Update);
		WrappedUpdate->ComponentId = test::TestFieldTypes::ComponentId;
		WrappedUpdate->EntityId = Op.EntityId;

		HandleUpdate(WrappedUpdate);
	}
}

void UCallbackDispatcher::OnTestFieldTypesAuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
	const worker::ComponentId ComponentId = test::TestFieldTypes::ComponentId;

	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
	if(Callback != nullptr && (*Callback) != nullptr)
	{
		(*Callback)(Op);
	}

	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
}

void UCallbackDispatcher::OnTestEmptyComponentUpdateOp(const worker::ComponentUpdateOp<test::TestEmpty>& Op) {
	UTestEmptyComponentUpdate* WrappedUpdate = NewObject<UTestEmptyComponentUpdate>();
	if (WrappedUpdate)
	{
		WrappedUpdate->InitInternal(Op.Update);
		WrappedUpdate->ComponentId = test::TestEmpty::ComponentId;
		WrappedUpdate->EntityId = Op.EntityId;

		HandleUpdate(WrappedUpdate);
	}
}

void UCallbackDispatcher::OnTestEmptyAuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
	const worker::ComponentId ComponentId = test::TestEmpty::ComponentId;

	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
	if(Callback != nullptr && (*Callback) != nullptr)
	{
		(*Callback)(Op);
	}

	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
}

void UCallbackDispatcher::OnTestCommandResponseTypesComponentUpdateOp(const worker::ComponentUpdateOp<test::TestCommandResponseTypes>& Op) {
	UTestCommandResponseTypesComponentUpdate* WrappedUpdate = NewObject<UTestCommandResponseTypesComponentUpdate>();
	if (WrappedUpdate)
	{
		WrappedUpdate->InitInternal(Op.Update);
		WrappedUpdate->ComponentId = test::TestCommandResponseTypes::ComponentId;
		WrappedUpdate->EntityId = Op.EntityId;

		HandleUpdate(WrappedUpdate);
	}
}

void UCallbackDispatcher::OnTestCommandResponseTypesAuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
	const worker::ComponentId ComponentId = test::TestCommandResponseTypes::ComponentId;

	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
	if(Callback != nullptr && (*Callback) != nullptr)
	{
		(*Callback)(Op);
	}

	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
}

void UCallbackDispatcher::OnComponentWithSimilarlyNamedPropertyAndEventComponentUpdateOp(const worker::ComponentUpdateOp<test::ComponentWithSimilarlyNamedPropertyAndEvent>& Op) {
	UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate* WrappedUpdate = NewObject<UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate>();
	if (WrappedUpdate)
	{
		WrappedUpdate->InitInternal(Op.Update);
		WrappedUpdate->ComponentId = test::ComponentWithSimilarlyNamedPropertyAndEvent::ComponentId;
		WrappedUpdate->EntityId = Op.EntityId;

		HandleUpdate(WrappedUpdate);
	}
}

void UCallbackDispatcher::OnComponentWithSimilarlyNamedPropertyAndEventAuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
	const worker::ComponentId ComponentId = test::ComponentWithSimilarlyNamedPropertyAndEvent::ComponentId;

	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
	if(Callback != nullptr && (*Callback) != nullptr)
	{
		(*Callback)(Op);
	}

	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
}

