// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineClasses/SpatialNetBitReader.h"
#include "Interop/SpatialReceiver.h"

namespace improbable
{

class ComponentReader
{
public:
	ComponentReader(class USpatialNetDriver* InNetDriver, FObjectReferencesMap& InObjectReferencesMap, TSet<FUnrealObjectRef>& InUnresolvedRefs);

	void ApplyComponentData(const Worker_ComponentData& ComponentData, UObject* Object, USpatialActorChannel* Channel, bool bIsHandover);
	void ApplyComponentUpdate(const Worker_ComponentUpdate& ComponentUpdate, UObject* Object, USpatialActorChannel* Channel, bool bIsHandover);

private:
	void ApplySchemaObject(Schema_Object* ComponentObject, UObject* Object, USpatialActorChannel* Channel, bool bIsInitialData, TArray<Schema_FieldId>* ClearedIds = nullptr);
	void ApplyHandoverSchemaObject(Schema_Object* ComponentObject, UObject* Object, USpatialActorChannel* Channel, bool bIsInitialData, TArray<Schema_FieldId>* ClearedIds = nullptr);

	void ApplyProperty(Schema_Object* Object, Schema_FieldId FieldId, uint32 Index, UProperty* Property, uint8* Data, int32 Offset, int32 ParentIndex);
	void ApplyArray(Schema_Object* Object, Schema_FieldId FieldId, UArrayProperty* Property, uint8* Data, int32 Offset, int32 ParentIndex);

	uint32 GetPropertyCount(const Schema_Object* Object, Schema_FieldId Id, UProperty* Property);

private:
	class USpatialPackageMapClient* PackageMap;
	class USpatialNetDriver* NetDriver;
	class USpatialTypebindingManager* TypebindingManager;
	FObjectReferencesMap& ObjectReferencesMap;
	TSet<FUnrealObjectRef>& UnresolvedRefs;
};

}
