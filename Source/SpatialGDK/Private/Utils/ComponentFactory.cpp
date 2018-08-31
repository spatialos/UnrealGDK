
#include "Utils/ComponentFactory.h"
#include "Utils/RepLayoutUtils.h"

#include "SpatialActorChannel.h"
#include "SpatialNetDriver.h"
#include "SpatialPackageMapClient.h"
#include "SpatialMemoryWriter.h"
#include "SpatialConstants.h"

ComponentFactory::ComponentFactory(FUnresolvedObjectsMap& UnresolvedObjectsMap, USpatialNetDriver* NetDriver)
	: NetDriver(NetDriver)
	, PackageMap(NetDriver->PackageMap)
	, TypebindingManager(NetDriver->TypebindingManager)
	, PendingUnresolvedObjectsMap(UnresolvedObjectsMap)
{ }

bool ComponentFactory::FillSchemaObject(Schema_Object* ComponentObject, const FPropertyChangeState& Changes, EReplicatedPropertyGroup PropertyGroup, bool bIsInitialData, std::vector<Schema_FieldId>* ClearedIds /*= nullptr*/)
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

				AddProperty(ComponentObject, HandleIterator.Handle, Cmd.Property, Data, UnresolvedObjects, ClearedIds);

				if (UnresolvedObjects.Num() == 0)
				{
					bWroteSomething = true;
				}
				else
				{
					if (!bIsInitialData)
					{
						// We have to write something if it's initial data. ...I think. ?????????????????????
						Schema_ClearField(ComponentObject, HandleIterator.Handle);
					}

					PendingUnresolvedObjectsMap.Add(HandleIterator.Handle, UnresolvedObjects);
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

void ComponentFactory::AddProperty(Schema_Object* Object, Schema_FieldId Id, UProperty* Property, const uint8* Data, TSet<const UObject*>& UnresolvedObjects, std::vector<Schema_FieldId>* ClearedIds)
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
			TSharedPtr<FRepLayout> RepLayout = NetDriver->GetStructRepLayout(Struct);

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
		UnrealObjectRef ObjectRef = SpatialConstants::NULL_OBJECT_REF;

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
			if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
			{
				// A legal static object reference should never be unresolved.
				check(!ObjectValue->IsFullNameStableForNetworking());
				UnresolvedObjects.Add(ObjectValue);
				ObjectRef = SpatialConstants::NULL_OBJECT_REF;
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
			AddProperty(Object, Id, ArrayProperty->Inner, ArrayHelper.GetRawPtr(i), UnresolvedObjects, ClearedIds);
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
			AddProperty(Object, Id, EnumProperty->GetUnderlyingProperty(), Data, UnresolvedObjects, ClearedIds);
		}
	}
	else
	{
		Schema_AddString(Object, Id, "Unknown Field");
	}
}

std::vector<Worker_ComponentData> ComponentFactory::CreateComponentDatas(UObject* Object, const FPropertyChangeState& PropertyChangeState)
{
	std::vector<Worker_ComponentData> ComponentData;

	FClassInfo* Info = TypebindingManager->FindClassInfoByClass(Object->GetClass());
	check(Info);

	ComponentData.push_back(CreateComponentData(Info->SingleClientComponent, PropertyChangeState, GROUP_SingleClient));
	ComponentData.push_back(CreateComponentData(Info->MultiClientComponent, PropertyChangeState, GROUP_MultiClient));

	return ComponentData;
}

Worker_ComponentData ComponentFactory::CreateComponentData(Worker_ComponentId ComponentId, const struct FPropertyChangeState& Changes, EReplicatedPropertyGroup PropertyGroup)
{
	Worker_ComponentData ComponentData = {};
	ComponentData.component_id = ComponentId;
	ComponentData.schema_type = Schema_CreateComponentData(ComponentId);
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(ComponentData.schema_type);

	FillSchemaObject(ComponentObject, Changes, PropertyGroup, true);

	return ComponentData;
}

std::vector<Worker_ComponentUpdate> ComponentFactory::CreateComponentUpdates(UObject* Object, const FPropertyChangeState& PropertyChangeState)
{
	std::vector<Worker_ComponentUpdate> ComponentUpdates;

	FClassInfo* Info = TypebindingManager->FindClassInfoByClass(Object->GetClass());
	check(Info);

	bool wroteSomething = false;
	ComponentUpdates.push_back(CreateComponentUpdate(Info->SingleClientComponent, PropertyChangeState, GROUP_SingleClient, wroteSomething));
	ComponentUpdates.push_back(CreateComponentUpdate(Info->MultiClientComponent, PropertyChangeState, GROUP_MultiClient, wroteSomething));

	return ComponentUpdates;
}

Worker_ComponentUpdate ComponentFactory::CreateComponentUpdate(Worker_ComponentId ComponentId, const struct FPropertyChangeState& Changes, EReplicatedPropertyGroup PropertyGroup, bool& bWroteSomething)
{
	Worker_ComponentUpdate ComponentUpdate = {};

	ComponentUpdate.component_id = ComponentId;
	ComponentUpdate.schema_type = Schema_CreateComponentUpdate(ComponentId);
	Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(ComponentUpdate.schema_type);

	std::vector<Schema_FieldId> ClearedIds;

	FillSchemaObject(ComponentObject, Changes, PropertyGroup, false, &ClearedIds);

	for (Schema_FieldId Id : ClearedIds)
	{
		Schema_AddComponentUpdateClearedField(ComponentUpdate.schema_type, Id);
	}

	if (!bWroteSomething)
	{
		Schema_DestroyComponentUpdate(ComponentUpdate.schema_type);
	}

	return ComponentUpdate;
}
