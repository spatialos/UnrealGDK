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
	else if (Property->IsA(UNameProperty::StaticClass()) || Property->IsA(UStrProperty::StaticClass()))
	{
		DataType = TEXT("string");
	}
	else if (Property->IsA(UClassProperty::StaticClass()))
	{
		DataType = TEXT("uint32");	// Note: We hash the static class path names to UClass objects.
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

void WriteSchemaMigratableField(FCodeWriter& Writer, const TSharedPtr<FUnrealProperty> MigratableProp, const int FieldCounter)
{
	Writer.Printf("%s %s = %d;",
		*PropertyToSchemaType(MigratableProp->Property, false),
		*SchemaFieldName(MigratableProp),
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

int GenerateTypeBindingSchema(FCodeWriter& Writer, int ComponentId, UClass* Class, TSharedPtr<FUnrealType> TypeInfo, FString SchemaPath)
{
	FComponentIdGenerator IdGenerator(ComponentId);

	Writer.Printf(R"""(
		// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
		// Note that this file has been generated automatically
		package improbable.unreal.generated.%s;

		import "improbable/unreal/gdk/core_types.schema";)""", *UnrealNameToSchemaTypeName(Class->GetName().ToLower()));
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
				FieldCounter);
		}
		Writer.Outdent().Print("}");
	}

	// Worker-worker replicated properties.
	Writer.Printf("component %s {", *SchemaMigratableDataName(Class));
	Writer.Indent();
	Writer.Printf("id = %d;", IdGenerator.GetNextAvailableId());
	int FieldCounter = 0;
	for (auto& Prop : GetFlatMigratableData(TypeInfo))
	{
		FieldCounter++;
		WriteSchemaMigratableField(Writer,
			Prop.Value,
			FieldCounter);
	}
	Writer.Outdent().Print("}");

	// RPC components.
	FUnrealRPCsByType RPCsByType = GetAllRPCsByType(TypeInfo);
	TArray<FString> RPCTypeOwners = GetRPCTypeOwners(TypeInfo);

	// Remove underscores
	for (auto& RPCTypeOwner : RPCTypeOwners)
	{
		RPCTypeOwner = UnrealNameToSchemaTypeName(RPCTypeOwner);
	}

	TMap<FString, TSharedPtr<FCodeWriter>> RPCTypeCodeWriterMap;

	for (auto& RPCTypeOwner : RPCTypeOwners)
	{
		Writer.Printf("import \"improbable/unreal/generated/Unreal%sTypes.schema\";", *RPCTypeOwner);
		TSharedPtr<FCodeWriter> RPCTypeOwnerSchemaWriter = MakeShared<FCodeWriter>();
		RPCTypeCodeWriterMap.Add(*RPCTypeOwner, RPCTypeOwnerSchemaWriter);
		RPCTypeOwnerSchemaWriter->Printf(R"""(
			// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
			// Note that this file has been generated automatically
			package improbable.unreal.generated.%s;

			import "improbable/unreal/gdk/core_types.schema";)""", *RPCTypeOwner.ToLower());
		RPCTypeOwnerSchemaWriter->PrintNewLine();
	}
	Writer.PrintNewLine();

	for (auto Group : GetRPCTypes())
	{
		// Generate schema RPC command types
		for (auto& RPC : RPCsByType[Group])
		{
			FString TypeStr = SchemaRPCRequestType(RPC->Function);

			// Get the correct code writer for this RPC.
			FString RPCOwnerName = UnrealNameToSchemaTypeName(*RPC->Function->GetOuter()->GetName());
			TSharedPtr<FCodeWriter> RPCTypeOwnerSchemaWriter = RPCTypeCodeWriterMap[*RPCOwnerName];

			RPCTypeOwnerSchemaWriter->Printf("type %s {", *TypeStr);
			RPCTypeOwnerSchemaWriter->Indent();

			// Recurse into functions properties and build a complete transitive property list.
			TArray<TSharedPtr<FUnrealProperty>> ParamList = GetFlatRPCParameters(RPC);

			// RPC target sub-object offset.
			RPCTypeOwnerSchemaWriter->Printf("uint32 target_subobject_offset = 1;");
			FieldCounter = 1;

			for (auto& Param : ParamList)
			{
				FieldCounter++;
				WriteSchemaRPCField(RPCTypeOwnerSchemaWriter,
					Param,
					FieldCounter);
			}
			RPCTypeOwnerSchemaWriter->Outdent().Print("}");
		}
	}

	// Save RPC type owner schema files to disk.
	for (auto& RPCTypeOwner : RPCTypeOwners)
	{
		TSharedPtr<FCodeWriter> RPCTypeOwnerSchemaWriter = RPCTypeCodeWriterMap[*RPCTypeOwner];
		FString RPCTypeOwnerSchemaFilename = FString::Printf(TEXT("Unreal%sTypes"), *RPCTypeOwner);
		RPCTypeOwnerSchemaWriter->WriteToFile(FString::Printf(TEXT("%s%s.schema"), *SchemaPath, *RPCTypeOwnerSchemaFilename));
	}

	for (auto Group : GetRPCTypes())
	{
		Writer.Printf("component %s {", *SchemaRPCComponentName(Group, Class));
		Writer.Indent();
		Writer.Printf("id = %i;", IdGenerator.GetNextAvailableId());
		for (auto& RPC : RPCsByType[Group])
		{
			if (Group == ERPCType::RPC_NetMulticast)
			{
				checkf(RPC->bReliable == false, TEXT("%s: Unreal GDK currently does not support Reliable Multicast RPCs"), *RPC->Function->GetName());

				Writer.Printf("event %s.%s %s;",
					*UnrealNameToSchemaTypeName(*RPC->Function->GetOuter()->GetName()).ToLower(),
					*SchemaRPCRequestType(RPC->Function),
					*SchemaRPCName(Class, RPC->Function));
			}
			else
			{
				Writer.Printf("command UnrealRPCCommandResponse %s(%s.%s);",
					*SchemaRPCName(Class, RPC->Function),
					*UnrealNameToSchemaTypeName(*RPC->Function->GetOuter()->GetName()).ToLower(),
					*SchemaRPCRequestType(RPC->Function));
			}
		}
		Writer.Outdent().Print("}");
	}

	return IdGenerator.GetNumUsedIds();
}
