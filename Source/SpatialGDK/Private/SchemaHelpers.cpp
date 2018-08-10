// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SchemaHelpers.h"

#include "Net/RepLayout.h"
#include "SpatialMemoryWriter.h"

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

	bool operator==(const UnrealObjectRef& Other) const
	{
		return Entity == Other.Entity &&
			   Offset == Other.Offset &&
			   ((!Path && !Other.Path) || (Path && Other.Path && *Path == *Other.Path)) &&
			   ((!Outer && !Other.Outer) || (Outer && Other.Outer && *Outer == *Other.Outer));
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

void Schema_AddProperty(Schema_Object* Object, Schema_FieldId Id, UProperty* Property, const uint8* Data, USpatialPackageMapClient* PackageMap)
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
	else if (UNameProperty* NameProperty = Cast<UNameProperty>(Property))
	{
		Schema_AddString(Object, Id, TCHAR_TO_UTF8(*NameProperty->GetPropertyValue(Data).ToString()));
	}
	else if (UStrProperty* StrProperty = Cast<UStrProperty>(Property))
	{
		Schema_AddString(Object, Id, TCHAR_TO_UTF8(*StrProperty->GetPropertyValue(Data)));
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
	else if (UTextProperty* TextProperty = Cast<UTextProperty>(Property))
	{
		Schema_AddString(Object, Id, TCHAR_TO_UTF8(*TextProperty->GetPropertyValue(Data).ToString()));
	}
	else if (UArrayProperty* ArrayProperty = Cast<UArrayProperty>(Property))
	{
		FScriptArrayHelper ArrayHelper(ArrayProperty, Data);
		for (int i = 0; i < ArrayHelper.Num(); i++)
		{
			Schema_AddProperty(Object, Id, ArrayProperty->Inner, ArrayHelper.GetRawPtr(i), PackageMap);
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
			Schema_AddProperty(Object, Id, EnumProperty->GetUnderlyingProperty(), Data, PackageMap);
		}
	}
	else
	{
		Schema_AddString(Object, Id, "");
	}
}



void CreateDynamicData(Worker_ComponentData& ComponentData, Worker_ComponentId ComponentId, const FPropertyChangeState& Changes, USpatialPackageMapClient* PackageMap)
{
	ComponentData.component_id = ComponentId;
	ComponentData.schema_type = Schema_CreateComponentData(ComponentId);
	auto ComponentObject = Schema_GetComponentDataFields(ComponentData.schema_type);

	// Populate the replicated data component updates from the replicated property changelist.
	if (Changes.RepChanged.Num() > 0)
	{
		FChangelistIterator ChangelistIterator(Changes.RepChanged, 0);
		FRepHandleIterator HandleIterator(ChangelistIterator, Changes.RepCmds, Changes.RepBaseHandleToCmdIndex, 0, 1, 0, Changes.RepCmds.Num() - 1);
		while (HandleIterator.NextHandle())
		{
			const FRepLayoutCmd& Cmd = Changes.RepCmds[HandleIterator.CmdIndex];
			const uint8* Data = Changes.SourceData + HandleIterator.ArrayOffset + Cmd.Offset;

			Schema_AddProperty(ComponentObject, HandleIterator.Handle, Cmd.Property, Data, PackageMap);

			if (Cmd.Type == REPCMD_DynamicArray)
			{
				if (!HandleIterator.JumpOverArray())
				{
					break;
				}
			}
		}
	}

	// Populate the handover data component update from the handover property changelist.
	//for (uint16 ChangedHandle : Changes.HandoverChanged)
	//{
	//}
}
