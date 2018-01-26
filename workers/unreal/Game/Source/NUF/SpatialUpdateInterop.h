// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialTypeBinding.h"
#include "SpatialUpdateInterop.generated.h"

class USpatialOS;
class USpatialActorChannel;
class USpatialNetDriver;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialUpdateInterop, Log, All);

UCLASS()
class NUF_API USpatialUpdateInterop : public UObject
{
	GENERATED_BODY()
public:
	USpatialUpdateInterop() = default;
	~USpatialUpdateInterop() = default;

	void Init(bool bClient, USpatialOS* Instance, USpatialNetDriver* Driver);
	void Tick(float DeltaTime);

	USpatialActorChannel* GetClientActorChannel(const worker::EntityId& EntityId) const;
	void AddClientActorChannel(const worker::EntityId& EntityId, USpatialActorChannel* Channel);

	void HandleRPCInvocation(const AActor* const TargetActor, const UFunction* const Function, FFrame* const DuplicateFrame, const worker::EntityId& Target) const;
	void RegisterInteropType(UClass* Class, USpatialTypeBinding* Binding);
	void UnregisterInteropType(UClass* Class);
	const USpatialTypeBinding* GetTypeBindingByClass(UClass* Class) const;

	void SendSpatialUpdate(USpatialActorChannel* Channel, const TArray<uint16>& Changed);
	void ReceiveSpatialUpdate(USpatialActorChannel* Channel, FNetBitWriter& IncomingPayload);

	USpatialOS* GetSpatialOS() const
	{
		return SpatialOSInstance;
	}

	USpatialNetDriver* GetNetDriver() const 
	{
		return NetDriver;	
	}

private:
	UPROPERTY()
	USpatialOS* SpatialOSInstance;

	UPROPERTY()
	USpatialNetDriver* NetDriver;

	UPROPERTY()
	bool bIsClient;

	UPROPERTY()
	UPackageMap* PackageMap;

	// Type interop bindings.
	UPROPERTY()
	TMap<UClass*, USpatialTypeBinding*> TypeBinding;

	// On clients, there is a 1 to 1 mapping between an actor and an actor channel (as there's just one NetConnection).
	TMap<worker::EntityId, USpatialActorChannel*> EntityToClientActorChannel;

private:
	void SetComponentInterests(USpatialActorChannel* ActorChannel, const worker::EntityId& EntityId);

	friend class USpatialInteropBlock;
};
