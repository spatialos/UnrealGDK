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

	void ApplyComponentData(const Worker_ComponentData& ComponentData, UObject& Object, USpatialActorChannel& Channel,
							bool& bOutReferencesChanged);
	void ApplyComponentData(Worker_ComponentId ComponentId, Schema_ComponentData* Data, UObject& Object, USpatialActorChannel& Channel,
							bool& bOutReferencesChanged);
	void ApplyComponentUpdate(Worker_ComponentId ComponentId, Schema_ComponentUpdate* Update, UObject& Object,
							  USpatialActorChannel& Channel, bool& bOutReferencesChanged);

private:
	enum EProcessFieldType
	{
		ProcessFieldType_OnlySpecialCase,
		ProcessFieldType_AllButSpecialCase,
		ProcessFieldType_All
	};

	struct ApplySchemaObjectDataStruct
	{
		ApplySchemaObjectDataStruct(Schema_Object* inComponentObject, UObject& inObject, USpatialActorChannel& inChannel, bool inIsInitialData,
									const TArray<Schema_FieldId>& inUpdatedIds, Worker_ComponentId inComponentId, bool& inOutReferencesChanged);

		FObjectReplicator* Replicator;
		Schema_Object* ComponentObject;
		UObject& Object;
		USpatialActorChannel& Channel;
		const bool bIsInitialData;

		const TArray<Schema_FieldId>& UpdatedIds;
		const Worker_ComponentId ComponentId;

		TArray<FSpatialGDKSpanId> CauseSpanIds;
		TMap<GDK_PROPERTY(Property)*, FSpatialGDKSpanId> PropertySpanIds;
		TArray<GDK_PROPERTY(Property)*> RepNotifies;

		EProcessFieldType ProcessFieldType;
		bool& bOutReferencesChanged;
	};

	void ApplySchemaObject(ApplySchemaObjectDataStruct ApplySchemaObjectData);

	void ApplySchemaObjectFields(ApplySchemaObjectDataStruct& ApplySchemaObjectData);

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
