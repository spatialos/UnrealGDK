// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include <improbable/worker.h>

#include "CoreMinimal.h"
#include "SpatialUpdateInterop.generated.h"

class USpatialOS;
class USpatialActorChannel;
class USpatialNetDriver;

class FOutBunch;

struct FTypeBinding
{
	worker::Dispatcher::CallbackKey SingleClientUpdateCallback;
	worker::Dispatcher::CallbackKey MultiClientUpdateCallback;
	worker::ComponentId SingleClientComponentId;
	TFunction<void(FOutBunch*, worker::Connection*, const worker::EntityId)> SendSpatialUpdateFunction;
};

using TypeBindingMap = TMap<UClass*, FTypeBinding>;

UCLASS()
class NUF_API USpatialUpdateInterop : public UObject
{
	GENERATED_BODY()
public:
	USpatialUpdateInterop();

	void Init(bool bClient, USpatialOS* Instance, USpatialNetDriver* Driver);
	void Tick(float DeltaTime);

	USpatialActorChannel* GetClientActorChannel(const worker::EntityId& EntityId) const;

	void RegisterInteropType(UClass* Class, FTypeBinding Binding);
	void UnregisterInteropType(UClass* Class);
	const FTypeBinding* GetTypeBindingByClass(UClass* Class) const;

	void SendSpatialUpdate(USpatialActorChannel* Channel, FOutBunch* BunchPtr);
	void ReceiveSpatialUpdate(USpatialActorChannel* Channel, FNetBitWriter& Payload);

private:
	UPROPERTY()
	USpatialOS* SpatialOSInstance;

	UPROPERTY()
	USpatialNetDriver* NetDriver;

	UPROPERTY()
	bool bIsClient;

	// On clients, there is a 1 to 1 mapping between an actor and an actor channel (as there's just one NetConnection).
	TMap<worker::EntityId, USpatialActorChannel*> EntityToClientActorChannel;

	// Type interop registration.
	TypeBindingMap TypeBinding;

	worker::Dispatcher::CallbackKey ComponentUpdateCallback;

private:
	void SetComponentInterests(USpatialActorChannel* ActorChannel, const worker::EntityId& EntityId);
};
