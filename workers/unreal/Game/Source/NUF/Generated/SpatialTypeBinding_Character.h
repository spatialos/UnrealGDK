// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// Note that this file has been generated automatically

#pragma once

#include <improbable/worker.h>
#include <improbable/view.h>
#include <unreal/generated/UnrealCharacter.h>
#include <unreal/core_types.h>
#include "SpatialHandlePropertyMap.h"
#include "SpatialUpdateInterop.h"


class FSpatialTypeBinding_Character : public FSpatialTypeBinding
{
public:
	static const RepHandlePropertyMap& GetHandlePropertyMap();
	void BindToView() override;
	void UnbindFromView() override;
	worker::ComponentId GetReplicatedGroupComponentId(EReplicatedPropertyGroup Group) const override;
	void SendComponentUpdates(const TArray<uint16>& Changed,const uint8* RESTRICT SourceData, const TArray<FRepLayoutCmd>& Cmds, const TArray<FHandleToCmdIndex>& BaseHandleToCmdIndex, const worker::EntityId& EntityId) const override;
	worker::Entity CreateActorEntity(const FVector& Position, const FString& Metadata) const override;
private:
	worker::Dispatcher::CallbackKey SingleClientCallback;
	worker::Dispatcher::CallbackKey MultiClientCallback;
};
