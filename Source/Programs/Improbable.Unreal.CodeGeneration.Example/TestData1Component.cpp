// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#include "TestData1Component.h"
#include "CallbackDispatcher.h"

#include "TestType1.h"
#include "SpatialOSViewTypes.h"

const worker::ComponentId UTestData1Component::ComponentId = 1100;

UTestData1Component::UTestData1Component()
: ComponentUpdater(nullptr)
, OneTextEventWrapper(nullptr)
, Snapshot(nullptr)
{
}

void UTestData1Component::BeginDestroy()
{
	Super::BeginDestroy();

	Callbacks.Reset(nullptr);
	
	delete Snapshot;
	Snapshot = nullptr;
}

FComponentId UTestData1Component::GetComponentId()
{
    return FComponentId(ComponentId);
}

void UTestData1Component::Init(const TWeakPtr<SpatialOSConnection>& InConnection, const TWeakPtr<SpatialOSView>& InView,
                          worker::EntityId InEntityId, UCallbackDispatcher* InCallbackDispatcher)
{
    USpatialOsComponent::Init(InConnection, InView, InEntityId, InCallbackDispatcher);

	InCallbackDispatcher->AddComponentUpdateCallback(InEntityId, ComponentId, [this](UComponentUpdateOpWrapperBase& Op) { OnComponentUpdateDispatcherCallback(Op); });
	InCallbackDispatcher->AddAuthorityChangeCallback(InEntityId, ComponentId, [this](const worker::AuthorityChangeOp& Op) { OnAuthorityChangeDispatcherCallback(Op); });
	ComponentUpdater = NewObject<UTestData1ComponentUpdate>(this);

	auto LockedView = View.Pin();
	if(LockedView.IsValid())
	{
		Callbacks->Add(LockedView->OnCommandRequest<test::TestData1::Commands::RequestTestdata2>(
			std::bind(&UTestData1Component::OnRequestTestdata2CommandRequestDispatcherCallback, this,
					  std::placeholders::_1)));
	
	}
}

void  UTestData1Component::Disable(const worker::EntityId InEntityId, UCallbackDispatcher* CallbackDispatcher)
{
	if(CallbackDispatcher)
	{
		CallbackDispatcher->RemoveComponentUpdateCallback(InEntityId, ComponentId);
		CallbackDispatcher->RemoveAuthorityChangeCallback(InEntityId, ComponentId);
	}
}

void UTestData1Component::SendComponentUpdate(UTestData1ComponentUpdate* update)
{
	SendComponentUpdateInternal(update);
}

void UTestData1Component::ApplyInitialState(const UAddComponentOpWrapperBase& AddComponentOp)
{
	const UTestData1AddComponentOp& DerivedOp = dynamic_cast<const UTestData1AddComponentOp&>(AddComponentOp);
    auto update = test::TestData1::Update::FromInitialData(*(DerivedOp.Data.data()));
    ApplyComponentUpdate(update);
}

void UTestData1Component::GenerateSnapshot()
{
	if (Snapshot)
	{
		return;
	}

	Snapshot = new FTestData1ComponentSnapshot();
	check(Snapshot);

	Snapshot->StringProperty = StringProperty;
	Snapshot->Int32Property = Int32Property;
	Snapshot->CoordinatesProperty = CoordinatesProperty;
}

void UTestData1Component::ReplicateChanges(float DeltaSeconds)
{
	TriggerAutomaticComponentUpdate(DeltaSeconds);
}

void UTestData1Component::TriggerAutomaticComponentUpdate(float DeltaSeconds)
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

void UTestData1Component::TriggerManualComponentUpdate()
{
	if (GetAuthority() == EAuthority::NotAuthoritative)
	{
		return;
	}

	ReplicateChangesInternal();
}

void UTestData1Component::OneTextEvent(UTextEvent* Data)
{
	ComponentUpdater->AddOneTextEventInternal(Data);
	bHasEventQueued = true;
}

	


void UTestData1Component::OnComponentUpdateDispatcherCallback(UComponentUpdateOpWrapperBase& Op)
{
	UTestData1ComponentUpdate& ThisOp = dynamic_cast<UTestData1ComponentUpdate&>(Op);
    ApplyComponentUpdate(ThisOp.GetUnderlyingInternal());
}

void UTestData1Component::OnAuthorityChangeDispatcherCallback(const worker::AuthorityChangeOp& Op)
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

void UTestData1Component::ReplicateChangesInternal()
{
	checkSlow(Snapshot);
		
	bool bShouldSendUpdate = false;
		
	if (StringProperty != Snapshot->StringProperty)
	{
		ComponentUpdater->SetStringPropertyInternal(StringProperty);
		Snapshot->StringProperty = StringProperty;
		bShouldSendUpdate = true;
	}
	if (Int32Property != Snapshot->Int32Property)
	{
		ComponentUpdater->SetInt32PropertyInternal(Int32Property);
		Snapshot->Int32Property = Int32Property;
		bShouldSendUpdate = true;
	}
	if (!CoordinatesProperty.Equals(Snapshot->CoordinatesProperty))
	{
		ComponentUpdater->SetCoordinatesPropertyInternal(CoordinatesProperty);
		Snapshot->CoordinatesProperty = CoordinatesProperty;
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

void UTestData1Component::SendComponentUpdateInternal(UTestData1ComponentUpdate* update)
{
	auto LockedConnection = Connection.Pin();

	if(LockedConnection.IsValid())
	{
	    LockedConnection->SendComponentUpdate<test::TestData1>(EntityId, update->GetUnderlyingInternal());
	}
}

void UTestData1Component::ApplyComponentUpdate(const test::TestData1::Update& update)
{
    if (!update.string_property().empty())
    {
		StringProperty = FString((*(update.string_property().data())).c_str());
    }

    if (!update.int32_property().empty())
    {
		Int32Property = static_cast<int>((*(update.int32_property().data())));
    }

    if (!update.coordinates_property().empty())
    {
		CoordinatesProperty = USpatialOSConversionFunctionLibrary::SpatialOsCoordinatesToUnrealCoordinates(FVector(static_cast<float>((*(update.coordinates_property().data())).x()), static_cast<float>((*(update.coordinates_property().data())).y()), static_cast<float>((*(update.coordinates_property().data())).z())));
    }


	NotifyUpdate(update);
}

void UTestData1Component::NotifyUpdate(const test::TestData1::Update& update)
{
    if (!update.string_property().empty())
    {
        OnStringPropertyUpdate.Broadcast();
    }

    if (!update.int32_property().empty())
    {
        OnInt32PropertyUpdate.Broadcast();
    }

    if (!update.coordinates_property().empty())
    {
        OnCoordinatesPropertyUpdate.Broadcast();
    }

    if (!update.one_text_event().empty())
    {
        for (auto& val : update.one_text_event())
        {
			if (OneTextEventWrapper == nullptr) { OneTextEventWrapper = NewObject<UTextEvent>(this); } OneTextEventWrapper->Init(val);
            OnOneTextEvent.Broadcast(OneTextEventWrapper);
        }
    }


    OnComponentUpdate.Broadcast();

    if (!bIsComponentReady)
    {
        bIsComponentReady = true;
        OnComponentReady.Broadcast();
    }
}

FString UTestData1Component::GetStringProperty()
{
    return StringProperty;
}
int UTestData1Component::GetInt32Property()
{
    return Int32Property;
}
FVector UTestData1Component::GetCoordinatesProperty()
{
    return CoordinatesProperty;
}

void UTestData1Component::OnRequestTestdata2CommandRequestDispatcherCallback(
    const worker::CommandRequestOp<test::TestData1::Commands::RequestTestdata2>& op)
{
    if (op.EntityId != EntityId)
    {
        return;
    }
    auto request = NewObject<UTestType1>()->Init(op.Request);
	
	URequestTestdata2CommandResponder* RequestTestdata2Responder = NewObject<URequestTestdata2CommandResponder>(this);
	RequestTestdata2Responder->Init(Connection, op.RequestId, request, op.CallerWorkerId);
	
    OnRequestTestdata2CommandRequest.Broadcast(RequestTestdata2Responder);
}
