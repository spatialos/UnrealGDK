// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SchemaHelpers.h"

#include "Net/RepLayout.h"
#include "SpatialMemoryReader.h"
#include "SpatialMemoryWriter.h"

#include <set>

struct UnrealObjectRef
{
	UnrealObjectRef() = default;
	UnrealObjectRef(const UnrealObjectRef& In)
		: Entity(In.Entity)
		, Offset(In.Offset)
		, Path(In.Path ? new std::string(*In.Path) : nullptr)
		, Outer(In.Outer ? new UnrealObjectRef(*In.Outer) : nullptr)
	{}

	UnrealObjectRef(const improbable::unreal::UnrealObjectRef& In)
		: Entity(In.entity())
		, Offset(In.offset())
		, Path(In.path() ? new std::string(*In.path()) : nullptr)
		, Outer(In.outer() ? new UnrealObjectRef(*In.outer()) : nullptr)
	{}

	UnrealObjectRef& operator=(const UnrealObjectRef& In)
	{
		Entity = In.Entity;
		Offset = In.Offset;
		Path.reset(In.Path ? new std::string(*In.Path) : nullptr);
		Outer.reset(In.Outer ? new UnrealObjectRef(*In.Outer) : nullptr);
		return *this;
	}

	improbable::unreal::UnrealObjectRef ToCppAPI() const
	{
		improbable::unreal::UnrealObjectRef CppAPIRef;
		CppAPIRef.set_entity(Entity);
		CppAPIRef.set_offset(Offset);
		if (Path)
		{
			CppAPIRef.set_path(*Path);
		}
		if (Outer)
		{
			CppAPIRef.set_outer(Outer->ToCppAPI());
		}
		return CppAPIRef;
	}

	FString ToString() const
	{
		return FString::Printf(TEXT("(entity ID: %lld, offset: %u)"), Entity, Offset);
	}

	bool operator==(const UnrealObjectRef& Other) const
	{
		return Entity == Other.Entity &&
			   Offset == Other.Offset &&
			   ((!Path && !Other.Path) || (Path && Other.Path && *Path == *Other.Path)) &&
			   ((!Outer && !Other.Outer) || (Outer && Other.Outer && *Outer == *Other.Outer));
	}

	bool operator!=(const UnrealObjectRef& Other) const
	{
		return !operator==(Other);
	}

	Worker_EntityId Entity;
	std::uint32_t Offset;
	std::unique_ptr<std::string> Path;
	std::unique_ptr<UnrealObjectRef> Outer;
};

const UnrealObjectRef NULL_OBJECT_REF = UnrealObjectRef(SpatialConstants::NULL_OBJECT_REF);
const UnrealObjectRef UNRESOLVED_OBJECT_REF = UnrealObjectRef(SpatialConstants::UNRESOLVED_OBJECT_REF);

void Schema_AddObjectRef(Schema_Object* Object, Schema_FieldId Id, const UnrealObjectRef& ObjectRef)
{
	auto ObjectRefObject = Schema_AddObject(Object, Id);

	Schema_AddEntityId(ObjectRefObject, 1, ObjectRef.Entity);
	Schema_AddUint32(ObjectRefObject, 2, ObjectRef.Offset);
	if (ObjectRef.Path)
	{
		Schema_AddString(ObjectRefObject, 3, *ObjectRef.Path);
	}
	if (ObjectRef.Outer)
	{
		Schema_AddObjectRef(ObjectRefObject, 4, *ObjectRef.Outer);
	}
}

UnrealObjectRef Schema_GetObjectRef(Schema_Object* Object, Schema_FieldId Id);

UnrealObjectRef Schema_IndexObjectRef(Schema_Object* Object, Schema_FieldId Id, std::uint32_t Index)
{
	UnrealObjectRef ObjectRef;

	auto ObjectRefObject = Schema_IndexObject(Object, Id, Index);

	ObjectRef.Entity = Schema_GetEntityId(ObjectRefObject, 1);
	ObjectRef.Offset = Schema_GetUint32(ObjectRefObject, 2);
	if (Schema_GetBytesCount(ObjectRefObject, 3) > 0)
	{
		ObjectRef.Path.reset(new std::string(Schema_GetString(ObjectRefObject, 3)));
	}
	if (Schema_GetObjectCount(ObjectRefObject, 4) > 0)
	{
		ObjectRef.Outer.reset(new UnrealObjectRef(Schema_GetObjectRef(ObjectRefObject, 4)));
	}

	return ObjectRef;
}

UnrealObjectRef Schema_GetObjectRef(Schema_Object* Object, Schema_FieldId Id)
{
	return Schema_IndexObjectRef(Object, Id, 0);
}

void Schema_AddProperty(Schema_Object* Object, Schema_FieldId Id, UProperty* Property, const uint8* Data, USpatialPackageMapClient* PackageMap, std::vector<Schema_FieldId>* ClearedIds)
{
	if (UStructProperty* StructProperty = Cast<UStructProperty>(Property))
	{
		UScriptStruct* Struct = StructProperty->Struct;
		TSet<const UObject*> UnresolvedObjects;
		TArray<uint8> ValueData;
		FSpatialMemoryWriter ValueDataWriter(ValueData, PackageMap, UnresolvedObjects);

		if (Struct->StructFlags & STRUCT_NetSerializeNative)
		{
			UScriptStruct::ICppStructOps* CppStructOps = Struct->GetCppStructOps();
			check(CppStructOps); // else should not have STRUCT_NetSerializeNative
			bool bSuccess = true;
			CppStructOps->NetSerialize(ValueDataWriter, PackageMap, bSuccess, const_cast<uint8*>(Data));
			checkf(bSuccess, TEXT("NetSerialize on %s failed."), *Struct->GetStructCPPName());
		}
		else
		{
			Struct->SerializeBin(ValueDataWriter, const_cast<uint8*>(Data));
		}

		Schema_AddString(Object, Id, std::string(reinterpret_cast<char*>(ValueData.GetData()), ValueData.Num()));
	}
	else if (UBoolProperty* BoolProperty = Cast<UBoolProperty>(Property))
	{
		Schema_AddBool(Object, Id, (std::uint8_t)BoolProperty->GetPropertyValue(Data));
	}
	else if (UFloatProperty* FloatProperty = Cast<UFloatProperty>(Property))
	{
		Schema_AddFloat(Object, Id, FloatProperty->GetPropertyValue(Data));
	}
	else if (UDoubleProperty* DoubleProperty = Cast<UDoubleProperty>(Property))
	{
		Schema_AddDouble(Object, Id, DoubleProperty->GetPropertyValue(Data));
	}
	else if (UInt8Property* Int8Property = Cast<UInt8Property>(Property))
	{
		Schema_AddInt32(Object, Id, (std::int32_t)Int8Property->GetPropertyValue(Data));
	}
	else if (UInt16Property* Int16Property = Cast<UInt16Property>(Property))
	{
		Schema_AddInt32(Object, Id, (std::int32_t)Int16Property->GetPropertyValue(Data));
	}
	else if (UIntProperty* IntProperty = Cast<UIntProperty>(Property))
	{
		Schema_AddInt32(Object, Id, IntProperty->GetPropertyValue(Data));
	}
	else if (UInt64Property* Int64Property = Cast<UInt64Property>(Property))
	{
		Schema_AddInt64(Object, Id, Int64Property->GetPropertyValue(Data));
	}
	else if (UByteProperty* ByteProperty = Cast<UByteProperty>(Property))
	{
		Schema_AddUint32(Object, Id, (std::uint32_t)ByteProperty->GetPropertyValue(Data));
	}
	else if (UUInt16Property* UInt16Property = Cast<UUInt16Property>(Property))
	{
		Schema_AddUint32(Object, Id, (std::uint32_t)UInt16Property->GetPropertyValue(Data));
	}
	else if (UUInt32Property* UInt32Property = Cast<UUInt32Property>(Property))
	{
		Schema_AddUint32(Object, Id, UInt32Property->GetPropertyValue(Data));
	}
	else if (UUInt64Property* UInt64Property = Cast<UUInt64Property>(Property))
	{
		Schema_AddUint64(Object, Id, UInt64Property->GetPropertyValue(Data));
	}
	else if (UObjectPropertyBase* ObjectProperty = Cast<UObjectPropertyBase>(Property))
	{
		UnrealObjectRef ObjectRef = NULL_OBJECT_REF;

		UObject* ObjectValue = ObjectProperty->GetObjectPropertyValue(Data);
		if (ObjectValue != nullptr)
		{
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(ObjectValue);
			if (!NetGUID.IsValid())
			{
				if (ObjectValue->IsFullNameStableForNetworking())
				{
					NetGUID = PackageMap->ResolveStablyNamedObject(ObjectValue);
				}
			}
			ObjectRef = UnrealObjectRef(PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID));
			if (ObjectRef == UNRESOLVED_OBJECT_REF)
			{
				// A legal static object reference should never be unresolved.
				check(!ObjectValue->IsFullNameStableForNetworking());
				// TODO: Queue up unresolved object
				ObjectRef = NULL_OBJECT_REF;
			}
		}

		Schema_AddObjectRef(Object, Id, ObjectRef);
	}
	else if (UNameProperty* NameProperty = Cast<UNameProperty>(Property))
	{
		Schema_AddString(Object, Id, TCHAR_TO_UTF8(*NameProperty->GetPropertyValue(Data).ToString()));
	}
	else if (UStrProperty* StrProperty = Cast<UStrProperty>(Property))
	{
		Schema_AddString(Object, Id, TCHAR_TO_UTF8(*StrProperty->GetPropertyValue(Data)));
	}
	else if (UTextProperty* TextProperty = Cast<UTextProperty>(Property))
	{
		Schema_AddString(Object, Id, TCHAR_TO_UTF8(*TextProperty->GetPropertyValue(Data).ToString()));
	}
	else if (UArrayProperty* ArrayProperty = Cast<UArrayProperty>(Property))
	{
		FScriptArrayHelper ArrayHelper(ArrayProperty, Data);
		for (int i = 0; i < ArrayHelper.Num(); i++)
		{
			Schema_AddProperty(Object, Id, ArrayProperty->Inner, ArrayHelper.GetRawPtr(i), PackageMap, ClearedIds);
		}

		if (ArrayHelper.Num() == 0 && ClearedIds)
		{
			ClearedIds->push_back(Id);
		}
	}
	else if (UEnumProperty* EnumProperty = Cast<UEnumProperty>(Property))
	{
		if (EnumProperty->ElementSize < 4)
		{
			Schema_AddUint32(Object, Id, (std::uint32_t)EnumProperty->GetUnderlyingProperty()->GetUnsignedIntPropertyValue(Data));
		}
		else
		{
			Schema_AddProperty(Object, Id, EnumProperty->GetUnderlyingProperty(), Data, PackageMap, ClearedIds);
		}
	}
	else
	{
		Schema_AddString(Object, Id, "");
	}
}

std::uint32_t Schema_GetPropertyCount(const Schema_Object* Object, Schema_FieldId Id, UProperty* Property)
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
		return Schema_GetPropertyCount(Object, Id, ArrayProperty->Inner);
	}
	else if (UEnumProperty* EnumProperty = Cast<UEnumProperty>(Property))
	{
		if (EnumProperty->ElementSize < 4)
		{
			return Schema_GetUint32Count(Object, Id);
		}
		else
		{
			return Schema_GetPropertyCount(Object, Id, EnumProperty->GetUnderlyingProperty());
		}
	}
	else
	{
		checkf(false, TEXT("What is this"));
		return 0;
	}
}

void Schema_GetProperty(Schema_Object* Object, Schema_FieldId Id, std::uint32_t Index, UProperty* Property, uint8* Data, USpatialPackageMapClient* PackageMap)
{
	if (UStructProperty* StructProperty = Cast<UStructProperty>(Property))
	{
		UScriptStruct* Struct = StructProperty->Struct;
		auto ValueDataStr = Schema_IndexString(Object, Id, Index);
		TArray<uint8> ValueData;
		ValueData.Append(reinterpret_cast<const uint8*>(ValueDataStr.data()), ValueDataStr.size());
		FSpatialMemoryReader ValueDataReader(ValueData, PackageMap);
		if (Struct->StructFlags & STRUCT_NetSerializeNative)
		{
			UScriptStruct::ICppStructOps* CppStructOps = Struct->GetCppStructOps();
			check(CppStructOps); // else should not have STRUCT_NetSerializeNative
			bool bSuccess = true;
			CppStructOps->NetSerialize(ValueDataReader, PackageMap, bSuccess, Data);
			checkf(bSuccess, TEXT("NetSerialize on %s failed."), *Struct->GetStructCPPName());
		}
		else
		{
			Struct->SerializeBin(ValueDataReader, Data);
		}
	}
	else if (UBoolProperty* BoolProperty = Cast<UBoolProperty>(Property))
	{
		BoolProperty->SetPropertyValue(Data, Schema_IndexBool(Object, Id, Index));
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
		ByteProperty->SetPropertyValue(Data, (uint32)Schema_IndexUint32(Object, Id, Index));
	}
	else if (UUInt16Property* UInt16Property = Cast<UUInt16Property>(Property))
	{
		UInt16Property->SetPropertyValue(Data, (uint32)Schema_IndexUint32(Object, Id, Index));
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
		check(ObjectRef != UNRESOLVED_OBJECT_REF);
		if (ObjectRef == NULL_OBJECT_REF)
		{
			ObjectProperty->SetObjectPropertyValue(Data, nullptr);
		}
		else
		{
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef.ToCppAPI());
			if (NetGUID.IsValid())
			{
				UObject* ObjectValue = PackageMap->GetObjectFromNetGUID(NetGUID, true);
				checkf(ObjectValue, TEXT("An object ref %s should map to a valid object."), *ObjectRef.ToString());
				checkf(ObjectValue->IsA(ObjectProperty->PropertyClass), TEXT("Object ref %s maps to object %s with the wrong class."), *ObjectRef.ToString(), *ObjectValue->GetFullName());
				ObjectProperty->SetObjectPropertyValue(Data, ObjectValue);
			}
			else
			{
				// TODO: Queue up unresolved object ref
			}
		}
	}
	else if (UNameProperty* NameProperty = Cast<UNameProperty>(Property))
	{
		NameProperty->SetPropertyValue(Data, FName(Schema_IndexString(Object, Id, Index).data()));
	}
	else if (UStrProperty* StrProperty = Cast<UStrProperty>(Property))
	{
		StrProperty->SetPropertyValue(Data, FString(UTF8_TO_TCHAR(Schema_IndexString(Object, Id, Index).c_str())));
	}
	else if (UTextProperty* TextProperty = Cast<UTextProperty>(Property))
	{
		TextProperty->SetPropertyValue(Data, FText::FromString(FString(UTF8_TO_TCHAR(Schema_IndexString(Object, Id, Index).c_str()))));
	}
	else if (UArrayProperty* ArrayProperty = Cast<UArrayProperty>(Property))
	{
		// TODO: This is potentionally very very jank
		FScriptArrayHelper ArrayHelper(ArrayProperty, Data);

		int Count = Schema_GetPropertyCount(Object, Id, ArrayProperty->Inner);
		ArrayHelper.Resize(Count);

		for (int i = 0; i < Count; i++)
		{
			Schema_GetProperty(Object, Id, i, ArrayProperty->Inner, ArrayHelper.GetRawPtr(i), PackageMap);
		}
	}
	else if (UEnumProperty* EnumProperty = Cast<UEnumProperty>(Property))
	{
		if (EnumProperty->ElementSize < 4)
		{
			EnumProperty->GetUnderlyingProperty()->SetIntPropertyValue(Data, (uint64)Schema_IndexUint32(Object, Id, Index));
		}
		else
		{
			Schema_GetProperty(Object, Id, Index, EnumProperty->GetUnderlyingProperty(), Data, PackageMap);
		}
	}
	else
	{
		checkf(false, TEXT("What is this"));
	}
}

EAlsoReplicatedPropertyGroup GetAlsoGroupFromCondition(ELifetimeCondition Condition)
{
	switch (Condition)
	{
	case COND_AutonomousOnly:
	case COND_OwnerOnly:
		return AGROUP_SingleClient;
	default:
		return AGROUP_MultiClient;
	}
}

void Schema_ReadDynamicObject(Schema_Object* ComponentObject, USpatialActorChannel* Channel, USpatialPackageMapClient* PackageMap, EAlsoReplicatedPropertyGroup PropertyGroup, bool IsUpdate = false, const std::set<Schema_FieldId>& ClearedIds = std::set<Schema_FieldId>())
{
	Channel->PreReceiveSpatialUpdate(Channel->Actor);

	auto RepState = Channel->ActorReplicator->RepState;
	auto& Cmds = Channel->ActorReplicator->RepLayout->Cmds;
	auto& BaseHandleToCmdIndex = Channel->ActorReplicator->RepLayout->BaseHandleToCmdIndex;
	auto& Parents = Channel->ActorReplicator->RepLayout->Parents;

	auto Handles = Channel->GetAllPropertyHandles(*Channel->ActorReplicator);

	if (Handles.Num() > 0)
	{
		FChangelistIterator ChangelistIterator(Handles, 0);
		FRepHandleIterator HandleIterator(ChangelistIterator, Cmds, BaseHandleToCmdIndex, 0, 1, 0, Cmds.Num() - 1);
		while (HandleIterator.NextHandle())
		{
			const FRepLayoutCmd& Cmd = Cmds[HandleIterator.CmdIndex];
			const FRepParentCmd& Parent = Parents[Cmd.ParentIndex];

			if (GetAlsoGroupFromCondition(Parent.Condition) == PropertyGroup)
			{
				// This swaps Role/RemoteRole as we write it
				const FRepLayoutCmd& SwappedCmd = Parent.RoleSwapIndex != -1 ? Cmds[Parents[Parent.RoleSwapIndex].CmdStart] : Cmd;

				uint8* Data = (uint8*)Channel->Actor + HandleIterator.ArrayOffset + SwappedCmd.Offset;

				if (!IsUpdate || Schema_GetPropertyCount(ComponentObject, HandleIterator.Handle, SwappedCmd.Property) > 0 || ClearedIds.find(HandleIterator.Handle) != ClearedIds.end())
				{
					Schema_GetProperty(ComponentObject, HandleIterator.Handle, 0, SwappedCmd.Property, Data, PackageMap);
				}
			}

			if (Cmd.Type == REPCMD_DynamicArray)
			{
				if (!HandleIterator.JumpOverArray())
				{
					break;
				}
			}
		}
	}

	Channel->PostReceiveSpatialUpdate(Channel->Actor, TArray<UProperty*>());
}

void ReadDynamicData(const Worker_ComponentData& ComponentData, USpatialActorChannel* Channel, USpatialPackageMapClient* PackageMap, EAlsoReplicatedPropertyGroup PropertyGroup)
{
	auto ComponentObject = Schema_GetComponentDataFields(ComponentData.schema_type);

	Schema_ReadDynamicObject(ComponentObject, Channel, PackageMap, PropertyGroup);
}

void ReceiveDynamicUpdate(const Worker_ComponentUpdate& ComponentUpdate, USpatialActorChannel* Channel, USpatialPackageMapClient* PackageMap, EAlsoReplicatedPropertyGroup PropertyGroup)
{
	auto ComponentObject = Schema_GetComponentUpdateFields(ComponentUpdate.schema_type);

	std::vector<Schema_FieldId> ClearedIdsList(Schema_GetComponentUpdateClearedFieldCount(ComponentUpdate.schema_type));
	Schema_GetComponentUpdateClearedFieldList(ComponentUpdate.schema_type, ClearedIdsList.data());

	std::set<Schema_FieldId> ClearedIds;

	for (auto FieldId : ClearedIdsList)
	{
		ClearedIds.insert(FieldId);
	}

	Schema_ReadDynamicObject(ComponentObject, Channel, PackageMap, PropertyGroup, true, ClearedIds);
}

bool Schema_FillDynamicObject(Schema_Object* ComponentObject, const FPropertyChangeState& Changes, USpatialPackageMapClient* PackageMap, EAlsoReplicatedPropertyGroup PropertyGroup, std::vector<Schema_FieldId>* ClearedIds = nullptr)
{
	bool bWroteSomething = false;

	// Populate the replicated data component updates from the replicated property changelist.
	if (Changes.RepChanged.Num() > 0)
	{
		FChangelistIterator ChangelistIterator(Changes.RepChanged, 0);
		FRepHandleIterator HandleIterator(ChangelistIterator, Changes.RepCmds, Changes.RepBaseHandleToCmdIndex, 0, 1, 0, Changes.RepCmds.Num() - 1);
		while (HandleIterator.NextHandle())
		{
			const FRepLayoutCmd& Cmd = Changes.RepCmds[HandleIterator.CmdIndex];
			const FRepParentCmd& Parent = Changes.Parents[Cmd.ParentIndex];

			if (GetAlsoGroupFromCondition(Parent.Condition) == PropertyGroup)
			{
				const uint8* Data = Changes.SourceData + HandleIterator.ArrayOffset + Cmd.Offset;

				Schema_AddProperty(ComponentObject, HandleIterator.Handle, Cmd.Property, Data, PackageMap, ClearedIds);

				bWroteSomething = true;
			}

			if (Cmd.Type == REPCMD_DynamicArray)
			{
				if (!HandleIterator.JumpOverArray())
				{
					break;
				}
			}
		}
	}

	// TODO: Handover
	// Populate the handover data component update from the handover property changelist.
	//for (uint16 ChangedHandle : Changes.HandoverChanged)
	//{
	//}

	return bWroteSomething;
}

Worker_ComponentData CreateDynamicData(Worker_ComponentId ComponentId, const FPropertyChangeState& Changes, USpatialPackageMapClient* PackageMap, EAlsoReplicatedPropertyGroup PropertyGroup)
{
	Worker_ComponentData ComponentData = {};
	ComponentData.component_id = ComponentId;
	ComponentData.schema_type = Schema_CreateComponentData(ComponentId);
	auto ComponentObject = Schema_GetComponentDataFields(ComponentData.schema_type);

	Schema_FillDynamicObject(ComponentObject, Changes, PackageMap, PropertyGroup);

	return ComponentData;
}

Worker_ComponentUpdate CreateDynamicUpdate(Worker_ComponentId ComponentId, const FPropertyChangeState& Changes, USpatialPackageMapClient* PackageMap, EAlsoReplicatedPropertyGroup PropertyGroup, bool& bWroteSomething)
{
	Worker_ComponentUpdate ComponentUpdate = {};

	ComponentUpdate.component_id = ComponentId;
	ComponentUpdate.schema_type = Schema_CreateComponentUpdate(ComponentId);
	auto ComponentObject = Schema_GetComponentUpdateFields(ComponentUpdate.schema_type);

	std::vector<Schema_FieldId> ClearedIds;

	bWroteSomething = Schema_FillDynamicObject(ComponentObject, Changes, PackageMap, PropertyGroup, &ClearedIds);

	for (auto Id : ClearedIds)
	{
		Schema_AddComponentUpdateClearedField(ComponentUpdate.schema_type, Id);
	}

	if (!bWroteSomething)
	{
		Schema_DestroyComponentUpdate(ComponentUpdate.schema_type);
	}

	return ComponentUpdate;
}
