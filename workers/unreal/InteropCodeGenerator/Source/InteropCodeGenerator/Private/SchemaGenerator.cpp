// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SchemaGenerator.h"

#include "Algo/Reverse.h"
#include "Algo/Transform.h"

#include "Utils/CodeWriter.h"
#include "Utils/ComponentIdGenerator.h"

FString SchemaReplicatedDataName(EReplicatedPropertyGroup Group, UStruct* Type)
{
	return FString::Printf(TEXT("Unreal%s%sRepData"), *Type->GetName(), *GetReplicatedPropertyGroupName(Group));
}

FString SchemaMigratableDataName(UStruct* Type)
{
	return FString::Printf(TEXT("Unreal%sMigratableData"), *Type->GetName());
}

FString SchemaRPCComponentName(ERPCType RpcType, UStruct* Type)
{
	return FString::Printf(TEXT("Unreal%s%sRPCs"), *Type->GetName(), *GetRPCTypeName(RpcType));
}

FString SchemaRPCRequestType(UFunction* Function)
{
	return FString::Printf(TEXT("Unreal%sRequest"), *Function->GetName());
}

FString SchemaRPCResponseType(UFunction* Function)
{
	return FString::Printf(TEXT("Unreal%sResponse"), *Function->GetName());
}

FString SchemaFieldName(const TSharedPtr<FUnrealProperty> Property)
{
	// Transform the property chain into a chain of names.
	TArray<FString> ChainNames;
	Algo::Transform(GetPropertyChain(Property), ChainNames, [](const TSharedPtr<FUnrealProperty>& Property) -> FString
	{
		return Property->Property->GetName().ToLower();
	});

	// Prefix is required to disambiguate between properties in the generated code and UActorComponent/UObject properties
	// which the generated code extends :troll:.
	return TEXT("field_") + FString::Join(ChainNames, TEXT("_"));
}

FString SchemaCommandName(UFunction* Function)
{
	return Function->GetName().ToLower();
}

FString CPPCommandClassName(UFunction* Function)
{
	FString SchemaName = SchemaCommandName(Function);
	SchemaName[0] = FChar::ToUpper(SchemaName[0]);
	return SchemaName;
}

FString PropertyToSchemaType(UProperty* Property)
{
	FString DataType;

	if (Property->IsA(UStructProperty::StaticClass()))
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
	else if (Property->IsA(UObjectPropertyBase::StaticClass()))
	{
		DataType = TEXT("UnrealObjectRef");
	}
	else if (Property->IsA(UArrayProperty::StaticClass()))
	{
		DataType = PropertyToSchemaType(Cast<UArrayProperty>(Property)->Inner);
		DataType = FString::Printf(TEXT("list<%s>"), *DataType);
	}
	else
	{
		DataType = TEXT("bytes");
	}

	return DataType;
}

int GenerateTypeBindingSchema(FCodeWriter& Writer, int ComponentId, UClass* Class, TSharedPtr<FUnrealType> TypeInfo)
{
	FComponentIdGenerator IdGenerator(ComponentId);

	Writer.Print(R"""(
		// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
		// Note that this file has been generated automatically
		package improbable.unreal;

		import "improbable/vector3.schema";
		import "improbable/unreal/core_types.schema";)""");
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
			Writer.Printf("%s %s = %d; // %s",
				*PropertyToSchemaType(RepProp.Value->Property),
				*SchemaFieldName(RepProp.Value),
				FieldCounter,
				*GetLifetimeConditionAsString(RepProp.Value->ReplicationData->Condition)
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
			*RepLayoutTypeToSchemaType(Prop.Value->MigratableData->RepLayoutType),
			*SchemaFieldName(Prop.Value),
			FieldCounter
		);
	}
	Writer.Outdent().Print("}");

	// RPC components.
	FUnrealRPCsByType RPCsByType = GetAllRPCsByType(TypeInfo);
	for (auto Group : GetRPCTypes())
	{
		// Generate schema RPC command types
		for (auto& RPC : RPCsByType[Group])
		{
			FString TypeStr = SchemaRPCRequestType(RPC->Function);

			Writer.Printf("type %s {", *TypeStr);
			Writer.Indent();

			// Recurse into functions properties and build a complete transitive property list.
			TArray<TSharedPtr<FUnrealProperty>> ParamList = GetFlatRPCParameters(RPC);

			// RPC target subobject offset.
			Writer.Printf("uint32 target_subobject_offset = 1;");
			FieldCounter = 1;
			for (auto& Param : ParamList)
			{
				FieldCounter++;
				Writer.Printf("%s %s = %d;",
					*PropertyToSchemaType(Param->Property),
					*SchemaFieldName(Param),
					FieldCounter
				);
			}
			Writer.Outdent().Print("}");
		}
	}
	Writer.PrintNewLine();

	for (auto Group : GetRPCTypes())
	{
		// Generate ClientRPCs component
		Writer.Printf("component %s {", *SchemaRPCComponentName(Group, Class));
		Writer.Indent();
		Writer.Printf("id = %i;", IdGenerator.GetNextAvailableId());
		for (auto& RPC : RPCsByType[Group])
		{
			Writer.Printf("command UnrealRPCCommandResponse %s(%s);",
				*SchemaCommandName(RPC->Function),
				*SchemaRPCRequestType(RPC->Function));
		}
		Writer.Outdent().Print("}");
	}

	return IdGenerator.GetNumUsedIds();
}
