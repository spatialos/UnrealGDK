// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SchemaGenerator.h"

#include "Algo/Reverse.h"

#include "Utils/CodeWriter.h"
#include "Utils/ComponentIdGenerator.h"
#include "Utils/DataTypeUtilities.h"

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

void WriteSchemaRepField(FCodeWriter& Writer, const TSharedPtr<FUnrealProperty> RepProp, const FString& PropertyPath, const int FieldCounter)
{
	Writer.Printf("%s %s = %d; // %s // %s",
		*PropertyToSchemaType(RepProp->Property, false),
		*SchemaFieldName(RepProp),
		FieldCounter,
		*GetLifetimeConditionAsString(RepProp->ReplicationData->Condition),
		*PropertyPath
	);
}

void WriteSchemaHandoverField(FCodeWriter& Writer, const TSharedPtr<FUnrealProperty> HandoverProp, const int FieldCounter)
{
	Writer.Printf("%s %s = %d;",
		*PropertyToSchemaType(HandoverProp->Property, false),
		*SchemaFieldName(HandoverProp),
		FieldCounter
	);
}

void WriteSchemaRPCField(TSharedPtr<FCodeWriter> Writer, const TSharedPtr<FUnrealProperty> RPCProp, const int FieldCounter)
{
	Writer->Printf("%s %s = %d;",
		*PropertyToSchemaType(RPCProp->Property, true),
		*SchemaFieldName(RPCProp),
		FieldCounter
	);
}

// core_types.schema should only be included if any components in the file have
// 1. An UnrealObjectRef
// 2. A list of UnrealObjectRefs
// 3. A RPC
bool ShouldIncludeCoreTypes(TSharedPtr<FUnrealType>& TypeInfo)
{
	FUnrealFlatRepData RepData = GetFlatRepData(TypeInfo);

	for (auto& PropertyGroup : RepData)
	{
		for (auto& PropertyPair : PropertyGroup.Value)
		{
			UProperty* Property = PropertyPair.Value->Property;
			if(Property->IsA(UObjectPropertyBase::StaticClass()))
			{
				return true;
			}

			if (Property->IsA(UArrayProperty::StaticClass()))
			{
				if (Cast<UArrayProperty>(Property)->Inner->IsA(UObjectPropertyBase::StaticClass()))
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

	return false;
}

int GenerateTypeBindingSchema(FCodeWriter& Writer, int ComponentId, UClass* Class, TSharedPtr<FUnrealType> TypeInfo, FString SchemaPath)
{
	FComponentIdGenerator IdGenerator(ComponentId);

	Writer.Printf(R"""(
		// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
		// Note that this file has been generated automatically
		package improbable.unreal.generated.%s;)""",
		*UnrealNameToSchemaTypeName(Class->GetName().ToLower()));

	if (ShouldIncludeCoreTypes(TypeInfo))
	{
		Writer.PrintNewLine();
		Writer.Printf("import \"improbable/unreal/gdk/core_types.schema\";");
	}

	Writer.PrintNewLine();

	FUnrealFlatRepData RepData = GetFlatRepData(TypeInfo);

	// Client-server replicated properties.
	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		Writer.Printf("component %s {", *SchemaReplicatedDataName(Group, Class));
		Writer.Indent();
		Writer.Printf("id = %d;", IdGenerator.GetNextAvailableId());
		int FieldCounter = 0;
		for (auto& RepProp : RepData[Group])
		{
			FString ParentClassName = TEXT("");

			// This loop will add the owner class of each field in the component. Meant for short-term debugging only.
			// TODO UNR-166: Delete this when InteropCodegen is in a more complete state.
			FString PropertyPath;
			TSharedPtr<FUnrealProperty> UnrealProperty = RepProp.Value;
			while (UnrealProperty->ContainerType != nullptr)
			{
				TSharedPtr<FUnrealType> ContainerType = UnrealProperty->ContainerType.Pin();
				check(ContainerType.IsValid());
				if (ContainerType->ParentProperty != nullptr)
				{
					TSharedPtr<FUnrealProperty> ParentProperty = ContainerType->ParentProperty.Pin();
					if (ParentProperty.IsValid())
					{
						PropertyPath += FString::Printf(TEXT("%s::%s"), *ContainerType->Type->GetName(), *ParentProperty->Property->GetName());
					}
					UnrealProperty = ParentProperty;
				}
				else
				{
					break;
				}
			}

			if (UObject* ObjOuter = UnrealProperty->Property->GetOuter())
			{
				PropertyPath += FString::Printf(TEXT("::%s"), *ObjOuter->GetName());
			}

			FieldCounter++;
			WriteSchemaRepField(Writer,
				RepProp.Value,
				PropertyPath,
				RepProp.Value->ReplicationData->Handle);
		}
		Writer.Outdent().Print("}");
	}

	// Handover (server to server) replicated properties.
	Writer.Printf("component %s {", *SchemaHandoverDataName(Class));
	Writer.Indent();
	Writer.Printf("id = %d;", IdGenerator.GetNextAvailableId());
	int FieldCounter = 0;
	for (auto& Prop : GetFlatHandoverData(TypeInfo))
	{
		FieldCounter++;
		WriteSchemaHandoverField(Writer,
			Prop.Value,
			FieldCounter);
	}
	Writer.Outdent().Print("}");

	// RPC components.
	FUnrealRPCsByType RPCsByType = GetAllRPCsByType(TypeInfo);

	TArray<FString> ReliableMulticasts;

	for (auto Group : GetRPCTypes())
	{
		Writer.Printf("component %s {", *SchemaRPCComponentName(Group, Class));
		Writer.Indent();
		Writer.Printf("id = %i;", IdGenerator.GetNextAvailableId());
		for (auto& RPC : RPCsByType[Group])
		{
			if (Group == ERPCType::RPC_NetMulticast)
			{
				if (RPC->bReliable)
				{
					ReliableMulticasts.Add(FString::Printf(TEXT("%s::%s"), *GetFullCPPName(Class), *RPC->Function->GetName()));
				}

				Writer.Printf("event UnrealRPCCommandRequest %s;",
					*SchemaRPCName(Class, RPC->Function));
			}
			else
			{
				Writer.Printf("command UnrealRPCCommandResponse %s(UnrealRPCCommandRequest);",
					*SchemaRPCName(Class, RPC->Function));
			}
		}
		Writer.Outdent().Print("}");
	}

	if (ReliableMulticasts.Num() > 0)
	{
		FString AllReliableMulticasts;
		for (const FString& FunctionName : ReliableMulticasts)
		{
			AllReliableMulticasts += FunctionName + TEXT("\n");					
		}

		UE_LOG(LogTemp, Warning, TEXT("Unreal GDK currently does not support Reliable Multicast RPCs. These RPC will be treated as unreliable:\n%s"), *AllReliableMulticasts);
	}

	return IdGenerator.GetNumUsedIds();
}
