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
	return AlphaNumericSanitization(UnrealName);
}

FString AlphaNumericSanitization(const FString& InString)
{
	FRegexMatcher AlphaNumericPaternmatcher(AlphaNumericPatern, InString);

	FString SanitzedString;

	while (AlphaNumericPaternmatcher.FindNext())
	{
		int32 NextCharacter = AlphaNumericPaternmatcher.GetMatchBeginning();
		SanitzedString += InString[NextCharacter];
	}

	return SanitzedString;
}

FString UnrealNameToSchemaComponentName(const FString& UnrealName)
{
	FString SchemaTypeName = UnrealNameToSchemaTypeName(UnrealName);
	SchemaTypeName[0] = FChar::ToUpper(SchemaTypeName[0]);
	return SchemaTypeName;
}

FString SchemaReplicatedDataName(EReplicatedPropertyGroup Group, UStruct* Type, bool bPrependNamespace /*= false*/)
{
	return FString::Printf(TEXT("%s%s%s"), bPrependNamespace ? *GetNamespace(Type) : TEXT(""), *UnrealNameToSchemaComponentName(Type->GetName()), *GetReplicatedPropertyGroupName(Group));
}

FString SchemaHandoverDataName(UStruct* Type, bool bPrependNamespace /*= false*/)
{
	return FString::Printf(TEXT("%s%sHandover"), bPrependNamespace ? *GetNamespace(Type) : TEXT(""), *UnrealNameToSchemaComponentName(Type->GetName()));
}

FString SchemaRPCComponentName(ERPCType RpcType, UStruct* Type, bool bPrependNamespace /*= false*/)
{
	return FString::Printf(TEXT("%s%s%sRPCs"), bPrependNamespace ? *GetNamespace(Type) : TEXT(""), *UnrealNameToSchemaComponentName(Type->GetName()), *GetRPCTypeName(RpcType));
}

FString SchemaRPCName(UClass* Class, UFunction* Function)
{
	return UnrealNameToSchemaTypeName(Function->GetName().ToLower());
}

FString SchemaFieldName(const TSharedPtr<FUnrealProperty> Property)
{
	// Transform the property chain into a chain of names.
	TArray<FString> ChainNames;
	Algo::Transform(GetPropertyChain(Property), ChainNames, [](const TSharedPtr<FUnrealProperty>& Property) -> FString
	{
		FString PropName = Property->Property->GetName().ToLower();
		if (Property->Property->ArrayDim > 1)
		{
			PropName.Append(FString::FromInt(Property->StaticArrayIndex));
		}
		return UnrealNameToSchemaTypeName(PropName);
	});

	// Prefix is required to disambiguate between properties in the generated code and UActorComponent/UObject properties
	// which the generated code extends :troll:.
	FString FieldName = FString::Join(ChainNames, TEXT("_"));
	return FieldName;
}
