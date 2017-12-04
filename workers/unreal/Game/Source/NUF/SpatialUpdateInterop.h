// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include <improbable/worker.h>

#include "CoreMinimal.h"
#include "SpatialUpdateInterop.generated.h"

class USpatialOS;
class USpatialActorChannel;
class USpatialNetDriver;

UCLASS()
class NUF_API USpatialUpdateInterop : public UObject
{
	GENERATED_BODY()
public:
	USpatialUpdateInterop();

	void Init(bool bClient, USpatialOS* Instance, USpatialNetDriver* Driver);

	void Tick(float DeltaTime);

private:
	UPROPERTY()
	USpatialOS* SpatialOSInstance;

	UPROPERTY()
	USpatialNetDriver* NetDriver;

	UPROPERTY()
	bool bIsClient;

	// TODO: Remove this once Girays stuff is merged. See the implementation of Tick(...)
	UPROPERTY()
	bool WaitingForGuid;

	// On clients, there is a 1 to 1 mapping between an actor and an actor channel (as there's just one NetConnection).
	TMap<worker::EntityId, USpatialActorChannel*> EntityToClientActorChannel;

	worker::Dispatcher::CallbackKey ComponentUpdateCallback;
};
