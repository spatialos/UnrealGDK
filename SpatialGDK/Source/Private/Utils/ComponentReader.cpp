// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ComponentReader.h"

#include "Net/DataReplication.h"
#include "Net/RepLayout.h"

#include "EngineClasses/SpatialNetBitReader.h"
#include "Interop/SpatialConditionMapFilter.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"
#include "Utils/RepLayoutUtils.h"

namespace improbable
{

ComponentReader::ComponentReader(USpatialNetDriver* InNetDriver, FObjectReferencesMap& InObjectReferencesMap, TSet<FUnrealObjectRef>& InUnresolvedRefs)
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

	TArray<uint32> UpdateFields;
	UpdateFields.SetNum(Schema_GetUniqueFieldIdCount(ComponentObject));
	Schema_GetUniqueFieldIds(ComponentObject, UpdateFields.GetData());

	if (UpdateFields.Num() == 0)
	{
		return;
	}

	if(Object->IsPendingKill())
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

	for (uint32 FieldId : UpdateFields)
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
	TArray<uint32> UpdateFields;
	UpdateFields.SetNum(Schema_GetUniqueFieldIdCount(ComponentObject));
	Schema_GetUniqueFieldIds(ComponentObject, UpdateFields.GetData());

	if (UpdateFields.Num() == 0)
	{
		return;
	}

	FClassInfo* ClassInfo = TypebindingManager->FindClassInfoByClass(Object->GetClass());
	check(ClassInfo);

	Channel->PreReceiveSpatialUpdate(Object);

	for (uint32 FieldId : UpdateFields)
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

void ComponentReader::ApplyProperty(Schema_Object* Object, Schema_FieldId FieldId, uint32 Index, UProperty* Property, uint8* Data, int32 Offset, int32 ParentIndex)
{
	if (UStructProperty* StructProperty = Cast<UStructProperty>(Property))
	{
		TArray<uint8> ValueData = IndexPayloadFromSchema(Object, FieldId, Index);
		// A bit hacky, we should probably include the number of bits with the data instead.
		int64 CountBits = ValueData.Num() * 8;
		TSet<FUnrealObjectRef> NewUnresolvedRefs;
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
		BoolProperty->SetPropertyValue(Data, Schema_IndexBool(Object, FieldId, Index) != 0);
	}
	else if (UFloatProperty* FloatProperty = Cast<UFloatProperty>(Property))
	{
		FloatProperty->SetPropertyValue(Data, Schema_IndexFloat(Object, FieldId, Index));
	}
	else if (UDoubleProperty* DoubleProperty = Cast<UDoubleProperty>(Property))
	{
		DoubleProperty->SetPropertyValue(Data, Schema_IndexDouble(Object, FieldId, Index));
	}
	else if (UInt8Property* Int8Property = Cast<UInt8Property>(Property))
	{
		Int8Property->SetPropertyValue(Data, (int8)Schema_IndexInt32(Object, FieldId, Index));
	}
	else if (UInt16Property* Int16Property = Cast<UInt16Property>(Property))
	{
		Int16Property->SetPropertyValue(Data, (int16)Schema_IndexInt32(Object, FieldId, Index));
	}
	else if (UIntProperty* IntProperty = Cast<UIntProperty>(Property))
	{
		IntProperty->SetPropertyValue(Data, Schema_IndexInt32(Object, FieldId, Index));
	}
	else if (UInt64Property* Int64Property = Cast<UInt64Property>(Property))
	{
		Int64Property->SetPropertyValue(Data, Schema_IndexInt64(Object, FieldId, Index));
	}
	else if (UByteProperty* ByteProperty = Cast<UByteProperty>(Property))
	{
		ByteProperty->SetPropertyValue(Data, (uint8)Schema_IndexUint32(Object, FieldId, Index));
	}
	else if (UUInt16Property* UInt16Property = Cast<UUInt16Property>(Property))
	{
		UInt16Property->SetPropertyValue(Data, (uint16)Schema_IndexUint32(Object, FieldId, Index));
	}
	else if (UUInt32Property* UInt32Property = Cast<UUInt32Property>(Property))
	{
		UInt32Property->SetPropertyValue(Data, Schema_IndexUint32(Object, FieldId, Index));
	}
	else if (UUInt64Property* UInt64Property = Cast<UUInt64Property>(Property))
	{
		UInt64Property->SetPropertyValue(Data, Schema_IndexUint64(Object, FieldId, Index));
	}
	else if (UObjectPropertyBase* ObjectProperty = Cast<UObjectPropertyBase>(Property))
	{
		FUnrealObjectRef ObjectRef = IndexObjectRefFromSchema(Object, FieldId, Index);
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

		UObject* Outer = Property->GetOuter();
		if (Outer->IsA<UStruct>())
		{
			UStruct* Owner = Cast<UStruct>(Outer);
			FString ContextName = Property->GetName() + TEXT("_Context");
			UProperty* ContextProperty = Owner->FindPropertyByName(*ContextName);
			const int32 PropertyOffsetDiff = ContextProperty->GetOffset_ForInternal() - Property->GetOffset_ForInternal();
			FUnrealObjectRef& Context = *(reinterpret_cast<FUnrealObjectRef*>(const_cast<uint8*>(Data) + PropertyOffsetDiff));
			Context = ObjectRef;
		}

		if (!bUnresolved && ObjectReferencesMap.Find(Offset))
		{
			ObjectReferencesMap.Remove(Offset);
		}
	}
	else if (UNameProperty* NameProperty = Cast<UNameProperty>(Property))
	{
		NameProperty->SetPropertyValue(Data, FName(*IndexStringFromSchema(Object, FieldId, Index)));
	}
	else if (UStrProperty* StrProperty = Cast<UStrProperty>(Property))
	{
		StrProperty->SetPropertyValue(Data, IndexStringFromSchema(Object, FieldId, Index));
	}
	else if (UTextProperty* TextProperty = Cast<UTextProperty>(Property))
	{
		TextProperty->SetPropertyValue(Data, FText::FromString(IndexStringFromSchema(Object, FieldId, Index)));
	}
	else if (UEnumProperty* EnumProperty = Cast<UEnumProperty>(Property))
	{
		if (EnumProperty->ElementSize < 4)
		{
			EnumProperty->GetUnderlyingProperty()->SetIntPropertyValue(Data, (uint64)Schema_IndexUint32(Object, FieldId, Index));
		}
		else
		{
			ApplyProperty(Object, FieldId, Index, EnumProperty->GetUnderlyingProperty(), Data, Offset, ParentIndex);
		}
	}
	else
	{
		checkf(false, TEXT("Tried to read unknown property in field %d"), FieldId);
	}
}

void ComponentReader::ApplyArray(Schema_Object* Object, Schema_FieldId FieldId, UArrayProperty* Property, uint8* Data, int32 Offset, int32 ParentIndex)
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

	int Count = GetPropertyCount(Object, FieldId, Property->Inner);
	ArrayHelper.Resize(Count);

	for (int i = 0; i < Count; i++)
	{
		int32 ElementOffset = i * Property->Inner->ElementSize;
		ApplyProperty(Object, FieldId, i, Property->Inner, ArrayHelper.GetRawPtr(i), ElementOffset, ParentIndex);
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

uint32 ComponentReader::GetPropertyCount(const Schema_Object* Object, Schema_FieldId FieldId, UProperty* Property)
{
	if (UStructProperty* StructProperty = Cast<UStructProperty>(Property))
	{
		return Schema_GetBytesCount(Object, FieldId);
	}
	else if (UBoolProperty* BoolProperty = Cast<UBoolProperty>(Property))
	{
		return Schema_GetBoolCount(Object, FieldId);
	}
	else if (UFloatProperty* FloatProperty = Cast<UFloatProperty>(Property))
	{
		return Schema_GetFloatCount(Object, FieldId);
	}
	else if (UDoubleProperty* DoubleProperty = Cast<UDoubleProperty>(Property))
	{
		return Schema_GetDoubleCount(Object, FieldId);
	}
	else if (UInt8Property* Int8Property = Cast<UInt8Property>(Property))
	{
		return Schema_GetInt32Count(Object, FieldId);
	}
	else if (UInt16Property* Int16Property = Cast<UInt16Property>(Property))
	{
		return Schema_GetInt32Count(Object, FieldId);
	}
	else if (UIntProperty* IntProperty = Cast<UIntProperty>(Property))
	{
		return Schema_GetInt32Count(Object, FieldId);
	}
	else if (UInt64Property* Int64Property = Cast<UInt64Property>(Property))
	{
		return Schema_GetInt64Count(Object, FieldId);
	}
	else if (UByteProperty* ByteProperty = Cast<UByteProperty>(Property))
	{
		return Schema_GetUint32Count(Object, FieldId);
	}
	else if (UUInt16Property* UInt16Property = Cast<UUInt16Property>(Property))
	{
		return Schema_GetUint32Count(Object, FieldId);
	}
	else if (UUInt32Property* UInt32Property = Cast<UUInt32Property>(Property))
	{
		return Schema_GetUint32Count(Object, FieldId);
	}
	else if (UUInt64Property* UInt64Property = Cast<UUInt64Property>(Property))
	{
		return Schema_GetUint64Count(Object, FieldId);
	}
	else if (UObjectPropertyBase* ObjectProperty = Cast<UObjectPropertyBase>(Property))
	{
		return Schema_GetObjectCount(Object, FieldId);
	}
	else if (UNameProperty* NameProperty = Cast<UNameProperty>(Property))
	{
		return Schema_GetBytesCount(Object, FieldId);
	}
	else if (UStrProperty* StrProperty = Cast<UStrProperty>(Property))
	{
		return Schema_GetBytesCount(Object, FieldId);
	}
	else if (UTextProperty* TextProperty = Cast<UTextProperty>(Property))
	{
		return Schema_GetBytesCount(Object, FieldId);
	}
	else if (UArrayProperty* ArrayProperty = Cast<UArrayProperty>(Property))
	{
		return GetPropertyCount(Object, FieldId, ArrayProperty->Inner);
	}
	else if (UEnumProperty* EnumProperty = Cast<UEnumProperty>(Property))
	{
		if (EnumProperty->ElementSize < 4)
		{
			return Schema_GetUint32Count(Object, FieldId);
		}
		else
		{
			return GetPropertyCount(Object, FieldId, EnumProperty->GetUnderlyingProperty());
		}
	}
	else
	{
		checkf(false, TEXT("Tried to get count of unknown property in field %d"), FieldId);
		return 0;
	}
}

}
