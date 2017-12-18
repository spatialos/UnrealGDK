// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// Note that this file has been generated automatically

#pragma once

#include <unreal/generated/UnrealPlayerController.h>
#include <unreal/core_types.h>
#include "SpatialHandlePropertyMap.h"

class USpatialActorChannel;

const RepHandlePropertyMap& GetHandlePropertyMap_PlayerController();
void ApplyUpdateToSpatial_SingleClient_PlayerController(FArchive& Reader, int32 Handle, UProperty* Property, improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData::Update& Update);
void ReceiveUpdateFromSpatial_SingleClient_PlayerController(USpatialActorChannel* ActorChannel, const improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData::Update& Update);
void ApplyUpdateToSpatial_MultiClient_PlayerController(FArchive& Reader, int32 Handle, UProperty* Property, improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData::Update& Update);
void ReceiveUpdateFromSpatial_MultiClient_PlayerController(USpatialActorChannel* ActorChannel, const improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData::Update& Update);
