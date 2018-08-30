#pragma once

class ComponentReader
{
public:
	ComponentFactory(USpatialPackageMapClient* PackageMap, UNetDriver* NetDriver, FObjectReferencesMap& ObjectReferenceMap, TSet<UnrealObjectRef>& UnresolvedRefs);

	void ApplyComponentData(Worker_ComponentData& ComponentData, UObject* Object, USpatialActorChannel* Channel, EReplicatedPropertyGroup PropertyGroup);
	void ApplyComponentUpdate(Worker_ComponentUpdate& ComponentUpdate, UObject* Object, USpatialActorChannel* Channel, EReplicatedPropertyGroup PropertyGroup);

private:
	void ApplySchemaObject(Schema_Object* ComponentObject, UObject* Object, USpatialActorChannel* Channel);

private:
	class USpatialPackageMapClient* PackageMap;
	class UNetDriver* Driver;
	FObjectReferencesMap& ObjectReferenceMap;
	TSet<UnrealObjectRef>& UnresolvedRefs;
};
