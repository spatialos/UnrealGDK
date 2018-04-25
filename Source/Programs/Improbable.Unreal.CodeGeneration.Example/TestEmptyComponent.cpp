// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#include "TestEmptyComponent.h"
#include "CallbackDispatcher.h"

#include "SpatialOSViewTypes.h"

const worker::ComponentId UTestEmptyComponent::ComponentId = 1104;

UTestEmptyComponent::UTestEmptyComponent()
: ComponentUpdater(nullptr)
, Snapshot(nullptr)
{
}

void UTestEmptyComponent::BeginDestroy()
{
	Super::BeginDestroy();

	Callbacks.Reset(nullptr);
	
	delete Snapshot;
	Snapshot = nullptr;
}

FComponentId UTestEmptyComponent::GetComponentId()
{
    return FComponentId(ComponentId);
}

void UTestEmptyComponent::Init(const TWeakPtr<SpatialOSConnection>& InConnection, const TWeakPtr<SpatialOSView>& InView,
                          worker::EntityId InEntityId, UCallbackDispatcher* InCallbackDispatcher)
{
    USpatialOsComponent::Init(InConnection, InView, InEntityId, InCallbackDispatcher);

	InCallbackDispatcher->AddComponentUpdateCallback(InEntityId, ComponentId, [this](UComponentUpdateOpWrapperBase& Op) { OnComponentUpdateDispatcherCallback(Op); });
	InCallbackDispatcher->AddAuthorityChangeCallback(InEntityId, ComponentId, [this](const worker::AuthorityChangeOp& Op) { OnAuthorityChangeDispatcherCallback(Op); });
	ComponentUpdater = NewObject<UTestEmptyComponentUpdate>(this);

	auto LockedView = View.Pin();
	if(LockedView.IsValid())
	{
	
	}
}

void  UTestEmptyComponent::Disable(const worker::EntityId InEntityId, UCallbackDispatcher* CallbackDispatcher)
{
	if(CallbackDispatcher)
	{
		CallbackDispatcher->RemoveComponentUpdateCallback(InEntityId, ComponentId);
		CallbackDispatcher->RemoveAuthorityChangeCallback(InEntityId, ComponentId);
	}
}

void UTestEmptyComponent::SendComponentUpdate(UTestEmptyComponentUpdate* update)
{
	SendComponentUpdateInternal(update);
}

void UTestEmptyComponent::ApplyInitialState(const UAddComponentOpWrapperBase& AddComponentOp)
{
	const UTestEmptyAddComponentOp& DerivedOp = dynamic_cast<const UTestEmptyAddComponentOp&>(AddComponentOp);
    auto update = test::TestEmpty::Update::FromInitialData(*(DerivedOp.Data.data()));
    ApplyComponentUpdate(update);
}

void UTestEmptyComponent::GenerateSnapshot()
{
	if (Snapshot)
	{
		return;
	}

	Snapshot = new FTestEmptyComponentSnapshot();
	check(Snapshot);

}

void UTestEmptyComponent::ReplicateChanges(float DeltaSeconds)
{
	TriggerAutomaticComponentUpdate(DeltaSeconds);
}

void UTestEmptyComponent::TriggerAutomaticComponentUpdate(float DeltaSeconds)
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

void UTestEmptyComponent::TriggerManualComponentUpdate()
{
	if (GetAuthority() == EAuthority::NotAuthoritative)
	{
		return;
	}

	ReplicateChangesInternal();
}



void UTestEmptyComponent::OnComponentUpdateDispatcherCallback(UComponentUpdateOpWrapperBase& Op)
{
	UTestEmptyComponentUpdate& ThisOp = dynamic_cast<UTestEmptyComponentUpdate&>(Op);
    ApplyComponentUpdate(ThisOp.GetUnderlyingInternal());
}

void UTestEmptyComponent::OnAuthorityChangeDispatcherCallback(const worker::AuthorityChangeOp& Op)
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

void UTestEmptyComponent::ReplicateChangesInternal()
{
	checkSlow(Snapshot);
		
	bool bShouldSendUpdate = false;
		
	bShouldSendUpdate |= bHasEventQueued;
	if (bShouldSendUpdate)
	{
		SendComponentUpdateInternal(ComponentUpdater);
		ComponentUpdater->ResetInternal();
		bHasEventQueued = false;
	}
}

void UTestEmptyComponent::SendComponentUpdateInternal(UTestEmptyComponentUpdate* update)
{
	auto LockedConnection = Connection.Pin();

	if(LockedConnection.IsValid())
	{
	    LockedConnection->SendComponentUpdate<test::TestEmpty>(EntityId, update->GetUnderlyingInternal());
	}
}

void UTestEmptyComponent::ApplyComponentUpdate(const test::TestEmpty::Update& update)
{

	NotifyUpdate(update);
}

void UTestEmptyComponent::NotifyUpdate(const test::TestEmpty::Update& update)
{

    OnComponentUpdate.Broadcast();

    if (!bIsComponentReady)
    {
        bIsComponentReady = true;
        OnComponentReady.Broadcast();
    }
}


