// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include <improbable/worker.h>

#include "CoreMinimal.h"
#include "SpatialUpdateInterop.generated.h"

class USpatialOS;
class USpatialActorChannel;
class USpatialNetDriver;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialUpdateInterop, Log, All);

using FRPCHandler = const std::function<void(struct FFrame*, const worker::Entity, UPackageMap*)>;
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

class FSpatialTypeBinding
{
public:
	virtual void Init(USpatialUpdateInterop* UpdateInterop, UPackageMap* PackageMap);

	virtual void BindToView() = 0;
	virtual void UnbindFromView() = 0;
	virtual worker::ComponentId GetReplicatedGroupComponentId(EReplicatedPropertyGroup Group) const = 0;
	virtual void SendComponentUpdates(FOutBunch* OutgoingBunch, const worker::EntityId& EntityId) const = 0;
	virtual void SendRPCCommand(UFunction* Function, FFrame* RPCFrame, worker::EntityId Target) const = 0;

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


	void HandleRPCInvocation(AActor* TargetActor, UFunction* Function, FFrame* DuplicateFrame, worker::EntityId Target);
	void RegisterInteropType(UClass* Class, TSharedPtr<FSpatialTypeBinding> Binding);
	void UnregisterInteropType(UClass* Class);
	const FSpatialTypeBinding* GetTypeBindingByClass(UClass* Class) const;

	void SendSpatialUpdate(USpatialActorChannel* Channel, FOutBunch* OutgoingBunch);
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
};
