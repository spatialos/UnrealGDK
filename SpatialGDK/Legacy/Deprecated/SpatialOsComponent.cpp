// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialOSComponent.h"

#include "CallbackDispatcher.h"
#include "Commander.h"
#include "EntityPipeline.h"
#include "SpatialGDKViewTypes.h"
#include "SpatialGDKWorkerTypes.h"

USpatialOSComponent::USpatialOSComponent()
: MaxUpdatesPerSecond(30)
, Connection(nullptr)
, View(nullptr)
, EntityId(0)
, Callbacks(nullptr)
, TimeUntilNextUpdate(0)
, Authority(EAuthority::NotAuthoritative)
, bIsComponentReady(false)
, bHasEventQueued(false)
, Commander(nullptr)
{
}

void USpatialOSComponent::Init(const TWeakPtr<SpatialOSConnection>& InConnection,
							   const TWeakPtr<SpatialOSView>& InView, worker::EntityId InEntityId,
							   UCallbackDispatcher* CallbackDispatcher)
{
	this->Connection = InConnection;
	this->View = InView;
	this->EntityId = InEntityId;
	Callbacks.Reset(new improbable::unreal::callbacks::FScopedViewCallbacks(View));
}

void USpatialOSComponent::BeginDestroy()
{
	Super::BeginDestroy();
	Callbacks.Reset();
}

void USpatialOSComponent::ApplyInitialAuthority(const worker::AuthorityChangeOp& AuthChangeOp)
{
	OnAuthorityChangeDispatcherCallback(AuthChangeOp);
}

worker::EntityId USpatialOsComponent::GetEntityId()
{
  return EntityId;
}

bool USpatialOSComponent::HasAuthority()
{
	return Authority == EAuthority::Authoritative || Authority == EAuthority::AuthorityLossImminent;
}

EAuthority USpatialOSComponent::GetAuthority()
{
	return Authority;
}

bool USpatialOSComponent::IsComponentReady()
{
	return bIsComponentReady;
}

void USpatialOSComponent::SendAuthorityLossImminentAcknowledgement()
{
	auto LockedConnection = Connection.Pin();
	if (LockedConnection.IsValid())
	{
		LockedConnection->SendAuthorityLossImminentAcknowledgement(
			this->EntityId, this->GetComponentId().ToSpatialComponentId());
	}
}

void USpatialOSComponent::OnAuthorityChangeDispatcherCallback(const worker::AuthorityChangeOp& op)
{
	if (op.EntityId != EntityId)
	{
		return;
	}

	switch (op.Authority)
	{
		case worker::Authority::kNotAuthoritative:
			Authority = EAuthority::NotAuthoritative;
			break;
		case worker::Authority::kAuthoritative:
			Authority = EAuthority::Authoritative;
			break;
		case worker::Authority::kAuthorityLossImminent:
			Authority = EAuthority::AuthorityLossImminent;
			break;
	}
	OnAuthorityChange.Broadcast(Authority);
}

void USpatialOSComponent::OnRemoveComponentDispatcherCallback(const worker::RemoveComponentOp& op)
{
	if (op.EntityId != EntityId)
	{
		return;
	}
	bIsComponentReady = false;
}

UCommander* USpatialOSComponent::SendCommand()
{
	if (Commander == nullptr)
	{
		Commander =
			NewObject<UCommander>(this, UCommander::StaticClass())->Init(this, Connection, View);
	}
	return Commander;
}