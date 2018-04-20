// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialOSComponent.h"

#include "CallbackDispatcher.h"
#include "Commander.h"
#include "EntityPipeline.h"
#include "SpatialGDKViewTypes.h"
#include "SpatialGDKWorkerTypes.h"

USpatialOsComponent::USpatialOsComponent()
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

void USpatialOsComponent::Init(const TWeakPtr<SpatialOSConnection>& InConnection,
							   const TWeakPtr<SpatialOSView>& InView, worker::EntityId InEntityId,
							   UCallbackDispatcher* CallbackDispatcher)
{
	this->Connection = InConnection;
	this->View = InView;
	this->EntityId = InEntityId;
	Callbacks.Reset(new improbable::unreal::callbacks::FScopedViewCallbacks(View));
}

void USpatialOsComponent::BeginDestroy()
{
	Super::BeginDestroy();
	Callbacks.Reset();
}

void USpatialOsComponent::ApplyInitialAuthority(const worker::AuthorityChangeOp& AuthChangeOp)
{
	OnAuthorityChangeDispatcherCallback(AuthChangeOp);
}

worker::EntityId USpatialOsComponent::GetEntityId()
{
  return EntityId;
}

bool USpatialOsComponent::HasAuthority()
{
	return Authority == EAuthority::Authoritative || Authority == EAuthority::AuthorityLossImminent;
}

EAuthority USpatialOsComponent::GetAuthority()
{
	return Authority;
}

bool USpatialOsComponent::IsComponentReady()
{
	return bIsComponentReady;
}

void USpatialOsComponent::SendAuthorityLossImminentAcknowledgement()
{
	auto LockedConnection = Connection.Pin();
	if (LockedConnection.IsValid())
	{
		LockedConnection->SendAuthorityLossImminentAcknowledgement(
			this->EntityId, this->GetComponentId().ToSpatialComponentId());
	}
}

void USpatialOsComponent::OnAuthorityChangeDispatcherCallback(const worker::AuthorityChangeOp& op)
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

void USpatialOsComponent::OnRemoveComponentDispatcherCallback(const worker::RemoveComponentOp& op)
{
	if (op.EntityId != EntityId)
	{
		return;
	}
	bIsComponentReady = false;
}

UCommander* USpatialOsComponent::SendCommand()
{
	if (Commander == nullptr)
	{
		Commander =
			NewObject<UCommander>(this, UCommander::StaticClass())->Init(this, Connection, View);
	}
	return Commander;
}