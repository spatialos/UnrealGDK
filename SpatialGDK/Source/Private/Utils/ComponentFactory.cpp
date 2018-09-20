// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ComponentFactory.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetBitWriter.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "SpatialConstants.h"
#include "Utils/RepLayoutUtils.h"

namespace improbable
{

ComponentFactory::ComponentFactory(FUnresolvedObjectsMap& RepUnresolvedObjectsMap, FUnresolvedObjectsMap& HandoverUnresolvedObjectsMap, USpatialNetDriver* InNetDriver)
	: NetDriver(InNetDriver)
	, PackageMap(InNetDriver->PackageMap)
	, TypebindingManager(InNetDriver->TypebindingManager)
	, PendingRepUnresolvedObjectsMap(RepUnresolvedObjectsMap)
	, PendingHandoverUnresolvedObjectsMap(HandoverUnresolvedObjectsMap)
{ }

bool ComponentFactory::FillSchemaObject(Schema_Object* ComponentObject, UObject* Object, const FRepChangeState& Changes, EReplicatedPropertyGroup PropertyGroup, bool bIsInitialData, TArray<Schema_FieldId>* ClearedIds /*= nullptr*/)
{
	bool bWroteSomething = false;

	// Populate the replicated data component updates from the replicated property changelist.
	if (Changes.RepChanged.Num() > 0)
	{
		FChangelistIterator ChangelistIterator(Changes.RepChanged, 0);
		FRepHandleIterator HandleIterator(ChangelistIterator, Changes.RepLayout.Cmds, Changes.RepLayout.BaseHandleToCmdIndex, 0, 1, 0, Changes.RepLayout.Cmds.Num() - 1);
		while (HandleIterator.NextHandle())
		{
			const FRepLayoutCmd& Cmd = Changes.RepLayout.Cmds[HandleIterator.CmdIndex];
			const FRepParentCmd& Parent = Changes.RepLayout.Parents[Cmd.ParentIndex];

			if (GetGroupFromCondition(Parent.Condition) == PropertyGroup)
			{
				const uint8* Data = (uint8*)Object + Cmd.Offset;
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
						// Don't send updates for fields with unresolved objects, unless it's the initial data,
						// in which case all fields should be populated.
						Schema_ClearField(ComponentObject, HandleIterator.Handle);
					}

					PendingRepUnresolvedObjectsMap.Add(HandleIterator.Handle, UnresolvedObjects);
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

	return bWroteSomething;
}

bool ComponentFactory::FillHandoverSchemaObject(Schema_Object* ComponentObject, UObject* Object, const FHandoverChangeState& Changes, bool bIsInitialData, TArray<Schema_FieldId>* ClearedIds /* = nullptr */)
{
	bool bWroteSomething = false;

	FClassInfo* Info = TypebindingManager->FindClassInfoByClass(Object->GetClass());
	check(Info);

	for (uint16 ChangedHandle : Changes)
	{
		check(ChangedHandle > 0 && ChangedHandle - 1 < Info->HandoverProperties.Num());
		const FHandoverPropertyInfo& PropertyInfo = Info->HandoverProperties[ChangedHandle - 1];

		const uint8* Data = (uint8*)Object + PropertyInfo.Offset;
		TSet<const UObject*> UnresolvedObjects;

		AddProperty(ComponentObject, ChangedHandle, PropertyInfo.Property, Data, UnresolvedObjects, ClearedIds);

		if (UnresolvedObjects.Num() == 0)
		{
			bWroteSomething = true;
		}
		else
		{
			if (!bIsInitialData)
			{
				// Don't send updates for fields with unresolved objects, unless it's the initial data,
				// in which case all fields should be populated.
				Schema_ClearField(ComponentObject, ChangedHandle);
			}

			PendingHandoverUnresolvedObjectsMap.Add(ChangedHandle, UnresolvedObjects);
		}
	}

	return bWroteSomething;
}

void ComponentFactory::AddProperty(Schema_Object* Object, Schema_FieldId FieldId, UProperty* Property, const uint8* Data, TSet<const UObject*>& UnresolvedObjects, TArray<Schema_FieldId>* ClearedIds)
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

		AddPayloadToSchema(Object, FieldId, ValueDataWriter);
	}
	else if (UBoolProperty* BoolProperty = Cast<UBoolProperty>(Property))
	{
		Schema_AddBool(Object, FieldId, (uint8)BoolProperty->GetPropertyValue(Data));
	}
	else if (UFloatProperty* FloatProperty = Cast<UFloatProperty>(Property))
	{
		Schema_AddFloat(Object, FieldId, FloatProperty->GetPropertyValue(Data));
	}
	else if (UDoubleProperty* DoubleProperty = Cast<UDoubleProperty>(Property))
	{
		Schema_AddDouble(Object, FieldId, DoubleProperty->GetPropertyValue(Data));
	}
	else if (UInt8Property* Int8Property = Cast<UInt8Property>(Property))
	{
		Schema_AddInt32(Object, FieldId, (int32)Int8Property->GetPropertyValue(Data));
	}
	else if (UInt16Property* Int16Property = Cast<UInt16Property>(Property))
	{
		Schema_AddInt32(Object, FieldId, (int32)Int16Property->GetPropertyValue(Data));
	}
	else if (UIntProperty* IntProperty = Cast<UIntProperty>(Property))
	{
		Schema_AddInt32(Object, FieldId, IntProperty->GetPropertyValue(Data));
	}
	else if (UInt64Property* Int64Property = Cast<UInt64Property>(Property))
	{
		Schema_AddInt64(Object, FieldId, Int64Property->GetPropertyValue(Data));
	}
	else if (UByteProperty* ByteProperty = Cast<UByteProperty>(Property))
	{
		Schema_AddUint32(Object, FieldId, (uint32)ByteProperty->GetPropertyValue(Data));
	}
	else if (UUInt16Property* UInt16Property = Cast<UUInt16Property>(Property))
	{
		Schema_AddUint32(Object, FieldId, (uint32)UInt16Property->GetPropertyValue(Data));
	}
	else if (UUInt32Property* UInt32Property = Cast<UUInt32Property>(Property))
	{
		Schema_AddUint32(Object, FieldId, UInt32Property->GetPropertyValue(Data));
	}
	else if (UUInt64Property* UInt64Property = Cast<UUInt64Property>(Property))
	{
		Schema_AddUint64(Object, FieldId, UInt64Property->GetPropertyValue(Data));
	}
	else if (UObjectPropertyBase* ObjectProperty = Cast<UObjectPropertyBase>(Property))
	{
		improbable::UnrealObjectRef ObjectRef = SpatialConstants::NULL_OBJECT_REF;

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
			ObjectRef = improbable::UnrealObjectRef(PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID));
			if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
			{
				// A legal static object reference should never be unresolved.
				check(!ObjectValue->IsFullNameStableForNetworking());
				UnresolvedObjects.Add(ObjectValue);
				ObjectRef = SpatialConstants::NULL_OBJECT_REF;
			}
		}

		AddObjectRefToSchema(Object, FieldId, ObjectRef);
	}
	else if (UNameProperty* NameProperty = Cast<UNameProperty>(Property))
	{
		AddStringToSchema(Object, FieldId, NameProperty->GetPropertyValue(Data).ToString());
	}
	else if (UStrProperty* StrProperty = Cast<UStrProperty>(Property))
	{
		AddStringToSchema(Object, FieldId, StrProperty->GetPropertyValue(Data));
	}
	else if (UTextProperty* TextProperty = Cast<UTextProperty>(Property))
	{
		AddStringToSchema(Object, FieldId, TextProperty->GetPropertyValue(Data).ToString());
	}
	else if (UArrayProperty* ArrayProperty = Cast<UArrayProperty>(Property))
	{
		FScriptArrayHelper ArrayHelper(ArrayProperty, Data);
		for (int i = 0; i < ArrayHelper.Num(); i++)
		{
			AddProperty(Object, FieldId, ArrayProperty->Inner, ArrayHelper.GetRawPtr(i), UnresolvedObjects, ClearedIds);
		}

		if (ArrayHelper.Num() == 0 && ClearedIds)
		{
			ClearedIds->Add(FieldId);
		}
	}
	else if (UEnumProperty* EnumProperty = Cast<UEnumProperty>(Property))
	{
		if (EnumProperty->ElementSize < 4)
		{
			Schema_AddUint32(Object, FieldId, (uint32)EnumProperty->GetUnderlyingProperty()->GetUnsignedIntPropertyValue(Data));
		}
		else
		{
			AddProperty(Object, FieldId, EnumProperty->GetUnderlyingProperty(), Data, UnresolvedObjects, ClearedIds);
		}
	}
	else
	{
		checkf(false, TEXT("Tried to add unknown property in field %d"), FieldId);
	}
}

TArray<Worker_ComponentData> ComponentFactory::CreateComponentDatas(UObject* Object, const FRepChangeState& RepChangeState, const FHandoverChangeState& HandoverChangeState)
{
	TArray<Worker_ComponentData> ComponentDatas;

	FClassInfo* Info = TypebindingManager->FindClassInfoByClass(Object->GetClass());
	check(Info);

	ComponentDatas.Add(CreateComponentData(Info->SingleClientComponent, Object, RepChangeState, GROUP_SingleClient));
	ComponentDatas.Add(CreateComponentData(Info->MultiClientComponent, Object, RepChangeState, GROUP_MultiClient));
	ComponentDatas.Add(CreateHandoverComponentData(Info->HandoverComponent, Object, HandoverChangeState));

	return ComponentDatas;
}

Worker_ComponentData ComponentFactory::CreateComponentData(Worker_ComponentId ComponentId, UObject* Object, const FRepChangeState& Changes, EReplicatedPropertyGroup PropertyGroup)
{
	Worker_ComponentData ComponentData = {};
	ComponentData.component_id = ComponentId;
	ComponentData.schema_type = Schema_CreateComponentData(ComponentId);
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(ComponentData.schema_type);

	FillSchemaObject(ComponentObject, Object, Changes, PropertyGroup, true);

	return ComponentData;
}

Worker_ComponentData ComponentFactory::CreateEmptyComponentData(Worker_ComponentId ComponentId)
{
	Worker_ComponentData ComponentData = {};
	ComponentData.component_id = ComponentId;
	ComponentData.schema_type = Schema_CreateComponentData(ComponentId);

	return ComponentData;
}

Worker_ComponentData ComponentFactory::CreateHandoverComponentData(Worker_ComponentId ComponentId, UObject* Object, const FHandoverChangeState& Changes)
{
	Worker_ComponentData ComponentData = CreateEmptyComponentData(ComponentId);
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(ComponentData.schema_type);

	FillHandoverSchemaObject(ComponentObject, Object, Changes, true);

	return ComponentData;
}

TArray<Worker_ComponentUpdate> ComponentFactory::CreateComponentUpdates(UObject* Object, const FRepChangeState* RepChangeState, const FHandoverChangeState* HandoverChangeState)
{
	TArray<Worker_ComponentUpdate> ComponentUpdates;

	FClassInfo* Info = TypebindingManager->FindClassInfoByClass(Object->GetClass());
	check(Info);

	if (RepChangeState)
	{
		bool bWroteSomething = false;
		Worker_ComponentUpdate SingleClientUpdate = CreateComponentUpdate(Info->SingleClientComponent, Object, *RepChangeState, GROUP_SingleClient, bWroteSomething);
		if (bWroteSomething)
		{
			ComponentUpdates.Add(SingleClientUpdate);
		}
		bWroteSomething = false;
		Worker_ComponentUpdate MultiClientUpdate = CreateComponentUpdate(Info->MultiClientComponent, Object, *RepChangeState, GROUP_MultiClient, bWroteSomething);
		if (bWroteSomething)
		{
			ComponentUpdates.Add(MultiClientUpdate);
		}
	}

	if (HandoverChangeState)
	{
		bool bWroteSomething = false;
		Worker_ComponentUpdate HandoverUpdate = CreateHandoverComponentUpdate(Info->HandoverComponent, Object, *HandoverChangeState, bWroteSomething);
		if (bWroteSomething)
		{
			ComponentUpdates.Add(HandoverUpdate);
		}
	}

	return ComponentUpdates;
}

Worker_ComponentUpdate ComponentFactory::CreateComponentUpdate(Worker_ComponentId ComponentId, UObject* Object, const FRepChangeState& Changes, EReplicatedPropertyGroup PropertyGroup, bool& bWroteSomething)
{
	Worker_ComponentUpdate ComponentUpdate = {};

	ComponentUpdate.component_id = ComponentId;
	ComponentUpdate.schema_type = Schema_CreateComponentUpdate(ComponentId);
	Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(ComponentUpdate.schema_type);

	TArray<Schema_FieldId> ClearedIds;

	bWroteSomething = FillSchemaObject(ComponentObject, Object, Changes, PropertyGroup, false, &ClearedIds);

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

Worker_ComponentUpdate ComponentFactory::CreateHandoverComponentUpdate(Worker_ComponentId ComponentId, UObject* Object, const FHandoverChangeState& Changes, bool& bWroteSomething)
{
	Worker_ComponentUpdate ComponentUpdate = {};

	ComponentUpdate.component_id = ComponentId;
	ComponentUpdate.schema_type = Schema_CreateComponentUpdate(ComponentId);
	Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(ComponentUpdate.schema_type);

	TArray<Schema_FieldId> ClearedIds;

	bWroteSomething = FillHandoverSchemaObject(ComponentObject, Object, Changes, false, &ClearedIds);

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

}
