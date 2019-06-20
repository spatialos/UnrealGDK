// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineClasses/SpatialNetBitReader.h"
#include "Interop/SpatialReceiver.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialComponentReader, All, All);

namespace SpatialGDK
{

class ComponentReader
{
public:
	ComponentReader(class USpatialNetDriver* InNetDriver, FObjectReferencesMap& InObjectReferencesMap, TSet<FUnrealObjectRef>& InUnresolvedRefs);

	void ApplyComponentData(const Worker_ComponentData& ComponentData, UObject* Object, USpatialActorChannel* Channel, bool bIsHandover);
	void ApplyComponentUpdate(const Worker_ComponentUpdate& ComponentUpdate, UObject* Object, USpatialActorChannel* Channel, bool bIsHandover);

private:
	void ApplySchemaObject(Schema_Object* ComponentObject, UObject* Object, USpatialActorChannel* Channel, bool bIsInitialData, TArray<Schema_FieldId>& UpdatedIds);
	void ApplyHandoverSchemaObject(Schema_Object* ComponentObject, UObject* Object, USpatialActorChannel* Channel, bool bIsInitialData, TArray<Schema_FieldId>& UpdatedIds);

	void ApplyProperty(Schema_Object* Object, Schema_FieldId FieldId, FObjectReferencesMap& InObjectReferencesMap, uint32 Index, UProperty* Property, uint8* Data, int32 Offset, int32 CmdIndex, int32 ParentIndex);
	void ApplyArray(Schema_Object* Object, Schema_FieldId FieldId, FObjectReferencesMap& InObjectReferencesMap, UArrayProperty* Property, uint8* Data, int32 Offset, int32 CmdIndex, int32 ParentIndex);

	uint32 GetPropertyCount(const Schema_Object* Object, Schema_FieldId Id, UProperty* Property);

private:
	class USpatialPackageMapClient* PackageMap;
	class USpatialNetDriver* NetDriver;
	class USpatialClassInfoManager* ClassInfoManager;
	FObjectReferencesMap& RootObjectReferencesMap;
	TSet<FUnrealObjectRef>& UnresolvedRefs;
};

} // namespace SpatialGDK
