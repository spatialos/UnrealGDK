// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#include "TestCommandResponseTypesComponent.h"
#include "CallbackDispatcher.h"

#include "TestType1.h"
#include "improbable/standard_library.h"
#include "SpatialOSConversionFunctionLibrary.h"
#include "improbable/vector3.h"
#include "SpatialOSViewTypes.h"

const worker::ComponentId UTestCommandResponseTypesComponent::ComponentId = 1105;

UTestCommandResponseTypesComponent::UTestCommandResponseTypesComponent()
: ComponentUpdater(nullptr)
, Snapshot(nullptr)
{
}

void UTestCommandResponseTypesComponent::BeginDestroy()
{
	Super::BeginDestroy();

	Callbacks.Reset(nullptr);
	
	delete Snapshot;
	Snapshot = nullptr;
}

FComponentId UTestCommandResponseTypesComponent::GetComponentId()
{
    return FComponentId(ComponentId);
}

void UTestCommandResponseTypesComponent::Init(const TWeakPtr<SpatialOSConnection>& InConnection, const TWeakPtr<SpatialOSView>& InView,
                          worker::EntityId InEntityId, UCallbackDispatcher* InCallbackDispatcher)
{
    USpatialOsComponent::Init(InConnection, InView, InEntityId, InCallbackDispatcher);

	InCallbackDispatcher->AddComponentUpdateCallback(InEntityId, ComponentId, [this](UComponentUpdateOpWrapperBase& Op) { OnComponentUpdateDispatcherCallback(Op); });
	InCallbackDispatcher->AddAuthorityChangeCallback(InEntityId, ComponentId, [this](const worker::AuthorityChangeOp& Op) { OnAuthorityChangeDispatcherCallback(Op); });
	ComponentUpdater = NewObject<UTestCommandResponseTypesComponentUpdate>(this);

	auto LockedView = View.Pin();
	if(LockedView.IsValid())
	{
		Callbacks->Add(LockedView->OnCommandRequest<test::TestCommandResponseTypes::Commands::TestCommandUserType>(
			std::bind(&UTestCommandResponseTypesComponent::OnTestCommandUserTypeCommandRequestDispatcherCallback, this,
					  std::placeholders::_1)));
		Callbacks->Add(LockedView->OnCommandRequest<test::TestCommandResponseTypes::Commands::TestCommandCoordinates>(
			std::bind(&UTestCommandResponseTypesComponent::OnTestCommandCoordinatesCommandRequestDispatcherCallback, this,
					  std::placeholders::_1)));
		Callbacks->Add(LockedView->OnCommandRequest<test::TestCommandResponseTypes::Commands::TestCommandVector3d>(
			std::bind(&UTestCommandResponseTypesComponent::OnTestCommandVector3dCommandRequestDispatcherCallback, this,
					  std::placeholders::_1)));
		Callbacks->Add(LockedView->OnCommandRequest<test::TestCommandResponseTypes::Commands::TestCommandVector3f>(
			std::bind(&UTestCommandResponseTypesComponent::OnTestCommandVector3fCommandRequestDispatcherCallback, this,
					  std::placeholders::_1)));
	
	}
}

void  UTestCommandResponseTypesComponent::Disable(const worker::EntityId InEntityId, UCallbackDispatcher* CallbackDispatcher)
{
	if(CallbackDispatcher)
	{
		CallbackDispatcher->RemoveComponentUpdateCallback(InEntityId, ComponentId);
		CallbackDispatcher->RemoveAuthorityChangeCallback(InEntityId, ComponentId);
	}
}

void UTestCommandResponseTypesComponent::SendComponentUpdate(UTestCommandResponseTypesComponentUpdate* update)
{
	SendComponentUpdateInternal(update);
}

void UTestCommandResponseTypesComponent::ApplyInitialState(const UAddComponentOpWrapperBase& AddComponentOp)
{
	const UTestCommandResponseTypesAddComponentOp& DerivedOp = dynamic_cast<const UTestCommandResponseTypesAddComponentOp&>(AddComponentOp);
    auto update = test::TestCommandResponseTypes::Update::FromInitialData(*(DerivedOp.Data.data()));
    ApplyComponentUpdate(update);
}

void UTestCommandResponseTypesComponent::GenerateSnapshot()
{
	if (Snapshot)
	{
		return;
	}

	Snapshot = new FTestCommandResponseTypesComponentSnapshot();
	check(Snapshot);

}

void UTestCommandResponseTypesComponent::ReplicateChanges(float DeltaSeconds)
{
	TriggerAutomaticComponentUpdate(DeltaSeconds);
}

void UTestCommandResponseTypesComponent::TriggerAutomaticComponentUpdate(float DeltaSeconds)
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

void UTestCommandResponseTypesComponent::TriggerManualComponentUpdate()
{
	if (GetAuthority() == EAuthority::NotAuthoritative)
	{
		return;
	}

	ReplicateChangesInternal();
}



void UTestCommandResponseTypesComponent::OnComponentUpdateDispatcherCallback(UComponentUpdateOpWrapperBase& Op)
{
	UTestCommandResponseTypesComponentUpdate& ThisOp = dynamic_cast<UTestCommandResponseTypesComponentUpdate&>(Op);
    ApplyComponentUpdate(ThisOp.GetUnderlyingInternal());
}

void UTestCommandResponseTypesComponent::OnAuthorityChangeDispatcherCallback(const worker::AuthorityChangeOp& Op)
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

void UTestCommandResponseTypesComponent::ReplicateChangesInternal()
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

void UTestCommandResponseTypesComponent::SendComponentUpdateInternal(UTestCommandResponseTypesComponentUpdate* update)
{
	auto LockedConnection = Connection.Pin();

	if(LockedConnection.IsValid())
	{
	    LockedConnection->SendComponentUpdate<test::TestCommandResponseTypes>(EntityId, update->GetUnderlyingInternal());
	}
}

void UTestCommandResponseTypesComponent::ApplyComponentUpdate(const test::TestCommandResponseTypes::Update& update)
{

	NotifyUpdate(update);
}

void UTestCommandResponseTypesComponent::NotifyUpdate(const test::TestCommandResponseTypes::Update& update)
{

    OnComponentUpdate.Broadcast();

    if (!bIsComponentReady)
    {
        bIsComponentReady = true;
        OnComponentReady.Broadcast();
    }
}


void UTestCommandResponseTypesComponent::OnTestCommandUserTypeCommandRequestDispatcherCallback(
    const worker::CommandRequestOp<test::TestCommandResponseTypes::Commands::TestCommandUserType>& op)
{
    if (op.EntityId != EntityId)
    {
        return;
    }
    auto request = NewObject<UTestType1>()->Init(op.Request);
	
	UTestCommandUserTypeCommandResponder* TestCommandUserTypeResponder = NewObject<UTestCommandUserTypeCommandResponder>(this);
	TestCommandUserTypeResponder->Init(Connection, op.RequestId, request, op.CallerWorkerId);
	
    OnTestCommandUserTypeCommandRequest.Broadcast(TestCommandUserTypeResponder);
}
void UTestCommandResponseTypesComponent::OnTestCommandCoordinatesCommandRequestDispatcherCallback(
    const worker::CommandRequestOp<test::TestCommandResponseTypes::Commands::TestCommandCoordinates>& op)
{
    if (op.EntityId != EntityId)
    {
        return;
    }
    auto request = USpatialOSConversionFunctionLibrary::SpatialOsCoordinatesToUnrealCoordinates(FVector(static_cast<float>(op.Request.x()), static_cast<float>(op.Request.y()), static_cast<float>(op.Request.z())));
	
	UTestCommandCoordinatesCommandResponder* TestCommandCoordinatesResponder = NewObject<UTestCommandCoordinatesCommandResponder>(this);
	TestCommandCoordinatesResponder->Init(Connection, op.RequestId, request, op.CallerWorkerId);
	
    OnTestCommandCoordinatesCommandRequest.Broadcast(TestCommandCoordinatesResponder);
}
void UTestCommandResponseTypesComponent::OnTestCommandVector3dCommandRequestDispatcherCallback(
    const worker::CommandRequestOp<test::TestCommandResponseTypes::Commands::TestCommandVector3d>& op)
{
    if (op.EntityId != EntityId)
    {
        return;
    }
    auto request = FVector(static_cast<float>(op.Request.x()), static_cast<float>(op.Request.y()), static_cast<float>(op.Request.z()));
	
	UTestCommandVector3dCommandResponder* TestCommandVector3dResponder = NewObject<UTestCommandVector3dCommandResponder>(this);
	TestCommandVector3dResponder->Init(Connection, op.RequestId, request, op.CallerWorkerId);
	
    OnTestCommandVector3dCommandRequest.Broadcast(TestCommandVector3dResponder);
}
void UTestCommandResponseTypesComponent::OnTestCommandVector3fCommandRequestDispatcherCallback(
    const worker::CommandRequestOp<test::TestCommandResponseTypes::Commands::TestCommandVector3f>& op)
{
    if (op.EntityId != EntityId)
    {
        return;
    }
    auto request = FVector(static_cast<float>(op.Request.x()), static_cast<float>(op.Request.y()), static_cast<float>(op.Request.z()));
	
	UTestCommandVector3fCommandResponder* TestCommandVector3fResponder = NewObject<UTestCommandVector3fCommandResponder>(this);
	TestCommandVector3fResponder->Init(Connection, op.RequestId, request, op.CallerWorkerId);
	
    OnTestCommandVector3fCommandRequest.Broadcast(TestCommandVector3fResponder);
}
