// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialTypeBinding.h"
#include "SpatialUpdateInterop.generated.h"

class USpatialOS;
class USpatialActorChannel;
class USpatialPackageMapClient;
class USpatialNetDriver;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialUpdateInterop, Log, All);

UCLASS()
class NUF_API USpatialUpdateInterop : public UObject
{
	GENERATED_BODY()
public:
	USpatialUpdateInterop();

	void Init(bool bClient, USpatialOS* Instance, USpatialNetDriver* Driver, FTimerManager* TimerManager);
	void Tick(float DeltaTime);

	USpatialActorChannel* GetClientActorChannel(const worker::EntityId& EntityId) const;
	void AddClientActorChannel(const worker::EntityId& EntityId, USpatialActorChannel* Channel);

	void RegisterInteropType(UClass* Class, USpatialTypeBinding* Binding);
	void UnregisterInteropType(UClass* Class);
	USpatialTypeBinding* GetTypeBindingByClass(UClass* Class) const;

	void SendSpatialUpdate(USpatialActorChannel* Channel, const TArray<uint16>& Changed);
	void ReceiveSpatialUpdate(USpatialActorChannel* Channel, FNetBitWriter& IncomingPayload);
	void InvokeRPC(const AActor* const TargetActor, const UFunction* const Function, FFrame* const DuplicateFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);

	USpatialOS* GetSpatialOS() const
	{
		return SpatialOSInstance;
	}

	USpatialNetDriver* GetNetDriver() const 
	{
		return NetDriver;	
	}

	FTimerManager& GetTimerManager() const
	{
		return *TimerManager;
	}

private:
	UPROPERTY()
	USpatialOS* SpatialOSInstance;

	UPROPERTY()
	USpatialNetDriver* NetDriver;

	UPROPERTY()
	bool bIsClient;

	UPROPERTY()
	USpatialPackageMapClient* PackageMap;

	// Timer manager.
	FTimerManager* TimerManager;

	// Type interop bindings.
	UPROPERTY()
	TMap<UClass*, USpatialTypeBinding*> TypeBinding;

	// On clients, there is a 1 to 1 mapping between an actor and an actor channel (as there's just one NetConnection).
	TMap<worker::EntityId, USpatialActorChannel*> EntityToClientActorChannel;
private:
	void SetComponentInterests(USpatialActorChannel* ActorChannel, const worker::EntityId& EntityId);

	friend class USpatialInteropBlock;
};
