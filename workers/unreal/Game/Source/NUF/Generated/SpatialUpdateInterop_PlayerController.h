// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// Note that this file has been generated automatically

#pragma once

#include <improbable/worker.h>
#include <improbable/view.h>
#include <unreal/generated/UnrealPlayerController.h>
#include <unreal/core_types.h>
#include "SpatialHandlePropertyMap.h"
#include "SpatialUpdateInterop.h"

const RepHandlePropertyMap& GetHandlePropertyMap_PlayerController();

class FSpatialTypeBinding_PlayerController : public FSpatialTypeBinding
{
public:
	void BindToView() override;
	void UnbindFromView() override;
	worker::ComponentId GetReplicatedGroupComponentId(EReplicatedPropertyGroup Group) const override;
	void SendComponentUpdates(const TArray<uint16>& Changed,
		const uint8* RESTRICT SourceData,
		const TArray< FRepLayoutCmd >& Cmds,
		const TArray< FHandleToCmdIndex >& BaseHandleToCmdIndex,
		const worker::EntityId& EntityId) const override;
private:
	worker::Dispatcher::CallbackKey SingleClientCallback;
	worker::Dispatcher::CallbackKey MultiClientCallback;
};
