// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SchemaGenerator.h"

#include "Algo/Reverse.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SCS_Node.h"
#include "UObject/TextProperty.h"

#include "Interop/SpatialClassInfoManager.h"
#include "Utils/CodeWriter.h"
#include "Utils/ComponentIdGenerator.h"
#include "Utils/DataTypeUtilities.h"

DEFINE_LOG_CATEGORY(LogSchemaGenerator);

ESchemaComponentType PropertyGroupToSchemaComponentType(EReplicatedPropertyGroup Group)
{
	if (Group == REP_MultiClient)
	{
		return SCHEMA_Data;
	}
	else if (Group == REP_SingleClient)
	{
		return SCHEMA_OwnerOnly;
	}
	else
	{
		checkNoEntry();
		return SCHEMA_Invalid;
	}
}

ESchemaComponentType RPCTypeToSchemaComponentType(ERPCType RPC)
{
	if (RPC == RPC_Client)
	{
		return SCHEMA_ClientReliableRPC;
	}
	else if (RPC == RPC_Server)
	{
		return SCHEMA_ServerReliableRPC;
	}
	else if (RPC == RPC_NetMulticast)
	{
		return SCHEMA_NetMulticastRPC;
	}
	else if (RPC == RPC_CrossServer)
	{
		return SCHEMA_CrossServerRPC;
	}
	else
	{
		checkNoEntry();
		return SCHEMA_Invalid;
	}

}

// Given a RepLayout cmd type (a data type supported by the replication system). Generates the corresponding
// type used in schema.
FString PropertyToSchemaType(UProperty* Property, bool bIsRPCProperty)
{
	FString DataType;

	// For RPC arguments we may wish to handle them differently.
	if (bIsRPCProperty)
	{
		if (Property->ArrayDim > 1) // Static arrays in RPC arguments are replicated as lists.
		{
			DataType = PropertyToSchemaType(Property, false); // Have to get the type of the property inside the static array.
			DataType = FString::Printf(TEXT("list<%s>"), *DataType);
			return DataType;
		}
	}

	if (Property->IsA(UStructProperty::StaticClass()))
	{
		UStructProperty* StructProp = Cast<UStructProperty>(Property);
		UScriptStruct* Struct = StructProp->Struct;
		if (Struct->StructFlags & STRUCT_NetSerializeNative)
		{
			// Specifically when NetSerialize is implemented for a struct we want to use 'bytes'.
			// This includes RepMovement and UniqueNetId.
			DataType = TEXT("bytes");
		}
		else
		{
			DataType = TEXT("bytes");
		}
	}
	else if (Property->IsA(UBoolProperty::StaticClass()))
	{
		DataType = TEXT("bool");
	}
	else if (Property->IsA(UFloatProperty::StaticClass()))
	{
		DataType = TEXT("float");
	}
	else if (Property->IsA(UDoubleProperty::StaticClass()))
	{
		DataType = TEXT("double");
	}
	else if (Property->IsA(UInt8Property::StaticClass()))
	{
		DataType = TEXT("int32");
	}
	else if (Property->IsA(UInt16Property::StaticClass()))
	{
		DataType = TEXT("int32");
	}
	else if (Property->IsA(UIntProperty::StaticClass()))
	{
		DataType = TEXT("int32");
	}
	else if (Property->IsA(UInt64Property::StaticClass()))
	{
		DataType = TEXT("int64");
	}
	else if (Property->IsA(UByteProperty::StaticClass()))
	{
		DataType = TEXT("uint32"); // uint8 not supported in schema.
	}
	else if (Property->IsA(UUInt16Property::StaticClass()))
	{
		DataType = TEXT("uint32");
	}
	else if (Property->IsA(UUInt32Property::StaticClass()))
	{
		DataType = TEXT("uint32");
	}
	else if (Property->IsA(UUInt64Property::StaticClass()))
	{
		DataType = TEXT("uint64");
	}
	else if (Property->IsA(UNameProperty::StaticClass()) || Property->IsA(UStrProperty::StaticClass()) || Property->IsA(UTextProperty::StaticClass()))
	{
		DataType = TEXT("string");
	}
	else if (Property->IsA(UObjectPropertyBase::StaticClass()))
	{
		DataType = TEXT("UnrealObjectRef");
	}
	else if (Property->IsA(UArrayProperty::StaticClass()))
	{
		DataType = PropertyToSchemaType(Cast<UArrayProperty>(Property)->Inner, bIsRPCProperty);
		DataType = FString::Printf(TEXT("list<%s>"), *DataType);
	}
	else if (Property->IsA(UEnumProperty::StaticClass()))
	{
		DataType = GetEnumDataType(Cast<UEnumProperty>(Property));
	}
	else
	{
		DataType = TEXT("bytes");
	}

	return DataType;
}

void WriteSchemaRepField(FCodeWriter& Writer, const TSharedPtr<FUnrealProperty> RepProp, const int FieldCounter)
{
	Writer.Printf("{0} {1} = {2};",
		*PropertyToSchemaType(RepProp->Property, false),
		*SchemaFieldName(RepProp),
		FieldCounter
	);
}

void WriteSchemaHandoverField(FCodeWriter& Writer, const TSharedPtr<FUnrealProperty> HandoverProp, const int FieldCounter)
{
	Writer.Printf("{0} {1} = {2};",
		*PropertyToSchemaType(HandoverProp->Property, false),
		*SchemaFieldName(HandoverProp),
		FieldCounter
	);
}

void WriteSchemaRPCField(TSharedPtr<FCodeWriter> Writer, const TSharedPtr<FUnrealProperty> RPCProp, const int FieldCounter)
{
	Writer->Printf("{0} {1} = {2};",
		*PropertyToSchemaType(RPCProp->Property, true),
		*SchemaFieldName(RPCProp),
		FieldCounter
	);
}

bool IsReplicatedSubobject(TSharedPtr<FUnrealType> TypeInfo)
{
	for (auto& PropertyGroup : GetFlatRepData(TypeInfo))
	{
		if (PropertyGroup.Value.Num() > 0)
		{
			return true;
		}
	}

	if (GetFlatHandoverData(TypeInfo).Num() > 0)
	{
		return true;
	}

	if (TypeInfo->RPCs.Num() > 0)
	{
		return true;
	}

	return false;
}

void GenerateSubobjectSchema(UClass* Class, TSharedPtr<FUnrealType> TypeInfo, FString SchemaPath)
{
	FCodeWriter Writer;

	Writer.Printf(R"""(
		// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
		// Note that this file has been generated automatically
		package unreal.generated;)""");

	bool bShouldIncludeCoreTypes = false;

	// Only include core types if the subobject has replicated references to other UObjects
	FUnrealFlatRepData RepData = GetFlatRepData(TypeInfo);
	for (auto& PropertyGroup : RepData)
	{
		for (auto& PropertyPair : PropertyGroup.Value)
		{
			UProperty* Property = PropertyPair.Value->Property;
			if (Property->IsA<UObjectPropertyBase>())
			{
				bShouldIncludeCoreTypes = true;
			}

			if (Property->IsA<UArrayProperty>())
			{
				if (Cast<UArrayProperty>(Property)->Inner->IsA<UObjectPropertyBase>())
				{
					bShouldIncludeCoreTypes = true;
				}
			}
		}
	}

	if (bShouldIncludeCoreTypes)
	{
		Writer.PrintNewLine();
		Writer.Printf("import \"unreal/gdk/core_types.schema\";");
	}

	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		if (RepData[Group].Num() == 0)
		{
			continue;
		}

		// If this class is an Actor Component, it MUST have bReplicates at field ID 1.
		if (Group == REP_MultiClient && Class->IsChildOf<UActorComponent>())
		{
			TSharedPtr<FUnrealProperty> ExpectedReplicatesPropData = RepData[Group].FindRef(SpatialConstants::ACTOR_COMPONENT_REPLICATES_ID);
			const UProperty* ReplicatesProp = UActorComponent::StaticClass()->FindPropertyByName("bReplicates");

			if (!(ExpectedReplicatesPropData.IsValid() && ExpectedReplicatesPropData->Property == ReplicatesProp))
			{
				UE_LOG(LogSchemaGenerator, Error, TEXT("Did not find ActorComponent->bReplicates at field %d for class %s. Modifying the base Actor Component class is currently not supported."),
					SpatialConstants::ACTOR_COMPONENT_REPLICATES_ID,
					*Class->GetName());
			}
		}

		Writer.PrintNewLine();
		Writer.Printf("type {0} {", *SchemaReplicatedDataName(Group, Class));
		Writer.Indent();
		for (auto& RepProp : RepData[Group])
		{
			WriteSchemaRepField(Writer,
				RepProp.Value,
				RepProp.Value->ReplicationData->Handle);
		}
		Writer.Outdent().Print("}");
	}

	FCmdHandlePropertyMap HandoverData = GetFlatHandoverData(TypeInfo);
	if (HandoverData.Num() > 0)
	{
		Writer.PrintNewLine();

		Writer.Printf("type {0} {", *SchemaHandoverDataName(Class));
		Writer.Indent();
		int FieldCounter = 0;
		for (auto& Prop : HandoverData)
		{
			FieldCounter++;
			WriteSchemaHandoverField(Writer,
				Prop.Value,
				FieldCounter);
		}
		Writer.Outdent().Print("}");
	}

	Writer.WriteToFile(FString::Printf(TEXT("%s%s.schema"), *SchemaPath, *ClassToSchemaName[Class]));
}

int GenerateActorSchema(int ComponentId, UClass* Class, TSharedPtr<FUnrealType> TypeInfo, FString SchemaPath)
{
	const FSchemaData* const SchemaData = ClassPathToSchema.Find(*Class->GetPathName());

	FCodeWriter Writer;

	FComponentIdGenerator IdGenerator(ComponentId);

	Writer.Printf(R"""(
		// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
		// Note that this file has been generated automatically
		package unreal.generated.{0};)""",
		*ClassToSchemaName[Class].ToLower());

	// Will always be included since AActor has replicated pointers to other actors
	Writer.PrintNewLine();
	Writer.Printf("import \"unreal/gdk/core_types.schema\";");

	FSchemaData ActorSchemaData;

	FUnrealFlatRepData RepData = GetFlatRepData(TypeInfo);

	// Client-server replicated properties.
	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		if (RepData[Group].Num() == 0)
		{
			continue;
		}

		const Worker_ComponentId CachedComponentId = SchemaData ? SchemaData->SchemaComponents[PropertyGroupToSchemaComponentType(Group)] : SpatialConstants::INVALID_COMPONENT_ID;

		Writer.PrintNewLine();

		Writer.Printf("component {0} {", *SchemaReplicatedDataName(Group, Class));
		Writer.Indent();
		Writer.Printf("id = {0};", IdGenerator.GetNextAvailableId(CachedComponentId));

		ActorSchemaData.SchemaComponents[PropertyGroupToSchemaComponentType(Group)] = IdGenerator.GetCurrentId();

		int FieldCounter = 0;
		for (auto& RepProp : RepData[Group])
		{
			FieldCounter++;
			WriteSchemaRepField(Writer,
				RepProp.Value,
				RepProp.Value->ReplicationData->Handle);
		}

		Writer.Outdent().Print("}");
	}

	FCmdHandlePropertyMap HandoverData = GetFlatHandoverData(TypeInfo);
	if (HandoverData.Num() > 0)
	{
		const Worker_ComponentId CachedComponentId = SchemaData ? SchemaData->SchemaComponents[ESchemaComponentType::SCHEMA_Handover] : SpatialConstants::INVALID_COMPONENT_ID;

		Writer.PrintNewLine();

		// Handover (server to server) replicated properties.
		Writer.Printf("component {0} {", *SchemaHandoverDataName(Class));
		Writer.Indent();
		Writer.Printf("id = {0};", IdGenerator.GetNextAvailableId(CachedComponentId));

		ActorSchemaData.SchemaComponents[ESchemaComponentType::SCHEMA_Handover] = IdGenerator.GetCurrentId();

		int FieldCounter = 0;
		for (auto& Prop : HandoverData)
		{
			FieldCounter++;
			WriteSchemaHandoverField(Writer,
				Prop.Value,
				FieldCounter);
		}
		Writer.Outdent().Print("}");
	}

	GenerateSubobjectSchemaForActor(IdGenerator, Class, TypeInfo, SchemaPath, ActorSchemaData);

	ClassPathToSchema.Add(Class->GetPathName(), ActorSchemaData);

	Writer.WriteToFile(FString::Printf(TEXT("%s%s.schema"), *SchemaPath, *ClassToSchemaName[Class]));

	return IdGenerator.GetNumUsedIds();
}

FSubobjectSchemaData GenerateSubobjectSpecificSchema(FCodeWriter& Writer, FComponentIdGenerator& IdGenerator, FString PropertyName, TSharedPtr<FUnrealType>& TypeInfo, UClass* ComponentClass, UClass* ActorClass, int MapIndex)
{
	const FSchemaData* const SchemaData = ClassPathToSchema.Find(*ActorClass->GetPathName());
	const FSubobjectSchemaData* const SubobjectSchemaData = SchemaData ? SchemaData->SubobjectData.Find(MapIndex) : nullptr;
	
	FUnrealFlatRepData RepData = GetFlatRepData(TypeInfo);

	FSubobjectSchemaData SubobjectData;
	SubobjectData.ClassPath = ComponentClass->GetPathName();

	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		if (RepData[Group].Num() == 0)
		{
			continue;
		}

		const Worker_ComponentId CachedComponentId = SubobjectSchemaData ? SubobjectSchemaData->SchemaComponents[PropertyGroupToSchemaComponentType(Group)] : SpatialConstants::INVALID_COMPONENT_ID;

		Writer.PrintNewLine();

		FString ComponentName = PropertyName + GetReplicatedPropertyGroupName(Group);
		Writer.Printf("component {0} {", *ComponentName);
		Writer.Indent();
		Writer.Printf("id = {0};", IdGenerator.GetNextAvailableId(CachedComponentId));
		Writer.Printf("data {0};", *SchemaReplicatedDataName(Group, ComponentClass));
		Writer.Outdent().Print("}");

		SubobjectData.SchemaComponents[PropertyGroupToSchemaComponentType(Group)] = IdGenerator.GetCurrentId();
	}

	FCmdHandlePropertyMap HandoverData = GetFlatHandoverData(TypeInfo);
	if (HandoverData.Num() > 0)
	{
		const Worker_ComponentId CachedComponentId = SubobjectSchemaData ? SubobjectSchemaData->SchemaComponents[ESchemaComponentType::SCHEMA_Handover] : SpatialConstants::INVALID_COMPONENT_ID;

		Writer.PrintNewLine();

		// Handover (server to server) replicated properties.
		Writer.Printf("component {0} {", *(PropertyName + TEXT("Handover")));
		Writer.Indent();
		Writer.Printf("id = {0};", IdGenerator.GetNextAvailableId(CachedComponentId));
		Writer.Printf("data {0};", *SchemaHandoverDataName(ComponentClass));
		Writer.Outdent().Print("}");

		SubobjectData.SchemaComponents[ESchemaComponentType::SCHEMA_Handover] = IdGenerator.GetCurrentId();
	}

	return SubobjectData;
}

void GenerateSubobjectSchemaForActor(FComponentIdGenerator& IdGenerator, UClass* ActorClass, TSharedPtr<FUnrealType> TypeInfo, FString SchemaPath, FSchemaData& ActorSchemaData)
{
	FCodeWriter Writer;

	Writer.Printf(R"""(
		// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
		// Note that this file has been generated automatically
		package unreal.generated.{0}.subobjects;)""",
		*ClassToSchemaName[ActorClass].ToLower());

	Writer.PrintNewLine();

	GenerateActorIncludes(Writer, TypeInfo);

	FSubobjectMap Subobjects = GetAllSubobjects(TypeInfo);

	bool bHasComponents = false;

	for (auto& It : Subobjects)
	{
		uint32 Offset = It.Key;
		TSharedPtr<FUnrealType>& SubobjectTypeInfo = It.Value;
		UClass* SubobjectClass = Cast<UClass>(SubobjectTypeInfo->Type);

		FSubobjectSchemaData SubobjectData;

		if (IsReplicatedSubobject(SubobjectTypeInfo) && SchemaGeneratedClasses.Contains(SubobjectClass))
		{
			bHasComponents = true;
			SubobjectData = GenerateSubobjectSpecificSchema(Writer, IdGenerator, UnrealNameToSchemaComponentName(SubobjectTypeInfo->Name.ToString()), SubobjectTypeInfo, SubobjectClass, ActorClass, Offset);
		}
		else
		{
			SubobjectData.ClassPath = SubobjectClass->GetPathName();
		}

		SubobjectData.Name = SubobjectTypeInfo->Name;
		ActorSchemaData.SubobjectData.Add(Offset, SubobjectData);
		ClassPathToSchema.Add(SubobjectClass->GetPathName(), FSchemaData());
	}

	if (bHasComponents)
	{
		Writer.WriteToFile(FString::Printf(TEXT("%s%sComponents.schema"), *SchemaPath, *ClassToSchemaName[ActorClass]));
	}
}

void GenerateActorIncludes(FCodeWriter& Writer, TSharedPtr<FUnrealType>& TypeInfo)
{
	TSet<UStruct*> AlreadyImported;

	bool bImportCoreTypes = false;

	for (auto& PropertyPair : TypeInfo->Properties)
	{
		UProperty* Property = PropertyPair.Key;
		UObjectProperty* ObjectProperty = Cast<UObjectProperty>(Property);

		TSharedPtr<FUnrealType>& PropertyTypeInfo = PropertyPair.Value->Type;

		if (ObjectProperty && PropertyTypeInfo.IsValid())
		{
			UObject* Value = PropertyTypeInfo->Object;

			if (Value != nullptr && !Value->IsEditorOnly() && IsReplicatedSubobject(PropertyTypeInfo))
			{
				// Only include core types if a subobject has any RPCs
				bImportCoreTypes |= PropertyTypeInfo->RPCs.Num() > 0;

				UClass* Class = Value->GetClass();
				if (!AlreadyImported.Contains(Class) && SchemaGeneratedClasses.Contains(Class))
				{
					Writer.Printf("import \"unreal/generated/Subobjects/{0}.schema\";", *ClassToSchemaName[Class]);
					AlreadyImported.Add(Class);
				}
			}
		}
	}

	if (bImportCoreTypes)
	{
		Writer.Printf("import \"unreal/gdk/core_types.schema\";");
	}
}
