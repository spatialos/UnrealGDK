// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "DataTypeUtilities.h"

#include "Algo/Transform.h"
#include "Internationalization/Regex.h"

#include "SpatialGDKEditorSchemaGenerator.h"
#include "Utils/GDKPropertyMacros.h"

// Regex pattern matcher to match alphanumeric characters.
const FRegexPattern AlphanumericPattern(TEXT("[A-Za-z0-9]"));

FString GetEnumDataType(const GDK_PROPERTY(EnumProperty) * EnumProperty)
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

FString PropertyToSchemaType(const GDK_PROPERTY(Property) * Property)
{
	FString DataType;

	if (Property->IsA(GDK_PROPERTY(StructProperty)::StaticClass()))
	{
		const GDK_PROPERTY(StructProperty)* StructProp = GDK_CASTFIELD<GDK_PROPERTY(StructProperty)>(Property);
		UScriptStruct* Struct = StructProp->Struct;
		DataType = TEXT("bytes");
	}
	else if (Property->IsA(GDK_PROPERTY(BoolProperty)::StaticClass()))
	{
		DataType = TEXT("bool");
	}
	else if (Property->IsA(GDK_PROPERTY(FloatProperty)::StaticClass()))
	{
		DataType = TEXT("float");
	}
	else if (Property->IsA(GDK_PROPERTY(DoubleProperty)::StaticClass()))
	{
		DataType = TEXT("double");
	}
	else if (Property->IsA(GDK_PROPERTY(Int8Property)::StaticClass()))
	{
		DataType = TEXT("int32");
	}
	else if (Property->IsA(GDK_PROPERTY(Int16Property)::StaticClass()))
	{
		DataType = TEXT("int32");
	}
	else if (Property->IsA(GDK_PROPERTY(IntProperty)::StaticClass()))
	{
		DataType = TEXT("int32");
	}
	else if (Property->IsA(GDK_PROPERTY(Int64Property)::StaticClass()))
	{
		DataType = TEXT("int64");
	}
	else if (Property->IsA(GDK_PROPERTY(ByteProperty)::StaticClass()))
	{
		DataType = TEXT("uint32"); // uint8 not supported in schema.
	}
	else if (Property->IsA(GDK_PROPERTY(UInt16Property)::StaticClass()))
	{
		DataType = TEXT("uint32");
	}
	else if (Property->IsA(GDK_PROPERTY(UInt32Property)::StaticClass()))
	{
		DataType = TEXT("uint32");
	}
	else if (Property->IsA(GDK_PROPERTY(UInt64Property)::StaticClass()))
	{
		DataType = TEXT("uint64");
	}
	else if (Property->IsA(GDK_PROPERTY(NameProperty)::StaticClass()) || Property->IsA(GDK_PROPERTY(StrProperty)::StaticClass())
			 || Property->IsA(GDK_PROPERTY(TextProperty)::StaticClass()))
	{
		DataType = TEXT("string");
	}
	else if (Property->IsA(GDK_PROPERTY(ObjectPropertyBase)::StaticClass()))
	{
		DataType = TEXT("UnrealObjectRef");
	}
	else if (Property->IsA(GDK_PROPERTY(ArrayProperty)::StaticClass()))
	{
		DataType = PropertyToSchemaType(GDK_CASTFIELD<GDK_PROPERTY(ArrayProperty)>(Property)->Inner);
		DataType = FString::Printf(TEXT("list<%s>"), *DataType);
	}
	else if (Property->IsA(GDK_PROPERTY(EnumProperty)::StaticClass()))
	{
		DataType = GetEnumDataType(GDK_CASTFIELD<GDK_PROPERTY(EnumProperty)>(Property));
	}
	else
	{
		DataType = TEXT("bytes");
	}

	return DataType;
}

FString UnrealNameToSchemaName(const FString& UnrealName, bool bWarnAboutRename /* = false */)
{
	FString Sanitized = AlphanumericSanitization(UnrealName);
	if (Sanitized.IsValidIndex(0) && FChar::IsDigit(Sanitized[0]))
	{
		FString Result = TEXT("ZZ") + Sanitized;
		if (bWarnAboutRename)
		{
			UE_LOG(LogSpatialGDKSchemaGenerator, Warning,
				   TEXT("%s starts with a digit (potentially after removing non-alphanumeric characters), so its schema name was changed "
						"to %s instead. To remove this warning, rename your asset."),
				   *UnrealName, *Result);
		}
		return Result;
	}
	return Sanitized;
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

FString SchemaReplicatedDataName(TMap<FString, FString> const& ClassPathToSchemaName, EReplicatedPropertyGroup Group,
								 FString const& ClassPath)
{
	return FString::Printf(TEXT("%s%s"), *UnrealNameToSchemaComponentName(ClassPathToSchemaName[ClassPath]),
						   *GetReplicatedPropertyGroupName(Group));
}

FString SchemaHandoverDataName(TMap<FString, FString> const& ClassPathToSchemaName, FString const& ClassPath)
{
	return FString::Printf(TEXT("%sHandover"), *UnrealNameToSchemaComponentName(ClassPathToSchemaName[ClassPath]));
}

FString SchemaFieldName(const TSharedPtr<FUnrealOfflineProperty> Property)
{
	// Transform the property chain into a chain of names.
	TArray<FString> ChainNames;
	Algo::Transform(GetPropertyChain(Property), ChainNames, [](const TSharedPtr<FUnrealOfflineProperty>& Property) -> FString {
		FString PropName = Property->PropertyName.ToLower();
		if (Property->ArrayDim > 1)
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
