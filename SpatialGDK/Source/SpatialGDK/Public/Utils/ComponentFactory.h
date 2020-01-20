// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/SpatialClassInfoManager.h"
#include "Schema/Interest.h"
#include "Utils/RepDataUtils.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

DECLARE_LOG_CATEGORY_EXTERN(LogComponentFactory, Log, All);

class USpatialNetDriver;
class USpatialPackageMap;
class USpatialClassInfoManager;
class USpatialLatencyTracer;
class USpatialPackageMapClient;

class UNetDriver;
class UProperty;

enum EReplicatedPropertyGroup : uint32;

namespace SpatialGDK
{

class SPATIALGDK_API ComponentFactory
{
public:
	ComponentFactory(bool bInterestDirty, USpatialNetDriver* InNetDriver, USpatialLatencyTracer* LatencyTracer);

	TArray<Worker_ComponentData> CreateComponentDatas(UObject* Object, const FClassInfo& Info, const FRepChangeState& RepChangeState, const FHandoverChangeState& HandoverChangeState, TArray<TraceKey>* OutLatencyTraceId = nullptr);
	TArray<Worker_ComponentUpdate> CreateComponentUpdates(UObject* Object, const FClassInfo& Info, Worker_EntityId EntityId, const FRepChangeState* RepChangeState, const FHandoverChangeState* HandoverChangeState, TArray<TraceKey>* OutLatencyTraceId = nullptr);

	Worker_ComponentData CreateHandoverComponentData(Worker_ComponentId ComponentId, UObject* Object, const FClassInfo& Info, const FHandoverChangeState& Changes, TraceKey& OutLatencyTraceId);

	static Worker_ComponentData CreateEmptyComponentData(Worker_ComponentId ComponentId);

private:
	Worker_ComponentData CreateComponentData(Worker_ComponentId ComponentId, UObject* Object, const FRepChangeState& Changes, ESchemaComponentType PropertyGroup, TraceKey& OutLatencyTraceId);
	Worker_ComponentUpdate CreateComponentUpdate(Worker_ComponentId ComponentId, UObject* Object, const FRepChangeState& Changes, ESchemaComponentType PropertyGroup, bool& bWroteSomething, TraceKey& OutLatencyTraceId);

	bool FillSchemaObject(Schema_Object* ComponentObject, UObject* Object, const FRepChangeState& Changes, ESchemaComponentType PropertyGroup, bool bIsInitialData, TraceKey& OutLatencyTraceId, TArray<Schema_FieldId>* ClearedIds = nullptr);

	Worker_ComponentUpdate CreateHandoverComponentUpdate(Worker_ComponentId ComponentId, UObject* Object, const FClassInfo& Info, const FHandoverChangeState& Changes, bool& bWroteSomething, TraceKey& OutLatencyTraceId);

	bool FillHandoverSchemaObject(Schema_Object* ComponentObject, UObject* Object, const FClassInfo& Info, const FHandoverChangeState& Changes, bool bIsInitialData, TraceKey& OutLatencyTraceId, TArray<Schema_FieldId>* ClearedIds = nullptr);

	void AddProperty(Schema_Object* Object, Schema_FieldId FieldId, UProperty* Property, const uint8* Data, TArray<Schema_FieldId>* ClearedIds);

	USpatialNetDriver* NetDriver;
	USpatialPackageMapClient* PackageMap;
	USpatialClassInfoManager* ClassInfoManager;

	bool bInterestHasChanged;

	USpatialLatencyTracer* LatencyTracer;
};

} // namespace SpatialGDK
