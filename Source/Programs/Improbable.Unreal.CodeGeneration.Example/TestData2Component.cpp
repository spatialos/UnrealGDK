// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#include "TestData2Component.h"
#include "CallbackDispatcher.h"

#include "TestType2.h"
#include "SpatialOSViewTypes.h"

const worker::ComponentId UTestData2Component::ComponentId = 1101;

UTestData2Component::UTestData2Component()
: ComponentUpdater(nullptr)
, TwoTextEventWrapper(nullptr)
, Snapshot(nullptr)
{
}

void UTestData2Component::BeginDestroy()
{
	Super::BeginDestroy();

	Callbacks.Reset(nullptr);
	
	delete Snapshot;
	Snapshot = nullptr;
}

FComponentId UTestData2Component::GetComponentId()
{
    return FComponentId(ComponentId);
}

void UTestData2Component::Init(const TWeakPtr<SpatialOSConnection>& InConnection, const TWeakPtr<SpatialOSView>& InView,
                          worker::EntityId InEntityId, UCallbackDispatcher* InCallbackDispatcher)
{
    USpatialOsComponent::Init(InConnection, InView, InEntityId, InCallbackDispatcher);

	InCallbackDispatcher->AddComponentUpdateCallback(InEntityId, ComponentId, [this](UComponentUpdateOpWrapperBase& Op) { OnComponentUpdateDispatcherCallback(Op); });
	InCallbackDispatcher->AddAuthorityChangeCallback(InEntityId, ComponentId, [this](const worker::AuthorityChangeOp& Op) { OnAuthorityChangeDispatcherCallback(Op); });
	ComponentUpdater = NewObject<UTestData2ComponentUpdate>(this);

	auto LockedView = View.Pin();
	if(LockedView.IsValid())
	{
		Callbacks->Add(LockedView->OnCommandRequest<test::TestData2::Commands::RequestTestdata1>(
			std::bind(&UTestData2Component::OnRequestTestdata1CommandRequestDispatcherCallback, this,
					  std::placeholders::_1)));
	
	}
}

void  UTestData2Component::Disable(const worker::EntityId InEntityId, UCallbackDispatcher* CallbackDispatcher)
{
	if(CallbackDispatcher)
	{
		CallbackDispatcher->RemoveComponentUpdateCallback(InEntityId, ComponentId);
		CallbackDispatcher->RemoveAuthorityChangeCallback(InEntityId, ComponentId);
	}
}

void UTestData2Component::SendComponentUpdate(UTestData2ComponentUpdate* update)
{
	SendComponentUpdateInternal(update);
}

void UTestData2Component::ApplyInitialState(const UAddComponentOpWrapperBase& AddComponentOp)
{
	const UTestData2AddComponentOp& DerivedOp = dynamic_cast<const UTestData2AddComponentOp&>(AddComponentOp);
    auto update = test::TestData2::Update::FromInitialData(*(DerivedOp.Data.data()));
    ApplyComponentUpdate(update);
}

void UTestData2Component::GenerateSnapshot()
{
	if (Snapshot)
	{
		return;
	}

	Snapshot = new FTestData2ComponentSnapshot();
	check(Snapshot);

	Snapshot->DoubleProperty = DoubleProperty;
}

void UTestData2Component::ReplicateChanges(float DeltaSeconds)
{
	TriggerAutomaticComponentUpdate(DeltaSeconds);
}

void UTestData2Component::TriggerAutomaticComponentUpdate(float DeltaSeconds)
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

void UTestData2Component::TriggerManualComponentUpdate()
{
	if (GetAuthority() == EAuthority::NotAuthoritative)
	{
		return;
	}

	ReplicateChangesInternal();
}

void UTestData2Component::TwoTextEvent(UTextEvent* Data)
{
	ComponentUpdater->AddTwoTextEventInternal(Data);
	bHasEventQueued = true;
}

	


void UTestData2Component::OnComponentUpdateDispatcherCallback(UComponentUpdateOpWrapperBase& Op)
{
	UTestData2ComponentUpdate& ThisOp = dynamic_cast<UTestData2ComponentUpdate&>(Op);
    ApplyComponentUpdate(ThisOp.GetUnderlyingInternal());
}

void UTestData2Component::OnAuthorityChangeDispatcherCallback(const worker::AuthorityChangeOp& Op)
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

void UTestData2Component::ReplicateChangesInternal()
{
	checkSlow(Snapshot);
		
	bool bShouldSendUpdate = false;
		
	if (DoubleProperty != Snapshot->DoubleProperty)
	{
		ComponentUpdater->SetDoublePropertyInternal(DoubleProperty);
		Snapshot->DoubleProperty = DoubleProperty;
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

void UTestData2Component::SendComponentUpdateInternal(UTestData2ComponentUpdate* update)
{
	auto LockedConnection = Connection.Pin();

	if(LockedConnection.IsValid())
	{
	    LockedConnection->SendComponentUpdate<test::TestData2>(EntityId, update->GetUnderlyingInternal());
	}
}

void UTestData2Component::ApplyComponentUpdate(const test::TestData2::Update& update)
{
    if (!update.double_property().empty())
    {
		DoubleProperty = static_cast<float>((*(update.double_property().data())));
    }


	NotifyUpdate(update);
}

void UTestData2Component::NotifyUpdate(const test::TestData2::Update& update)
{
    if (!update.double_property().empty())
    {
        OnDoublePropertyUpdate.Broadcast();
    }

    if (!update.two_text_event().empty())
    {
        for (auto& val : update.two_text_event())
        {
			if (TwoTextEventWrapper == nullptr) { TwoTextEventWrapper = NewObject<UTextEvent>(this); } TwoTextEventWrapper->Init(val);
            OnTwoTextEvent.Broadcast(TwoTextEventWrapper);
        }
    }


    OnComponentUpdate.Broadcast();

    if (!bIsComponentReady)
    {
        bIsComponentReady = true;
        OnComponentReady.Broadcast();
    }
}

float UTestData2Component::GetDoubleProperty()
{
    return DoubleProperty;
}

void UTestData2Component::OnRequestTestdata1CommandRequestDispatcherCallback(
    const worker::CommandRequestOp<test::TestData2::Commands::RequestTestdata1>& op)
{
    if (op.EntityId != EntityId)
    {
        return;
    }
    auto request = NewObject<UTestType2>()->Init(op.Request);
	
	URequestTestdata1CommandResponder* RequestTestdata1Responder = NewObject<URequestTestdata1CommandResponder>(this);
	RequestTestdata1Responder->Init(Connection, op.RequestId, request, op.CallerWorkerId);
	
    OnRequestTestdata1CommandRequest.Broadcast(RequestTestdata1Responder);
}
