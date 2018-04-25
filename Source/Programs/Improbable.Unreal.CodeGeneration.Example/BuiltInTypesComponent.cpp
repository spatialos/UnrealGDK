// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#include "BuiltInTypesComponent.h"
#include "CallbackDispatcher.h"

#include "SpatialOSViewTypes.h"

const worker::ComponentId UBuiltInTypesComponent::ComponentId = 1102;

UBuiltInTypesComponent::UBuiltInTypesComponent()
: ComponentUpdater(nullptr)
, Snapshot(nullptr)
{
}

void UBuiltInTypesComponent::BeginDestroy()
{
	Super::BeginDestroy();

	Callbacks.Reset(nullptr);
	
	delete Snapshot;
	Snapshot = nullptr;
}

FComponentId UBuiltInTypesComponent::GetComponentId()
{
    return FComponentId(ComponentId);
}

void UBuiltInTypesComponent::Init(const TWeakPtr<SpatialOSConnection>& InConnection, const TWeakPtr<SpatialOSView>& InView,
                          worker::EntityId InEntityId, UCallbackDispatcher* InCallbackDispatcher)
{
    USpatialOsComponent::Init(InConnection, InView, InEntityId, InCallbackDispatcher);

	InCallbackDispatcher->AddComponentUpdateCallback(InEntityId, ComponentId, [this](UComponentUpdateOpWrapperBase& Op) { OnComponentUpdateDispatcherCallback(Op); });
	InCallbackDispatcher->AddAuthorityChangeCallback(InEntityId, ComponentId, [this](const worker::AuthorityChangeOp& Op) { OnAuthorityChangeDispatcherCallback(Op); });
	ComponentUpdater = NewObject<UBuiltInTypesComponentUpdate>(this);

	auto LockedView = View.Pin();
	if(LockedView.IsValid())
	{
	
	}
}

void  UBuiltInTypesComponent::Disable(const worker::EntityId InEntityId, UCallbackDispatcher* CallbackDispatcher)
{
	if(CallbackDispatcher)
	{
		CallbackDispatcher->RemoveComponentUpdateCallback(InEntityId, ComponentId);
		CallbackDispatcher->RemoveAuthorityChangeCallback(InEntityId, ComponentId);
	}
}

void UBuiltInTypesComponent::SendComponentUpdate(UBuiltInTypesComponentUpdate* update)
{
	SendComponentUpdateInternal(update);
}

void UBuiltInTypesComponent::ApplyInitialState(const UAddComponentOpWrapperBase& AddComponentOp)
{
	const UBuiltInTypesAddComponentOp& DerivedOp = dynamic_cast<const UBuiltInTypesAddComponentOp&>(AddComponentOp);
    auto update = test::BuiltInTypes::Update::FromInitialData(*(DerivedOp.Data.data()));
    ApplyComponentUpdate(update);
}

void UBuiltInTypesComponent::GenerateSnapshot()
{
	if (Snapshot)
	{
		return;
	}

	Snapshot = new FBuiltInTypesComponentSnapshot();
	check(Snapshot);

	Snapshot->BoolProperty = BoolProperty;
	Snapshot->Uint32Property = Uint32Property;
	Snapshot->Int32Property = Int32Property;
	Snapshot->FloatProperty = FloatProperty;
	Snapshot->DoubleProperty = DoubleProperty;
	Snapshot->StringProperty = StringProperty;
	Snapshot->BytesProperty = BytesProperty;
	Snapshot->CoordinatesProperty = CoordinatesProperty;
	Snapshot->Vector3dProperty = Vector3dProperty;
	Snapshot->Vector3fProperty = Vector3fProperty;
	Snapshot->EntityIdProperty = EntityIdProperty;
}

void UBuiltInTypesComponent::ReplicateChanges(float DeltaSeconds)
{
	TriggerAutomaticComponentUpdate(DeltaSeconds);
}

void UBuiltInTypesComponent::TriggerAutomaticComponentUpdate(float DeltaSeconds)
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

void UBuiltInTypesComponent::TriggerManualComponentUpdate()
{
	if (GetAuthority() == EAuthority::NotAuthoritative)
	{
		return;
	}

	ReplicateChangesInternal();
}



void UBuiltInTypesComponent::OnComponentUpdateDispatcherCallback(UComponentUpdateOpWrapperBase& Op)
{
	UBuiltInTypesComponentUpdate& ThisOp = dynamic_cast<UBuiltInTypesComponentUpdate&>(Op);
    ApplyComponentUpdate(ThisOp.GetUnderlyingInternal());
}

void UBuiltInTypesComponent::OnAuthorityChangeDispatcherCallback(const worker::AuthorityChangeOp& Op)
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

void UBuiltInTypesComponent::ReplicateChangesInternal()
{
	checkSlow(Snapshot);
		
	bool bShouldSendUpdate = false;
		
	if (BoolProperty != Snapshot->BoolProperty)
	{
		ComponentUpdater->SetBoolPropertyInternal(BoolProperty);
		Snapshot->BoolProperty = BoolProperty;
		bShouldSendUpdate = true;
	}
	if (Uint32Property != Snapshot->Uint32Property)
	{
		ComponentUpdater->SetUint32PropertyInternal(Uint32Property);
		Snapshot->Uint32Property = Uint32Property;
		bShouldSendUpdate = true;
	}
	if (Int32Property != Snapshot->Int32Property)
	{
		ComponentUpdater->SetInt32PropertyInternal(Int32Property);
		Snapshot->Int32Property = Int32Property;
		bShouldSendUpdate = true;
	}
	if (!FMath::IsNearlyEqual(FloatProperty,Snapshot->FloatProperty,(float)KINDA_SMALL_NUMBER))
	{
		ComponentUpdater->SetFloatPropertyInternal(FloatProperty);
		Snapshot->FloatProperty = FloatProperty;
		bShouldSendUpdate = true;
	}
	if (DoubleProperty != Snapshot->DoubleProperty)
	{
		ComponentUpdater->SetDoublePropertyInternal(DoubleProperty);
		Snapshot->DoubleProperty = DoubleProperty;
		bShouldSendUpdate = true;
	}
	if (StringProperty != Snapshot->StringProperty)
	{
		ComponentUpdater->SetStringPropertyInternal(StringProperty);
		Snapshot->StringProperty = StringProperty;
		bShouldSendUpdate = true;
	}
	if (BytesProperty != Snapshot->BytesProperty)
	{
		ComponentUpdater->SetBytesPropertyInternal(BytesProperty);
		Snapshot->BytesProperty = BytesProperty;
		bShouldSendUpdate = true;
	}
	if (!CoordinatesProperty.Equals(Snapshot->CoordinatesProperty))
	{
		ComponentUpdater->SetCoordinatesPropertyInternal(CoordinatesProperty);
		Snapshot->CoordinatesProperty = CoordinatesProperty;
		bShouldSendUpdate = true;
	}
	if (!Vector3dProperty.Equals(Snapshot->Vector3dProperty))
	{
		ComponentUpdater->SetVector3dPropertyInternal(Vector3dProperty);
		Snapshot->Vector3dProperty = Vector3dProperty;
		bShouldSendUpdate = true;
	}
	if (!Vector3fProperty.Equals(Snapshot->Vector3fProperty))
	{
		ComponentUpdater->SetVector3fPropertyInternal(Vector3fProperty);
		Snapshot->Vector3fProperty = Vector3fProperty;
		bShouldSendUpdate = true;
	}
	if (EntityIdProperty != Snapshot->EntityIdProperty)
	{
		ComponentUpdater->SetEntityIdPropertyInternal(EntityIdProperty);
		Snapshot->EntityIdProperty = EntityIdProperty;
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

void UBuiltInTypesComponent::SendComponentUpdateInternal(UBuiltInTypesComponentUpdate* update)
{
	auto LockedConnection = Connection.Pin();

	if(LockedConnection.IsValid())
	{
	    LockedConnection->SendComponentUpdate<test::BuiltInTypes>(EntityId, update->GetUnderlyingInternal());
	}
}

void UBuiltInTypesComponent::ApplyComponentUpdate(const test::BuiltInTypes::Update& update)
{
    if (!update.bool_property().empty())
    {
		BoolProperty = (*(update.bool_property().data()));
    }

    if (!update.uint32_property().empty())
    {
		Uint32Property = static_cast<int>((*(update.uint32_property().data())));
    }

    if (!update.int32_property().empty())
    {
		Int32Property = static_cast<int>((*(update.int32_property().data())));
    }

    if (!update.float_property().empty())
    {
		FloatProperty = (*(update.float_property().data()));
    }

    if (!update.double_property().empty())
    {
		DoubleProperty = static_cast<float>((*(update.double_property().data())));
    }

    if (!update.string_property().empty())
    {
		StringProperty = FString((*(update.string_property().data())).c_str());
    }

    if (!update.bytes_property().empty())
    {
		BytesProperty = FString((*(update.bytes_property().data())).c_str());
    }

    if (!update.coordinates_property().empty())
    {
		CoordinatesProperty = USpatialOSConversionFunctionLibrary::SpatialOsCoordinatesToUnrealCoordinates(FVector(static_cast<float>((*(update.coordinates_property().data())).x()), static_cast<float>((*(update.coordinates_property().data())).y()), static_cast<float>((*(update.coordinates_property().data())).z())));
    }

    if (!update.vector3d_property().empty())
    {
		Vector3dProperty = FVector(static_cast<float>((*(update.vector3d_property().data())).x()), static_cast<float>((*(update.vector3d_property().data())).y()), static_cast<float>((*(update.vector3d_property().data())).z()));
    }

    if (!update.vector3f_property().empty())
    {
		Vector3fProperty = FVector(static_cast<float>((*(update.vector3f_property().data())).x()), static_cast<float>((*(update.vector3f_property().data())).y()), static_cast<float>((*(update.vector3f_property().data())).z()));
    }

    if (!update.entity_id_property().empty())
    {
		EntityIdProperty = FEntityId((*(update.entity_id_property().data())));
    }


	NotifyUpdate(update);
}

void UBuiltInTypesComponent::NotifyUpdate(const test::BuiltInTypes::Update& update)
{
    if (!update.bool_property().empty())
    {
        OnBoolPropertyUpdate.Broadcast();
    }

    if (!update.uint32_property().empty())
    {
        OnUint32PropertyUpdate.Broadcast();
    }

    if (!update.int32_property().empty())
    {
        OnInt32PropertyUpdate.Broadcast();
    }

    if (!update.float_property().empty())
    {
        OnFloatPropertyUpdate.Broadcast();
    }

    if (!update.double_property().empty())
    {
        OnDoublePropertyUpdate.Broadcast();
    }

    if (!update.string_property().empty())
    {
        OnStringPropertyUpdate.Broadcast();
    }

    if (!update.bytes_property().empty())
    {
        OnBytesPropertyUpdate.Broadcast();
    }

    if (!update.coordinates_property().empty())
    {
        OnCoordinatesPropertyUpdate.Broadcast();
    }

    if (!update.vector3d_property().empty())
    {
        OnVector3dPropertyUpdate.Broadcast();
    }

    if (!update.vector3f_property().empty())
    {
        OnVector3fPropertyUpdate.Broadcast();
    }

    if (!update.entity_id_property().empty())
    {
        OnEntityIdPropertyUpdate.Broadcast();
    }


    OnComponentUpdate.Broadcast();

    if (!bIsComponentReady)
    {
        bIsComponentReady = true;
        OnComponentReady.Broadcast();
    }
}

bool UBuiltInTypesComponent::GetBoolProperty()
{
    return BoolProperty;
}
int UBuiltInTypesComponent::GetUint32Property()
{
    return Uint32Property;
}
int UBuiltInTypesComponent::GetInt32Property()
{
    return Int32Property;
}
float UBuiltInTypesComponent::GetFloatProperty()
{
    return FloatProperty;
}
float UBuiltInTypesComponent::GetDoubleProperty()
{
    return DoubleProperty;
}
FString UBuiltInTypesComponent::GetStringProperty()
{
    return StringProperty;
}
FString UBuiltInTypesComponent::GetBytesProperty()
{
    return BytesProperty;
}
FVector UBuiltInTypesComponent::GetCoordinatesProperty()
{
    return CoordinatesProperty;
}
FVector UBuiltInTypesComponent::GetVector3dProperty()
{
    return Vector3dProperty;
}
FVector UBuiltInTypesComponent::GetVector3fProperty()
{
    return Vector3fProperty;
}
FEntityId UBuiltInTypesComponent::GetEntityIdProperty()
{
    return EntityIdProperty;
}

