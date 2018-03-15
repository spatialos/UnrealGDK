// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// Note that this file has been generated automatically
#pragma once

#include <improbable/worker.h>
#include <improbable/view.h>
#include <improbable/unreal/core_types.h>
#include <improbable/unreal/unreal_metadata.h>
#include <improbable/unreal/generated/UnrealCharacter.h>
#include "ScopedViewCallbacks.h"
#include "../SpatialTypeBinding.h"
#include "SpatialTypeBinding_Character.generated.h"

UCLASS()
class USpatialTypeBinding_Character : public USpatialTypeBinding
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
	void SendComponentUpdates(const FPropertyChangeState& Changes, USpatialActorChannel* Channel, const FEntityId& EntityId) const override;
	void SendRPCCommand(UObject* TargetObject, const UFunction* const Function, FFrame* const Frame) override;

	void ReceiveAddComponent(USpatialActorChannel* Channel, UAddComponentOpWrapperBase* AddComponentOp) const override;
	void ApplyQueuedStateToChannel(USpatialActorChannel* ActorChannel) override;

private:
	improbable::unreal::callbacks::FScopedViewCallbacks ViewCallbacks;

	// Pending updates.
	TMap<FEntityId, improbable::unreal::UnrealCharacterSingleClientReplicatedData::Data> PendingSingleClientData;
	TMap<FEntityId, improbable::unreal::UnrealCharacterMultiClientReplicatedData::Data> PendingMultiClientData;

	// RPC to sender map.
	using FRPCSender = void (USpatialTypeBinding_Character::*)(worker::Connection* const, struct FFrame* const, UObject*);
	TMap<FName, FRPCSender> RPCToSenderMap;

	// Component update helper functions.
	void BuildSpatialComponentUpdate(
		const FPropertyChangeState& Changes,
		USpatialActorChannel* Channel,
		improbable::unreal::UnrealCharacterSingleClientReplicatedData::Update& SingleClientUpdate,
		bool& bSingleClientUpdateChanged,
		improbable::unreal::UnrealCharacterMultiClientReplicatedData::Update& MultiClientUpdate,
		bool& bMultiClientUpdateChanged) const;
	void ServerSendUpdate_SingleClient(
		const uint8* RESTRICT Data,
		int32 Handle,
		UProperty* Property,
		USpatialActorChannel* Channel,
		improbable::unreal::UnrealCharacterSingleClientReplicatedData::Update& OutUpdate) const;
	void ServerSendUpdate_MultiClient(
		const uint8* RESTRICT Data,
		int32 Handle,
		UProperty* Property,
		USpatialActorChannel* Channel,
		improbable::unreal::UnrealCharacterMultiClientReplicatedData::Update& OutUpdate) const;
	void ClientReceiveUpdate_SingleClient(
		USpatialActorChannel* ActorChannel,
		const improbable::unreal::UnrealCharacterSingleClientReplicatedData::Update& Update) const;
	void ClientReceiveUpdate_MultiClient(
		USpatialActorChannel* ActorChannel,
		const improbable::unreal::UnrealCharacterMultiClientReplicatedData::Update& Update) const;

	// RPC command sender functions.
	void RootMotionDebugClientPrintOnScreen_SendCommand(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientCheatWalk_SendCommand(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientCheatGhost_SendCommand(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientCheatFly_SendCommand(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientVeryShortAdjustPosition_SendCommand(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientAdjustRootMotionSourcePosition_SendCommand(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientAdjustRootMotionPosition_SendCommand(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientAdjustPosition_SendCommand(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientAckGoodMove_SendCommand(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ServerMoveOld_SendCommand(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ServerMoveDualHybridRootMotion_SendCommand(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ServerMoveDual_SendCommand(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ServerMove_SendCommand(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);

	// RPC command request handler functions.
	void RootMotionDebugClientPrintOnScreen_OnCommandRequest(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Rootmotiondebugclientprintonscreen>& Op);
	void ClientCheatWalk_OnCommandRequest(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatwalk>& Op);
	void ClientCheatGhost_OnCommandRequest(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatghost>& Op);
	void ClientCheatFly_OnCommandRequest(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatfly>& Op);
	void ClientVeryShortAdjustPosition_OnCommandRequest(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientveryshortadjustposition>& Op);
	void ClientAdjustRootMotionSourcePosition_OnCommandRequest(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustrootmotionsourceposition>& Op);
	void ClientAdjustRootMotionPosition_OnCommandRequest(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustrootmotionposition>& Op);
	void ClientAdjustPosition_OnCommandRequest(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustposition>& Op);
	void ClientAckGoodMove_OnCommandRequest(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientackgoodmove>& Op);
	void ServerMoveOld_OnCommandRequest(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermoveold>& Op);
	void ServerMoveDualHybridRootMotion_OnCommandRequest(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermovedualhybridrootmotion>& Op);
	void ServerMoveDual_OnCommandRequest(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermovedual>& Op);
	void ServerMove_OnCommandRequest(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermove>& Op);

	// RPC command response handler functions.
	void RootMotionDebugClientPrintOnScreen_OnCommandResponse(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Rootmotiondebugclientprintonscreen>& Op);
	void ClientCheatWalk_OnCommandResponse(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatwalk>& Op);
	void ClientCheatGhost_OnCommandResponse(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatghost>& Op);
	void ClientCheatFly_OnCommandResponse(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatfly>& Op);
	void ClientVeryShortAdjustPosition_OnCommandResponse(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientveryshortadjustposition>& Op);
	void ClientAdjustRootMotionSourcePosition_OnCommandResponse(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustrootmotionsourceposition>& Op);
	void ClientAdjustRootMotionPosition_OnCommandResponse(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustrootmotionposition>& Op);
	void ClientAdjustPosition_OnCommandResponse(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustposition>& Op);
	void ClientAckGoodMove_OnCommandResponse(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientackgoodmove>& Op);
	void ServerMoveOld_OnCommandResponse(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermoveold>& Op);
	void ServerMoveDualHybridRootMotion_OnCommandResponse(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermovedualhybridrootmotion>& Op);
	void ServerMoveDual_OnCommandResponse(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermovedual>& Op);
	void ServerMove_OnCommandResponse(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermove>& Op);
};
