// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// Note that this file has been generated automatically

#pragma once

#include <generated/UnrealCharacter.h>
#include "SpatialHandlePropertyMap.h"

class USpatialActorChannel;

const RepHandlePropertyMap& GetHandlePropertyMap_Character();
void ApplyUpdateToSpatial_Character(FArchive& Reader, int32 Handle, UProperty* Property, improbable::unreal::UnrealCharacterReplicatedData::Update& Update);
void ReceiveUpdateFromSpatial_Character(USpatialActorChannel* ActorChannel, const improbable::unreal::UnrealCharacterReplicatedData::Update& Update);
