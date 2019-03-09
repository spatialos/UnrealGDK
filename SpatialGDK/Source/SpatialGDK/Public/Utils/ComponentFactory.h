// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/SpatialClassInfoManager.h"
#include "Schema/Interest.h"
#include "Utils/RepDataUtils.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

class USpatialNetDriver;
class USpatialPackageMap;
class USpatialClassInfoManager;
class USpatialPackageMapClient;

class UNetDriver;
class UProperty;

enum EReplicatedPropertyGroup : uint32;

using FUnresolvedObjectsSet = TSet<TWeakObjectPtr<const UObject>>;
using FUnresolvedObjectsMap = TMap<Schema_FieldId, FUnresolvedObjectsSet>;

namespace improbable
{

class SPATIALGDK_API ComponentFactory
{
public:
	ComponentFactory(FUnresolvedObjectsMap& RepUnresolvedObjectsMap, FUnresolvedObjectsMap& HandoverUnresolvedObjectsMap, USpatialNetDriver* InNetDriver);

	TArray<Worker_ComponentData> CreateComponentDatas(UObject* Object, const FClassInfo& Info, const FRepChangeState& RepChangeState, const FHandoverChangeState& HandoverChangeState);
	TArray<Worker_ComponentUpdate> CreateComponentUpdates(UObject* Object, const FClassInfo& Info, Worker_EntityId EntityId, const FRepChangeState* RepChangeState, const FHandoverChangeState* HandoverChangeState);

	static Worker_ComponentData CreateEmptyComponentData(Worker_ComponentId ComponentId);

private:
	Worker_ComponentData CreateComponentData(Worker_ComponentId ComponentId, UObject* Object, const FRepChangeState& Changes, ESchemaComponentType PropertyGroup);
	Worker_ComponentUpdate CreateComponentUpdate(Worker_ComponentId ComponentId, UObject* Object, const FRepChangeState& Changes, ESchemaComponentType PropertyGroup, bool& bWroteSomething);

	bool FillSchemaObject(Schema_Object* ComponentObject, UObject* Object, const FRepChangeState& Changes, ESchemaComponentType PropertyGroup, bool bIsInitialData, TArray<Schema_FieldId>* ClearedIds = nullptr);

	Worker_ComponentData CreateHandoverComponentData(Worker_ComponentId ComponentId, UObject* Object, const FClassInfo& Info, const FHandoverChangeState& Changes);
	Worker_ComponentUpdate CreateHandoverComponentUpdate(Worker_ComponentId ComponentId, UObject* Object, const FClassInfo& Info, const FHandoverChangeState& Changes, bool& bWroteSomething);

	bool FillHandoverSchemaObject(Schema_Object* ComponentObject, UObject* Object, const FClassInfo& Info, const FHandoverChangeState& Changes, bool bIsInitialData, TArray<Schema_FieldId>* ClearedIds = nullptr);

	Worker_ComponentData CreateInterestComponentData(UObject* Object, const FClassInfo& Info);
	Worker_ComponentUpdate CreateInterestComponentUpdate(UObject* Object, const FClassInfo& Info);
	improbable::Interest CreateInterestComponent(UObject* Object, const FClassInfo& Info);
	void AddObjectToComponentInterest(UObject* Object, UObjectPropertyBase* Property, uint8* Data, improbable::ComponentInterest& ComponentInterest);

	void AddProperty(Schema_Object* Object, Schema_FieldId FieldId, UProperty* Property, const uint8* Data, FUnresolvedObjectsSet& UnresolvedObjects, TArray<Schema_FieldId>* ClearedIds);

	USpatialNetDriver* NetDriver;
	USpatialPackageMapClient* PackageMap;
	USpatialClassInfoManager* ClassInfoManager;

	FUnresolvedObjectsMap& PendingRepUnresolvedObjectsMap;
	FUnresolvedObjectsMap& PendingHandoverUnresolvedObjectsMap;

	bool bInterestHasChanged;
};

}
