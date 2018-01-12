// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include <improbable/worker.h>

#include "CoreMinimal.h"
#include "Net/RepLayout.h"
#include "SpatialUpdateInterop.generated.h"

class USpatialOS;
class USpatialActorChannel;
class USpatialNetDriver;

class FOutBunch;

enum EReplicatedPropertyGroup
{
	GROUP_SingleClient,
	GROUP_MultiClient
};

inline EReplicatedPropertyGroup GetGroupFromCondition(ELifetimeCondition Condition)
{
	switch (Condition)
	{
	case COND_AutonomousOnly:
	case COND_OwnerOnly:
		return GROUP_SingleClient;
	default:
		return GROUP_MultiClient;
	}
}

class USpatialUpdateInterop;

//todo: (cc: @david) This code is merged into master, but the UObjects held inside a 
// non-uclass will not be ref counted properly and may be removed under us.
// It will mostly "work" because it's only used under USpatialUpdateInterop's context, but I think we can do something more robust.
class FSpatialTypeBinding
{
public:
	void Init(USpatialUpdateInterop* UpdateInterop, UPackageMap* PackageMap);

	virtual void BindToView() = 0;
	virtual void UnbindFromView() = 0;
	virtual worker::ComponentId GetReplicatedGroupComponentId(EReplicatedPropertyGroup Group) const = 0;
	virtual void SendComponentUpdates(FInBunch* OutgoingBunch, const worker::EntityId& EntityId) const = 0;
	virtual void SendComponentUpdates(const TArray<uint16>& Changed,
									  const uint8* RESTRICT SourceData,
									  const TArray<FRepLayoutCmd>& Cmds,
									  const TArray<FHandleToCmdIndex>& BaseHandleToCmdIndex,
									  const worker::EntityId& EntityId) const = 0;

protected:
	USpatialUpdateInterop* UpdateInterop;
	UPackageMap* PackageMap;
};

UCLASS()
class NUF_API USpatialUpdateInterop : public UObject
{
	GENERATED_BODY()
public:
	USpatialUpdateInterop();

	void Init(bool bClient, USpatialOS* Instance, USpatialNetDriver* Driver);
	void Tick(float DeltaTime);

	USpatialActorChannel* GetClientActorChannel(const worker::EntityId& EntityId) const;

	void RegisterInteropType(UClass* Class, TSharedPtr<FSpatialTypeBinding> Binding);
	void UnregisterInteropType(UClass* Class);
	const FSpatialTypeBinding* GetTypeBindingByClass(UClass* Class) const;

	void SendSpatialUpdate(USpatialActorChannel* Channel, FOutBunch* OutgoingBunch);
	void SendSpatialUpdate(USpatialActorChannel* Channel, const TArray< uint16 >& Changed);
	void ReceiveSpatialUpdate(USpatialActorChannel* Channel, FNetBitWriter& IncomingPayload);

	USpatialOS* GetSpatialOS() const
	{
		return SpatialOSInstance;
	}

private:
	UPROPERTY()
	USpatialOS* SpatialOSInstance;

	UPROPERTY()
	USpatialNetDriver* NetDriver;

	UPROPERTY()
	bool bIsClient;

	// Type interop bindings.
	TMap<UClass*, TSharedPtr<FSpatialTypeBinding>> TypeBinding;

	// On clients, there is a 1 to 1 mapping between an actor and an actor channel (as there's just one NetConnection).
	TMap<worker::EntityId, USpatialActorChannel*> EntityToClientActorChannel;

private:
	void SetComponentInterests(USpatialActorChannel* ActorChannel, const worker::EntityId& EntityId);

	friend class USpatialInteropBlock;
};
