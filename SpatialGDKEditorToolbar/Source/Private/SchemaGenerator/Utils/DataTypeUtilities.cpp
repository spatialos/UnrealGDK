// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "DataTypeUtilities.h"

#include "Algo/Transform.h"

// Regex pattern matcher to match alphanumeric characters.
const FRegexPattern AlphanumericPattern(TEXT("[A-Z,a-z,0-9]"));

FString GetNamespace(UStruct* Struct)
{
	return FString::Printf(TEXT("improbable::unreal::generated::%s::"), *UnrealNameToSchemaName(Struct->GetName().ToLower()));
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
	return NonAlphanumericAsciiCharacterConverter(UnrealName);
}

FString UnrealClassNameToSchemaName(const FString& UnrealName)
{
	// Class names cannot have underscores so remove them.
	// This is a limitation from schema where component names do not support underscores. We use the class name as part of the components name.
	FString SchemaClassName = UnrealName.Replace(TEXT("_"), TEXT("")).Replace(TEXT(" "), TEXT(""));
	return NonAlphanumericAsciiCharacterConverter(SchemaClassName);
}

FString NonAlphanumericAsciiCharacterConverter(const FString& InString)
{
	FString ConvertedString;

	const int AsciiZero = int('0');
	const int AsciiNine = int('9');

	const int AsciiUpperA = int('A');
	const int AsciiUpperZ = int('Z');

	const int AsciiLowerA = int('a');
	const int AsciiLowerZ = int('z');

	for (auto& Char : InString)
	{
		int CharAscii = int(Char);

		// If we have an alphanumeric character then use it.
		if((CharAscii >= AsciiZero && CharAscii <= AsciiNine)
		|| (CharAscii >= AsciiUpperA && CharAscii <= AsciiUpperZ)
		|| (CharAscii >= AsciiLowerA && CharAscii <= AsciiLowerZ))
		{
			ConvertedString += Char;
		}
		else
		{
			// If this is a non-alphanumeric character then use the string converted ASCII code.
			ConvertedString += ConvertAsciiCodeToHexString(CharAscii);
		}
	}

	return ConvertedString;
}

FString ConvertAsciiCodeToHexString(int ASCIICode)
{
	return FString::Printf(TEXT("0x%x"), ASCIICode);
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
	FString SchemaTypeName = UnrealClassNameToSchemaName(UnrealName);
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
