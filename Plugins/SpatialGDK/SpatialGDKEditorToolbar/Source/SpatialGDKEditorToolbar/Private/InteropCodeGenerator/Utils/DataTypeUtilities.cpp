// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "DataTypeUtilities.h"

#include "Algo/Transform.h"

FString GetNamespace(UStruct* Struct)
{
	return FString::Printf(TEXT("improbable::unreal::generated::%s::"), *UnrealNameToSchemaTypeName(Struct->GetName().ToLower()));
}

FString GetEnumDataType(const UEnumProperty* EnumProperty)
{
	FString DataType;

	if (EnumProperty->ElementSize < 4)
	{
		// schema types don't include support for 8 or 16 bit data types
		DataType = TEXT("uint32");
	}
	else
	{
		DataType = EnumProperty->GetUnderlyingProperty()->GetCPPType();
	}

	return DataType;
}

FString UnrealNameToSchemaTypeName(const FString& UnrealName)
{
	// Note: Removing underscores to avoid naming mismatch between how schema compiler and interop generator process schema identifiers.
	return UnrealName.Replace(TEXT("_"), TEXT(""));
}

FString SchemaReplicatedDataName(EReplicatedPropertyGroup Group, UStruct* Type, bool bPrependNamespace /*= false*/)
{
	return FString::Printf(TEXT("%s%s%sRepData"), bPrependNamespace ? *GetNamespace(Type) : TEXT(""), *UnrealNameToSchemaTypeName(Type->GetName()), *GetReplicatedPropertyGroupName(Group));
}

FString SchemaMigratableDataName(UStruct* Type, bool bPrependNamespace /*= false*/)
{
	return FString::Printf(TEXT("%s%sMigratableData"), bPrependNamespace ? *GetNamespace(Type) : TEXT(""), *UnrealNameToSchemaTypeName(Type->GetName()));
}

FString SchemaRPCComponentName(ERPCType RpcType, UStruct* Type, bool bPrependNamespace /*= false*/)
{
	return FString::Printf(TEXT("%s%s%sRPCs"), bPrependNamespace ? *GetNamespace(Type) : TEXT(""), *UnrealNameToSchemaTypeName(Type->GetName()), *GetRPCTypeName(RpcType));
}

FString SchemaRPCRequestType(UFunction* Function, bool bPrependNamespace /*= false*/)
{
	return FString::Printf(TEXT("%s%sRequest"), bPrependNamespace ? *GetNamespace(Function->GetOwnerClass()) : TEXT(""), *UnrealNameToSchemaTypeName(Function->GetName()));
}

FString SchemaRPCResponseType(UFunction* Function)
{
	return FString::Printf(TEXT("%sResponse"), *UnrealNameToSchemaTypeName(Function->GetName()));
}

FString SchemaRPCName(UClass* Class, UFunction* Function)
{
	return UnrealNameToSchemaTypeName(Function->GetName().ToLower());
}

FString CPPCommandClassName(UClass* Class, UFunction* Function)
{
	FString SchemaName = SchemaRPCName(Class, Function);
	SchemaName[0] = FChar::ToUpper(SchemaName[0]);
	return SchemaName;
}

FString SchemaFieldName(const TSharedPtr<FUnrealProperty> Property)
{
	// Transform the property chain into a chain of names.
	TArray<FString> ChainNames;
	Algo::Transform(GetPropertyChain(Property), ChainNames, [](const TSharedPtr<FUnrealProperty>& Property) -> FString
	{
		FString PropName = Property->Property->GetName().ToLower();
		if (Property->StaticArrayIndex >= 0)
		{
			PropName.Append(FString::FromInt(Property->StaticArrayIndex));
		}
		return UnrealNameToSchemaTypeName(PropName);
	});

	// Prefix is required to disambiguate between properties in the generated code and UActorComponent/UObject properties
	// which the generated code extends :troll:.
	FString FieldName = TEXT("field_") + FString::Join(ChainNames, TEXT("_"));
	return FieldName;
}
