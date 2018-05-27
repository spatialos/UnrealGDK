// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SchemaGenerator.h"

#include "Algo/Reverse.h"
#include "Algo/Transform.h"

#include "Utils/CodeWriter.h"
#include "Utils/ComponentIdGenerator.h"

FString UnrealNameToSchemaTypeName(const FString& UnrealName)
{
	return UnrealName.Replace(TEXT("_"), TEXT(""));
}

FString SchemaReplicatedDataName(EReplicatedPropertyGroup Group, UStruct* Type)
{
	return FString::Printf(TEXT("Unreal%s%sRepData"), *UnrealNameToSchemaTypeName(Type->GetName()), *GetReplicatedPropertyGroupName(Group));
}

FString SchemaMigratableDataName(UStruct* Type)
{
	return FString::Printf(TEXT("Unreal%sMigratableData"), *UnrealNameToSchemaTypeName(Type->GetName()));
}

FString SchemaRPCComponentName(ERPCType RpcType, UStruct* Type)
{
	return FString::Printf(TEXT("Unreal%s%sRPCs"), *UnrealNameToSchemaTypeName(Type->GetName()), *GetRPCTypeName(RpcType));
}

FString SchemaRPCRequestType(UFunction* Function)
{
	return FString::Printf(TEXT("Unreal%sRequest"), *UnrealNameToSchemaTypeName(Function->GetName()));
}

FString SchemaRPCResponseType(UFunction* Function)
{
	return FString::Printf(TEXT("Unreal%sResponse"), *UnrealNameToSchemaTypeName(Function->GetName()));
}

FString SchemaFieldName(const TSharedPtr<FUnrealProperty> Property)
{
	// Transform the property chain into a chain of names.
	TArray<FString> ChainNames;
	Algo::Transform(GetPropertyChain(Property), ChainNames, [](const TSharedPtr<FUnrealProperty>& Property) -> FString
	{
		// Note: Removing underscores to avoid naming mismatch between how schema compiler and interop generator process schema identifiers.
		return Property->Property->GetName().ToLower().Replace(TEXT("_"), TEXT(""));
	});

	// Prefix is required to disambiguate between properties in the generated code and UActorComponent/UObject properties
	// which the generated code extends :troll:.
	return TEXT("field_") + FString::Join(ChainNames, TEXT("_"));
}

FString SchemaCommandName(UClass* Class, UFunction* Function)
{
	// Prepending the name of the class to the command name enables sibling classes. 
	FString CommandName = Class->GetName() + Function->GetName();
	// Note: Removing underscores to avoid naming mismatch between how schema compiler and interop generator process schema identifiers.
	CommandName = UnrealNameToSchemaTypeName(CommandName.ToLower());
	return CommandName;
}

FString CPPCommandClassName(UClass* Class, UFunction* Function)
{
	FString SchemaName = SchemaCommandName(Class, Function);
	SchemaName[0] = FChar::ToUpper(SchemaName[0]);
	return SchemaName;
}

FString PropertyToSchemaType(UProperty* Property, bool bWithinFixedArray)
{
	FString DataType;

	if (!bWithinFixedArray && Property->ArrayDim > 1)
	{
		DataType = PropertyToSchemaType(Property, true);
		DataType = FString::Printf(TEXT("list<%s>"), *DataType);
	}
	else if (Property->IsA(UArrayProperty::StaticClass()))
	{
		DataType = PropertyToSchemaType(Cast<UArrayProperty>(Property)->Inner, false);
		DataType = FString::Printf(TEXT("list<%s>"), *DataType);
	}
	else if (Property->IsA(UStructProperty::StaticClass()))
	{
		UStructProperty* StructProp = Cast<UStructProperty>(Property);
		UScriptStruct* Struct = StructProp->Struct;
		if (Struct->GetFName() == NAME_Vector ||
			Struct->GetName() == TEXT("Vector_NetQuantize100") ||
			Struct->GetName() == TEXT("Vector_NetQuantize10") ||
			Struct->GetName() == TEXT("Vector_NetQuantizeNormal") ||
			Struct->GetName() == TEXT("Vector_NetQuantize"))
		{
			DataType = TEXT("improbable.Vector3f"); // not well supported
		}
		else if (Struct->GetFName() == NAME_Rotator)
		{
			DataType = TEXT("UnrealFRotator");
		}
		else if (Struct->GetFName() == NAME_Plane)
		{
			DataType = TEXT("UnrealFPlane");
		}
		else
		{
			DataType = TEXT("bytes"); //this includes RepMovement and UniqueNetId
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
	else if (Property->IsA(UIntProperty::StaticClass()))
	{
		DataType = TEXT("int32");
	}
	else if (Property->IsA(UByteProperty::StaticClass()))
	{
		DataType = TEXT("uint32"); // uint8 not supported in schema.
	}
	else if (Property->IsA(UNameProperty::StaticClass()) || Property->IsA(UStrProperty::StaticClass()))
	{
		DataType = TEXT("string");
	}
	else if (Property->IsA(UUInt32Property::StaticClass()))
	{
		DataType = TEXT("uint32");
	}
	else if (Property->IsA(UUInt64Property::StaticClass()))
	{
		DataType = TEXT("bytes");
	}
	else if (Property->IsA(UClassProperty::StaticClass()))
	{
		DataType = TEXT("uint32");	// Note: We hash the static class path names to UClass objects.
	}
	else if (Property->IsA(UObjectPropertyBase::StaticClass()))
	{
		DataType = TEXT("UnrealObjectRef");
	}
	else
	{
		DataType = TEXT("bytes");
	}

	return DataType;
}

int GenerateTypeBindingSchema(FCodeWriter& Writer, int ComponentId, UClass* Class, TSharedPtr<FUnrealType> TypeInfo, FString SchemaPath)
{
	FComponentIdGenerator IdGenerator(ComponentId);

	Writer.Print(R"""(
		// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
		// Note that this file has been generated automatically
		package improbable.unreal.generated;

		import "improbable/vector3.schema";
		import "improbable/unreal/gdk/core_types.schema";)""");
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
			FieldCounter++;

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

			Writer.Printf("%s %s = %d; // %s // %s",
				*PropertyToSchemaType(RepProp.Value->Property, false),
				*SchemaFieldName(RepProp.Value),
				FieldCounter,
				*GetLifetimeConditionAsString(RepProp.Value->ReplicationData->Condition),
				*PropertyPath
			);
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
		Writer.Printf("%s %s = %d;",
			*PropertyToSchemaType(Prop.Value->Property, false),
			*SchemaFieldName(Prop.Value),
			FieldCounter
		);
	}
	Writer.Outdent().Print("}");

	// RPC components.
	FUnrealRPCsByType RPCsByType = GetAllRPCsByType(TypeInfo);
	TArray<FString> RPCTypeOwners = GetRPCTypeOwners(TypeInfo);

	// Remove underscores
	for(auto& RPCTypeOwner : RPCTypeOwners)
	{
		RPCTypeOwner = UnrealNameToSchemaTypeName(RPCTypeOwner);
	}

	TMap<FString, TSharedPtr<FCodeWriter>> RPCTypeCodeWriterMap;

	for (auto& RPCTypeOwner : RPCTypeOwners)
	{
		Writer.Printf("import \"improbable/unreal/generated/Unreal%sTypes.schema\";", *RPCTypeOwner);
		TSharedPtr<FCodeWriter> RPCTypeOwnerSchemaWriter = MakeShared<FCodeWriter>();
		RPCTypeCodeWriterMap.Add(*RPCTypeOwner, RPCTypeOwnerSchemaWriter);
		RPCTypeOwnerSchemaWriter->Print(R"""(
			// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
			// Note that this file has been generated automatically
			package improbable.unreal.generated;

			import "improbable/vector3.schema";
			import "improbable/unreal/gdk/core_types.schema";)""");
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

			RPCTypeOwnerSchemaWriter->Printf("type %s {" , *TypeStr);
			RPCTypeOwnerSchemaWriter->Indent();

			// Recurse into functions properties and build a complete transitive property list.
			TArray<TSharedPtr<FUnrealProperty>> ParamList = GetFlatRPCParameters(RPC);

			// RPC target sub-object offset.
			RPCTypeOwnerSchemaWriter->Printf("uint32 target_subobject_offset = 1;");
			FieldCounter = 1;
			for (auto& Param : ParamList)
			{
				FieldCounter++;
				RPCTypeOwnerSchemaWriter->Printf("%s %s = %d;",
					*PropertyToSchemaType(Param->Property, false),
					*SchemaFieldName(Param),
					FieldCounter
				);
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
		// Generate ClientRPCs component
		Writer.Printf("component %s {", *SchemaRPCComponentName(Group, Class));
		Writer.Indent();
		Writer.Printf("id = %i;", IdGenerator.GetNextAvailableId());
		for (auto& RPC : RPCsByType[Group])
		{
			Writer.Printf("command UnrealRPCCommandResponse %s(%s);",
				*SchemaCommandName(Class, RPC->Function),
				*SchemaRPCRequestType(RPC->Function));
		}
		Writer.Outdent().Print("}");
	}

	return IdGenerator.GetNumUsedIds();
}
