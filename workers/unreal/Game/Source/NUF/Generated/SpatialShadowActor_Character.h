#pragma once

#include <generated/UnrealNative.h>

class USpatialActorChannel;

struct RepHandleData
{
UProperty* Parent;
UProperty* Property;
int32 Offset;
};

const TMap<int32, RepHandleData>& GetHandlePropertyMap_Character();
void ApplyUpdateToSpatial_Character(FArchive& Reader, int32 Handle, UProperty* Property, improbable::unreal::UnrealACharacterReplicatedData::Update& Update);
void ReceiveUpdateFromSpatial_Character(USpatialActorChannel* ActorChannel, const improbable::unreal::UnrealACharacterReplicatedData::Update& Update);
