// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// Note that this file has been generated automatically
#pragma once

#include <improbable/worker.h>
#include <improbable/view.h>
#include <improbable/unreal/core_types.h>
#include <improbable/unreal/unreal_metadata.h>
#include <improbable/unreal/generated/UnrealWheeledVehicle.h>
#include "../SpatialTypeBinding.h"
#include "WheeledVehicle.h"
#include "WheeledVehicleMovementComponent.h"
#include "SpatialTypeBinding_WheeledVehicle.generated.h"

UCLASS()
class USpatialTypeBinding_WheeledVehicle : public USpatialTypeBinding
{
	GENERATED_BODY()

public:
	static const FRepHandlePropertyMap& GetHandlePropertyMap();

	UClass* GetBoundClass() const override;

	void Init(USpatialInterop* InInterop, USpatialPackageMapClient* InPackageMap) override;
	void BindToView() override;
	void UnbindFromView() override;
	worker::ComponentId GetReplicatedGroupComponentId(EReplicatedPropertyGroup Group) const override;

	worker::Entity CreateActorEntity(const FString& ClientWorkerId, const FVector& Position, const FString& Metadata, const FPropertyChangeState& InitialChanges, USpatialActorChannel* Channel) const override;
	void SendComponentUpdates(const FPropertyChangeState& Changes, USpatialActorChannel* Channel, const worker::EntityId& EntityId) const override;
	void SendRPCCommand(UObject* TargetObject, const UFunction* const Function, FFrame* const Frame) override;
	void ApplyQueuedStateToChannel(USpatialActorChannel* ActorChannel) override;

private:
	worker::Dispatcher::CallbackKey SingleClientAddCallback;
	worker::Dispatcher::CallbackKey SingleClientUpdateCallback;
	worker::Dispatcher::CallbackKey MultiClientAddCallback;
	worker::Dispatcher::CallbackKey MultiClientUpdateCallback;

	// Pending updates.
	TMap<worker::EntityId, improbable::unreal::UnrealWheeledVehicleSingleClientReplicatedData::Data> PendingSingleClientData;
	TMap<worker::EntityId, improbable::unreal::UnrealWheeledVehicleMultiClientReplicatedData::Data> PendingMultiClientData;

	// RPC sender and receiver callbacks.
	using FRPCSender = void (USpatialTypeBinding_WheeledVehicle::*)(worker::Connection* const, struct FFrame* const, UObject*);
	TMap<FName, FRPCSender> RPCToSenderMap;
	TArray<worker::Dispatcher::CallbackKey> RPCReceiverCallbacks;

	// Component update helper functions.
	void BuildSpatialComponentUpdate(
		const FPropertyChangeState& Changes,
		USpatialActorChannel* Channel,
		improbable::unreal::UnrealWheeledVehicleSingleClientReplicatedData::Update& SingleClientUpdate,
		bool& bSingleClientUpdateChanged,
		improbable::unreal::UnrealWheeledVehicleMultiClientReplicatedData::Update& MultiClientUpdate,
		bool& bMultiClientUpdateChanged) const;
	void ServerSendUpdate_SingleClient(
		const uint8* RESTRICT Data,
		int32 Handle,
		UProperty* Property,
		USpatialActorChannel* Channel,
		improbable::unreal::UnrealWheeledVehicleSingleClientReplicatedData::Update& OutUpdate) const;
	void ServerSendUpdate_MultiClient(
		const uint8* RESTRICT Data,
		int32 Handle,
		UProperty* Property,
		USpatialActorChannel* Channel,
		improbable::unreal::UnrealWheeledVehicleMultiClientReplicatedData::Update& OutUpdate) const;
	void ClientReceiveUpdate_SingleClient(
		USpatialActorChannel* ActorChannel,
		const improbable::unreal::UnrealWheeledVehicleSingleClientReplicatedData::Update& Update) const;
	void ClientReceiveUpdate_MultiClient(
		USpatialActorChannel* ActorChannel,
		const improbable::unreal::UnrealWheeledVehicleMultiClientReplicatedData::Update& Update) const;

	// RPC command sender functions.
	void ServerUpdateState_SendCommand(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);

	// RPC command request handler functions.
	void ServerUpdateState_OnCommandRequest(const worker::CommandRequestOp<improbable::unreal::UnrealWheeledVehicleServerRPCs::Commands::Serverupdatestate>& Op);

	// RPC command response handler functions.
	void ServerUpdateState_OnCommandResponse(const worker::CommandResponseOp<improbable::unreal::UnrealWheeledVehicleServerRPCs::Commands::Serverupdatestate>& Op);
};
