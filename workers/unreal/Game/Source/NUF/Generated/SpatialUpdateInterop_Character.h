// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// Note that this file has been generated automatically

#pragma once

#include <unreal/generated/UnrealCharacter.h>
#include <unreal/core_types.h>
#include "SpatialHandlePropertyMap.h"

class USpatialActorChannel;

const RepHandlePropertyMap& GetHandlePropertyMap_Character();
void ApplyUpdateToSpatial_SingleClient_Character(FArchive& Reader, int32 Handle, UProperty* Property, improbable::unreal::UnrealCharacterSingleClientReplicatedData::Update& Update);
void ReceiveUpdateFromSpatial_SingleClient_Character(USpatialActorChannel* ActorChannel, const improbable::unreal::UnrealCharacterSingleClientReplicatedData::Update& Update);
void ApplyUpdateToSpatial_MultiClient_Character(FArchive& Reader, int32 Handle, UProperty* Property, improbable::unreal::UnrealCharacterMultiClientReplicatedData::Update& Update);
void ReceiveUpdateFromSpatial_MultiClient_Character(USpatialActorChannel* ActorChannel, const improbable::unreal::UnrealCharacterMultiClientReplicatedData::Update& Update);
