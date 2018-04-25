// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#include "ComponentWithSimilarlyNamedPropertyAndEventComponent.h"
#include "CallbackDispatcher.h"

#include "SpatialOSViewTypes.h"

const worker::ComponentId UComponentWithSimilarlyNamedPropertyAndEventComponent::ComponentId = 1106;

UComponentWithSimilarlyNamedPropertyAndEventComponent::UComponentWithSimilarlyNamedPropertyAndEventComponent()
: ComponentUpdater(nullptr)
, MyValueUpdateWrapper(nullptr)
, Snapshot(nullptr)
{
}

void UComponentWithSimilarlyNamedPropertyAndEventComponent::BeginDestroy()
{
	Super::BeginDestroy();

	Callbacks.Reset(nullptr);
	
	delete Snapshot;
	Snapshot = nullptr;
}

FComponentId UComponentWithSimilarlyNamedPropertyAndEventComponent::GetComponentId()
{
    return FComponentId(ComponentId);
}

void UComponentWithSimilarlyNamedPropertyAndEventComponent::Init(const TWeakPtr<SpatialOSConnection>& InConnection, const TWeakPtr<SpatialOSView>& InView,
                          worker::EntityId InEntityId, UCallbackDispatcher* InCallbackDispatcher)
{
    USpatialOsComponent::Init(InConnection, InView, InEntityId, InCallbackDispatcher);

	InCallbackDispatcher->AddComponentUpdateCallback(InEntityId, ComponentId, [this](UComponentUpdateOpWrapperBase& Op) { OnComponentUpdateDispatcherCallback(Op); });
	InCallbackDispatcher->AddAuthorityChangeCallback(InEntityId, ComponentId, [this](const worker::AuthorityChangeOp& Op) { OnAuthorityChangeDispatcherCallback(Op); });
	ComponentUpdater = NewObject<UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate>(this);

	auto LockedView = View.Pin();
	if(LockedView.IsValid())
	{
	
	}
}

void  UComponentWithSimilarlyNamedPropertyAndEventComponent::Disable(const worker::EntityId InEntityId, UCallbackDispatcher* CallbackDispatcher)
{
	if(CallbackDispatcher)
	{
		CallbackDispatcher->RemoveComponentUpdateCallback(InEntityId, ComponentId);
		CallbackDispatcher->RemoveAuthorityChangeCallback(InEntityId, ComponentId);
	}
}

void UComponentWithSimilarlyNamedPropertyAndEventComponent::SendComponentUpdate(UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate* update)
{
	SendComponentUpdateInternal(update);
}

void UComponentWithSimilarlyNamedPropertyAndEventComponent::ApplyInitialState(const UAddComponentOpWrapperBase& AddComponentOp)
{
	const UComponentWithSimilarlyNamedPropertyAndEventAddComponentOp& DerivedOp = dynamic_cast<const UComponentWithSimilarlyNamedPropertyAndEventAddComponentOp&>(AddComponentOp);
    auto update = test::ComponentWithSimilarlyNamedPropertyAndEvent::Update::FromInitialData(*(DerivedOp.Data.data()));
    ApplyComponentUpdate(update);
}

void UComponentWithSimilarlyNamedPropertyAndEventComponent::GenerateSnapshot()
{
	if (Snapshot)
	{
		return;
	}

	Snapshot = new FComponentWithSimilarlyNamedPropertyAndEventComponentSnapshot();
	check(Snapshot);

	Snapshot->MyValue = MyValue;
}

void UComponentWithSimilarlyNamedPropertyAndEventComponent::ReplicateChanges(float DeltaSeconds)
{
	TriggerAutomaticComponentUpdate(DeltaSeconds);
}

void UComponentWithSimilarlyNamedPropertyAndEventComponent::TriggerAutomaticComponentUpdate(float DeltaSeconds)
{
	if (GetAuthority() == EAuthority::NotAuthoritative || MaxUpdatesPerSecond == 0)
	{
		return;
	}

	TimeUntilNextUpdate -= DeltaSeconds;
	if (TimeUntilNextUpdate > 0.0f)
	{
		return;
	}

	TimeUntilNextUpdate = 1.0f / static_cast<float>(MaxUpdatesPerSecond);

	ReplicateChangesInternal();
}

void UComponentWithSimilarlyNamedPropertyAndEventComponent::TriggerManualComponentUpdate()
{
	if (GetAuthority() == EAuthority::NotAuthoritative)
	{
		return;
	}

	ReplicateChangesInternal();
}

void UComponentWithSimilarlyNamedPropertyAndEventComponent::MyValueUpdate(UTestType1* Data)
{
	ComponentUpdater->AddMyValueUpdateInternal(Data);
	bHasEventQueued = true;
}

	


void UComponentWithSimilarlyNamedPropertyAndEventComponent::OnComponentUpdateDispatcherCallback(UComponentUpdateOpWrapperBase& Op)
{
	UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate& ThisOp = dynamic_cast<UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate&>(Op);
    ApplyComponentUpdate(ThisOp.GetUnderlyingInternal());
}

void UComponentWithSimilarlyNamedPropertyAndEventComponent::OnAuthorityChangeDispatcherCallback(const worker::AuthorityChangeOp& Op)
{
	if (Op.Authority == worker::Authority::kNotAuthoritative)
	{
		delete Snapshot;
		Snapshot = nullptr;
	}
	else
	{
		GenerateSnapshot();
	}

	Super::OnAuthorityChangeDispatcherCallback(Op);	
}

void UComponentWithSimilarlyNamedPropertyAndEventComponent::ReplicateChangesInternal()
{
	checkSlow(Snapshot);
		
	bool bShouldSendUpdate = false;
		
	if (MyValue != Snapshot->MyValue)
	{
		ComponentUpdater->SetMyValueInternal(MyValue);
		Snapshot->MyValue = MyValue;
		bShouldSendUpdate = true;
	}
	bShouldSendUpdate |= bHasEventQueued;
	if (bShouldSendUpdate)
	{
		SendComponentUpdateInternal(ComponentUpdater);
		ComponentUpdater->ResetInternal();
		bHasEventQueued = false;
	}
}

void UComponentWithSimilarlyNamedPropertyAndEventComponent::SendComponentUpdateInternal(UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate* update)
{
	auto LockedConnection = Connection.Pin();

	if(LockedConnection.IsValid())
	{
	    LockedConnection->SendComponentUpdate<test::ComponentWithSimilarlyNamedPropertyAndEvent>(EntityId, update->GetUnderlyingInternal());
	}
}

void UComponentWithSimilarlyNamedPropertyAndEventComponent::ApplyComponentUpdate(const test::ComponentWithSimilarlyNamedPropertyAndEvent::Update& update)
{
    if (!update.my_value().empty())
    {
		MyValue = static_cast<int>((*(update.my_value().data())));
    }


	NotifyUpdate(update);
}

void UComponentWithSimilarlyNamedPropertyAndEventComponent::NotifyUpdate(const test::ComponentWithSimilarlyNamedPropertyAndEvent::Update& update)
{
    if (!update.my_value().empty())
    {
        OnMyValueUpdate.Broadcast();
    }

    if (!update.my_value_update().empty())
    {
        for (auto& val : update.my_value_update())
        {
			if (MyValueUpdateWrapper == nullptr) { MyValueUpdateWrapper = NewObject<UTestType1>(this); } MyValueUpdateWrapper->Init(val);
            OnMyValueUpdate.Broadcast(MyValueUpdateWrapper);
        }
    }


    OnComponentUpdate.Broadcast();

    if (!bIsComponentReady)
    {
        bIsComponentReady = true;
        OnComponentReady.Broadcast();
    }
}

int UComponentWithSimilarlyNamedPropertyAndEventComponent::GetMyValue()
{
    return MyValue;
}

