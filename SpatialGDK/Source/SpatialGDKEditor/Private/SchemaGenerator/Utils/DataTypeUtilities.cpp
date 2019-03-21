// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "DataTypeUtilities.h"

#include "Algo/Transform.h"
#include "Internationalization/Regex.h"

// Regex pattern matcher to match alphanumeric characters.
const FRegexPattern AlphanumericPattern(TEXT("[A-Za-z0-9]"));

FString GetNamespace(UClass* Class)
{
	return FString::Printf(TEXT("improbable::unreal::generated::%s::"), *ClassToSchemaName[Class].ToLower());
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

FString UnrealNameToSchemaName(const FString& UnrealName)
{
	return AlphanumericSanitization(UnrealName);
}

FString AlphanumericSanitization(const FString& InString)
{
	FRegexMatcher AlphanumericPatternMatcher(AlphanumericPattern, InString);

	FString SanitizedString;

	while (AlphanumericPatternMatcher.FindNext())
	{
		int32 NextCharacter = AlphanumericPatternMatcher.GetMatchBeginning();
		SanitizedString += InString[NextCharacter];
	}

	return SanitizedString;
}

FString UnrealNameToSchemaComponentName(const FString& UnrealName)
{
	FString SchemaTypeName = UnrealNameToSchemaName(UnrealName);
	if (!SchemaTypeName.IsEmpty())
	{
		SchemaTypeName[0] = FChar::ToUpper(SchemaTypeName[0]);
	}
	return SchemaTypeName;
}

FString SchemaReplicatedDataName(EReplicatedPropertyGroup Group, UClass* Class, bool bPrependNamespace /*= false*/)
{
	return FString::Printf(TEXT("%s%s%s"), bPrependNamespace ? *GetNamespace(Class) : TEXT(""), *UnrealNameToSchemaComponentName(ClassToSchemaName[Class]), *GetReplicatedPropertyGroupName(Group));
}

FString SchemaHandoverDataName(UClass* Class, bool bPrependNamespace /*= false*/)
{
	return FString::Printf(TEXT("%s%sHandover"), bPrependNamespace ? *GetNamespace(Class) : TEXT(""), *UnrealNameToSchemaComponentName(ClassToSchemaName[Class]));
}

FString SchemaRPCComponentName(ERPCType RpcType, UClass* Class, bool bPrependNamespace /*= false*/)
{
	return FString::Printf(TEXT("%s%s%sRPCs"), bPrependNamespace ? *GetNamespace(Class) : TEXT(""), *UnrealNameToSchemaComponentName(ClassToSchemaName[Class]), *GetRPCTypeName(RpcType));
}

FString SchemaRPCName(UFunction* Function)
{
	return UnrealNameToSchemaName(Function->GetName().ToLower());
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
		return UnrealNameToSchemaName(PropName);
	});

	// Prefix is required to disambiguate between properties in the generated code and UActorComponent/UObject properties
	// which the generated code extends :troll:.
	FString FieldName = FString::Join(ChainNames, TEXT("_"));
	return FieldName;
}
