// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/ComponentReader.h"

#include "Engine/BlueprintGeneratedClass.h"
#include "Net/DataReplication.h"
#include "Net/RepLayout.h"
#include "UObject/TextProperty.h"

#include "EngineClasses/SpatialNetBitReader.h"
#include "Interop/SpatialConditionMapFilter.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"
#include "Utils/RepLayoutUtils.h"

DEFINE_LOG_CATEGORY(LogSpatialComponentReader);

namespace improbable
{

ComponentReader::ComponentReader(USpatialNetDriver* InNetDriver, FObjectReferencesMap& InObjectReferencesMap, TSet<FUnrealObjectRef>& InUnresolvedRefs)
	: PackageMap(InNetDriver->PackageMap)
	, NetDriver(InNetDriver)
	, ClassInfoManager(InNetDriver->ClassInfoManager)
	, RootObjectReferencesMap(InObjectReferencesMap)
	, UnresolvedRefs(InUnresolvedRefs)
{
}

void ComponentReader::ApplyComponentData(const Worker_ComponentData& ComponentData, UObject* Object, USpatialActorChannel* Channel, bool bIsHandover)
{
	if (Object->IsPendingKill())
	{
		return;
	}

	Schema_Object* ComponentObject = Schema_GetComponentDataFields(ComponentData.schema_type);

	TArray<uint32> UpdatedIds;
	UpdatedIds.SetNumUninitialized(Schema_GetUniqueFieldIdCount(ComponentObject));
	Schema_GetUniqueFieldIds(ComponentObject, UpdatedIds.GetData());

	if (bIsHandover)
	{
		ApplyHandoverSchemaObject(ComponentObject, Object, Channel, true, UpdatedIds);
	}
	else
	{
		ApplySchemaObject(ComponentObject, Object, Channel, true, UpdatedIds);
	}
}

void ComponentReader::ApplyComponentUpdate(const Worker_ComponentUpdate& ComponentUpdate, UObject* Object, USpatialActorChannel* Channel, bool bIsHandover)
{
	if (Object->IsPendingKill())
	{
		return;
	}

	Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(ComponentUpdate.schema_type);

	// Retrieve all the fields that have been updated in this component update
	TArray<uint32> UpdatedIds;
	UpdatedIds.SetNumUninitialized(Schema_GetUniqueFieldIdCount(ComponentObject));
	Schema_GetUniqueFieldIds(ComponentObject, UpdatedIds.GetData());

	// Retrieve all the fields that have been cleared (eg. list with no entries)
	TArray<Schema_FieldId> ClearedIds;
	ClearedIds.SetNumUninitialized(Schema_GetComponentUpdateClearedFieldCount(ComponentUpdate.schema_type));
	Schema_GetComponentUpdateClearedFieldList(ComponentUpdate.schema_type, ClearedIds.GetData());

	// Merge cleared fields into updated fields to ensure they will be processed (Schema_FieldId == uint32)
	UpdatedIds.Append(ClearedIds);

	if (UpdatedIds.Num() > 0)
	{
		if (bIsHandover)
		{
			ApplyHandoverSchemaObject(ComponentObject, Object, Channel, false, UpdatedIds);
		}
		else
		{
			ApplySchemaObject(ComponentObject, Object, Channel, false, UpdatedIds);
		}
	}
}

void ComponentReader::ApplySchemaObject(Schema_Object* ComponentObject, UObject* Object, USpatialActorChannel* Channel, bool bIsInitialData, TArray<Schema_FieldId>& UpdatedIds)
{
	FObjectReplicator& Replicator = Channel->PreReceiveSpatialUpdate(Object);

	TSharedPtr<FRepState> RepState = Replicator.RepState;
	TArray<FRepLayoutCmd>& Cmds = Replicator.RepLayout->Cmds;
	TArray<FHandleToCmdIndex>& BaseHandleToCmdIndex = Replicator.RepLayout->BaseHandleToCmdIndex;
	TArray<FRepParentCmd>& Parents = Replicator.RepLayout->Parents;

	bool bIsAuthServer = Channel->IsAuthoritativeServer();
	bool bAutonomousProxy = Channel->IsClientAutonomousProxy();
	bool bIsClient = NetDriver->GetNetMode() == NM_Client;

	FSpatialConditionMapFilter ConditionMap(Channel, bIsClient);

	TArray<UProperty*> RepNotifies;

	for (uint32 FieldId : UpdatedIds)
	{
		// FieldId is the same as rep handle
		check(FieldId > 0 && (int)FieldId - 1 < BaseHandleToCmdIndex.Num());
		const FRepLayoutCmd& Cmd = Cmds[BaseHandleToCmdIndex[FieldId - 1].CmdIndex];
		const FRepParentCmd& Parent = Parents[Cmd.ParentIndex];

		if (NetDriver->IsServer() || ConditionMap.IsRelevant(Parent.Condition))
		{
			// This swaps Role/RemoteRole as we write it
			const FRepLayoutCmd& SwappedCmd = (!bIsAuthServer && Parent.RoleSwapIndex != -1) ? Cmds[Parents[Parent.RoleSwapIndex].CmdStart] : Cmd;

			uint8* Data = (uint8*)Object + SwappedCmd.Offset;

			if (Cmd.Type == ERepLayoutCmdType::DynamicArray)
			{
				UArrayProperty* ArrayProperty = Cast<UArrayProperty>(Cmd.Property);
				bool bProcessedArray = false;

				// Check if this is a FastArraySerializer array so we can simulate the FFastArraySerializerItem PreReplicatedRemove and PostReplicatedAdd calls.
				if (UStructProperty* ParentStruct = Cast<UStructProperty>(Parent.Property))
				{
					if (ParentStruct->Struct->IsChildOf(FFastArraySerializer::StaticStruct()))
					{
						// Read array into a temporary array so the appropriate remove/add operations can be processed
						FScriptArray TempArray;
						// Populate array with existing data so compare will incorporate non-replicated entities
						Cmd.Property->CopyCompleteValue((void*)&TempArray, Data);

						ApplyArray(ComponentObject, FieldId, RootObjectReferencesMap, ArrayProperty, (uint8*)&TempArray, SwappedCmd.Offset, Cmd.ParentIndex);
						bProcessedArray = true;

						if (!Cmd.Property->Identical((void*)&TempArray, Data))
						{
							FSpatialNetDeltaSerializeInfo Parms;
							Parms.NewArray = &TempArray;
							Parms.ArrayProperty = ArrayProperty;

							UScriptStruct::ICppStructOps* CppStructOps = ParentStruct->Struct->GetCppStructOps();
							check(CppStructOps);

							// This call resolves into FFastArraySerializer::SpatialFastArrayDeltaSerialize where our custom FFastArraySerializerItem
							// callback are triggered.
							CppStructOps->NetDeltaSerialize(Parms, ParentStruct->ContainerPtrToValuePtr<void>(Object, Parent.ArrayIndex));
						}
					}
				}

				if (!bProcessedArray)
				{
					ApplyArray(ComponentObject, FieldId, RootObjectReferencesMap, ArrayProperty, Data, SwappedCmd.Offset, Cmd.ParentIndex);
				}
			}
			else
			{
				ApplyProperty(ComponentObject, FieldId, RootObjectReferencesMap, 0, Cmd.Property, Data, SwappedCmd.Offset, Cmd.ParentIndex);
			}

			if (Cmd.Property->GetFName() == NAME_RemoteRole)
			{
				// Downgrade role from AutonomousProxy to SimulatedProxy if we aren't authoritative over
				// the client RPCs component.
				UByteProperty* ByteProperty = Cast<UByteProperty>(Cmd.Property);
				if (!bIsAuthServer && !bAutonomousProxy && ByteProperty->GetPropertyValue(Data) == ROLE_AutonomousProxy)
				{
					ByteProperty->SetPropertyValue(Data, ROLE_SimulatedProxy);
				}
			}

			// Parent.Property is the "root" replicated property, e.g. if a struct property was flattened
			if (Parent.Property->HasAnyPropertyFlags(CPF_RepNotify))
			{
				bool bIsIdentical = Cmd.Property->Identical(RepState->StaticBuffer.GetData() + SwappedCmd.Offset, Data);

				// Only call RepNotify for REPNOTIFY_Always if we are not applying initial data.
				if (bIsInitialData)
				{
					if (!bIsIdentical)
					{
						RepNotifies.AddUnique(Parent.Property);
					}
				}
				else
				{
					if (Parent.RepNotifyCondition == REPNOTIFY_Always || !bIsIdentical)
					{
						RepNotifies.AddUnique(Parent.Property);
					}
				}

			}
		}
	}

	Channel->RemoveRepNotifiesWithUnresolvedObjs(RepNotifies, *Replicator.RepLayout, RootObjectReferencesMap, Object);

	Channel->PostReceiveSpatialUpdate(Object, RepNotifies);
}

void ComponentReader::ApplyHandoverSchemaObject(Schema_Object* ComponentObject, UObject* Object, USpatialActorChannel* Channel, bool bIsInitialData, TArray<Schema_FieldId>& UpdatedIds)
{
	const FClassInfo& ClassInfo = ClassInfoManager->GetOrCreateClassInfoByClass(Object->GetClass());

	Channel->PreReceiveSpatialUpdate(Object);

	for (uint32 FieldId : UpdatedIds)
	{
		// FieldId is the same as handover handle
		check(FieldId > 0 && (int)FieldId - 1 < ClassInfo.HandoverProperties.Num());
		const FHandoverPropertyInfo& PropertyInfo = ClassInfo.HandoverProperties[FieldId - 1];

		uint8* Data = (uint8*)Object + PropertyInfo.Offset;

		if (UArrayProperty* ArrayProperty = Cast<UArrayProperty>(PropertyInfo.Property))
		{
			ApplyArray(ComponentObject, FieldId, RootObjectReferencesMap, ArrayProperty, Data, PropertyInfo.Offset, -1);
		}
		else
		{
			ApplyProperty(ComponentObject, FieldId, RootObjectReferencesMap, 0, PropertyInfo.Property, Data, PropertyInfo.Offset, -1);
		}
	}

	Channel->PostReceiveSpatialUpdate(Object, TArray<UProperty*>());
}

void ComponentReader::ApplyProperty(Schema_Object* Object, Schema_FieldId FieldId, FObjectReferencesMap& InObjectReferencesMap, uint32 Index, UProperty* Property, uint8* Data, int32 Offset, int32 ParentIndex)
{
	if (UStructProperty* StructProperty = Cast<UStructProperty>(Property))
	{
		TArray<uint8> ValueData = IndexBytesFromSchema(Object, FieldId, Index);
		// A bit hacky, we should probably include the number of bits with the data instead.
		int64 CountBits = ValueData.Num() * 8;
		TSet<FUnrealObjectRef> NewUnresolvedRefs;
		FSpatialNetBitReader ValueDataReader(PackageMap, ValueData.GetData(), CountBits, NewUnresolvedRefs);
		bool bHasUnmapped = false;

		ReadStructProperty(ValueDataReader, StructProperty, NetDriver, Data, bHasUnmapped);

		if (bHasUnmapped)
		{
			InObjectReferencesMap.Add(Offset, FObjectReferences(ValueData, CountBits, NewUnresolvedRefs, ParentIndex, Property));
			UnresolvedRefs.Append(NewUnresolvedRefs);
		}
		else if (InObjectReferencesMap.Find(Offset))
		{
			InObjectReferencesMap.Remove(Offset);
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
		check(ObjectRef != FUnrealObjectRef::UNRESOLVED_OBJECT_REF);
		bool bUnresolved = false;

		if (ObjectRef == FUnrealObjectRef::NULL_OBJECT_REF)
		{
			ObjectProperty->SetObjectPropertyValue(Data, nullptr);
		}
		else
		{
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
			if (NetGUID.IsValid())
			{
				UObject* ObjectValue = PackageMap->GetObjectFromNetGUID(NetGUID, true);
				if (ObjectValue == nullptr)
				{
					// At this point, we're unable to resolve a stably-named actor by path. This likely means either the actor doesn't exist, or
					// it's part of a streaming level that hasn't been streamed in. In this case, queue the actor based on it's level.
					FString FullPath;
					improbable::GetFullPathFromUnrealObjectReference(ObjectRef, FullPath);
					UE_LOG(LogSpatialComponentReader, Warning, TEXT("Object ref did not map to valid object, will be set to nullptr: %s %s"),
						*ObjectRef.ToString(), FullPath.IsEmpty() ? TEXT("[NO PATH]") : *FullPath);

					ObjectProperty->SetObjectPropertyValue(Data, nullptr);
					return;
				}

				checkf(ObjectValue->IsA(ObjectProperty->PropertyClass), TEXT("Object ref %s maps to object %s with the wrong class."), *ObjectRef.ToString(), *ObjectValue->GetFullName());
				ObjectProperty->SetObjectPropertyValue(Data, ObjectValue);
			}
			else
			{
				InObjectReferencesMap.Add(Offset, FObjectReferences(ObjectRef, ParentIndex, Property));
				UnresolvedRefs.Add(ObjectRef);
				bUnresolved = true;
			}
		}

		if (!bUnresolved && InObjectReferencesMap.Find(Offset))
		{
			InObjectReferencesMap.Remove(Offset);
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
			ApplyProperty(Object, FieldId, InObjectReferencesMap, Index, EnumProperty->GetUnderlyingProperty(), Data, Offset, ParentIndex);
		}
	}
	else
	{
		checkf(false, TEXT("Tried to read unknown property in field %d"), FieldId);
	}
}

void ComponentReader::ApplyArray(Schema_Object* Object, Schema_FieldId FieldId, FObjectReferencesMap& InObjectReferencesMap, UArrayProperty* Property, uint8* Data, int32 Offset, int32 ParentIndex)
{
	FObjectReferencesMap* ArrayObjectReferences;
	bool bNewArrayMap = false;
	if (FObjectReferences* ExistingEntry = InObjectReferencesMap.Find(Offset))
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
		ApplyProperty(Object, FieldId, *ArrayObjectReferences, i, Property->Inner, ArrayHelper.GetRawPtr(i), ElementOffset, ParentIndex);
	}

	if (ArrayObjectReferences->Num() > 0)
	{
		if (bNewArrayMap)
		{
			// FObjectReferences takes ownership over ArrayObjectReferences
			InObjectReferencesMap.Add(Offset, FObjectReferences(ArrayObjectReferences, ParentIndex, Property));
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
			InObjectReferencesMap.Remove(Offset);
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
