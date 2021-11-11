// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineClasses/SpatialNetBitReader.h"
#include "Interop/SpatialReceiver.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialComponentReader, All, All);

namespace SpatialGDK
{
struct FObjectRepNotifies;
class SpatialEventTracer;

class ComponentReader
{
public:
	explicit ComponentReader(class USpatialNetDriver* InNetDriver, FObjectReferencesMap& InObjectReferencesMap,
							 SpatialEventTracer* InEventTracer);

	void ApplyComponentData(const Worker_ComponentId ComponentId, Schema_ComponentData* Data, UObject& Object,
							USpatialActorChannel& Channel, FObjectRepNotifies& OutObjectRepNotifies, bool& bOutReferencesChanged);
	void ApplyComponentUpdate(const Worker_ComponentId ComponentId, Schema_ComponentUpdate* ComponentUpdate, UObject& Object,
							  USpatialActorChannel& Channel, FObjectRepNotifies& ObjectRepNotifiesOut, bool& bOutReferencesChanged);

private:
	void ApplySchemaObject(Schema_Object* ComponentObject, UObject& Object, USpatialActorChannel& Channel, bool bIsInitialData,
						   const TArray<Schema_FieldId>& UpdatedIds, Worker_ComponentId ComponentId,
						   FObjectRepNotifies& ObjectRepNotifiesOut, bool& bOutReferencesChanged);

	void ApplyProperty(Schema_Object* Object, Schema_FieldId FieldId, FObjectReferencesMap& InObjectReferencesMap, uint32 Index,
					   FProperty* Property, uint8* Data, int32 Offset, int32 CmdIndex, int32 ParentIndex, bool& bOutReferencesChanged);
	void ApplyArray(Schema_Object* Object, Schema_FieldId FieldId, FObjectReferencesMap& InObjectReferencesMap, FArrayProperty* Property,
					uint8* Data, int32 Offset, int32 CmdIndex, int32 ParentIndex, bool& bOutReferencesChanged);

	uint32 GetPropertyCount(const Schema_Object* Object, Schema_FieldId Id, FProperty* Property);

private:
	class USpatialPackageMapClient* PackageMap;
	class USpatialNetDriver* NetDriver;
	class USpatialClassInfoManager* ClassInfoManager;
	class SpatialEventTracer* EventTracer;
	FObjectReferencesMap& RootObjectReferencesMap;
};

} // namespace SpatialGDK
