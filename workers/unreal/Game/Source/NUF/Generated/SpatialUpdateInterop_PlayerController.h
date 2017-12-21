// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// Note that this file has been generated automatically

#pragma once

#include <improbable/worker.h>
#include <improbable/view.h>
#include <unreal/generated/UnrealPlayerController.h>
#include <unreal/core_types.h>
#include "SpatialHandlePropertyMap.h"
#include "SpatialUpdateInterop.h"

const FRepHandlePropertyMap& GetHandlePropertyMap_PlayerController();

class FSpatialTypeBinding_PlayerController : public FSpatialTypeBinding
{
public:
	void Init(USpatialUpdateInterop* UpdateInterop, UPackageMap* PackageMap) override;
	void BindToView() override;
	void UnbindFromView() override;
	worker::ComponentId GetReplicatedGroupComponentId(EReplicatedPropertyGroup Group) const override;
	void SendComponentUpdates(FOutBunch* BunchPtr, const worker::EntityId& EntityId) const override;
	void SendRPCCommand(UFunction* Function, FFrame* RPCFrame, worker::EntityId Target) const override;
private:
	TMap<FString, FRPCHandler> RPCToHandlerMap;
	worker::Dispatcher::CallbackKey SingleClientCallback;
	worker::Dispatcher::CallbackKey MultiClientCallback;
};
