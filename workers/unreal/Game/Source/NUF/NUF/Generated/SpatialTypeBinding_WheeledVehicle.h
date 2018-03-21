// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// Note that this file has been generated automatically
#pragma once

#include <improbable/worker.h>
#include <improbable/view.h>
#include <improbable/unreal/core_types.h>
#include <improbable/unreal/unreal_metadata.h>
#include <improbable/unreal/generated/UnrealWheeledVehicle.h>
#include "ScopedViewCallbacks.h"
#include "../SpatialTypeBinding.h"
#include "SpatialTypeBinding_WheeledVehicle.generated.h"

UCLASS()
class USpatialTypeBinding_WheeledVehicle : public USpatialTypeBinding
{
	GENERATED_BODY()

public:
	const FRepHandlePropertyMap& GetRepHandlePropertyMap() const override;
	const FMigratableHandlePropertyMap& GetMigratableHandlePropertyMap() const override;

	UClass* GetBoundClass() const override;

	void Init(USpatialInterop* InInterop, USpatialPackageMapClient* InPackageMap) override;
	void BindToView() override;
	void UnbindFromView() override;
	worker::ComponentId GetReplicatedGroupComponentId(EReplicatedPropertyGroup Group) const override;

	worker::Entity CreateActorEntity(const FString& ClientWorkerId, const FVector& Position, const FString& Metadata, const FPropertyChangeState& InitialChanges, USpatialActorChannel* Channel) const override;
	void SendComponentUpdates(const FPropertyChangeState& Changes, USpatialActorChannel* Channel, const FEntityId& EntityId) const override;
	void SendRPCCommand(UObject* TargetObject, const UFunction* const Function, FFrame* const Frame) override;

	void ReceiveAddComponent(USpatialActorChannel* Channel, UAddComponentOpWrapperBase* AddComponentOp) const override;
	void ApplyQueuedStateToChannel(USpatialActorChannel* ActorChannel) override;

private:
	improbable::unreal::callbacks::FScopedViewCallbacks ViewCallbacks;

	// Pending updates.
	TMap<FEntityId, improbable::unreal::UnrealWheeledVehicleSingleClientRepData::Data> PendingSingleClientData;
	TMap<FEntityId, improbable::unreal::UnrealWheeledVehicleMultiClientRepData::Data> PendingMultiClientData;

	// RPC to sender map.
	using FRPCSender = void (USpatialTypeBinding_WheeledVehicle::*)(worker::Connection* const, struct FFrame* const, UObject*);
	TMap<FName, FRPCSender> RPCToSenderMap;

	// Component update helper functions.
	void BuildSpatialComponentUpdate(
		const FPropertyChangeState& Changes,
		USpatialActorChannel* Channel,
		improbable::unreal::UnrealWheeledVehicleSingleClientRepData::Update& SingleClientUpdate,
		bool& bSingleClientUpdateChanged,
		improbable::unreal::UnrealWheeledVehicleMultiClientRepData::Update& MultiClientUpdate,
		bool& bMultiClientUpdateChanged,
		improbable::unreal::UnrealWheeledVehicleMigratableData::Update& MigratedDataUpdate,
		bool& bMigratedDataUpdateChanged) const;
	void ServerSendUpdate_SingleClient(const uint8* RESTRICT Data, int32 Handle, UProperty* Property, USpatialActorChannel* Channel, improbable::unreal::UnrealWheeledVehicleSingleClientRepData::Update& OutUpdate) const;
	void ServerSendUpdate_MultiClient(const uint8* RESTRICT Data, int32 Handle, UProperty* Property, USpatialActorChannel* Channel, improbable::unreal::UnrealWheeledVehicleMultiClientRepData::Update& OutUpdate) const;
	void ServerSendUpdate_Migratable(const uint8* RESTRICT Data, int32 Handle, UProperty* Property, USpatialActorChannel* Channel, improbable::unreal::UnrealWheeledVehicleMigratableData::Update& OutUpdate) const;
	void ReceiveUpdate_SingleClient(USpatialActorChannel* ActorChannel, const improbable::unreal::UnrealWheeledVehicleSingleClientRepData::Update& Update) const;
	void ReceiveUpdate_MultiClient(USpatialActorChannel* ActorChannel, const improbable::unreal::UnrealWheeledVehicleMultiClientRepData::Update& Update) const;
	void ReceiveUpdate_Migratable(USpatialActorChannel* ActorChannel, const improbable::unreal::UnrealWheeledVehicleMigratableData::Update& Update) const;

	// RPC command sender functions.
	void ServerUpdateState_SendCommand(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);

	// RPC command request handler functions.
	void ServerUpdateState_OnCommandRequest(const worker::CommandRequestOp<improbable::unreal::UnrealWheeledVehicleServerRPCs::Commands::Serverupdatestate>& Op);

	// RPC command response handler functions.
	void ServerUpdateState_OnCommandResponse(const worker::CommandResponseOp<improbable::unreal::UnrealWheeledVehicleServerRPCs::Commands::Serverupdatestate>& Op);
};
