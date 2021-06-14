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
	struct ApplySchemaObjectDataStruct
	{
		ApplySchemaObjectDataStruct(FObjectReplicator* inReplicator, Schema_Object* inComponentObject, UObject& inObject,
									USpatialActorChannel& inChannel, const TArray<Schema_FieldId>& inUpdatedIds,
									TArray<FSpatialGDKSpanId>& inCauseSpanIds,
									TMap<GDK_PROPERTY(Property) *, FSpatialGDKSpanId>& inPropertySpanIds,
									const Worker_ComponentId inComponentID, TArray<GDK_PROPERTY(Property) *>& inRepNotifies,
									bool inIsInitialData);

		FObjectReplicator* Replicator;
		Schema_Object* ComponentObject;
		UObject& Object;
		USpatialActorChannel& Channel;

		const TArray<Schema_FieldId>& UpdatedIds;
		TArray<FSpatialGDKSpanId>& CauseSpanIds;
		TMap<GDK_PROPERTY(Property)*, FSpatialGDKSpanId>& PropertySpanIds;
		const Worker_ComponentId ComponentId;

		TArray<GDK_PROPERTY(Property)*>& RepNotifies;

		const bool bIsInitialData;

		bool bOutReferencesChanged;
		bool bProcessOnlySpecialCases;
	};

	void ApplySchemaObject(Schema_Object* ComponentObject, UObject& Object, USpatialActorChannel& Channel, bool bIsInitialData,
						   const TArray<Schema_FieldId>& UpdatedIds, Worker_ComponentId ComponentId, bool& bOutReferencesChanged);

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

	// Special Case properties are properties that are used in building the FSpatialConditionMapFilter,
	// and the client must read them before rebuilding the condition map for all other properties.
	static const TArray<FName> SpecialCaseProperties;
};

} // namespace SpatialGDK
