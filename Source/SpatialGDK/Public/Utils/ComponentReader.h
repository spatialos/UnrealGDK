#pragma once

#include "SpatialEntityPipeline.h"

class ComponentReader
{
public:
	ComponentReader(USpatialPackageMapClient* PackageMap, UNetDriver* NetDriver, FObjectReferencesMap& ObjectReferenceMap, TSet<UnrealObjectRef>& UnresolvedRefs);

	void ApplyComponentData(Worker_ComponentData& ComponentData, UObject* Object, USpatialActorChannel* Channel, EReplicatedPropertyGroup PropertyGroup);
	void ApplyComponentUpdate(Worker_ComponentUpdate& ComponentUpdate, UObject* Object, USpatialActorChannel* Channel, EReplicatedPropertyGroup PropertyGroup);

private:
	void ApplySchemaObject(Schema_Object* ComponentObject, UObject* Object, USpatialActorChannel* Channel);
	void ApplyProperty(Schema_Object* Object, Schema_FieldId Id, std::uint32_t Index, UProperty* Property, uint8* Data, int32 Offset, uint16 ParentIndex);
	void ApplyArray(Schema_Object* Object, Schema_FieldId Id, UArrayProperty* Property, uint8* Data, int32 Offset, uint16 ParentIndex);

private:
	class USpatialPackageMapClient* PackageMap;
	class UNetDriver* Driver;
	FObjectReferencesMap& ObjectReferenceMap;
	TSet<UnrealObjectRef>& UnresolvedRefs;
};
