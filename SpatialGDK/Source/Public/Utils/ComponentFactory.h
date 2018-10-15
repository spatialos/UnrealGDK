// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/SpatialTypebindingManager.h"
#include "Utils/RepDataUtils.h"

#include <improbable/c_schema.h>
#include <improbable/c_worker.h>

class USpatialNetDriver;
class USpatialPackageMap;
class USpatialTypebindingManager;
class USpatialPackageMapClient;

class UNetDriver;
class UProperty;

enum EReplicatedPropertyGroup : uint32;

using FUnresolvedObjectsMap = TMap<Schema_FieldId, TSet<const UObject*>>;

namespace improbable
{

class SPATIALGDK_API ComponentFactory
{
public:
	ComponentFactory(FUnresolvedObjectsMap& RepUnresolvedObjectsMap, FUnresolvedObjectsMap& HandoverUnresolvedObjectsMap, USpatialNetDriver* InNetDriver);

	TArray<Worker_ComponentData> CreateComponentDatas(UObject* Object, const FRepChangeState& RepChangeState, const FHandoverChangeState& HandoverChangeState);
	TArray<Worker_ComponentUpdate> CreateComponentUpdates(UObject* Object, const FRepChangeState* RepChangeState, const FHandoverChangeState* HandoverChangeState);

	static Worker_ComponentData CreateEmptyComponentData(Worker_ComponentId ComponentId);

private:
	Worker_ComponentData CreateComponentData(Worker_ComponentId ComponentId, UObject* Object, const FRepChangeState& Changes, EReplicatedPropertyGroup PropertyGroup);
	Worker_ComponentUpdate CreateComponentUpdate(Worker_ComponentId ComponentId, UObject* Object, const FRepChangeState& Changes, EReplicatedPropertyGroup PropertyGroup, bool& bWroteSomething);

	bool FillSchemaObject(Schema_Object* ComponentObject, UObject* Object, const FRepChangeState& Changes, EReplicatedPropertyGroup PropertyGroup, bool bIsInitialData, TArray<Schema_FieldId>* ClearedIds = nullptr);

	Worker_ComponentData CreateHandoverComponentData(Worker_ComponentId ComponentId, UObject* Object, const FHandoverChangeState& Changes);
	Worker_ComponentUpdate CreateHandoverComponentUpdate(Worker_ComponentId ComponentId, UObject* Object, const FHandoverChangeState& Changes, bool& bWroteSomething);

	bool FillHandoverSchemaObject(Schema_Object* ComponentObject, UObject* Object, const FHandoverChangeState& Changes, bool bIsInitialData, TArray<Schema_FieldId>* ClearedIds = nullptr);

	void AddProperty(Schema_Object* Object, Schema_FieldId FieldId, UProperty* Property, const uint8* Data, TSet<const UObject*>& UnresolvedObjects, TArray<Schema_FieldId>* ClearedIds);

	void AssignUnrealObjectRefToContext(UProperty* Property, const uint8* Data, FUnrealObjectRef ObjectRef);

	USpatialNetDriver* NetDriver;
	USpatialPackageMapClient* PackageMap;
	USpatialTypebindingManager* TypebindingManager;

	FUnresolvedObjectsMap& PendingRepUnresolvedObjectsMap;
	FUnresolvedObjectsMap& PendingHandoverUnresolvedObjectsMap;
};

}
