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

FString SchemaWorkerReplicatedDataName(UStruct* Type)
{
	return FString::Printf(TEXT("Unreal%sWorkerRepData"), *Type->GetName());
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

FString RepLayoutTypeToSchemaType(ERepLayoutCmdType Type)
{
	FString DataType;
	switch (Type)
	{
	case REPCMD_DynamicArray:
		UE_LOG(LogTemp, Error, TEXT("RepLayoutTypeToSchemaType: Encountered a dynamic array REPCMD type."));
		break;
	case REPCMD_Return:
		UE_LOG(LogTemp, Error, TEXT("RepLayoutTypeToSchemaType: Encountered a return REPCMD type."));
		break;
	case REPCMD_PropertyBool:
		DataType = TEXT("bool");
		break;
	case REPCMD_PropertyInt:
		DataType = TEXT("int32");
		break;
	case REPCMD_PropertyFloat:
		DataType = TEXT("float");
		break;
	case REPCMD_PropertyByte:
		DataType = TEXT("uint32"); // uint8 not supported in schema.
		break;
	case REPCMD_PropertyString:
	case REPCMD_PropertyName:
		DataType = TEXT("string");
		break;
	case REPCMD_PropertyUInt32:
		DataType = TEXT("uint32");
		break;
	case REPCMD_PropertyRotator:
		DataType = TEXT("UnrealFRotator");
		break;
	case REPCMD_PropertyPlane:
		DataType = TEXT("UnrealFPlane");
		break;
	case REPCMD_PropertyVector:
	case REPCMD_PropertyVector100:
	case REPCMD_PropertyVectorNormal:
	case REPCMD_PropertyVector10:
	case REPCMD_PropertyVectorQ:
		DataType = TEXT("improbable.Vector3f"); // not well supported
		break;
	case REPCMD_PropertyObject:
		DataType = TEXT("UnrealObjectRef");
		break;
	case REPCMD_PropertyNetId:
	case REPCMD_Property:
		DataType = TEXT("bytes");
		break;
	case REPCMD_PropertyUInt64:
		DataType = TEXT("bytes"); // uint64 not supported in Unreal codegen.
		break;
	case REPCMD_RepMovement:
		DataType = TEXT("bytes");
		break;
	default:
		UE_LOG(LogTemp, Error, TEXT("RepLayoutTypeToSchemaType: Unhandled REPCMD Type: %d"), (int)Type);
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
	Writer.Print();

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
				*RepLayoutTypeToSchemaType(RepProp.Value->ReplicationData->RepLayoutType),
				*SchemaFieldName(RepProp.Value),
				FieldCounter,
				*GetLifetimeConditionAsString(RepProp.Value->ReplicationData->Condition)
			);
		}
		Writer.Outdent().Print("}");
	}

	// Worker-worker replicated properties.
	Writer.Printf("component %s {", *SchemaWorkerReplicatedDataName(Class));
	Writer.Indent();
	Writer.Printf("id = %d;", IdGenerator.GetNextAvailableId());

	int FieldCounter = 0;

	/*
	* The VC++ Linker has a limit of UINT16_MAX number of symbols in a DLL
	* CompleteData dramatically increases the number of symbols and aren't
	* necessarily being used, so for now we skip them.
	*/
	//for (auto& Prop : Layout.CompleteProperties)
	//{
	//	FieldCounter++;
	//	Writer.Printf("%s %s = %d;",
	//		*RepLayoutTypeToSchemaType(Prop.Type),
	//		*GetFullyQualifiedName(Prop.Chain),
	//		FieldCounter
	//	);
	//}

	Writer.Outdent().Print("}");

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
					*RepLayoutTypeToSchemaType(PropertyToRepLayoutType(Param->Property)),
					*SchemaFieldName(Param),
					FieldCounter
				);
			}
			Writer.Outdent().Print("}");
		}
	}
	Writer.Print();

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
