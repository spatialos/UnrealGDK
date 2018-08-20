// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SchemaHelpers.h"

#include "Net/RepLayout.h"
#include "SpatialMemoryReader.h"
#include "SpatialMemoryWriter.h"

#include <set>

uint32 GetTypeHash(const UnrealObjectRef& ObjectRef)
{
	uint32 Result = 1327u;
	Result = (Result * 977u) + std::hash<Worker_EntityId>{}(ObjectRef.Entity);
	Result = (Result * 977u) + std::hash<std::uint32_t>{}(ObjectRef.Offset);
	Result = (Result * 977u) + (ObjectRef.Path ? 1327u * (std::hash<std::string>{}(*ObjectRef.Path) + 977u) : 977u);
	Result = (Result * 977u) + (ObjectRef.Outer ? 1327u * (GetTypeHash(*ObjectRef.Outer) + 977u) : 977u);
	return Result;
}

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

void RepLayout_SerializeProperties(FRepLayout& RepLayout, FArchive& Ar, UPackageMap* Map, const int32 CmdStart, const int32 CmdEnd, void* Data, bool& bHasUnmapped);

void RepLayout_SerializeProperties_DynamicArray(FRepLayout& RepLayout, FArchive& Ar, UPackageMap* Map, const int32 CmdIndex, uint8* Data, bool& bHasUnmapped)
{
	const FRepLayoutCmd& Cmd = RepLayout.Cmds[CmdIndex];

	FScriptArray* Array = (FScriptArray*)Data;

	uint16 OutArrayNum = Array->Num();
	Ar << OutArrayNum;

	// If loading from the archive, OutArrayNum will contain the number of elements.
	// Otherwise, use the input number of elements.
	const int32 ArrayNum = Ar.IsLoading() ? (int32)OutArrayNum : Array->Num();

	// When loading, we may need to resize the array to properly fit the number of elements.
	if (Ar.IsLoading() && OutArrayNum != Array->Num())
	{
		FScriptArrayHelper ArrayHelper((UArrayProperty*)Cmd.Property, Data);
		ArrayHelper.Resize(OutArrayNum);
	}

	Data = (uint8*)Array->GetData();

	for (int32 i = 0; i < Array->Num() && !Ar.IsError(); i++)
	{
		RepLayout_SerializeProperties(RepLayout, Ar, Map, CmdIndex + 1, Cmd.EndCmd - 1, Data + i * Cmd.ElementSize, bHasUnmapped);
	}
}

void RepLayout_SerializeProperties(FRepLayout& RepLayout, FArchive& Ar, UPackageMap* Map, const int32 CmdStart, const int32 CmdEnd, void* Data, bool& bHasUnmapped)
{
	for (int32 CmdIndex = CmdStart; CmdIndex < CmdEnd && !Ar.IsError(); CmdIndex++)
	{
		const FRepLayoutCmd& Cmd = RepLayout.Cmds[CmdIndex];

		check(Cmd.Type != REPCMD_Return);

		if (Cmd.Type == REPCMD_DynamicArray)
		{
			RepLayout_SerializeProperties_DynamicArray(RepLayout, Ar, Map, CmdIndex, (uint8*)Data + Cmd.Offset, bHasUnmapped);
			CmdIndex = Cmd.EndCmd - 1;		// The -1 to handle the ++ in the for loop
			continue;
		}

		if (!Cmd.Property->NetSerializeItem(Ar, Map, (void*)((uint8*)Data + Cmd.Offset)))
		{
			bHasUnmapped = true;
		}
	}
}

void RepLayout_SerializePropertiesForStruct(FRepLayout& RepLayout, FArchive& Ar, UPackageMap* Map, void* Data, bool& bHasUnmapped)
{
	auto& Parents = RepLayout.Parents;
	for (int32 i = 0; i < Parents.Num(); i++)
	{
		RepLayout_SerializeProperties(RepLayout, Ar, Map, Parents[i].CmdStart, Parents[i].CmdEnd, Data, bHasUnmapped);

		if (Ar.IsError())
		{
			return;
		}
	}
}

void RepLayout_SendPropertiesForRPC(FRepLayout& RepLayout, FNetBitWriter& Writer, void* Data)
{
	auto& Parents = RepLayout.Parents;

	for (int32 i = 0; i < Parents.Num(); i++)
	{
		bool Send = true;

		if (!Cast<UBoolProperty>(Parents[i].Property))
		{
			// check for a complete match, including arrays
			// (we're comparing against zero data here, since 
			// that's the default.)
			Send = !Parents[i].Property->Identical_InContainer(Data, NULL, Parents[i].ArrayIndex);

			Writer.WriteBit(Send ? 1 : 0);
		}

		if (Send)
		{
			bool bHasUnmapped = false;
			RepLayout_SerializeProperties(RepLayout, Writer, Writer.PackageMap, Parents[i].CmdStart, Parents[i].CmdEnd, Data, bHasUnmapped);
		}
	}
}

void RepLayout_ReceivePropertiesForRPC(FRepLayout& RepLayout, FNetBitReader& Reader, void* Data)
{
	auto& Parents = RepLayout.Parents;

	for (int32 i = 0; i < Parents.Num(); i++)
	{
		if (Parents[i].ArrayIndex == 0 && (Parents[i].Property->PropertyFlags & CPF_ZeroConstructor) == 0)
		{
			// If this property needs to be constructed, make sure we do that
			Parents[i].Property->InitializeValue((uint8*)Data + Parents[i].Property->GetOffset_ForUFunction());
		}
	}

	//Reader.PackageMap->ResetTrackedGuids(true);

	for (int32 i = 0; i < Parents.Num(); i++)
	{
		if (Cast<UBoolProperty>(Parents[i].Property) || Reader.ReadBit())
		{
			bool bHasUnmapped = false;

			RepLayout_SerializeProperties(RepLayout, Reader, Reader.PackageMap, Parents[i].CmdStart, Parents[i].CmdEnd, Data, bHasUnmapped);

			if (Reader.IsError())
			{
				return;
			}

			if (bHasUnmapped)
			{
				//UE_LOG(LogTemp, Log, TEXT("Unable to resolve RPC parameter. Object[%d] %s. Function %s. Parameter %s."),
				//	Channel->ChIndex, *Object->GetName(), *Function->GetName(), *Parents[i].Property->GetName());
			}
		}
	}

	//if (Reader.PackageMap->GetTrackedUnmappedGuids().Num() > 0)
	//{
	//	UnmappedGuids = Reader.PackageMap->GetTrackedUnmappedGuids();
	//}

	//Reader.PackageMap->ResetTrackedGuids(false);
}

void Schema_AddProperty(Schema_Object* Object, Schema_FieldId Id, UProperty* Property, const uint8* Data, USpatialPackageMapClient* PackageMap, UNetDriver* Driver, TSet<const UObject*>& UnresolvedObjects, std::vector<Schema_FieldId>* ClearedIds)
{
	if (UStructProperty* StructProperty = Cast<UStructProperty>(Property))
	{
		UScriptStruct* Struct = StructProperty->Struct;
		FSpatialNetBitWriter ValueDataWriter(PackageMap, UnresolvedObjects);
		bool bHasUnmapped = false;

		if (Struct->StructFlags & STRUCT_NetSerializeNative)
		{
			UScriptStruct::ICppStructOps* CppStructOps = Struct->GetCppStructOps();
			check(CppStructOps); // else should not have STRUCT_NetSerializeNative
			bool bSuccess = true;
			if (!CppStructOps->NetSerialize(ValueDataWriter, PackageMap, bSuccess, const_cast<uint8*>(Data)))
			{
				bHasUnmapped = true;
			}
			checkf(bSuccess, TEXT("NetSerialize on %s failed."), *Struct->GetStructCPPName());
		}
		else
		{
			TSharedPtr<FRepLayout> RepLayout = Driver->GetStructRepLayout(Struct);

			RepLayout_SerializePropertiesForStruct(*RepLayout, ValueDataWriter, PackageMap, const_cast<uint8*>(Data), bHasUnmapped);
		}

		Schema_AddString(Object, Id, std::string(reinterpret_cast<char*>(ValueDataWriter.GetData()), ValueDataWriter.GetNumBytes()));
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
				UnresolvedObjects.Add(ObjectValue);
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
			Schema_AddProperty(Object, Id, ArrayProperty->Inner, ArrayHelper.GetRawPtr(i), PackageMap, Driver, UnresolvedObjects, ClearedIds);
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
			Schema_AddProperty(Object, Id, EnumProperty->GetUnderlyingProperty(), Data, PackageMap, Driver, UnresolvedObjects, ClearedIds);
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

void ReadStructProperty(FSpatialNetBitReader& Reader, USpatialPackageMapClient* PackageMap, UStructProperty* Property, uint8* Data, UNetDriver* Driver, bool& bOutHasUnmapped)
{
	UScriptStruct* Struct = Property->Struct;

	if (Struct->StructFlags & STRUCT_NetSerializeNative)
	{
		UScriptStruct::ICppStructOps* CppStructOps = Struct->GetCppStructOps();
		check(CppStructOps); // else should not have STRUCT_NetSerializeNative
		bool bSuccess = true;
		if (!CppStructOps->NetSerialize(Reader, PackageMap, bSuccess, Data))
		{
			bOutHasUnmapped = true;
		}
		checkf(bSuccess, TEXT("NetSerialize on %s failed."), *Struct->GetStructCPPName());
	}
	else
	{
		TSharedPtr<FRepLayout> RepLayout = Driver->GetStructRepLayout(Struct);

		RepLayout_SerializePropertiesForStruct(*RepLayout, Reader, PackageMap, Data, bOutHasUnmapped);
	}
}

void Schema_GetProperty(Schema_Object* Object, Schema_FieldId Id, std::uint32_t Index, UProperty* Property, uint8* Data, USpatialPackageMapClient* PackageMap, UNetDriver* Driver, FObjectReferencesMap& ObjectReferencesMap, TSet<UnrealObjectRef>& UnresolvedRefs, int32 Offset, uint16 ParentIndex)
{
	if (UStructProperty* StructProperty = Cast<UStructProperty>(Property))
	{
		auto ValueData = Schema_IndexString(Object, Id, Index);
		// A bit hacky, we should probably include the number of bits with the data instead.
		int64 CountBits = ValueData.size() * 8;
		FSpatialNetBitReader ValueDataReader(PackageMap, (uint8*)ValueData.data(), CountBits);
		bool bHasUnmapped = false;

		PackageMap->ResetTrackedObjectRefs(true);

		ReadStructProperty(ValueDataReader, PackageMap, StructProperty, Data, Driver, bHasUnmapped);

		if (bHasUnmapped)
		{
			ObjectReferencesMap.Add(Offset, FObjectReferences(TArray<uint8>((uint8*)ValueData.data(), ValueData.size()), CountBits, PackageMap->GetTrackedUnresolvedRefs(), ParentIndex, Property));
			UnresolvedRefs.Append(PackageMap->GetTrackedUnresolvedRefs());
		}
		else if (ObjectReferencesMap.Find(Offset))
		{
			ObjectReferencesMap.Remove(Offset);
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
		bool bUnresolved = false;

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
	else if (UEnumProperty* EnumProperty = Cast<UEnumProperty>(Property))
	{
		if (EnumProperty->ElementSize < 4)
		{
			EnumProperty->GetUnderlyingProperty()->SetIntPropertyValue(Data, (uint64)Schema_IndexUint32(Object, Id, Index));
		}
		else
		{
			Schema_GetProperty(Object, Id, Index, EnumProperty->GetUnderlyingProperty(), Data, PackageMap, Driver, ObjectReferencesMap, UnresolvedRefs, Offset, ParentIndex);
		}
	}
	else
	{
		checkf(false, TEXT("What is this"));
	}
}

void Schema_GetArrayProperty(Schema_Object* Object, Schema_FieldId Id, UArrayProperty* Property, uint8* Data, USpatialPackageMapClient* PackageMap, UNetDriver* Driver, FObjectReferencesMap& ObjectReferencesMap, TSet<UnrealObjectRef>& UnresolvedRefs, int32 Offset, uint16 ParentIndex)
{
	FObjectReferencesMap* ArrayObjectReferences;
	bool bNewArrayMap = false;
	if (auto ExistingEntry = ObjectReferencesMap.Find(Offset))
	{
		check(ExistingEntry->Array);
		ArrayObjectReferences = ExistingEntry->Array.Get();
	}
	else
	{
		bNewArrayMap = true;
		ArrayObjectReferences = new FObjectReferencesMap();
	}

	FScriptArrayHelper ArrayHelper(Property, Data);

	int Count = Schema_GetPropertyCount(Object, Id, Property->Inner);
	ArrayHelper.Resize(Count);

	for (int i = 0; i < Count; i++)
	{
		int32 ElementOffset = i * Property->Inner->ElementSize;
		Schema_GetProperty(Object, Id, i, Property->Inner, ArrayHelper.GetRawPtr(i), PackageMap, Driver, *ArrayObjectReferences, UnresolvedRefs, ElementOffset, ParentIndex);
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

void Schema_ReadDynamicObject(Schema_Object* ComponentObject, USpatialActorChannel* Channel, USpatialPackageMapClient* PackageMap, UNetDriver* Driver, EAlsoReplicatedPropertyGroup PropertyGroup, FObjectReferencesMap& ObjectReferencesMap, TSet<UnrealObjectRef>& UnresolvedRefs, bool bIsInitialData, const std::set<Schema_FieldId>& ClearedIds = std::set<Schema_FieldId>())
{
	Channel->PreReceiveSpatialUpdate(Channel->Actor);

	auto RepState = Channel->ActorReplicator->RepState;
	auto& Cmds = Channel->ActorReplicator->RepLayout->Cmds;
	auto& BaseHandleToCmdIndex = Channel->ActorReplicator->RepLayout->BaseHandleToCmdIndex;
	auto& Parents = Channel->ActorReplicator->RepLayout->Parents;

	TArray<UProperty*> RepNotifies;

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

				uint8* Data = (uint8*)Channel->Actor + SwappedCmd.Offset;

				if (bIsInitialData || Schema_GetPropertyCount(ComponentObject, HandleIterator.Handle, Cmd.Property) > 0 || ClearedIds.find(HandleIterator.Handle) != ClearedIds.end())
				{
					if (Cmd.Type == REPCMD_DynamicArray)
					{
						Schema_GetArrayProperty(ComponentObject, HandleIterator.Handle, Cast<UArrayProperty>(Cmd.Property), Data, PackageMap, Driver, ObjectReferencesMap, UnresolvedRefs, SwappedCmd.Offset, Cmd.ParentIndex);
					}
					else
					{
						Schema_GetProperty(ComponentObject, HandleIterator.Handle, 0, Cmd.Property, Data, PackageMap, Driver, ObjectReferencesMap, UnresolvedRefs, SwappedCmd.Offset, Cmd.ParentIndex);
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

			if (Cmd.Type == REPCMD_DynamicArray)
			{
				if (!HandleIterator.JumpOverArray())
				{
					break;
				}
			}
		}
	}

	Channel->PostReceiveSpatialUpdate(Channel->Actor, RepNotifies);
}

void ReadDynamicData(const Worker_ComponentData& ComponentData, USpatialActorChannel* Channel, USpatialPackageMapClient* PackageMap, UNetDriver* Driver, EAlsoReplicatedPropertyGroup PropertyGroup, FObjectReferencesMap& ObjectReferencesMap, TSet<UnrealObjectRef>& UnresolvedRefs)
{
	auto ComponentObject = Schema_GetComponentDataFields(ComponentData.schema_type);

	Schema_ReadDynamicObject(ComponentObject, Channel, PackageMap, Driver, PropertyGroup, ObjectReferencesMap, UnresolvedRefs, true);
}

void ReceiveDynamicUpdate(const Worker_ComponentUpdate& ComponentUpdate, USpatialActorChannel* Channel, USpatialPackageMapClient* PackageMap, UNetDriver* Driver, EAlsoReplicatedPropertyGroup PropertyGroup, FObjectReferencesMap& ObjectReferencesMap, TSet<UnrealObjectRef>& UnresolvedRefs)
{
	auto ComponentObject = Schema_GetComponentUpdateFields(ComponentUpdate.schema_type);

	std::vector<Schema_FieldId> ClearedIdsList(Schema_GetComponentUpdateClearedFieldCount(ComponentUpdate.schema_type));
	Schema_GetComponentUpdateClearedFieldList(ComponentUpdate.schema_type, ClearedIdsList.data());

	std::set<Schema_FieldId> ClearedIds;

	for (auto FieldId : ClearedIdsList)
	{
		ClearedIds.insert(FieldId);
	}

	Schema_ReadDynamicObject(ComponentObject, Channel, PackageMap, Driver, PropertyGroup, ObjectReferencesMap, UnresolvedRefs, false, ClearedIds);
}

bool Schema_FillDynamicObject(Schema_Object* ComponentObject, const FPropertyChangeState& Changes, USpatialPackageMapClient* PackageMap, UNetDriver* Driver, EAlsoReplicatedPropertyGroup PropertyGroup, FUnresolvedObjectsMap& UnresolvedObjectsMap, bool bIsInitialData, std::vector<Schema_FieldId>* ClearedIds = nullptr)
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
				TSet<const UObject*> UnresolvedObjects;

				Schema_AddProperty(ComponentObject, HandleIterator.Handle, Cmd.Property, Data, PackageMap, Driver, UnresolvedObjects, ClearedIds);

				if (UnresolvedObjects.Num() == 0)
				{
					bWroteSomething = true;
				}
				else
				{
					if (!bIsInitialData)
					{
						// We have to write something if it's initial data. ...I think.
						Schema_ClearField(ComponentObject, HandleIterator.Handle);
					}

					UnresolvedObjectsMap.Add(HandleIterator.Handle, UnresolvedObjects);
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

	// TODO: Handover
	// Populate the handover data component update from the handover property changelist.
	//for (uint16 ChangedHandle : Changes.HandoverChanged)
	//{
	//}

	return bWroteSomething;
}

Worker_ComponentData CreateDynamicData(Worker_ComponentId ComponentId, const FPropertyChangeState& Changes, USpatialPackageMapClient* PackageMap, UNetDriver* Driver, EAlsoReplicatedPropertyGroup PropertyGroup, FUnresolvedObjectsMap& UnresolvedObjectsMap)
{
	Worker_ComponentData ComponentData = {};
	ComponentData.component_id = ComponentId;
	ComponentData.schema_type = Schema_CreateComponentData(ComponentId);
	auto ComponentObject = Schema_GetComponentDataFields(ComponentData.schema_type);

	Schema_FillDynamicObject(ComponentObject, Changes, PackageMap, Driver, PropertyGroup, UnresolvedObjectsMap, true);

	return ComponentData;
}

Worker_ComponentUpdate CreateDynamicUpdate(Worker_ComponentId ComponentId, const FPropertyChangeState& Changes, USpatialPackageMapClient* PackageMap, UNetDriver* Driver, EAlsoReplicatedPropertyGroup PropertyGroup, FUnresolvedObjectsMap& UnresolvedObjectsMap, bool& bWroteSomething)
{
	Worker_ComponentUpdate ComponentUpdate = {};

	ComponentUpdate.component_id = ComponentId;
	ComponentUpdate.schema_type = Schema_CreateComponentUpdate(ComponentId);
	auto ComponentObject = Schema_GetComponentUpdateFields(ComponentUpdate.schema_type);

	std::vector<Schema_FieldId> ClearedIds;

	bWroteSomething = Schema_FillDynamicObject(ComponentObject, Changes, PackageMap, Driver, PropertyGroup, UnresolvedObjectsMap, false, &ClearedIds);

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

Worker_CommandRequest CreateRPCCommandRequest(UObject* TargetObject, UFunction* Function, void* Parameters, USpatialPackageMapClient* PackageMap, UNetDriver* Driver, Worker_ComponentId ComponentId, Schema_FieldId CommandIndex, Worker_EntityId& OutEntityId)
{
	Worker_CommandRequest CommandRequest = {};

	CommandRequest.component_id = ComponentId;
	CommandRequest.schema_type = Schema_CreateCommandRequest(ComponentId, CommandIndex);
	auto RequestObject = Schema_GetCommandRequestObject(CommandRequest.schema_type);

	auto TargetObjectRef = UnrealObjectRef(PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject)));
	if (TargetObjectRef == UNRESOLVED_OBJECT_REF)
	{
		// TODO: Handle RPC to unresolved object
		checkNoEntry();
	}

	Schema_AddUint32(RequestObject, 1, TargetObjectRef.Offset);
	OutEntityId = TargetObjectRef.Entity;

	TSet<const UObject*> UnresolvedObjects;
	FSpatialNetBitWriter PayloadWriter(PackageMap, UnresolvedObjects);

	TSharedPtr<FRepLayout> RepLayout = Driver->GetFunctionRepLayout(Function);
	RepLayout_SendPropertiesForRPC(*RepLayout, PayloadWriter, Parameters);

	// TODO: Check for unresolved objects in the payload

	Schema_AddString(RequestObject, 2, std::string(reinterpret_cast<char*>(PayloadWriter.GetData()), PayloadWriter.GetNumBytes()));

	return CommandRequest;
}

void ReceiveRPCCommandRequest(const Worker_CommandRequest& CommandRequest, Worker_EntityId EntityId, UFunction* Function, USpatialPackageMapClient* PackageMap, UNetDriver* Driver, UObject*& OutTargetObject, void* Data)
{
	auto RequestObject = Schema_GetCommandRequestObject(CommandRequest.schema_type);

	UnrealObjectRef TargetObjectRef;
	TargetObjectRef.Entity = EntityId;
	TargetObjectRef.Offset = Schema_GetUint32(RequestObject, 1);

	FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef.ToCppAPI());
	if (!TargetNetGUID.IsValid())
	{
		// TODO: Handle RPC to unresolved object
		checkNoEntry();
	}

	OutTargetObject = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
	checkf(OutTargetObject, TEXT("Object Ref %s (NetGUID %s) does not correspond to a UObject."), *TargetObjectRef.ToString(), *TargetNetGUID.ToString());

	auto PayloadData = Schema_GetString(RequestObject, 2);
	// A bit hacky, we should probably include the number of bits with the data instead.
	int64 CountBits = PayloadData.size() * 8;
	FSpatialNetBitReader PayloadReader(PackageMap, (uint8*)PayloadData.data(), CountBits);

	TSharedPtr<FRepLayout> RepLayout = Driver->GetFunctionRepLayout(Function);
	RepLayout_ReceivePropertiesForRPC(*RepLayout, PayloadReader, Data);

	// TODO: Check for unresolved objects in the payload
}

Worker_ComponentUpdate CreateMulticastUpdate(UObject* TargetObject, UFunction* Function, void* Parameters, USpatialPackageMapClient* PackageMap, UNetDriver* Driver, Worker_ComponentId ComponentId, Schema_FieldId EventIndex, Worker_EntityId& OutEntityId)
{
	Worker_ComponentUpdate ComponentUpdate = {};

	ComponentUpdate.component_id = ComponentId;
	ComponentUpdate.schema_type = Schema_CreateComponentUpdate(ComponentId);
	auto EventsObject = Schema_GetComponentUpdateEvents(ComponentUpdate.schema_type);
	auto EventData = Schema_AddObject(EventsObject, EventIndex);

	auto TargetObjectRef = UnrealObjectRef(PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject)));
	if (TargetObjectRef == UNRESOLVED_OBJECT_REF)
	{
		// TODO: Handle RPC to unresolved object
		checkNoEntry();
	}

	Schema_AddUint32(EventData, 1, TargetObjectRef.Offset);
	OutEntityId = TargetObjectRef.Entity;

	TSet<const UObject*> UnresolvedObjects;
	FSpatialNetBitWriter PayloadWriter(PackageMap, UnresolvedObjects);

	TSharedPtr<FRepLayout> RepLayout = Driver->GetFunctionRepLayout(Function);
	RepLayout_SendPropertiesForRPC(*RepLayout, PayloadWriter, Parameters);

	// TODO: Check for unresolved objects in the payload

	Schema_AddString(EventData, 2, std::string(reinterpret_cast<char*>(PayloadWriter.GetData()), PayloadWriter.GetNumBytes()));

	return ComponentUpdate;
}

void ReceiveMulticastUpdate(const Worker_ComponentUpdate& ComponentUpdate, Worker_EntityId EntityId, const TArray<UFunction*>& RPCArray, USpatialPackageMapClient* PackageMap, UNetDriver* Driver)
{
	auto EventsObject = Schema_GetComponentUpdateEvents(ComponentUpdate.schema_type);

	for (Schema_FieldId EventIndex = 1; (int)EventIndex <= RPCArray.Num(); EventIndex++)
	{
		auto Function = RPCArray[EventIndex - 1];
		for (uint32 i = 0; i < Schema_GetObjectCount(EventsObject, EventIndex); i++)
		{
			uint8* Parms = (uint8*)FMemory_Alloca(Function->ParmsSize);
			FMemory::Memzero(Parms, Function->ParmsSize);

			auto EventData = Schema_IndexObject(EventsObject, EventIndex, i);

			UnrealObjectRef TargetObjectRef;
			TargetObjectRef.Entity = EntityId;
			TargetObjectRef.Offset = Schema_GetUint32(EventData, 1);

			FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef.ToCppAPI());
			if (!TargetNetGUID.IsValid())
			{
				// TODO: Handle RPC to unresolved object
				checkNoEntry();
			}

			auto TargetObject = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
			checkf(TargetObject, TEXT("Object Ref %s (NetGUID %s) does not correspond to a UObject."), *TargetObjectRef.ToString(), *TargetNetGUID.ToString());

			auto PayloadData = Schema_GetString(EventData, 2);
			// A bit hacky, we should probably include the number of bits with the data instead.
			int64 CountBits = PayloadData.size() * 8;
			FSpatialNetBitReader PayloadReader(PackageMap, (uint8*)PayloadData.data(), CountBits);

			TSharedPtr<FRepLayout> RepLayout = Driver->GetFunctionRepLayout(Function);
			RepLayout_ReceivePropertiesForRPC(*RepLayout, PayloadReader, Parms);

			// TODO: Check for unresolved objects in the payload

			TargetObject->ProcessEvent(Function, Parms);

			// Destroy the parameters.
			// warning: highly dependent on UObject::ProcessEvent freeing of parms!
			for (TFieldIterator<UProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
			{
				It->DestroyValue_InContainer(Parms);
			}
		}
	}
}

void ResolveObjectReferences(FRepLayout& RepLayout, UObject* ReplicatedObject, FObjectReferencesMap& ObjectReferencesMap, USpatialPackageMapClient* PackageMap, UNetDriver* Driver, uint8* RESTRICT StoredData, uint8* RESTRICT Data, int32 MaxAbsOffset, TArray<UProperty*>& RepNotifies, bool& bOutSomeObjectsWereMapped, bool& bOutStillHasUnresolved)
{
	for (auto It = ObjectReferencesMap.CreateIterator(); It; ++It)
	{
		int32 AbsOffset = It.Key();

		if (AbsOffset >= MaxAbsOffset)
		{
			UE_LOG(LogTemp, Log, TEXT("!!! ResolveObjectReferences: Removed unresolved reference: AbsOffset >= MaxAbsOffset: %d"), AbsOffset);
			It.RemoveCurrent();
			continue;
		}

		auto& ObjectReferences = It.Value();
		auto Property = ObjectReferences.Property;
		auto& Parent = RepLayout.Parents[ObjectReferences.ParentIndex];

		if (ObjectReferences.Array)
		{
			check(Property->IsA<UArrayProperty>());

			auto* StoredArray = (FScriptArray*)(StoredData + AbsOffset);
			auto* Array = (FScriptArray*)(Data + AbsOffset);

			int32 NewMaxOffset = FMath::Min(StoredArray->Num(), Array->Num()) * Property->ElementSize;

			bool bArrayHasUnresolved = false;
			ResolveObjectReferences(RepLayout, ReplicatedObject, *ObjectReferences.Array, PackageMap, Driver, (uint8*)StoredArray->GetData(), (uint8*)Array->GetData(), NewMaxOffset, RepNotifies, bOutSomeObjectsWereMapped, bArrayHasUnresolved);
			if (!bArrayHasUnresolved)
			{
				It.RemoveCurrent();
			}
			else
			{
				bOutStillHasUnresolved = true;
			}
			continue;
		}

		bool bResolvedSomeRefs = false;
		UObject* SinglePropObject = nullptr;

		for (auto UnresolvedIt = ObjectReferences.UnresolvedRefs.CreateIterator(); UnresolvedIt; ++UnresolvedIt)
		{
			auto& ObjectRef = *UnresolvedIt;

			auto NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef.ToCppAPI());
			if (NetGUID.IsValid())
			{
				UObject* Object = PackageMap->GetObjectFromNetGUID(NetGUID, true);
				check(Object);

				UE_LOG(LogTemp, Log, TEXT("!!! ResolveObjectReferences: Resolved object ref: Offset: %d, Object ref: %s, PropName: %s, ObjName: %s"), AbsOffset, *ObjectRef.ToString(), *Property->GetNameCPP(), *Object->GetName());

				UnresolvedIt.RemoveCurrent();
				bResolvedSomeRefs = true;

				if (ObjectReferences.bSingleProp)
				{
					SinglePropObject = Object;
				}
			}
		}

		if (bResolvedSomeRefs)
		{
			if (!bOutSomeObjectsWereMapped)
			{
				ReplicatedObject->PreNetReceive();
				bOutSomeObjectsWereMapped = true;
			}

			if (Parent.Property->HasAnyPropertyFlags(CPF_RepNotify))
			{
				Property->CopySingleValue(StoredData + AbsOffset, Data + AbsOffset);
			}

			if (ObjectReferences.bSingleProp)
			{
				auto ObjectProperty = Cast<UObjectPropertyBase>(Property);
				check(ObjectProperty);

				ObjectProperty->SetObjectPropertyValue(Data + AbsOffset, SinglePropObject);
			}
			else
			{
				FSpatialNetBitReader Reader(PackageMap, ObjectReferences.Buffer.GetData(), ObjectReferences.NumBufferBits);
				check(Property->IsA<UStructProperty>());
				ReadStructProperty(Reader, PackageMap, Cast<UStructProperty>(Property), Data + AbsOffset, Driver, bOutStillHasUnresolved);
			}

			if (Parent.Property->HasAnyPropertyFlags(CPF_RepNotify))
			{
				if (Parent.RepNotifyCondition == REPNOTIFY_Always || !Property->Identical(StoredData + AbsOffset, Data + AbsOffset))
				{
					RepNotifies.AddUnique(Parent.Property);
				}
			}
		}

		if (ObjectReferences.UnresolvedRefs.Num() > 0)
		{
			bOutStillHasUnresolved = true;
		}
		else
		{
			It.RemoveCurrent();
		}
	}
}
