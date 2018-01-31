// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// Note that this file has been generated automatically
#pragma once

#include <improbable/worker.h>
#include <improbable/view.h>
#include <unreal/generated/UnrealCharacter.h>
#include <unreal/core_types.h>
#include "SpatialHandlePropertyMap.h"
#include "SpatialTypeBinding.h"
#include "SpatialTypeBinding_Character.generated.h"

UCLASS()
class USpatialTypeBinding_Character : public USpatialTypeBinding
{
	GENERATED_BODY()

public:
	static const FRepHandlePropertyMap& GetHandlePropertyMap();

	void Init(USpatialUpdateInterop* InUpdateInterop, USpatialPackageMapClient* InPackageMap) override;
	void BindToView() override;
	void UnbindFromView() override;
	worker::ComponentId GetReplicatedGroupComponentId(EReplicatedPropertyGroup Group) const override;

	worker::Entity CreateActorEntity(const FVector& Position, const FString& Metadata, const FPropertyChangeState& InitialChanges, USpatialActorChannel* Channel) const override;
	void SendComponentUpdates(const FPropertyChangeState& Changes, USpatialActorChannel* Channel, const worker::EntityId& EntityId) const override;
	void SendRPCCommand(AActor* TargetActor, const UFunction* const Function, FFrame* const DuplicateFrame, USpatialActorChannel* Channel) override;

	void ApplyQueuedStateToChannel(USpatialActorChannel* ActorChannel) override;

private:
	worker::Dispatcher::CallbackKey SingleClientAddCallback;
	worker::Dispatcher::CallbackKey SingleClientUpdateCallback;
	worker::Dispatcher::CallbackKey MultiClientAddCallback;
	worker::Dispatcher::CallbackKey MultiClientUpdateCallback;

	// Pending updates.
	TMap<worker::EntityId, improbable::unreal::UnrealCharacterSingleClientReplicatedData::Data> PendingSingleClientData;
	TMap<worker::EntityId, improbable::unreal::UnrealCharacterMultiClientReplicatedData::Data> PendingMultiClientData;

	// RPC sender and receiver callbacks.
	using FRPCSender = void (USpatialTypeBinding_Character::*)(worker::Connection* const, struct FFrame* const, USpatialActorChannel*, AActor*);
	TMap<FName, FRPCSender> RPCToSenderMap;
	TArray<worker::Dispatcher::CallbackKey> RPCReceiverCallbacks;

	// Component update helper functions.
	void BuildSpatialComponentUpdate(
		const FPropertyChangeState& Changes,
		USpatialActorChannel* Channel,
		improbable::unreal::UnrealCharacterSingleClientReplicatedData::Update& SingleClientUpdate,
		bool& bSingleClientUpdateChanged,
		improbable::unreal::UnrealCharacterMultiClientReplicatedData::Update& MultiClientUpdate,
		bool& bMultiClientUpdateChanged) const;
	void ApplyUpdateToSpatial_SingleClient(
		const uint8* RESTRICT Data,
		int32 Handle,
		UProperty* Property,
		USpatialActorChannel* Channel,
		improbable::unreal::UnrealCharacterSingleClientReplicatedData::Update& OutUpdate) const;
	void ApplyUpdateToSpatial_MultiClient(
		const uint8* RESTRICT Data,
		int32 Handle,
		UProperty* Property,
		USpatialActorChannel* Channel,
		improbable::unreal::UnrealCharacterMultiClientReplicatedData::Update& OutUpdate) const;
	void ReceiveUpdateFromSpatial_SingleClient(
		USpatialActorChannel* ActorChannel,
		const improbable::unreal::UnrealCharacterSingleClientReplicatedData::Update& Update) const;
	void ReceiveUpdateFromSpatial_MultiClient(
		USpatialActorChannel* ActorChannel,
		const improbable::unreal::UnrealCharacterMultiClientReplicatedData::Update& Update) const;

	// RPC sender functions.
	void ClientCheatWalk_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, AActor* TargetActor);
	void ClientCheatGhost_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, AActor* TargetActor);
	void ClientCheatFly_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, AActor* TargetActor);

	// RPC sender response functions.
	void ClientCheatWalk_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatwalk>& Op);
	void ClientCheatGhost_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatghost>& Op);
	void ClientCheatFly_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatfly>& Op);

	// RPC receiver functions.
	void ClientCheatWalk_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatwalk>& Op);
	void ClientCheatGhost_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatghost>& Op);
	void ClientCheatFly_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatfly>& Op);
};
