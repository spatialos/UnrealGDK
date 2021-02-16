// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineClasses/SpatialNetBitReader.h"
#include "Interop/SpatialReceiver.h"
#include "Utils/GDKPropertyMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialComponentReader, All, All);

namespace SpatialGDK
{
class SpatialEventTracer;

class ComponentReader
{
public:
	explicit ComponentReader(class USpatialNetDriver* InNetDriver, FObjectReferencesMap& InObjectReferencesMap,
							 SpatialEventTracer* InEventTracer);

	void ApplyComponentData(const Worker_ComponentData& ComponentData, UObject& Object, USpatialActorChannel& Channel, bool bIsHandover,
							bool& bOutReferencesChanged);
	void ApplyComponentData(Worker_ComponentId ComponentId, Schema_ComponentData* Data, UObject& Object, USpatialActorChannel& Channel,
							bool bIsHandover, bool& bOutReferencesChanged);
	void ApplyComponentUpdate(Worker_ComponentId ComponentId, Schema_ComponentUpdate* Update, UObject& Object,
							  USpatialActorChannel& Channel, bool bIsHandover, bool& bOutReferencesChanged);

private:
	void ApplySchemaObject(Schema_Object* ComponentObject, UObject& Object, USpatialActorChannel& Channel, bool bIsInitialData,
						   const TArray<Schema_FieldId>& UpdatedIds, Worker_ComponentId ComponentId, bool& bOutReferencesChanged);
	void ApplyHandoverSchemaObject(Schema_Object* ComponentObject, UObject& Object, USpatialActorChannel& Channel, bool bIsInitialData,
								   const TArray<Schema_FieldId>& UpdatedIds, Worker_ComponentId ComponentId, bool& bOutReferencesChanged);

	void ApplyProperty(Schema_Object* Object, Schema_FieldId FieldId, FObjectReferencesMap& InObjectReferencesMap, uint32 Index,
					   GDK_PROPERTY(Property) * Property, uint8* Data, int32 Offset, int32 CmdIndex, int32 ParentIndex,
					   bool& bOutReferencesChanged);
	void ApplyArray(Schema_Object* Object, Schema_FieldId FieldId, FObjectReferencesMap& InObjectReferencesMap,
					GDK_PROPERTY(ArrayProperty) * Property, uint8* Data, int32 Offset, int32 CmdIndex, int32 ParentIndex,
					bool& bOutReferencesChanged);

	uint32 GetPropertyCount(const Schema_Object* Object, Schema_FieldId Id, GDK_PROPERTY(Property) * Property);

private:
	class USpatialPackageMapClient* PackageMap;
	class USpatialNetDriver* NetDriver;
	class USpatialClassInfoManager* ClassInfoManager;
	class SpatialEventTracer* EventTracer;
	FObjectReferencesMap& RootObjectReferencesMap;
};

} // namespace SpatialGDK
