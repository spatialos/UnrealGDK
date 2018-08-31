// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "improbable/c_schema.h"
#include "improbable/c_worker.h"

#include <map>
#include <vector>

#include "SpatialConstants.h"


using FUnresolvedObjectsMap = TMap<Schema_FieldId, TSet<const UObject*>>;

// Structure for storing references of unresolved object refs
struct FObjectReferences;

using FObjectReferencesMap = TMap<int32, FObjectReferences>;

void ReadDynamicData(const Worker_ComponentData& ComponentData, UObject* TargetObject, class USpatialActorChannel* Channel, class USpatialPackageMapClient* PackageMap, class UNetDriver* Driver, EReplicatedPropertyGroup PropertyGroup, bool bAutonomousProxy, FObjectReferencesMap& ObjectReferencesMap, TSet<UnrealObjectRef>& UnresolvedRefs);
void ReceiveDynamicUpdate(const Worker_ComponentUpdate& ComponentUpdate, UObject* TargetObject, class USpatialActorChannel* Channel, class USpatialPackageMapClient* PackageMap, class UNetDriver* Driver, EReplicatedPropertyGroup PropertyGroup, bool bAutonomousProxy, FObjectReferencesMap& ObjectReferencesMap, TSet<UnrealObjectRef>& UnresolvedRefs);

Worker_ComponentData CreateDynamicData(Worker_ComponentId ComponentId, const struct FPropertyChangeState& Changes, class USpatialPackageMapClient* PackageMap, class UNetDriver* Driver, EReplicatedPropertyGroup PropertyGroup, FUnresolvedObjectsMap& UnresolvedObjectsMap);
Worker_ComponentUpdate CreateDynamicUpdate(Worker_ComponentId ComponentId, const struct FPropertyChangeState& Changes, class USpatialPackageMapClient* PackageMap, class UNetDriver* Driver, EReplicatedPropertyGroup PropertyGroup, FUnresolvedObjectsMap& UnresolvedObjectsMap, bool& bWroteSomething);

Worker_CommandRequest CreateRPCCommandRequest(UObject* TargetObject, UFunction* Function, void* Parameters, class USpatialPackageMapClient* PackageMap, class UNetDriver* Driver, Worker_ComponentId ComponentId, Schema_FieldId CommandIndex, Worker_EntityId& OutEntityId, const UObject*& OutUnresolvedObject);
void ReceiveRPCCommandRequest(const Worker_CommandRequest& CommandRequest, Worker_EntityId EntityId, UFunction* Function, class USpatialPackageMapClient* PackageMap, class UNetDriver* Driver, UObject*& OutTargetObject, void* Data);

Worker_ComponentUpdate CreateMulticastUpdate(UObject* TargetObject, UFunction* Function, void* Parameters, class USpatialPackageMapClient* PackageMap, class UNetDriver* Driver, Worker_ComponentId ComponentId, Schema_FieldId EventIndex, Worker_EntityId& OutEntityId, const UObject*& OutUnresolvedObject);
void ReceiveMulticastUpdate(const Worker_ComponentUpdate& ComponentUpdate, Worker_EntityId EntityId, const TArray<UFunction*>& RPCArray, class USpatialPackageMapClient* PackageMap, class UNetDriver* Driver);

void ResolveObjectReferences(class FRepLayout& RepLayout, UObject* ReplicatedObject, FObjectReferencesMap& ObjectReferencesMap, class USpatialPackageMapClient* PackageMap, class UNetDriver* Driver, uint8* RESTRICT StoredData, uint8* RESTRICT Data, int32 MaxAbsOffset, TArray<UProperty*>& RepNotifies, bool& bOutSomeObjectsWereMapped, bool& bOutStillHasUnresolved);
