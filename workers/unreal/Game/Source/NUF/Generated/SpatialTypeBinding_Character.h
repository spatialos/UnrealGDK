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
	void Init(USpatialUpdateInterop* InUpdateInterop, UPackageMap* InPackageMap) override;
	void BindToView() override;
	void UnbindFromView() override;
	worker::ComponentId GetReplicatedGroupComponentId(EReplicatedPropertyGroup Group) const override;
	void SendComponentUpdates(const FPropertyChangeState& Changes, const worker::EntityId& EntityId) const override;
	worker::Entity CreateActorEntity(const FVector& Position, const FString& Metadata, const FPropertyChangeState& InitialChanges) const override;
	void SendRPCCommand(const UFunction* const Function, FFrame* const RPCFrame, const worker::EntityId& Target) const override;
private:
	TMap<FName, FRPCSender> RPCToSenderMap;
	worker::Dispatcher::CallbackKey SingleClientCallback;
	worker::Dispatcher::CallbackKey MultiClientCallback;
	TArray<worker::Dispatcher::CallbackKey> RPCReceiverCallbacks;

	void ClientCheatWalkReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatwalk>& Op);
	void ClientCheatGhostReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatghost>& Op);
	void ClientCheatFlyReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatfly>& Op);
};
