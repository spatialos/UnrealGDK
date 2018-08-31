#pragma once

#include <improbable/c_worker.h>
#include <improbable/c_schema.h>

#include <vector>

class USpatialNetDriver;
class USpatialPackageMap;
class USpatialTypebindingManager;

using FUnresolvedObjectsMap = TMap<Schema_FieldId, TSet<const UObject*>>;

class UNetDriver;
class USpatialPackageMapClient;
class UProperty;

class ComponentFactory 
{
public:
	ComponentFactory(FUnresolvedObjectsMap& UnresolvedObjectsMap, USpatialNetDriver* NetDriver);

	std::vector<Worker_ComponentData> CreateComponentDatas(UObject* Object, const FPropertyChangeState& PropertyChangeState);
	std::vector<Worker_ComponentUpdate> CreateComponentUpdates(UObject* Object, const FPropertyChangeState& PropertyChangeState);

private:
	Worker_ComponentData CreateComponentData(Worker_ComponentId ComponentId, const struct FPropertyChangeState& Changes, EReplicatedPropertyGroup PropertyGroup);
	Worker_ComponentUpdate CreateComponentUpdate(Worker_ComponentId ComponentId, const struct FPropertyChangeState& Changes, EReplicatedPropertyGroup PropertyGroup, bool& bWroteSomething);

	bool FillSchemaObject(Schema_Object* ComponentObject, const FPropertyChangeState& Changes, EReplicatedPropertyGroup PropertyGroup, bool bIsInitialData, std::vector<Schema_FieldId>* ClearedIds = nullptr);
	void AddProperty(Schema_Object* Object, Schema_FieldId Id, UProperty* Property, const uint8* Data, TSet<const UObject*>& UnresolvedObjects, std::vector<Schema_FieldId>* ClearedIds);

	USpatialNetDriver* NetDriver;
	USpatialPackageMapClient* PackageMap;
	USpatialTypebindingManager* TypebindingManager;

	FUnresolvedObjectsMap& PendingUnresolvedObjectsMap;
};
