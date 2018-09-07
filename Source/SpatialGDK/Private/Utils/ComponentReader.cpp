// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ComponentReader.h"

#include "Net/DataReplication.h"
#include "Net/RepLayout.h"

#include "EngineClasses/SpatialNetBitReader.h"
#include "Interop/SpatialConditionMapFilter.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"
#include "Utils/RepLayoutUtils.h"

ComponentReader::ComponentReader(USpatialNetDriver* InNetDriver, FObjectReferencesMap& InObjectReferencesMap, TSet<UnrealObjectRef>& InUnresolvedRefs)
	: PackageMap(InNetDriver->PackageMap)
	, NetDriver(InNetDriver)
	, TypebindingManager(InNetDriver->TypebindingManager)
	, ObjectReferencesMap(InObjectReferencesMap)
	, UnresolvedRefs(InUnresolvedRefs)
{
}

void ComponentReader::ApplyComponentData(const Worker_ComponentData& ComponentData, UObject* Object, USpatialActorChannel* Channel, bool bIsHandover)
{
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(ComponentData.schema_type);

	if (bIsHandover)
	{
		ApplyHandoverSchemaObject(ComponentObject, Object, Channel, true);
	}
	else
	{
		ApplySchemaObject(ComponentObject, Object, Channel, true);
	}
}

void ComponentReader::ApplyComponentUpdate(const Worker_ComponentUpdate& ComponentUpdate, UObject* Object, USpatialActorChannel* Channel, bool bIsHandover)
{
	Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(ComponentUpdate.schema_type);

	TArray<Schema_FieldId> ClearedIds;
	ClearedIds.SetNum(Schema_GetComponentUpdateClearedFieldCount(ComponentUpdate.schema_type));
	Schema_GetComponentUpdateClearedFieldList(ComponentUpdate.schema_type, ClearedIds.GetData());

	if (bIsHandover)
	{
		ApplyHandoverSchemaObject(ComponentObject, Object, Channel, false, &ClearedIds);
	}
	else
	{
		ApplySchemaObject(ComponentObject, Object, Channel, false, &ClearedIds);
	}
}

void ComponentReader::ApplySchemaObject(Schema_Object* ComponentObject, UObject* Object, USpatialActorChannel* Channel, bool bIsInitialData, TArray<Schema_FieldId>* ClearedIds)
{
	bool bAutonomousProxy = Channel->IsClientAutonomousProxy();

	TArray<std::uint32_t> UpdateFields;
	UpdateFields.SetNum(Schema_GetUniqueFieldIdCount(ComponentObject));
	Schema_GetUniqueFieldIds(ComponentObject, UpdateFields.GetData());

	if (UpdateFields.Num() == 0)
	{
		return;
	}

	FObjectReplicator& Replicator = Channel->PreReceiveSpatialUpdate(Object);

	FRepState* RepState = Replicator.RepState;
	TArray<FRepLayoutCmd>& Cmds = Replicator.RepLayout->Cmds;
	TArray<FHandleToCmdIndex>& BaseHandleToCmdIndex = Replicator.RepLayout->BaseHandleToCmdIndex;
	TArray<FRepParentCmd>& Parents = Replicator.RepLayout->Parents;

	bool bIsServer = NetDriver->IsServer();
	FSpatialConditionMapFilter ConditionMap(Channel, bAutonomousProxy);

	TArray<UProperty*> RepNotifies;

	for (std::uint32_t FieldId : UpdateFields)
	{
		// FieldId is the same as rep handle
		check(FieldId > 0 && (int)FieldId - 1 < BaseHandleToCmdIndex.Num());
		const FRepLayoutCmd& Cmd = Cmds[BaseHandleToCmdIndex[FieldId - 1].CmdIndex];
		const FRepParentCmd& Parent = Parents[Cmd.ParentIndex];

		if (bIsServer || ConditionMap.IsRelevant(Parent.Condition))
		{
			// This swaps Role/RemoteRole as we write it
			const FRepLayoutCmd& SwappedCmd = (!bIsServer && Parent.RoleSwapIndex != -1) ? Cmds[Parents[Parent.RoleSwapIndex].CmdStart] : Cmd;

			uint8* Data = (uint8*)Object + SwappedCmd.Offset;

			if (bIsInitialData || GetPropertyCount(ComponentObject, FieldId, Cmd.Property) > 0 || ClearedIds->Find(FieldId) != INDEX_NONE)
			{
				if (Cmd.Type == REPCMD_DynamicArray)
				{
					ApplyArray(ComponentObject, FieldId, Cast<UArrayProperty>(Cmd.Property), Data, SwappedCmd.Offset, Cmd.ParentIndex);
				}
				else
				{
					ApplyProperty(ComponentObject, FieldId, 0, Cmd.Property, Data, SwappedCmd.Offset, Cmd.ParentIndex);
				}

				if (Cmd.Property->GetFName() == NAME_RemoteRole)
				{
					// Downgrade role from AutonomousProxy to SimulatedProxy if we aren't authoritative over
					// the client RPCs component.
					UByteProperty* ByteProperty = Cast<UByteProperty>(Cmd.Property);
					if (!bIsServer && !bAutonomousProxy && ByteProperty->GetPropertyValue(Data) == ROLE_AutonomousProxy)
					{
						ByteProperty->SetPropertyValue(Data, ROLE_SimulatedProxy);
					}
				}

				// Parent.Property is the "root" replicated property, e.g. if a struct property was flattened
				if (Parent.Property->HasAnyPropertyFlags(CPF_RepNotify))
				{
					if (Parent.RepNotifyCondition == REPNOTIFY_Always || !Cmd.Property->Identical(RepState->StaticBuffer.GetData() + SwappedCmd.Offset, Data))
					{
						RepNotifies.AddUnique(Parent.Property);
					}
				}
			}
		}
	}

	Channel->PostReceiveSpatialUpdate(Object, RepNotifies);
}

void ComponentReader::ApplyHandoverSchemaObject(Schema_Object* ComponentObject, UObject* Object, USpatialActorChannel* Channel, bool bIsInitialData, TArray<Schema_FieldId>* ClearedIds)
{
	TArray<std::uint32_t> UpdateFields;
	UpdateFields.SetNum(Schema_GetUniqueFieldIdCount(ComponentObject));
	Schema_GetUniqueFieldIds(ComponentObject, UpdateFields.GetData());

	if (UpdateFields.Num() == 0)
	{
		return;
	}

	FClassInfo* ClassInfo = TypebindingManager->FindClassInfoByClass(Object->GetClass());
	check(ClassInfo);

	Channel->PreReceiveSpatialUpdate(Object);

	for (std::uint32_t FieldId : UpdateFields)
	{
		// FieldId is the same as handover handle
		check(FieldId > 0 && (int)FieldId - 1 < ClassInfo->HandoverProperties.Num());
		const FHandoverPropertyInfo& PropertyInfo = ClassInfo->HandoverProperties[FieldId - 1];

		uint8* Data = (uint8*)Object + PropertyInfo.Offset;

		if (bIsInitialData || GetPropertyCount(ComponentObject, FieldId, PropertyInfo.Property) > 0 || ClearedIds->Find(FieldId) != INDEX_NONE)
		{
			if (UArrayProperty* ArrayProperty = Cast<UArrayProperty>(PropertyInfo.Property))
			{
				ApplyArray(ComponentObject, FieldId, ArrayProperty, Data, PropertyInfo.Offset, -1);
			}
			else
			{
				ApplyProperty(ComponentObject, FieldId, 0, PropertyInfo.Property, Data, PropertyInfo.Offset, -1);
			}
		}
	}

	Channel->PostReceiveSpatialUpdate(Object, TArray<UProperty*>());
}

void ComponentReader::ApplyProperty(Schema_Object* Object, Schema_FieldId Id, std::uint32_t Index, UProperty* Property, uint8* Data, int32 Offset, uint16 ParentIndex)
{
	if (UStructProperty* StructProperty = Cast<UStructProperty>(Property))
	{
		TArray<uint8> ValueData = Schema_IndexPayload(Object, Id, Index);
		// A bit hacky, we should probably include the number of bits with the data instead.
		int64 CountBits = ValueData.Num() * 8;
		TSet<UnrealObjectRef> NewUnresolvedRefs;
		FSpatialNetBitReader ValueDataReader(PackageMap, ValueData.GetData(), CountBits, NewUnresolvedRefs);
		bool bHasUnmapped = false;

		ReadStructProperty(ValueDataReader, StructProperty, NetDriver, Data, bHasUnmapped);

		if (bHasUnmapped)
		{
			ObjectReferencesMap.Add(Offset, FObjectReferences(ValueData, CountBits, NewUnresolvedRefs, ParentIndex, Property));
			UnresolvedRefs.Append(NewUnresolvedRefs);
		}
		else if (ObjectReferencesMap.Find(Offset))
		{
			ObjectReferencesMap.Remove(Offset);
		}
	}
	else if (UBoolProperty* BoolProperty = Cast<UBoolProperty>(Property))
	{
		BoolProperty->SetPropertyValue(Data, Schema_IndexBool(Object, Id, Index) != 0);
	}
	else if (UFloatProperty* FloatProperty = Cast<UFloatProperty>(Property))
	{
		FloatProperty->SetPropertyValue(Data, Schema_IndexFloat(Object, Id, Index));
	}
	else if (UDoubleProperty* DoubleProperty = Cast<UDoubleProperty>(Property))
	{
		DoubleProperty->SetPropertyValue(Data, Schema_IndexDouble(Object, Id, Index));
	}
	else if (UInt8Property* Int8Property = Cast<UInt8Property>(Property))
	{
		Int8Property->SetPropertyValue(Data, (int8)Schema_IndexInt32(Object, Id, Index));
	}
	else if (UInt16Property* Int16Property = Cast<UInt16Property>(Property))
	{
		Int16Property->SetPropertyValue(Data, (int16)Schema_IndexInt32(Object, Id, Index));
	}
	else if (UIntProperty* IntProperty = Cast<UIntProperty>(Property))
	{
		IntProperty->SetPropertyValue(Data, Schema_IndexInt32(Object, Id, Index));
	}
	else if (UInt64Property* Int64Property = Cast<UInt64Property>(Property))
	{
		Int64Property->SetPropertyValue(Data, Schema_IndexInt64(Object, Id, Index));
	}
	else if (UByteProperty* ByteProperty = Cast<UByteProperty>(Property))
	{
		ByteProperty->SetPropertyValue(Data, (uint8)Schema_IndexUint32(Object, Id, Index));
	}
	else if (UUInt16Property* UInt16Property = Cast<UUInt16Property>(Property))
	{
		UInt16Property->SetPropertyValue(Data, (uint16)Schema_IndexUint32(Object, Id, Index));
	}
	else if (UUInt32Property* UInt32Property = Cast<UUInt32Property>(Property))
	{
		UInt32Property->SetPropertyValue(Data, Schema_IndexUint32(Object, Id, Index));
	}
	else if (UUInt64Property* UInt64Property = Cast<UUInt64Property>(Property))
	{
		UInt64Property->SetPropertyValue(Data, Schema_IndexUint64(Object, Id, Index));
	}
	else if (UObjectPropertyBase* ObjectProperty = Cast<UObjectPropertyBase>(Property))
	{
		UnrealObjectRef ObjectRef = Schema_IndexObjectRef(Object, Id, Index);
		check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
		bool bUnresolved = false;

		if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
		{
			ObjectProperty->SetObjectPropertyValue(Data, nullptr);
		}
		else
		{
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
			if (NetGUID.IsValid())
			{
				UObject* ObjectValue = PackageMap->GetObjectFromNetGUID(NetGUID, true);
				checkf(ObjectValue, TEXT("An object ref %s should map to a valid object."), *ObjectRef.ToString());
				checkf(ObjectValue->IsA(ObjectProperty->PropertyClass), TEXT("Object ref %s maps to object %s with the wrong class."), *ObjectRef.ToString(), *ObjectValue->GetFullName());
				ObjectProperty->SetObjectPropertyValue(Data, ObjectValue);
			}
			else
			{
				ObjectReferencesMap.Add(Offset, FObjectReferences(ObjectRef, ParentIndex, Property));
				UnresolvedRefs.Add(ObjectRef);
				bUnresolved = true;
			}
		}

		if (!bUnresolved && ObjectReferencesMap.Find(Offset))
		{
			ObjectReferencesMap.Remove(Offset);
		}
	}
	else if (UNameProperty* NameProperty = Cast<UNameProperty>(Property))
	{
		NameProperty->SetPropertyValue(Data, FName(*Schema_IndexString(Object, Id, Index)));
	}
	else if (UStrProperty* StrProperty = Cast<UStrProperty>(Property))
	{
		StrProperty->SetPropertyValue(Data, Schema_IndexString(Object, Id, Index));
	}
	else if (UTextProperty* TextProperty = Cast<UTextProperty>(Property))
	{
		TextProperty->SetPropertyValue(Data, FText::FromString(Schema_IndexString(Object, Id, Index)));
	}
	else if (UEnumProperty* EnumProperty = Cast<UEnumProperty>(Property))
	{
		if (EnumProperty->ElementSize < 4)
		{
			EnumProperty->GetUnderlyingProperty()->SetIntPropertyValue(Data, (uint64)Schema_IndexUint32(Object, Id, Index));
		}
		else
		{
			ApplyProperty(Object, Id, Index, EnumProperty->GetUnderlyingProperty(), Data, Offset, ParentIndex);
		}
	}
	else
	{
		checkf(false, TEXT("What is this"));
	}
}

void ComponentReader::ApplyArray(Schema_Object* Object, Schema_FieldId Id, UArrayProperty* Property, uint8* Data, int32 Offset, uint16 ParentIndex)
{
	FObjectReferencesMap* ArrayObjectReferences;
	bool bNewArrayMap = false;
	if (FObjectReferences* ExistingEntry = ObjectReferencesMap.Find(Offset))
	{
		check(ExistingEntry->Array);
		check(ExistingEntry->ParentIndex == ParentIndex && ExistingEntry->Property == Property);
		ArrayObjectReferences = ExistingEntry->Array.Get();
	}
	else
	{
		bNewArrayMap = true;
		ArrayObjectReferences = new FObjectReferencesMap();
	}

	FScriptArrayHelper ArrayHelper(Property, Data);

	int Count = GetPropertyCount(Object, Id, Property->Inner);
	ArrayHelper.Resize(Count);

	for (int i = 0; i < Count; i++)
	{
		int32 ElementOffset = i * Property->Inner->ElementSize;
		ApplyProperty(Object, Id, i, Property->Inner, ArrayHelper.GetRawPtr(i), ElementOffset, ParentIndex);
	}

	if (ArrayObjectReferences->Num() > 0)
	{
		if (bNewArrayMap)
		{
			// FObjectReferences takes ownership over ArrayObjectReferences
			ObjectReferencesMap.Add(Offset, FObjectReferences(ArrayObjectReferences, ParentIndex, Property));
		}
	}
	else
	{
		if (bNewArrayMap)
		{
			delete ArrayObjectReferences;
		}
		else
		{
			ObjectReferencesMap.Remove(Offset);
		}
	}
}

std::uint32_t ComponentReader::GetPropertyCount(const Schema_Object* Object, Schema_FieldId Id, UProperty* Property)
{
	if (UStructProperty* StructProperty = Cast<UStructProperty>(Property))
	{
		return Schema_GetBytesCount(Object, Id);
	}
	else if (UBoolProperty* BoolProperty = Cast<UBoolProperty>(Property))
	{
		return Schema_GetBoolCount(Object, Id);
	}
	else if (UFloatProperty* FloatProperty = Cast<UFloatProperty>(Property))
	{
		return Schema_GetFloatCount(Object, Id);
	}
	else if (UDoubleProperty* DoubleProperty = Cast<UDoubleProperty>(Property))
	{
		return Schema_GetDoubleCount(Object, Id);
	}
	else if (UInt8Property* Int8Property = Cast<UInt8Property>(Property))
	{
		return Schema_GetInt32Count(Object, Id);
	}
	else if (UInt16Property* Int16Property = Cast<UInt16Property>(Property))
	{
		return Schema_GetInt32Count(Object, Id);
	}
	else if (UIntProperty* IntProperty = Cast<UIntProperty>(Property))
	{
		return Schema_GetInt32Count(Object, Id);
	}
	else if (UInt64Property* Int64Property = Cast<UInt64Property>(Property))
	{
		return Schema_GetInt64Count(Object, Id);
	}
	else if (UByteProperty* ByteProperty = Cast<UByteProperty>(Property))
	{
		return Schema_GetUint32Count(Object, Id);
	}
	else if (UUInt16Property* UInt16Property = Cast<UUInt16Property>(Property))
	{
		return Schema_GetUint32Count(Object, Id);
	}
	else if (UUInt32Property* UInt32Property = Cast<UUInt32Property>(Property))
	{
		return Schema_GetUint32Count(Object, Id);
	}
	else if (UUInt64Property* UInt64Property = Cast<UUInt64Property>(Property))
	{
		return Schema_GetUint64Count(Object, Id);
	}
	else if (UObjectPropertyBase* ObjectProperty = Cast<UObjectPropertyBase>(Property))
	{
		return Schema_GetObjectCount(Object, Id);
	}
	else if (UNameProperty* NameProperty = Cast<UNameProperty>(Property))
	{
		return Schema_GetBytesCount(Object, Id);
	}
	else if (UStrProperty* StrProperty = Cast<UStrProperty>(Property))
	{
		return Schema_GetBytesCount(Object, Id);
	}
	else if (UTextProperty* TextProperty = Cast<UTextProperty>(Property))
	{
		return Schema_GetBytesCount(Object, Id);
	}
	else if (UArrayProperty* ArrayProperty = Cast<UArrayProperty>(Property))
	{
		return GetPropertyCount(Object, Id, ArrayProperty->Inner);
	}
	else if (UEnumProperty* EnumProperty = Cast<UEnumProperty>(Property))
	{
		if (EnumProperty->ElementSize < 4)
		{
			return Schema_GetUint32Count(Object, Id);
		}
		else
		{
			return GetPropertyCount(Object, Id, EnumProperty->GetUnderlyingProperty());
		}
	}
	else
	{
		checkf(false, TEXT("What is this"));
		return 0;
	}
}
