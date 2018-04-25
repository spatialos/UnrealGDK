// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#include "TestFieldTypesComponent.h"
#include "CallbackDispatcher.h"

#include "SpatialOSViewTypes.h"

const worker::ComponentId UTestFieldTypesComponent::ComponentId = 1103;

UTestFieldTypesComponent::UTestFieldTypesComponent()
: ListProperty(nullptr)
, MapProperty(nullptr)
, OptionProperty(nullptr)
, UserTypeProperty(nullptr)
, ComponentUpdater(nullptr)
, Snapshot(nullptr)
{
}

void UTestFieldTypesComponent::BeginDestroy()
{
	Super::BeginDestroy();

	Callbacks.Reset(nullptr);
	
	delete Snapshot;
	Snapshot = nullptr;
}

FComponentId UTestFieldTypesComponent::GetComponentId()
{
    return FComponentId(ComponentId);
}

void UTestFieldTypesComponent::Init(const TWeakPtr<SpatialOSConnection>& InConnection, const TWeakPtr<SpatialOSView>& InView,
                          worker::EntityId InEntityId, UCallbackDispatcher* InCallbackDispatcher)
{
    USpatialOsComponent::Init(InConnection, InView, InEntityId, InCallbackDispatcher);

	InCallbackDispatcher->AddComponentUpdateCallback(InEntityId, ComponentId, [this](UComponentUpdateOpWrapperBase& Op) { OnComponentUpdateDispatcherCallback(Op); });
	InCallbackDispatcher->AddAuthorityChangeCallback(InEntityId, ComponentId, [this](const worker::AuthorityChangeOp& Op) { OnAuthorityChangeDispatcherCallback(Op); });
	ComponentUpdater = NewObject<UTestFieldTypesComponentUpdate>(this);

	auto LockedView = View.Pin();
	if(LockedView.IsValid())
	{
	
	}
}

void  UTestFieldTypesComponent::Disable(const worker::EntityId InEntityId, UCallbackDispatcher* CallbackDispatcher)
{
	if(CallbackDispatcher)
	{
		CallbackDispatcher->RemoveComponentUpdateCallback(InEntityId, ComponentId);
		CallbackDispatcher->RemoveAuthorityChangeCallback(InEntityId, ComponentId);
	}
}

void UTestFieldTypesComponent::SendComponentUpdate(UTestFieldTypesComponentUpdate* update)
{
	SendComponentUpdateInternal(update);
}

void UTestFieldTypesComponent::ApplyInitialState(const UAddComponentOpWrapperBase& AddComponentOp)
{
	const UTestFieldTypesAddComponentOp& DerivedOp = dynamic_cast<const UTestFieldTypesAddComponentOp&>(AddComponentOp);
    auto update = test::TestFieldTypes::Update::FromInitialData(*(DerivedOp.Data.data()));
    ApplyComponentUpdate(update);
}

void UTestFieldTypesComponent::GenerateSnapshot()
{
	if (Snapshot)
	{
		return;
	}

	Snapshot = new FTestFieldTypesComponentSnapshot();
	check(Snapshot);

	Snapshot->BuiltInProperty = BuiltInProperty;
	Snapshot->EnumProperty = EnumProperty;
	Snapshot->ListProperty = ListProperty->GetUnderlying();
	Snapshot->MapProperty = MapProperty->GetUnderlying();
	Snapshot->OptionProperty = OptionProperty->GetUnderlying();
	Snapshot->UserTypeProperty = UserTypeProperty->GetUnderlying();
}

void UTestFieldTypesComponent::ReplicateChanges(float DeltaSeconds)
{
	TriggerAutomaticComponentUpdate(DeltaSeconds);
}

void UTestFieldTypesComponent::TriggerAutomaticComponentUpdate(float DeltaSeconds)
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

void UTestFieldTypesComponent::TriggerManualComponentUpdate()
{
	if (GetAuthority() == EAuthority::NotAuthoritative)
	{
		return;
	}

	ReplicateChangesInternal();
}



void UTestFieldTypesComponent::OnComponentUpdateDispatcherCallback(UComponentUpdateOpWrapperBase& Op)
{
	UTestFieldTypesComponentUpdate& ThisOp = dynamic_cast<UTestFieldTypesComponentUpdate&>(Op);
    ApplyComponentUpdate(ThisOp.GetUnderlyingInternal());
}

void UTestFieldTypesComponent::OnAuthorityChangeDispatcherCallback(const worker::AuthorityChangeOp& Op)
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

void UTestFieldTypesComponent::ReplicateChangesInternal()
{
	checkSlow(Snapshot);
		
	bool bShouldSendUpdate = false;
		
	if (BuiltInProperty != Snapshot->BuiltInProperty)
	{
		ComponentUpdater->SetBuiltInPropertyInternal(BuiltInProperty);
		Snapshot->BuiltInProperty = BuiltInProperty;
		bShouldSendUpdate = true;
	}
	if (EnumProperty != Snapshot->EnumProperty)
	{
		ComponentUpdater->SetEnumPropertyInternal(EnumProperty);
		Snapshot->EnumProperty = EnumProperty;
		bShouldSendUpdate = true;
	}
	if (ListProperty && (*ListProperty != Snapshot->ListProperty))
	{
		ComponentUpdater->SetListPropertyInternal(ListProperty);
		Snapshot->ListProperty = ListProperty->GetUnderlying();
		bShouldSendUpdate = true;
	}
	if (MapProperty && (*MapProperty != Snapshot->MapProperty))
	{
		ComponentUpdater->SetMapPropertyInternal(MapProperty);
		Snapshot->MapProperty = MapProperty->GetUnderlying();
		bShouldSendUpdate = true;
	}
	if (OptionProperty && (*OptionProperty != Snapshot->OptionProperty))
	{
		ComponentUpdater->SetOptionPropertyInternal(OptionProperty);
		Snapshot->OptionProperty = OptionProperty->GetUnderlying();
		bShouldSendUpdate = true;
	}
	if (UserTypeProperty && (UserTypeProperty->GetUnderlying() != Snapshot->UserTypeProperty))
	{
		ComponentUpdater->SetUserTypePropertyInternal(UserTypeProperty);
		Snapshot->UserTypeProperty = UserTypeProperty->GetUnderlying();
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

void UTestFieldTypesComponent::SendComponentUpdateInternal(UTestFieldTypesComponentUpdate* update)
{
	auto LockedConnection = Connection.Pin();

	if(LockedConnection.IsValid())
	{
	    LockedConnection->SendComponentUpdate<test::TestFieldTypes>(EntityId, update->GetUnderlyingInternal());
	}
}

void UTestFieldTypesComponent::ApplyComponentUpdate(const test::TestFieldTypes::Update& update)
{
    if (!update.built_in_property().empty())
    {
		BuiltInProperty = static_cast<int>((*(update.built_in_property().data())));
    }

    if (!update.enum_property().empty())
    {
		EnumProperty = static_cast<ETestEnum>((*(update.enum_property().data())));
    }

    if (!update.list_property().empty())
    {
		if (ListProperty == nullptr) { ListProperty = NewObject<UStdStringList>(this); } ListProperty->Init((*(update.list_property().data())));
    }

    if (!update.map_property().empty())
    {
		if (MapProperty == nullptr) { MapProperty = NewObject<UWorkerEntityIdToTestTestType1Map>(this); } MapProperty->Init((*(update.map_property().data())));
    }

    if (!update.option_property().empty())
    {
		if (OptionProperty == nullptr) { OptionProperty = NewObject<UTestTestType2Option>(this); } OptionProperty->Init((*(update.option_property().data())));
    }

    if (!update.user_type_property().empty())
    {
		if (UserTypeProperty == nullptr) { UserTypeProperty = NewObject<UListMapOptionUserTypeData>(this); } UserTypeProperty->Init((*(update.user_type_property().data())));
    }


	NotifyUpdate(update);
}

void UTestFieldTypesComponent::NotifyUpdate(const test::TestFieldTypes::Update& update)
{
    if (!update.built_in_property().empty())
    {
        OnBuiltInPropertyUpdate.Broadcast();
    }

    if (!update.enum_property().empty())
    {
        OnEnumPropertyUpdate.Broadcast();
    }

    if (!update.list_property().empty())
    {
        OnListPropertyUpdate.Broadcast();
    }

    if (!update.map_property().empty())
    {
        OnMapPropertyUpdate.Broadcast();
    }

    if (!update.option_property().empty())
    {
        OnOptionPropertyUpdate.Broadcast();
    }

    if (!update.user_type_property().empty())
    {
        OnUserTypePropertyUpdate.Broadcast();
    }


    OnComponentUpdate.Broadcast();

    if (!bIsComponentReady)
    {
        bIsComponentReady = true;
        OnComponentReady.Broadcast();
    }
}

int UTestFieldTypesComponent::GetBuiltInProperty()
{
    return BuiltInProperty;
}
ETestEnum UTestFieldTypesComponent::GetEnumProperty()
{
    return EnumProperty;
}
UStdStringList* UTestFieldTypesComponent::GetListProperty()
{
    return ListProperty;
}
UWorkerEntityIdToTestTestType1Map* UTestFieldTypesComponent::GetMapProperty()
{
    return MapProperty;
}
UTestTestType2Option* UTestFieldTypesComponent::GetOptionProperty()
{
    return OptionProperty;
}
UListMapOptionUserTypeData* UTestFieldTypesComponent::GetUserTypeProperty()
{
    return UserTypeProperty;
}

