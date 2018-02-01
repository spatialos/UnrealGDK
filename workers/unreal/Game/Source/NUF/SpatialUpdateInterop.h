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

using FUntypedRequestId = decltype(worker::RequestId<void>::Id);
struct FRPCRequestResult
{
	UObject* UnresolvedObject;
	FUntypedRequestId RequestId;

	FRPCRequestResult(UObject* UnresolvedObject) : UnresolvedObject{ UnresolvedObject }, RequestId{ 0 } {}
	FRPCRequestResult(FUntypedRequestId RequestId) : UnresolvedObject{ nullptr }, RequestId{ RequestId } {}
};

// Storage for a command request.
class FCommandRequestContext
{
public:
	using FRequestFunction = TFunction<FRPCRequestResult()>;

	FCommandRequestContext(FRequestFunction SendCommandRequest) :
		SendCommandRequest{ SendCommandRequest },
		NumFailures{ 0 }
	{
	}

	FRequestFunction SendCommandRequest;
	uint32 NumFailures;
};

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
	void InvokeRPC(AActor* TargetActor, const UFunction* const Function, FFrame* const DuplicateFrame, USpatialActorChannel* Channel);

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

	void SendCommandRequest(FCommandRequestContext::FRequestFunction Function);
	void HandleCommandResponse(const FString& RPCName, FUntypedRequestId RequestId, const worker::EntityId& EntityId, const worker::StatusCode& StatusCode, const FString& Message);

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

	// Outgoing RPCs (for retry logic).
	TMap<FUntypedRequestId, FCommandRequestContext> OutgoingRPCs;
private:
	void SetComponentInterests(USpatialActorChannel* ActorChannel, const worker::EntityId& EntityId);

	friend class USpatialInteropBlock;
};
