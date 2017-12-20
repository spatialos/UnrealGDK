// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// Note that this file has been generated automatically

#pragma once

#include <unreal/generated/UnrealPlayerController.h>
#include <unreal/core_types.h>
#include "SpatialHandlePropertyMap.h"
#include "SpatialUpdateInterop.h"

const RepHandlePropertyMap& GetHandlePropertyMap_PlayerController();
void ApplyUpdateToSpatial_SingleClient_PlayerController(FArchive& Reader, int32 Handle, UProperty* Property, UPackageMap* PackageMap, improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData::Update& Update);
void ReceiveUpdateFromSpatial_SingleClient_PlayerController(USpatialUpdateInterop* UpdateInterop, UPackageMap* PackageMap, const worker::ComponentUpdateOp<improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData>& Op);
void ApplyUpdateToSpatial_MultiClient_PlayerController(FArchive& Reader, int32 Handle, UProperty* Property, UPackageMap* PackageMap, improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData::Update& Update);
void ReceiveUpdateFromSpatial_MultiClient_PlayerController(USpatialUpdateInterop* UpdateInterop, UPackageMap* PackageMap, const worker::ComponentUpdateOp<improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData>& Op);
