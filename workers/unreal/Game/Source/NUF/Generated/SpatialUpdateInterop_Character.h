// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// Note that this file has been generated automatically

#pragma once

#include <unreal/generated/UnrealCharacter.h>
#include <unreal/core_types.h>
#include "SpatialHandlePropertyMap.h"
#include "SpatialUpdateInterop.h"

const RPCHandlerFunctionsMap& GetRPCHandlerFunctionMap_Character();
const RepHandlePropertyMap& GetHandlePropertyMap_Character();
void ApplyUpdateToSpatial_SingleClient_Character(FArchive& Reader, int32 Handle, UProperty* Property, UPackageMap* PackageMap, improbable::unreal::UnrealCharacterSingleClientReplicatedData::Update& Update);
void ReceiveUpdateFromSpatial_SingleClient_Character(USpatialUpdateInterop* UpdateInterop, UPackageMap* PackageMap, const worker::ComponentUpdateOp<improbable::unreal::UnrealCharacterSingleClientReplicatedData>& Op);
void ApplyUpdateToSpatial_MultiClient_Character(FArchive& Reader, int32 Handle, UProperty* Property, UPackageMap* PackageMap, improbable::unreal::UnrealCharacterMultiClientReplicatedData::Update& Update);
void ReceiveUpdateFromSpatial_MultiClient_Character(USpatialUpdateInterop* UpdateInterop, UPackageMap* PackageMap, const worker::ComponentUpdateOp<improbable::unreal::UnrealCharacterMultiClientReplicatedData>& Op);
void SomeRPCHandler(struct FFrame* TempFrame, worker::EntityId Target, UPackageMap* PackageMap);
