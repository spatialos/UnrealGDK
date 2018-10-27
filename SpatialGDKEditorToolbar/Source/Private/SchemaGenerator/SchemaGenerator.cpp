// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SchemaGenerator.h"

#include "Algo/Reverse.h"

#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SCS_Node.h"
#include "SpatialTypebindingManager.h"
#include "Utils/CodeWriter.h"
#include "Utils/ComponentIdGenerator.h"
#include "Utils/DataTypeUtilities.h"

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
		return SCHEMA_ClientRPC;
	}
	else if (RPC == RPC_Server)
	{
		return SCHEMA_ServerRPC;
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

// core_types.schema should only be included if any components in the file have
// 1. An UnrealObjectRef
// 2. A list of UnrealObjectRefs
// 3. An RPC
bool ShouldIncludeCoreTypes(TSharedPtr<FUnrealType>& TypeInfo)
{
	FUnrealFlatRepData RepData = GetFlatRepData(TypeInfo);

	for (auto& PropertyGroup : RepData)
	{
		for (auto& PropertyPair : PropertyGroup.Value)
		{
			UProperty* Property = PropertyPair.Value->Property;
			if(Property->IsA<UObjectPropertyBase>())
			{
				return true;
			}

			if (Property->IsA<UArrayProperty>())
			{
				if (Cast<UArrayProperty>(Property)->Inner->IsA<UObjectPropertyBase>())
				{
					return true;
				}
			}
		}
	}

	if (TypeInfo->RPCs.Num() > 0)
	{
		return true;
	}

	for (auto& PropertyPair : TypeInfo->Properties)
	{
		UProperty* Property = PropertyPair.Key;
		if (Property->IsA<UObjectPropertyBase>() && PropertyPair.Value->Type.IsValid())
		{
			if (PropertyPair.Value->Type->RPCs.Num() > 0)
			{
				return true;
			}
		}
	}

	return false;
}

bool IsReplicatedSubobject(TSharedPtr<FUnrealType> TypeInfo)
{
	if (GetFlatRepData(TypeInfo)[REP_MultiClient].Num() > 0 || GetFlatRepData(TypeInfo)[REP_SingleClient].Num() > 0)
	{
		return true;
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

	if (ShouldIncludeCoreTypes(TypeInfo))
	{
		Writer.PrintNewLine();
		Writer.Printf("import \"unreal/gdk/core_types.schema\";");
	}

	FUnrealFlatRepData RepData = GetFlatRepData(TypeInfo);

	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		if (RepData[Group].Num() == 0)
		{
			continue;
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

	Writer.WriteToFile(FString::Printf(TEXT("%s%s.schema"), *SchemaPath, *UnrealNameToSchemaTypeName(Class->GetName())));
}


int GenerateActorSchema(int ComponentId, UClass* Class, TSharedPtr<FUnrealType> TypeInfo, FString SchemaPath)
{
	FCodeWriter Writer;

	FComponentIdGenerator IdGenerator(ComponentId);

	Writer.Printf(R"""(
		// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
		// Note that this file has been generated automatically
		package unreal.generated.{0};)""",
		*UnrealNameToSchemaTypeName(Class->GetName().ToLower()));

	if (ShouldIncludeCoreTypes(TypeInfo))
	{
		Writer.PrintNewLine();
		Writer.Printf("import \"unreal/gdk/core_types.schema\";");
	}

	FSchemaData ActorSchemaData;

	FUnrealFlatRepData RepData = GetFlatRepData(TypeInfo);

	// Client-server replicated properties.
	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		if (RepData[Group].Num() == 0)
		{
			continue;
		}

		Writer.PrintNewLine();

		Writer.Printf("component {0} {", *SchemaReplicatedDataName(Group, Class));
		Writer.Indent();
		Writer.Printf("id = {0};", IdGenerator.GetNextAvailableId());

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
		Writer.PrintNewLine();

		// Handover (server to server) replicated properties.
		Writer.Printf("component {0} {", *SchemaHandoverDataName(Class));
		Writer.Indent();
		Writer.Printf("id = {0};", IdGenerator.GetNextAvailableId());

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

	// RPC components.
	FUnrealRPCsByType RPCsByType = GetAllRPCsByType(TypeInfo);

	TArray<FString> ReliableMulticasts;

	for (auto Group : GetRPCTypes())
	{
		if (RPCsByType[Group].Num() == 0 && Group != RPC_Client)
		{
			continue;
		}

		Writer.PrintNewLine();

		Writer.Printf("component {0} {", *SchemaRPCComponentName(Group, Class));
		Writer.Indent();
		Writer.Printf("id = {0};", IdGenerator.GetNextAvailableId());

		ActorSchemaData.SchemaComponents[RPCTypeToSchemaComponentType(Group)] = IdGenerator.GetCurrentId();

		for (auto& RPC : RPCsByType[Group])
		{
			if (Group == ERPCType::RPC_NetMulticast)
			{
				if (RPC->bReliable)
				{
					ReliableMulticasts.Add(FString::Printf(TEXT("%s::%s"), *GetFullCPPName(Class), *RPC->Function->GetName()));
				}

				Writer.Printf("event UnrealRPCCommandRequest {0};",
					*SchemaRPCName(Class, RPC->Function));
			}
			else
			{
				Writer.Printf("command UnrealRPCCommandResponse {0}(UnrealRPCCommandRequest);",
					*SchemaRPCName(Class, RPC->Function));
			}
		}
		Writer.Outdent().Print("}");
	}

	GenerateSubobjectSchemaForActor(IdGenerator, Class, TypeInfo, SchemaPath, ActorSchemaData);

	if (ReliableMulticasts.Num() > 0)
	{
		FString AllReliableMulticasts;
		for (const FString& FunctionName : ReliableMulticasts)
		{
			AllReliableMulticasts += FunctionName + TEXT("\n");					
		}

		UE_LOG(LogTemp, Warning, TEXT("Unreal GDK currently does not support Reliable Multicast RPCs. These RPC will be treated as unreliable:\n%s"), *AllReliableMulticasts);
	}

	ClassToSchema.Add(Class, ActorSchemaData);

	Writer.WriteToFile(FString::Printf(TEXT("%s%s.schema"), *SchemaPath, *UnrealNameToSchemaTypeName(Class->GetName())));

	return IdGenerator.GetNumUsedIds();
}

FSubobjectSchemaData GenerateSubobjectSpecificSchema(FCodeWriter& Writer, FComponentIdGenerator& IdGenerator, FString PropertyName, TSharedPtr<FUnrealType>& TypeInfo, UClass* ComponentClass)
{
	FUnrealFlatRepData RepData = GetFlatRepData(TypeInfo);

	FSubobjectSchemaData SubobjectData;
	SubobjectData.Class = ComponentClass;

	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		if (RepData[Group].Num() == 0)
		{
			continue;
		}

		Writer.PrintNewLine();

		FString ComponentName = PropertyName + GetReplicatedPropertyGroupName(Group);
		Writer.Printf("component {0} {", *ComponentName);
		Writer.Indent();
		Writer.Printf("id = {0};", IdGenerator.GetNextAvailableId());
		Writer.Printf("data {0};", *SchemaReplicatedDataName(Group, ComponentClass));
		Writer.Outdent().Print("}");

		SubobjectData.SchemaComponents[PropertyGroupToSchemaComponentType(Group)] = IdGenerator.GetCurrentId();
	}

	FCmdHandlePropertyMap HandoverData = GetFlatHandoverData(TypeInfo);
	if (HandoverData.Num() > 0)
	{
		Writer.PrintNewLine();

		// Handover (server to server) replicated properties.
		Writer.Printf("component {0} {", *(PropertyName + TEXT("Handover")));
		Writer.Indent();
		Writer.Printf("id = {0};", IdGenerator.GetNextAvailableId());
		Writer.Printf("data {0};", *SchemaHandoverDataName(ComponentClass));
		Writer.Outdent().Print("}");

		SubobjectData.SchemaComponents[ESchemaComponentType::SCHEMA_Handover] = IdGenerator.GetCurrentId();
	}

	FUnrealRPCsByType RPCsByType = GetAllRPCsByType(TypeInfo);

	for (auto Group : GetRPCTypes())
	{
		if (RPCsByType[Group].Num() == 0)
		{
			continue;
		}

		Writer.PrintNewLine();

		FString ComponentName = PropertyName + GetRPCTypeName(Group) + TEXT("RPCs");
		Writer.Printf("component {0} {", *ComponentName);
		Writer.Indent();
		Writer.Printf("id = {0};", IdGenerator.GetNextAvailableId());
		for (auto& RPC : RPCsByType[Group])
		{
			if (Group == ERPCType::RPC_NetMulticast)
			{
				Writer.Printf("event UnrealRPCCommandRequest {0};",
					*SchemaRPCName(Cast<UClass>(ComponentClass), RPC->Function));
			}
			else
			{
				Writer.Printf("command UnrealRPCCommandResponse {0}(UnrealRPCCommandRequest);",
					*SchemaRPCName(Cast<UClass>(ComponentClass), RPC->Function));
			}
		}
		Writer.Outdent().Print("}");

		SubobjectData.SchemaComponents[RPCTypeToSchemaComponentType(Group)] = IdGenerator.GetCurrentId();
	}

	return SubobjectData;
}

void GenerateSubobjectSchemaForActor(FComponentIdGenerator& IdGenerator, UClass* ActorClass, TSharedPtr<FUnrealType> TypeInfo, FString SchemaPath, FSchemaData& ActorSchemaData)
{
	FCodeWriter Writer;

	Writer.Printf(R"""(
		// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
		// Note that this file has been generated automatically
		package unreal.generated.{0}.components;)""",
		*UnrealNameToSchemaTypeName(TypeInfo->Type->GetName().ToLower()));

	Writer.PrintNewLine();

	GenerateActorIncludes(Writer, TypeInfo);

	bool bHasComponents = false;
	TSet<UObject*> SeenComponents;
	int32 CurrentOffset = 1;

	for (auto& PropertyPair : TypeInfo->Properties)
	{
		UProperty* Property = PropertyPair.Key;
		UObjectProperty* ObjectProperty = Cast<UObjectProperty>(Property);

		TSharedPtr<FUnrealType>& PropertyTypeInfo = PropertyPair.Value->Type;

		if (ObjectProperty && PropertyTypeInfo.IsValid())
		{
			UObject* Value = PropertyTypeInfo->Object;

			if (Value != nullptr && !Value->IsEditorOnly())
			{
				if (!SeenComponents.Contains(Value))
				{
					SeenComponents.Add(Value);

					FSubobjectSchemaData SubobjectData;

					if (IsReplicatedSubobject(PropertyTypeInfo))
					{
						bHasComponents = true;
						SubobjectData = GenerateSubobjectSpecificSchema(Writer, IdGenerator, UnrealNameToSchemaTypeName(PropertyTypeInfo->Name), PropertyTypeInfo, Value->GetClass());
					}
					else
					{
						SubobjectData.Class = Value->GetClass();
					}

					SubobjectData.Name = PropertyTypeInfo->Name;
					ActorSchemaData.SubobjectData.Add(CurrentOffset, SubobjectData);
					ClassToSchema.Add(Value->GetClass(), FSchemaData());
				}

				CurrentOffset++;
			}
		}
	}

	if (bHasComponents)
	{
		Writer.WriteToFile(FString::Printf(TEXT("%s%sComponents.schema"), *SchemaPath, *UnrealNameToSchemaTypeName(ActorClass->GetName())));
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
				bImportCoreTypes |= PropertyTypeInfo->RPCs.Num() > 0;

				if (!AlreadyImported.Contains(Value->GetClass()))
				{
					Writer.Printf("import \"unreal/generated/ActorComponents/{0}.schema\";", *UnrealNameToSchemaTypeName(Value->GetClass()->GetName()));
					AlreadyImported.Add(Value->GetClass());
				}
			}
		}
	}

	if (bImportCoreTypes)
	{
		Writer.Printf("import \"unreal/gdk/core_types.schema\";");
	}
}
