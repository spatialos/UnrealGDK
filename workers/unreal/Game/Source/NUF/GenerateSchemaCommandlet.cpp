#include "GenerateSchemaCommandlet.h"

#include "Utils/CodeWriter.h"

// For GenerateSchemaFromClass
#include "Net/DataReplication.h"
#include "GameFramework/Character.h"
#include "Misc/FileHelper.h"
#include "Components/ArrowComponent.h"

// Hack to access private members of FRepLayout.
#define private public
#include "Net/RepLayout.h"
#undef private

namespace
{
struct FPropertyInfo
{
	UProperty* Property;
	ERepLayoutCmdType Type;
	// Properties that were traversed to reach this property, including the property itself.
	TArray<UProperty*> Chain;

	bool operator==(const FPropertyInfo& Other) const
	{
		return Property == Other.Property && Type == Other.Type && Chain == Other.Chain;
	}
};

struct FRepLayoutEntry
{
	UProperty* Property;
	UProperty* Parent;
	TArray<UProperty*> Chain;
	ELifetimeCondition Condition;
	ERepLayoutCmdType Type;
	int32 Handle;
	int32 Offset;
};

struct FReplicatedPropertyInfo
{
	FRepLayoutEntry Entry;
	// Usually a singleton list containing the PropertyInfo this RepLayoutEntry refers to.
	// In some cases (such as a struct within a struct), this can refer to many properties in schema.
	TArray<FPropertyInfo> PropertyList;
};

enum EReplicatedPropertyGroup
{
	REP_SingleClient,
	REP_MultiClient
};

struct FPropertyLayout
{
	TMap<EReplicatedPropertyGroup, TArray<FReplicatedPropertyInfo>> ReplicatedProperties;
	TArray<FPropertyInfo> CompleteProperties;
};

FString GetLifetimeConditionAsString(ELifetimeCondition Condition)
{
	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("ELifetimeCondition"), true);
	if (!EnumPtr)
	{
		return FString("Invalid");
	}
	return EnumPtr->GetNameByValue((int64)Condition).ToString();
}

FString GetReplicatedPropertyGroupName(EReplicatedPropertyGroup Group)
{
	return Group == REP_SingleClient ? TEXT("SingleClient") : TEXT("MultiClient");
}

FString PropertySchemaName(UProperty* Property)
{
	FString FullPath = Property->GetFullGroupName(false);
	FullPath.ReplaceInline(TEXT("."), TEXT("_"));
	FullPath.ReplaceInline(SUBOBJECT_DELIMITER, TEXT("_"));
	FullPath.ToLowerInline();
	return FullPath;
}

FString PropertyGeneratedName(UProperty* Property)
{
	FString SchemaName = PropertySchemaName(Property);
	SchemaName[0] = FChar::ToUpper(SchemaName[0]);
	return SchemaName;
}

FString PropertyGeneratedName(const FString& SchemaName)
{
	TArray<FString> ScopeNames;
	SchemaName.ParseIntoArray(ScopeNames, TEXT("_"), false);
	FString GeneratedName;
	for (auto Scope : ScopeNames)
	{
		Scope[0] = FChar::ToUpper(Scope[0]);
		GeneratedName += Scope;
	}
	return GeneratedName;
}

ERepLayoutCmdType PropertyToRepLayoutType(UProperty* Property)
{
	UProperty * UnderlyingProperty = Property;
	if (UEnumProperty * EnumProperty = Cast< UEnumProperty >(Property))
	{
		UnderlyingProperty = EnumProperty->GetUnderlyingProperty();
	}

	// Try to special case to custom types we know about
	if (UnderlyingProperty->IsA(UStructProperty::StaticClass()))
	{
		UStructProperty * StructProp = Cast< UStructProperty >(UnderlyingProperty);
		UScriptStruct * Struct = StructProp->Struct;
		if (Struct->GetFName() == NAME_Vector)
		{
			return REPCMD_PropertyVector;
		}
		else if (Struct->GetFName() == NAME_Rotator)
		{
			return REPCMD_PropertyRotator;
		}
		else if (Struct->GetFName() == NAME_Plane)
		{
			return  REPCMD_PropertyPlane;
		}
		else if (Struct->GetName() == TEXT("Vector_NetQuantize100"))
		{
			return REPCMD_PropertyVector100;
		}
		else if (Struct->GetName() == TEXT("Vector_NetQuantize10"))
		{
			return REPCMD_PropertyVector10;
		}
		else if (Struct->GetName() == TEXT("Vector_NetQuantizeNormal"))
		{
			return REPCMD_PropertyVectorNormal;
		}
		else if (Struct->GetName() == TEXT("Vector_NetQuantize"))
		{
			return REPCMD_PropertyVectorQ;
		}
		else if (Struct->GetName() == TEXT("UniqueNetIdRepl"))
		{
			return REPCMD_PropertyNetId;
		}
		else if (Struct->GetName() == TEXT("RepMovement"))
		{
			return REPCMD_RepMovement;
		}
		else
		{
			return REPCMD_Property;
		}
	}
	else if (UnderlyingProperty->IsA(UBoolProperty::StaticClass()))
	{
		return REPCMD_PropertyBool;
	}
	else if (UnderlyingProperty->IsA(UFloatProperty::StaticClass()))
	{
		return REPCMD_PropertyFloat;
	}
	else if (UnderlyingProperty->IsA(UIntProperty::StaticClass()))
	{
		return REPCMD_PropertyInt;
	}
	else if (UnderlyingProperty->IsA(UByteProperty::StaticClass()))
	{
		return REPCMD_PropertyByte;
	}
	else if (UnderlyingProperty->IsA(UObjectPropertyBase::StaticClass()))
	{
		return REPCMD_PropertyObject;
	}
	else if (UnderlyingProperty->IsA(UNameProperty::StaticClass()))
	{
		return REPCMD_PropertyName;
	}
	else if (UnderlyingProperty->IsA(UUInt32Property::StaticClass()))
	{
		return REPCMD_PropertyUInt32;
	}
	else if (UnderlyingProperty->IsA(UUInt64Property::StaticClass()))
	{
		return REPCMD_PropertyUInt64;
	}
	else if (UnderlyingProperty->IsA(UStrProperty::StaticClass()))
	{
		return REPCMD_PropertyString;
	}
	else
	{
		return REPCMD_Property;
	}
}

FString RepLayoutTypeToSchemaType(ERepLayoutCmdType Type)
{
	FString DataType;
	switch (Type)
	{
	case REPCMD_DynamicArray:
	case REPCMD_Return:
		UE_LOG(LogTemp, Warning, TEXT("ENCOUNTERED A DYNAMIC ARRAY, SKIPPING"));
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
		UE_LOG(LogTemp, Warning, TEXT("UNHANDLED REPCMD Type"));
	}
	return DataType;
}

FString GetSchemaReplicatedDataName(EReplicatedPropertyGroup Group, UStruct* Type)
{
	return FString::Printf(TEXT("Unreal%s%sReplicatedData"), *Type->GetName(), *GetReplicatedPropertyGroupName(Group));
}

FString GetSchemaCompleteDataName(UStruct* Type)
{
	return FString::Printf(TEXT("Unreal%sCompleteData"), *Type->GetName());
}

FString GetFullyQualifiedName(TArray<UProperty*> Chain)
{
	TArray<FString> ChainNames;
	for (auto Prop : Chain)
	{
		ChainNames.Add(Prop->GetName().ToLower());
	}
	// Prefix is required to disambiguate between properties in the generated code and UActorComponent/UObject properties
	// which the generated code extends :troll:.
	return TEXT("field_") + FString::Join(ChainNames, TEXT("_"));
}

FString GetUnrealFieldLvalue(TArray<UProperty*> Chain)
{
	TArray<FString> ChainNames;
	for (auto Prop : Chain)
	{
		ChainNames.Add(Prop->GetName());
	}
	return TEXT("this->") + FString::Join(ChainNames, TEXT("->"));
}

// CDO - Class default object which contains Property
void VisitProperty(TArray<FPropertyInfo>& PropertyInfo, UObject* CDO, TArray<UProperty*> Stack, UProperty* Property)
{
	// Skip properties that make no sense to store in SpatialOS.
	if (Property->IsA<UMulticastDelegateProperty>())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s - multicast delegate, skipping"), *Property->GetName());
		return;
	}
	if (Property->GetPropertyFlags() & CPF_Transient && !(Property->GetPropertyFlags() & CPF_Net))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s - transient and not replicated, skipping"), *Property->GetName());
		return;
	}

	// Get property type.
	auto PropertyType = PropertyToRepLayoutType(Property);

	// If this property is a struct or object, we need to recurse into its properties.
	//
	// Usually, struct properties map directly to built in types like FString, or FPlane. However, custom structs map
	// directly to REPCMD_Property, so we need to make sure that it's a struct.
	if ((PropertyType == REPCMD_Property && Property->IsA<UStructProperty>()) ||
		(PropertyType == REPCMD_PropertyObject && Property->IsA<UObjectProperty>()))
	{
		// Get struct/class of property value.
		UStruct* PropertyValueStruct;
		if (PropertyType != REPCMD_PropertyObject)
		{ // UStruct property
			PropertyValueStruct = Cast<UStructProperty>(Property)->Struct;
		}
		else
		{ // UObject property
			UObjectProperty* ObjectProperty = Cast<UObjectProperty>(Property);
			UClass* PropertyValueClass = nullptr;

			// Obtain the class of the UObject property value.
			if (CDO)
			{
				// If we have the CDO, resolve the actual type pointed to by the pointer (to deal with polymorphism).
				UObject* Value = ObjectProperty->GetPropertyValue_InContainer(CDO);
				if (Value)
				{
					// If this is an editor-only property, skip it.
					if (Value->IsEditorOnly())
					{
						UE_LOG(LogTemp, Warning, TEXT("%s - editor only, skipping"), *Property->GetName());
						return;
					}

					// Make sure the owner of the property value is the CDO, otherwise this is a weak reference.
					if (Value->GetOuter() == CDO)
					{
						UE_LOG(LogTemp, Warning, TEXT("Property Class: %s Instance Class: %s"), *ObjectProperty->PropertyClass->GetName(), *Value->GetClass()->GetName());
						PropertyValueClass = Value->GetClass();
					}
					else
					{
						// The values outer is not us, store as weak reference.
						UE_LOG(LogTemp, Warning, TEXT("%s - %s weak reference (outer not this)"), *Property->GetName(), *ObjectProperty->PropertyClass->GetName());
					}
				}
				else
				{
					// If value is not set, then we clearly don't own it.
					UE_LOG(LogTemp, Warning, TEXT("%s - %s weak reference (null init)"), *Property->GetName(), *ObjectProperty->PropertyClass->GetName());
				}
			}
			else
			{
				// If we don't have a CDO, then this is a struct with a UObject property. In this case, the Struct will never be the owner.
				UE_LOG(LogTemp, Warning, TEXT("%s - %s weak reference (object inside struct)"), *Property->GetName(), *ObjectProperty->PropertyClass->GetName());
			}

			// Skip this property if it make no sense to store in SpatialOS.
			if (PropertyValueClass)
			{
				if (PropertyValueClass->IsChildOf<UClass>())
				{
					return;
				}
				if (!(PropertyValueClass->GetClassFlags() & CLASS_DefaultToInstanced))
				{
					UE_LOG(LogTemp, Warning, TEXT("%s - %s not instanced, skipping"), *Property->GetName(), *PropertyValueClass->GetName());
					return;
				}
				if (PropertyValueClass->IsChildOf(FTickFunction::StaticStruct()))
				{
					return;
				}
			}

			// Upcast property value type to struct.
			PropertyValueStruct = PropertyValueClass;
		}

		// If PropertyValueStruct is not nullptr, this property is an owning reference to the value.
		if (PropertyValueStruct)
		{
			// Instantiate CDO of this struct if it's a class.
			UObject* PropertyValueClassCDO = nullptr;
			if (Cast<UClass>(PropertyValueStruct) != nullptr)
			{
				PropertyValueClassCDO = Cast<UClass>(PropertyValueStruct)->GetDefaultObject();
			}

			// Recurse into properties.
			TArray<UProperty*> NewStack(Stack);
			NewStack.Add(Property);
			for (TFieldIterator<UProperty> It(PropertyValueStruct); It; ++It)
			{
				VisitProperty(PropertyInfo, PropertyValueClassCDO, NewStack, *It);
			}
			return;
		}

		// If PropertyValueStruct is nullptr, fall through to the weak reference case generated by
		// RepLayoutTypeToSchemaType.
	}

	Stack.Add(Property);
	PropertyInfo.Add({
		Property,
		PropertyType,
		Stack
		});
}

// Generates code to copy an Unreal PropertyValue into a SpatialOS component update.
void GenerateUnrealToSchemaConversion(FCodeWriter& Writer, const FString& Update, TArray<UProperty*> PropertyChain, const FString& PropertyValue)
{
	// Get result type.
	UProperty* Property = PropertyChain[PropertyChain.Num() - 1];
	FString SpatialValueSetter = Update + TEXT(".set_") + GetFullyQualifiedName(PropertyChain);

	if (UEnumProperty* EnumProperty = Cast<UEnumProperty>(Property))
	{
		Writer.Print(FString::Printf(TEXT("// UNSUPPORTED UEnumProperty - %s = %s;"), *SpatialValueSetter, *PropertyValue));
		//Writer.Print(FString::Printf(TEXT("auto Underlying = %s.GetValue()"), *PropertyValue));
		//return GenerateUnrealToSchemaConversion(Writer, EnumProperty->GetUnderlyingProperty(), TEXT("Underlying"), ResultName);
	}

	// Try to special case to custom types we know about
	if (Property->IsA(UStructProperty::StaticClass()))
	{
		UStructProperty * StructProp = Cast<UStructProperty>(Property);
		UScriptStruct * Struct = StructProp->Struct;
		if (Struct->GetFName() == NAME_Vector ||
			Struct->GetName() == TEXT("Vector_NetQuantize100") ||
			Struct->GetName() == TEXT("Vector_NetQuantize10") ||
			Struct->GetName() == TEXT("Vector_NetQuantizeNormal") ||
			Struct->GetName() == TEXT("Vector_NetQuantize"))
		{
			Writer.Print(FString::Printf(TEXT("%s(improbable::Vector3f(%s.X, %s.Y, %s.Z));"), *SpatialValueSetter, *PropertyValue, *PropertyValue, *PropertyValue));
		}
		else if (Struct->GetFName() == NAME_Rotator)
		{
			Writer.Print(FString::Printf(TEXT("%s(improbable::unreal::UnrealFRotator(%s.Yaw, %s.Pitch, %s.Roll));"), *SpatialValueSetter, *PropertyValue, *PropertyValue, *PropertyValue));
		}
		else if (Struct->GetFName() == NAME_Plane)
		{
			Writer.Print(FString::Printf(TEXT("%s(improbable::unreal::UnrealFPlane(%s.X, %s.Y, %s.Z, %s.W));"), *SpatialValueSetter, *PropertyValue, *PropertyValue, *PropertyValue, *PropertyValue));
		}
		else if (Struct->GetName() == TEXT("UniqueNetIdRepl"))
		{
			Writer.Print(FString::Printf(TEXT("// UNSUPPORTED UniqueNetIdRepl - %s = %s;"), *SpatialValueSetter, *PropertyValue));
		}
		else if (Struct->GetName() == TEXT("RepMovement"))
		{
			Writer.Print(FString::Printf(TEXT(R"""(TArray<uint8> ValueData;
			FMemoryWriter ValueDataWriter(ValueData);
			bool Success;
			%s.NetSerialize(ValueDataWriter, nullptr, Success);
			%s(std::string((char*)ValueData.GetData(), ValueData.Num()));)"""), *PropertyValue, *SpatialValueSetter));
		}
		else
		{
			for (TFieldIterator<UProperty> It(Struct); It; ++It)
			{
				Writer.Print(TEXT("{")).Indent();
				TArray<UProperty*> NewChain = PropertyChain;
				NewChain.Add(*It);
				GenerateUnrealToSchemaConversion(Writer, Update, NewChain, PropertyValue + TEXT(".") + (*It)->GetNameCPP());
				Writer.Outdent().Print(TEXT("}"));
			}
		}
	}
	else if (Property->IsA(UBoolProperty::StaticClass()))
	{
		Writer.Print(FString::Printf(TEXT("%s(%s != 0);"), *SpatialValueSetter, *PropertyValue));
	}
	else if (Property->IsA(UFloatProperty::StaticClass()))
	{
		Writer.Print(FString::Printf(TEXT("%s(%s);"), *SpatialValueSetter, *PropertyValue));
	}
	else if (Property->IsA(UIntProperty::StaticClass()))
	{
		Writer.Print(FString::Printf(TEXT("%s(%s);"), *SpatialValueSetter, *PropertyValue));
	}
	else if (Property->IsA(UByteProperty::StaticClass()))
	{
		Writer.Print(FString::Printf(TEXT("%s(uint32_t(%s));"), *SpatialValueSetter, *PropertyValue));
	}
	else if (Property->IsA(UObjectPropertyBase::StaticClass()))
	{
		Writer.Print(FString::Printf(TEXT("improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);\n%s(UObjectRef);"),
			*SpatialValueSetter));
	}
	else if (Property->IsA(UNameProperty::StaticClass()))
	{
		Writer.Print(FString::Printf(TEXT("%s(TCHAR_TO_UTF8(*%s.ToString()));"), *SpatialValueSetter, *PropertyValue));
	}
	else if (Property->IsA(UUInt32Property::StaticClass()))
	{
		Writer.Print(FString::Printf(TEXT("%s(uint32_t(%s));"), *SpatialValueSetter, *PropertyValue));
	}
	else if (Property->IsA(UUInt64Property::StaticClass()))
	{
		Writer.Print(FString::Printf(TEXT("%s(uint64_t(%s));"), *SpatialValueSetter, *PropertyValue));
	}
	else if (Property->IsA(UStrProperty::StaticClass()))
	{
		Writer.Print(FString::Printf(TEXT("%s(TCHAR_TO_UTF8(*%s));"), *SpatialValueSetter, *PropertyValue));
	}
	else
	{
		Writer.Print(TEXT("// UNSUPPORTED"));
	}
}

// Generates code to read a property in a SpatialOS component update and copy it to an Unreal PropertyValue.
void GenerateSchemaToUnrealConversion(FCodeWriter& Writer, const FString& Update, TArray<UProperty*> PropertyChain, const FString& PropertyValue, const FString& PropertyType)
{
	// Get result type.
	UProperty* Property = PropertyChain[PropertyChain.Num() - 1];
	FString SpatialValue = FString::Printf(TEXT("*(%s.%s().data())"), *Update, *GetFullyQualifiedName(PropertyChain));

	if (UEnumProperty* EnumProperty = Cast<UEnumProperty>(Property))
	{
		Writer.Print(FString::Printf(TEXT("// UNSUPPORTED (Enum) - %s %s;"), *PropertyValue, *SpatialValue));
		//Writer.Print(FString::Printf(TEXT("auto Underlying = %s.GetValue()"), *PropertyValue));
		//return GenerateUnrealToSchemaConversion(Writer, EnumProperty->GetUnderlyingProperty(), TEXT("Underlying"), ResultName);
	}

	// Try to special case to custom types we know about
	if (Property->IsA(UStructProperty::StaticClass()))
	{
		UStructProperty * StructProp = Cast<UStructProperty>(Property);
		UScriptStruct * Struct = StructProp->Struct;
		if (Struct->GetFName() == NAME_Vector ||
			Struct->GetName() == TEXT("Vector_NetQuantize100") ||
			Struct->GetName() == TEXT("Vector_NetQuantize10") ||
			Struct->GetName() == TEXT("Vector_NetQuantizeNormal") ||
			Struct->GetName() == TEXT("Vector_NetQuantize"))
		{
			Writer.Print(FString::Printf(TEXT("auto& Vector = %s;"), *SpatialValue));
			Writer.Print(FString::Printf(TEXT("%s.X = Vector.x();"), *PropertyValue));
			Writer.Print(FString::Printf(TEXT("%s.Y = Vector.y();"), *PropertyValue));
			Writer.Print(FString::Printf(TEXT("%s.Z = Vector.z();"), *PropertyValue));
		}
		else if (Struct->GetFName() == NAME_Rotator)
		{
			Writer.Print(FString::Printf(TEXT("auto& Rotator = %s;"), *SpatialValue));
			Writer.Print(FString::Printf(TEXT("%s.Yaw = Rotator.yaw();"), *PropertyValue));
			Writer.Print(FString::Printf(TEXT("%s.Pitch = Rotator.pitch();"), *PropertyValue));
			Writer.Print(FString::Printf(TEXT("%s.Roll = Rotator.roll();"), *PropertyValue));
		}
		else if (Struct->GetFName() == NAME_Plane)
		{
			Writer.Print(FString::Printf(TEXT("auto& Plane = %s;"), *SpatialValue));
			Writer.Print(FString::Printf(TEXT("%s.X = Plane.x();"), *PropertyValue));
			Writer.Print(FString::Printf(TEXT("%s.Y = Plane.y();"), *PropertyValue));
			Writer.Print(FString::Printf(TEXT("%s.Z = Plane.z();"), *PropertyValue));
			Writer.Print(FString::Printf(TEXT("%s.W = Plane.w();"), *PropertyValue));
		}
		else if (Struct->GetName() == TEXT("UniqueNetIdRepl"))
		{
			Writer.Print(FString::Printf(TEXT("// UNSUPPORTED UniqueNetIdRepl- %s %s;"), *PropertyValue, *SpatialValue));
		}
		else if (Struct->GetName() == TEXT("RepMovement"))
		{
			Writer.Print(FString::Printf(TEXT(R"""(auto& ValueDataStr = %s;
			TArray<uint8> ValueData;
			ValueData.Append((uint8*)ValueDataStr.data(), ValueDataStr.size());
			FMemoryReader ValueDataReader(ValueData);
			bool bSuccess;
			%s.NetSerialize(ValueDataReader, nullptr, bSuccess);)"""), *SpatialValue, *PropertyValue));
		}
		else
		{
			for (TFieldIterator<UProperty> It(Struct); It; ++It)
			{
				Writer.Print(TEXT("{")).Indent();
				TArray<UProperty*> NewChain = PropertyChain;
				NewChain.Add(*It);
				GenerateSchemaToUnrealConversion(Writer, Update, NewChain, PropertyValue + TEXT(".") + (*It)->GetNameCPP(), (*It)->GetCPPType());
				Writer.Outdent().Print(TEXT("}"));
			}
		}
	}
	else if (Property->IsA(UBoolProperty::StaticClass()))
	{
		Writer.Print(FString::Printf(TEXT("%s = %s;"), *PropertyValue, *SpatialValue));
	}
	else if (Property->IsA(UFloatProperty::StaticClass()))
	{
		Writer.Print(FString::Printf(TEXT("%s = %s;"), *PropertyValue, *SpatialValue));
	}
	else if (Property->IsA(UIntProperty::StaticClass()))
	{
		Writer.Print(FString::Printf(TEXT("%s = %s;"), *PropertyValue, *SpatialValue));
	}
	else if (Property->IsA(UByteProperty::StaticClass()))
	{
		Writer.Print(TEXT(R"""(// Byte properties are weird, because they can also be an enum in the form TEnumAsByte<...>.
		// Therefore, the code generator needs to cast to either TEnumAsByte<...> or uint8. However,
		// as TEnumAsByte<...> only has a uint8 constructor, we need to cast the SpatialOS value into
		// uint8 first, which causes "uint8(uint8(...))" to be generated for non enum bytes.)"""));
		Writer.Print(FString::Printf(TEXT("%s = %s(uint8(%s));"), *PropertyValue, *PropertyType, *SpatialValue));
	}
	else if (Property->IsA(UObjectPropertyBase::StaticClass()))
	{
		Writer.Print(TEXT("{"));
		Writer.Indent();
		Writer.Print(FString::Printf(TEXT("improbable::unreal::UnrealObjectRef TargetObject = %s;"), *SpatialValue));
		Writer.Print(FString::Printf(TEXT("FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromUnrealObjectRef(TargetObject);")));
		Writer.Print(FString::Printf(TEXT("%s = static_cast<%s>(SpatialPMC->GetObjectFromNetGUID(NetGUID, true));"), *PropertyValue, *PropertyType));
		Writer.Outdent().Print(TEXT("}"));
	}
	else if (Property->IsA(UNameProperty::StaticClass()))
	{
		Writer.Print(FString::Printf(TEXT("%s = FName((%s).data());"), *PropertyValue, *SpatialValue));
	}
	else if (Property->IsA(UUInt32Property::StaticClass()))
	{
		Writer.Print(FString::Printf(TEXT("%s = uint32(%s);"), *PropertyValue, *SpatialValue));
	}
	else if (Property->IsA(UUInt64Property::StaticClass()))
	{
		Writer.Print(FString::Printf(TEXT("%s = uint64(%s);"), *PropertyValue, *SpatialValue));
	}
	else if (Property->IsA(UStrProperty::StaticClass()))
	{
		Writer.Print(FString::Printf(TEXT("%s = FString(UTF8_TO_TCHAR(%s));"), *PropertyValue, *SpatialValue));
	}
	else
	{
		Writer.Print(TEXT("// UNSUPPORTED"));
	}
}

FPropertyLayout CreatePropertyLayout(UClass* Class)
{
	FPropertyLayout Layout;

	// Read the RepLayout for the class into a data structure.
	FRepLayout RepLayout;
	RepLayout.InitFromObjectClass(Class);
	TArray<TPair<int, FRepLayoutEntry>> RepLayoutProperties;
	for (int CmdIndex = 0; CmdIndex < RepLayout.Cmds.Num(); ++CmdIndex)
	{
		auto& Cmd = RepLayout.Cmds[CmdIndex];

		if (Cmd.Type == REPCMD_Return || Cmd.Type == REPCMD_DynamicArray)
			continue;

		if (Cmd.Property == nullptr)
			continue;

		// Get property and parent property from RepLayout.
		UProperty* Property = Cmd.Property;
		UProperty* ParentProperty = RepLayout.Parents[Cmd.ParentIndex].Property;
		if (ParentProperty == Property)
		{
			ParentProperty = nullptr;
		}
		TArray<UProperty*> PropertyChain;
		if (ParentProperty)
		{
			PropertyChain = { ParentProperty, Property };
		}
		else
		{
			PropertyChain = { Property };
		}

		int32 Handle = Cmd.RelativeHandle;
		RepLayoutProperties.Add(MakeTuple(Handle, FRepLayoutEntry{
			Property,
			ParentProperty,
			PropertyChain,
			RepLayout.Parents[Cmd.ParentIndex].Condition,
			(ERepLayoutCmdType)Cmd.Type,
			Handle,
			Cmd.Offset
			}));
	}

	// Recurse into class properties and build a complete property list.
	for (TFieldIterator<UProperty> It(Class); It; ++It)
	{
		VisitProperty(Layout.CompleteProperties, Class->GetDefaultObject(), {}, *It);
	}

	// Divide properties into replicated and complete properties.
	TArray<FReplicatedPropertyInfo> ReplicatedProperties;
	TArray<FPropertyInfo> CompletePropertiesToRemove;
	for (auto& Pair : RepLayoutProperties)
	{
		FRepLayoutEntry& Entry = Pair.Value;
		FReplicatedPropertyInfo Info{ Entry,{} };

		// Search for all properties with a property chain that begins with either {Parent, Property} or {Property}.
		Info.PropertyList = Layout.CompleteProperties.FilterByPredicate([Entry](const FPropertyInfo& Property)
		{
			if (Entry.Parent)
			{
				return Property.Chain.Num() >= 2 && Property.Chain[0] == Entry.Parent && Property.Chain[1] == Entry.Property;
			}
			else
			{
				return Property.Chain.Num() >= 1 && Property.Chain[0] == Entry.Property;
			}
		});
		ReplicatedProperties.Add(Info);

		// Append to the properties which need to be removed from the complete properties list.
		CompletePropertiesToRemove.Append(Info.PropertyList);
	}
	for (auto& PropertyToRemove : CompletePropertiesToRemove)
	{
		Layout.CompleteProperties.Remove(PropertyToRemove);
	}

	// Group replicated properties by single client (COND_AutonomousOnly and COND_OwnerOnly),
	// and multi-client (everything else).
	Layout.ReplicatedProperties.Add(REP_SingleClient);
	Layout.ReplicatedProperties.Add(REP_MultiClient);
	for (auto& RepProp : ReplicatedProperties)
	{
		switch (RepProp.Entry.Condition)
		{
		case COND_AutonomousOnly:
		case COND_OwnerOnly:
			Layout.ReplicatedProperties[REP_SingleClient].Add(RepProp);
			break;
		default:
			Layout.ReplicatedProperties[REP_MultiClient].Add(RepProp);
		}
	}

	return Layout;
}

void GenerateSchemaFromLayout(FCodeWriter& Writer, int ComponentId, UClass* Class, const FPropertyLayout& Layout)
{
	Writer.Print(TEXT("// Copyright (c) Improbable Worlds Ltd, All Rights Reserved"));
	Writer.Print(TEXT("// Note that this file has been generated automatically"));
	Writer.Print(TEXT("package improbable.unreal;"));
	Writer.Print();
	Writer.Print(TEXT("import \"improbable/vector3.schema\";"));
	Writer.Print(TEXT("import \"unreal/core_types.schema\";"));
	Writer.Print();

	TArray<EReplicatedPropertyGroup> RepPropertyGroups;
	Layout.ReplicatedProperties.GetKeys(RepPropertyGroups);

	// Replicated properties.
	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		Writer.Print(FString::Printf(TEXT("component %s {"), *GetSchemaReplicatedDataName(Group, Class)));
		Writer.Indent();
		Writer.Print(FString::Printf(TEXT("id = %d;"), ComponentId + (int)Group + 1));
		int FieldCounter = 0;
		for (auto& RepProp : Layout.ReplicatedProperties[Group])
		{
			for (auto& Prop : RepProp.PropertyList)
			{
				FieldCounter++;
				Writer.Print(
					FString::Printf(
						TEXT("%s %s = %d; // %s"),
						*RepLayoutTypeToSchemaType(Prop.Type),
						*GetFullyQualifiedName(Prop.Chain),
						FieldCounter,
						*GetLifetimeConditionAsString(RepProp.Entry.Condition)
					)
				);
			}
		}
		Writer.Outdent().Print(TEXT("}"));
	}

	// Complete properties.
	Writer.Print(FString::Printf(TEXT("component %s {"), *GetSchemaCompleteDataName(Class)));
	Writer.Indent();
	Writer.Print(FString::Printf(TEXT("id = %d;"), ComponentId));
	int FieldCounter = 0;
	for (auto& Prop : Layout.CompleteProperties)
	{
		FieldCounter++;
		Writer.Print(
			FString::Printf(
				TEXT("%s %s = %d;"),
				*RepLayoutTypeToSchemaType(Prop.Type),
				*GetFullyQualifiedName(Prop.Chain),
				FieldCounter
			)
		);
	}
	Writer.Outdent().Print(TEXT("}"));
}

void GenerateForwardingCodeFromLayout(
	FCodeWriter& HeaderWriter,
	FCodeWriter& SourceWriter,
	FString SchemaFilename,
	FString InteropFilename,
	UClass* Class,
	const FPropertyLayout& Layout)
{
	TMap<EReplicatedPropertyGroup, FString> UnrealToSpatialSignatureByGroup;
	TMap<EReplicatedPropertyGroup, FString> SpatialToUnrealSignatureByGroup;

	TArray<EReplicatedPropertyGroup> RepPropertyGroups;
	Layout.ReplicatedProperties.GetKeys(RepPropertyGroups);

	// Forwarding code header file.
	HeaderWriter.Print(TEXT("// Copyright (c) Improbable Worlds Ltd, All Rights Reserved"));
	HeaderWriter.Print(TEXT("// Note that this file has been generated automatically"));
	HeaderWriter.Print();
	HeaderWriter.Print(TEXT("#pragma once"));
	HeaderWriter.Print();
	HeaderWriter.Print(TEXT("#include <improbable/worker.h>"));
	HeaderWriter.Print(TEXT("#include <improbable/view.h>"));
	HeaderWriter.Print(FString::Printf(TEXT("#include <unreal/generated/%s.h>"), *SchemaFilename));
	HeaderWriter.Print(TEXT("#include <unreal/core_types.h>"));
	HeaderWriter.Print(TEXT("#include \"SpatialHandlePropertyMap.h\""));
	HeaderWriter.Print(TEXT("#include \"SpatialTypeBinding.h\""));
	HeaderWriter.Print(FString::Printf(TEXT("#include \"SpatialTypeBinding_%s.generated.h\""), *Class->GetName()));
	HeaderWriter.Print();

	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		UnrealToSpatialSignatureByGroup.Add(Group, FString::Printf(
			TEXT("void ApplyUpdateToSpatial_%s_%s(const uint8* RESTRICT Data, int32 Handle, UProperty* Property, UPackageMap* PackageMap, improbable::unreal::%s::Update& Update)"),
			*GetReplicatedPropertyGroupName(Group),
			*Class->GetName(),
			*GetSchemaReplicatedDataName(Group, Class)));
		SpatialToUnrealSignatureByGroup.Add(Group, FString::Printf(
			TEXT("void ReceiveUpdateFromSpatial_%s_%s(USpatialUpdateInterop* UpdateInterop, UPackageMap* PackageMap, const worker::ComponentUpdateOp<improbable::unreal::%s>& Op)"),
			*GetReplicatedPropertyGroupName(Group),
			*Class->GetName(),
			*GetSchemaReplicatedDataName(Group, Class)));
		//HeaderWriter.Print(FString::Printf(TEXT("%s;"), *UnrealToSpatialSignatureByGroup[Group]));
		//HeaderWriter.Print(FString::Printf(TEXT("%s;"), *SpatialToUnrealSignatureByGroup[Group]));
	}

	// Type binding class.
	HeaderWriter.Print(TEXT("UCLASS()"));
	HeaderWriter.Print(FString::Printf(TEXT("class USpatialTypeBinding_%s : public USpatialTypeBinding"), *Class->GetName()));
	HeaderWriter.Print(TEXT("{"));
	HeaderWriter.Indent();
	HeaderWriter.Print(TEXT("GENERATED_BODY()"));
	HeaderWriter.Outdent().Print(TEXT("public:")).Indent();
	HeaderWriter.Print(TEXT(R"""(static const FRepHandlePropertyMap& GetHandlePropertyMap();
	void BindToView() override;
	void UnbindFromView() override;
	worker::ComponentId GetReplicatedGroupComponentId(EReplicatedPropertyGroup Group) const override;
	void SendComponentUpdates(const FPropertyChangeState& Changes, const worker::EntityId& EntityId) const override;
	worker::Entity CreateActorEntity(const FVector& Position, const FString& Metadata, const FPropertyChangeState& InitialChanges) const override;)"""));
	HeaderWriter.Outdent().Print(TEXT("private:")).Indent();
	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		HeaderWriter.Print(FString::Printf(TEXT("worker::Dispatcher::CallbackKey %sCallback;"), *GetReplicatedPropertyGroupName(Group)));
	}
	HeaderWriter.Outdent();
	HeaderWriter.Print(TEXT("};"));

	// Forwarding code source file.
	SourceWriter.Print(TEXT("// Copyright (c) Improbable Worlds Ltd, All Rights Reserved"));
	SourceWriter.Print(TEXT("// Note that this file has been generated automatically"));
	SourceWriter.Print();
	SourceWriter.Print(FString::Printf(TEXT(R"""(#include "%s.h"
	#include "SpatialOS.h"
	#include "Engine.h"
	#include "SpatialActorChannel.h"
	#include "EntityBuilder.h"
	// TODO(David): Remove this once RPCs are merged, as we will no longer need a placeholder component.
	#include "improbable/player/player.h"
	#include "SpatialPackageMapClient.h"
	#include "SpatialUpdateInterop.h")"""), *InteropFilename));

	// Replicated property interop.
	SourceWriter.Print();
	SourceWriter.Print(TEXT("namespace {"));
	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		// Unreal -> Spatial.
		// ===========================================
		SourceWriter.Print();
		SourceWriter.Print(FString::Printf(TEXT("%s"), *UnrealToSpatialSignatureByGroup[Group]));
		SourceWriter.Print(TEXT("{"));
		SourceWriter.Indent();
		if (Layout.ReplicatedProperties[Group].Num() > 0)
		{
			SourceWriter.Print(TEXT("USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);"));
			SourceWriter.Print(TEXT("check(SpatialPMC);"));
			SourceWriter.Print(TEXT("switch (Handle)\n{"));
			SourceWriter.Indent();
			for (auto& RepProp : Layout.ReplicatedProperties[Group])
			{
				auto Handle = RepProp.Entry.Handle;
				UProperty* Property = RepProp.Entry.Property;

				SourceWriter.Print(FString::Printf(TEXT("case %d: // %s"), Handle, *GetFullyQualifiedName(RepProp.Entry.Chain)));
				SourceWriter.Print(TEXT("{"));
				SourceWriter.Indent();

				// Get unreal data by deserialising from the reader, convert and set the corresponding field in the update object.
				FString PropertyValueName = TEXT("Value");
				FString PropertyValueCppType = Property->GetCPPType();
				FString PropertyName = TEXT("Property");
				SourceWriter.Print(FString::Printf(TEXT("%s %s;"), *PropertyValueCppType, *PropertyValueName));
				//todo-giray: The reinterpret_cast below is ugly and we believe we can do this more gracefully using Property helper functions.
				if (Property->IsA(UObjectPropertyBase::StaticClass()))
				{
					SourceWriter.Print(FString::Printf(TEXT("%s = *(reinterpret_cast<%s const*>(Data));"), *PropertyValueName, *PropertyValueCppType));
					SourceWriter.Print(FString::Printf(TEXT("FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromObject(%s);"), *PropertyValueName));
				}
				else
				{
					SourceWriter.Print(FString::Printf(TEXT("%s = *(reinterpret_cast<const %s*>(Data));"), *PropertyValueName, *PropertyValueCppType));
				}
				SourceWriter.Print();
				GenerateUnrealToSchemaConversion(SourceWriter, TEXT("Update"), RepProp.Entry.Chain, PropertyValueName);
				SourceWriter.Print(TEXT("break;"));
				SourceWriter.Outdent();
				SourceWriter.Print(TEXT("}"));
			}
			SourceWriter.Outdent().Print(TEXT("default:"));
			SourceWriter.Indent();
			SourceWriter.Print(TEXT("checkf(false, TEXT(\"Unknown replication handle %d encountered when creating a SpatialOS update.\"));"));
			SourceWriter.Print(TEXT("break;"));
			SourceWriter.Outdent();
			SourceWriter.Print(TEXT("}"));
		}
		SourceWriter.Outdent();
		SourceWriter.Print(TEXT("}"));

		// Spatial -> Unreal.
		// ===========================================
		SourceWriter.Print();
		SourceWriter.Print(FString::Printf(TEXT("%s"), *SpatialToUnrealSignatureByGroup[Group]));
		SourceWriter.Print(TEXT("{"));
		SourceWriter.Indent();
		SourceWriter.Print(TEXT("FNetBitWriter OutputWriter(nullptr, 0); "));
		SourceWriter.Print(FString::Printf(TEXT("auto& HandleToPropertyMap = USpatialTypeBinding_%s::GetHandlePropertyMap();"), *Class->GetName()));
		SourceWriter.Print(TEXT("USpatialActorChannel* ActorChannel = UpdateInterop->GetClientActorChannel(Op.EntityId);"));
		SourceWriter.Print(TEXT("if (!ActorChannel)\n{"));
		SourceWriter.Indent();
		SourceWriter.Print(TEXT("return;"));
		SourceWriter.Outdent();
		SourceWriter.Print(TEXT("}"));
		SourceWriter.Print(TEXT("USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);"));
		SourceWriter.Print(TEXT("check(SpatialPMC);"));
		SourceWriter.Print(TEXT("ConditionMapFilter ConditionMap(ActorChannel);"));
		for (auto& RepProp : Layout.ReplicatedProperties[Group])
		{
			auto Handle = RepProp.Entry.Handle;
			UProperty* Property = RepProp.Entry.Property;

			// Check if only the first property is in the property list. This implies that the rest is also in the update, as
			// they are sent together atomically.
			SourceWriter.Print(FString::Printf(TEXT("if (!Op.Update.%s().empty())\n{"), *GetFullyQualifiedName(RepProp.PropertyList[0].Chain)));
			SourceWriter.Indent();

			// Check if the property is relevant.
			SourceWriter.Print(FString::Printf(TEXT("// %s"), *GetFullyQualifiedName(RepProp.Entry.Chain)));
			SourceWriter.Print(FString::Printf(TEXT("uint32 Handle = %d;"), Handle));
			SourceWriter.Print(TEXT("const FRepHandleData& Data = HandleToPropertyMap[Handle];"));
			SourceWriter.Print(TEXT("if (ConditionMap.IsRelevant(Data.Condition))\n{"));
			SourceWriter.Indent();

			// Write handle.
			SourceWriter.Print(TEXT("OutputWriter.SerializeIntPacked(Handle);"));
			SourceWriter.Print();

			// Convert update data to the corresponding Unreal type and serialize to OutputWriter.
			FString PropertyValueName = TEXT("Value");
			FString PropertyValueCppType = Property->GetCPPType();
			FString PropertyName = TEXT("Data.Property");
			SourceWriter.Print(FString::Printf(TEXT("%s %s;"), *PropertyValueCppType, *PropertyValueName));
			SourceWriter.Print();
			GenerateSchemaToUnrealConversion(SourceWriter, TEXT("Op.Update"), RepProp.Entry.Chain, PropertyValueName, PropertyValueCppType);
			SourceWriter.Print();
			SourceWriter.Print(FString::Printf(TEXT("%s->NetSerializeItem(OutputWriter, PackageMap, &%s);"), *PropertyName, *PropertyValueName));
			SourceWriter.Print(TEXT("UE_LOG(LogTemp, Log, TEXT(\"<- Handle: %d Property %s\"), Handle, *Data.Property->GetName());"));

			// End condition map check block.
			SourceWriter.Outdent();
			SourceWriter.Print(TEXT("}"));

			// End property block.
			SourceWriter.Outdent();
			SourceWriter.Print(TEXT("}"));
		}
		SourceWriter.Print(TEXT("UpdateInterop->ReceiveSpatialUpdate(ActorChannel, OutputWriter);"));
		SourceWriter.Outdent();
		SourceWriter.Print(TEXT("}"));
	}

	// BuildSpatialComponentUpdate
	// ===========================================
	SourceWriter.Print();
	SourceWriter.Print(TEXT("void BuildSpatialComponentUpdate(const FPropertyChangeState& Changes,"));
	SourceWriter.Indent().Indent();
	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		SourceWriter.Print(FString::Printf(TEXT("improbable::unreal::%s::Update& %sUpdate,"),
			*GetSchemaReplicatedDataName(Group, Class),
			*GetReplicatedPropertyGroupName(Group),
			*GetReplicatedPropertyGroupName(Group)));
		SourceWriter.Print(FString::Printf(TEXT("bool& b%sUpdateChanged,"),
			*GetReplicatedPropertyGroupName(Group)));
	}
	SourceWriter.Print(TEXT("UPackageMap* PackageMap)"));
	SourceWriter.Outdent().Outdent();

	SourceWriter.Print(TEXT("{"));
	SourceWriter.Indent();
	SourceWriter.Print(FString::Printf(TEXT(R"""(// Build up SpatialOS component updates.
	auto& PropertyMap = USpatialTypeBinding_%s::GetHandlePropertyMap();
	FChangelistIterator ChangelistIterator(Changes.Changed, 0);
	FRepHandleIterator HandleIterator(ChangelistIterator, Changes.Cmds, Changes.BaseHandleToCmdIndex, 0, 1, 0, Changes.Cmds.Num() - 1);
	while (HandleIterator.NextHandle())
	{)"""), *Class->GetName()));
	SourceWriter.Indent();
	SourceWriter.Print(FString::Printf(TEXT(R"""(const FRepLayoutCmd& Cmd = Changes.Cmds[HandleIterator.CmdIndex];
	const uint8* Data = Changes.SourceData + HandleIterator.ArrayOffset + Cmd.Offset;
	auto& PropertyMapData = PropertyMap[HandleIterator.Handle];)""")));
	SourceWriter.Print(FString::Printf(TEXT("UE_LOG(LogTemp, Log, TEXT(\"-> Handle: %%d Property %%s\"), HandleIterator.Handle, *Cmd.Property->GetName());")));

	SourceWriter.Print(TEXT("switch (GetGroupFromCondition(PropertyMapData.Condition))"));
	SourceWriter.Print(TEXT("{"));
	SourceWriter.Indent();
	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		SourceWriter.Outdent();
		SourceWriter.Print(FString::Printf(TEXT("case GROUP_%s:"), *GetReplicatedPropertyGroupName(Group)));
		SourceWriter.Indent();
		SourceWriter.Print(FString::Printf(TEXT("ApplyUpdateToSpatial_%s_%s(Data, HandleIterator.Handle, Cmd.Property, PackageMap, %sUpdate);"),
			*GetReplicatedPropertyGroupName(Group),
			*Class->GetName(),
			*GetReplicatedPropertyGroupName(Group)));
		SourceWriter.Print(FString::Printf(TEXT("b%sUpdateChanged = true;"),
			*GetReplicatedPropertyGroupName(Group)));
		SourceWriter.Print(TEXT("break;"));
	}
	SourceWriter.Outdent();
	SourceWriter.Print(TEXT("}"));
	SourceWriter.Outdent();
	SourceWriter.Print(TEXT("}"));
	SourceWriter.Outdent();
	SourceWriter.Print(TEXT("}"));

	// End of namespace.
	SourceWriter.Print(TEXT("} // ::"));

	// Class implementation.
	// ===========================================

	// Handle to Property map.
	// ===========================================
	SourceWriter.Print();
	SourceWriter.Print(FString::Printf(TEXT("const FRepHandlePropertyMap& USpatialTypeBinding_%s::GetHandlePropertyMap()"), *Class->GetName()));
	SourceWriter.Print(TEXT("{"));
	SourceWriter.Indent();
	SourceWriter.Print(TEXT("static FRepHandlePropertyMap* HandleToPropertyMapData = nullptr;"));
	SourceWriter.Print(TEXT("if (HandleToPropertyMapData == nullptr)"));
	SourceWriter.Print(TEXT("{")).Indent();
	SourceWriter.Print(FString::Printf(TEXT("UClass* Class = FindObject<UClass>(ANY_PACKAGE, TEXT(\"%s\"));"), *Class->GetName()));
	SourceWriter.Print(TEXT("HandleToPropertyMapData = new FRepHandlePropertyMap();"));
	SourceWriter.Print(TEXT("auto& HandleToPropertyMap = *HandleToPropertyMapData;"));

	// Reduce into single list of properties.
	TArray<FReplicatedPropertyInfo> ReplicatedProperties;
	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		ReplicatedProperties.Append(Layout.ReplicatedProperties[Group]);
	}

	for (auto& RepProp : ReplicatedProperties)
	{
		auto Handle = RepProp.Entry.Handle;
		if (RepProp.Entry.Parent)
		{
			SourceWriter.Print(FString::Printf(TEXT("HandleToPropertyMap.Add(%d, FRepHandleData{Class->FindPropertyByName(\"%s\"), nullptr, %s});"),
				Handle, *RepProp.Entry.Parent->GetName(), *GetLifetimeConditionAsString(RepProp.Entry.Condition)));
			SourceWriter.Print(FString::Printf(TEXT("HandleToPropertyMap[%d].Property = Cast<UStructProperty>(HandleToPropertyMap[%d].Parent)->Struct->FindPropertyByName(\"%s\");"),
				Handle, Handle, *RepProp.Entry.Property->GetName()));
		}
		else
		{
			SourceWriter.Print(FString::Printf(TEXT("HandleToPropertyMap.Add(%d, FRepHandleData{nullptr, Class->FindPropertyByName(\"%s\"), %s});"),
				Handle, *RepProp.Entry.Property->GetName(), *GetLifetimeConditionAsString(RepProp.Entry.Condition)));
		}
	}
	SourceWriter.Outdent().Print(TEXT("}"));
	SourceWriter.Print(TEXT("return *HandleToPropertyMapData;"));
	SourceWriter.Outdent();
	SourceWriter.Print(TEXT("}"));

	// BindToView
	// ===========================================
	SourceWriter.Print();
	SourceWriter.Print(FString::Printf(TEXT("void USpatialTypeBinding_%s::BindToView()"), *Class->GetName()));
	SourceWriter.Print(TEXT("{"));
	SourceWriter.Indent();
	SourceWriter.Print(TEXT("TSharedPtr<worker::View> View = UpdateInterop->GetSpatialOS()->GetView().Pin();"));
	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		SourceWriter.Print(FString::Printf(TEXT("%sCallback = View->OnComponentUpdate<improbable::unreal::%s>([this]("),
			*GetReplicatedPropertyGroupName(Group),
			*GetSchemaReplicatedDataName(Group, Class)));
		SourceWriter.Indent();
		SourceWriter.Print(FString::Printf(TEXT("const worker::ComponentUpdateOp<improbable::unreal::%s>& Op)"),
			*GetSchemaReplicatedDataName(Group, Class)));
		SourceWriter.Outdent();
		SourceWriter.Print(TEXT("{"));
		SourceWriter.Indent();
		SourceWriter.Print(FString::Printf(TEXT("ReceiveUpdateFromSpatial_%s_%s(UpdateInterop, PackageMap, Op);"),
			*GetReplicatedPropertyGroupName(Group),
			*Class->GetName()));
		SourceWriter.Outdent();
		SourceWriter.Print(TEXT("});"));
	}
	SourceWriter.Outdent();
	SourceWriter.Print(TEXT("}"));

	// UnbindFromView
	// ===========================================
	SourceWriter.Print();
	SourceWriter.Print(FString::Printf(TEXT("void USpatialTypeBinding_%s::UnbindFromView()"), *Class->GetName()));
	SourceWriter.Print(TEXT("{"));
	SourceWriter.Indent();
	SourceWriter.Print(TEXT("TSharedPtr<worker::View> View = UpdateInterop->GetSpatialOS()->GetView().Pin();"));
	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		SourceWriter.Print(FString::Printf(TEXT("View->Remove(%sCallback);"), *GetReplicatedPropertyGroupName(Group)));
	}
	SourceWriter.Outdent();
	SourceWriter.Print(TEXT("}"));

	// GetReplicatedGroupComponentId
	// ===========================================
	SourceWriter.Print();
	SourceWriter.Print(FString::Printf(TEXT("worker::ComponentId USpatialTypeBinding_%s::GetReplicatedGroupComponentId(EReplicatedPropertyGroup Group) const"),
		*Class->GetName()));
	SourceWriter.Print(TEXT("{"));
	SourceWriter.Indent();
	SourceWriter.Print(TEXT("switch (Group)"));
	SourceWriter.Print(TEXT("{"));
	SourceWriter.Indent();
	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		SourceWriter.Outdent();
		SourceWriter.Print(FString::Printf(TEXT("case GROUP_%s:"), *GetReplicatedPropertyGroupName(Group)));
		SourceWriter.Indent();
		SourceWriter.Print(FString::Printf(TEXT("return improbable::unreal::%s::ComponentId;"), *GetSchemaReplicatedDataName(Group, Class)));
	}
	SourceWriter.Outdent().Print(TEXT("default:")).Indent();
	SourceWriter.Print(TEXT("checkNoEntry();"));
	SourceWriter.Print(TEXT("return 0;"));
	SourceWriter.Outdent();
	SourceWriter.Print(TEXT("}"));
	SourceWriter.Outdent();
	SourceWriter.Print(TEXT("}"));

	// SendComponentUpdates
	// ===========================================
	SourceWriter.Print();
	SourceWriter.Print(FString::Printf(TEXT("void USpatialTypeBinding_%s::SendComponentUpdates(const FPropertyChangeState& Changes, const worker::EntityId& EntityId) const"),
		*Class->GetName()));
	SourceWriter.Print(TEXT("{"));
	SourceWriter.Indent();

	SourceWriter.Print(TEXT("// Build SpatialOS updates."));
	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		SourceWriter.Print(FString::Printf(TEXT("improbable::unreal::%s::Update %sUpdate;"),
			*GetSchemaReplicatedDataName(Group, Class),
			*GetReplicatedPropertyGroupName(Group)));
		SourceWriter.Print(FString::Printf(TEXT("bool %sUpdateChanged = false;"), *GetReplicatedPropertyGroupName(Group)));
	}

	SourceWriter.Print("BuildSpatialComponentUpdate(Changes,");
	SourceWriter.Indent();
	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		SourceWriter.Print(FString::Printf(TEXT("%sUpdate, %sUpdateChanged,"),
			*GetReplicatedPropertyGroupName(Group),
			*GetReplicatedPropertyGroupName(Group)));
	}
	SourceWriter.Print(TEXT("PackageMap);"));
	SourceWriter.Outdent();

	SourceWriter.Print();
	SourceWriter.Print(TEXT("// Send SpatialOS updates if anything changed."));
	SourceWriter.Print(TEXT("TSharedPtr<worker::Connection> Connection = UpdateInterop->GetSpatialOS()->GetConnection().Pin();"));
	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		SourceWriter.Print(FString::Printf(TEXT("if (%sUpdateChanged)"), *GetReplicatedPropertyGroupName(Group)));
		SourceWriter.Print(TEXT("{"));
		SourceWriter.Indent();
		SourceWriter.Print(FString::Printf(TEXT("Connection->SendComponentUpdate<improbable::unreal::%s>(EntityId, %sUpdate);"),
			*GetSchemaReplicatedDataName(Group, Class),
			*GetReplicatedPropertyGroupName(Group)));
		SourceWriter.Outdent();
		SourceWriter.Print(TEXT("}"));
	}

	SourceWriter.Outdent();
	SourceWriter.Print(TEXT("}"));

	// CreateActorEntity
	// ===========================================
	SourceWriter.Print();
	SourceWriter.Print(FString::Printf(TEXT("worker::Entity USpatialTypeBinding_%s::CreateActorEntity(const FVector& Position, const FString& Metadata, const FPropertyChangeState& InitialChanges) const"), *Class->GetName()));
	SourceWriter.Print(TEXT("{"));
	SourceWriter.Indent();
	
	// Set up initial data.
	SourceWriter.Print(TEXT("// Setup initial data."));
	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		SourceWriter.Print(FString::Printf(TEXT("improbable::unreal::%s::Data %sData;"),
			*GetSchemaReplicatedDataName(Group, Class),
			*GetReplicatedPropertyGroupName(Group)));
		SourceWriter.Print(FString::Printf(TEXT("improbable::unreal::%s::Update %sUpdate;"),
			*GetSchemaReplicatedDataName(Group, Class),
			*GetReplicatedPropertyGroupName(Group)));
		SourceWriter.Print(FString::Printf(TEXT("bool b%sUpdateChanged = false;"), *GetReplicatedPropertyGroupName(Group)));
	}
	SourceWriter.Print("BuildSpatialComponentUpdate(InitialChanges,");
	SourceWriter.Indent();
	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		SourceWriter.Print(FString::Printf(TEXT("%sUpdate, b%sUpdateChanged,"),
			*GetReplicatedPropertyGroupName(Group),
			*GetReplicatedPropertyGroupName(Group)));
	}
	SourceWriter.Print(TEXT("PackageMap);"));
	SourceWriter.Outdent();
	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		SourceWriter.Print(FString::Printf(TEXT("%sUpdate.ApplyTo(%sData);"),
			*GetReplicatedPropertyGroupName(Group),
			*GetReplicatedPropertyGroupName(Group)));
	}

	// Create Entity.
	SourceWriter.Print(TEXT(R"""(
		// Create entity.
		const improbable::Coordinates SpatialPosition = USpatialOSConversionFunctionLibrary::UnrealCoordinatesToSpatialOsCoordinatesCast(Position);
		improbable::WorkerAttributeSet UnrealWorkerAttributeSet{worker::List<std::string>{"UnrealWorker"}};
		improbable::WorkerAttributeSet UnrealClientAttributeSet{worker::List<std::string>{"UnrealClient"}};
		improbable::WorkerRequirementSet UnrealWorkerWritePermission{{UnrealWorkerAttributeSet}};
		improbable::WorkerRequirementSet UnrealClientWritePermission{{UnrealClientAttributeSet}};
		improbable::WorkerRequirementSet AnyWorkerReadPermission{{UnrealClientAttributeSet, UnrealWorkerAttributeSet}};
	)"""));
	SourceWriter.Print(TEXT("return improbable::unreal::FEntityBuilder::Begin()"));
	SourceWriter.Indent();
	SourceWriter.Print(TEXT(R"""(.AddPositionComponent(improbable::Position::Data{SpatialPosition}, UnrealWorkerWritePermission)
			.AddMetadataComponent(improbable::Metadata::Data{TCHAR_TO_UTF8(*Metadata)})
			.SetPersistence(true)
			.SetReadAcl(AnyWorkerReadPermission)
			.AddComponent<improbable::player::PlayerControlClient>(improbable::player::PlayerControlClient::Data{}, UnrealClientWritePermission))"""));
	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		SourceWriter.Print(FString::Printf(TEXT(".AddComponent<improbable::unreal::%s>(%sData, UnrealWorkerWritePermission)"),
			*GetSchemaReplicatedDataName(Group, Class), *GetReplicatedPropertyGroupName(Group)));
	}
	SourceWriter.Print(FString::Printf(TEXT(".AddComponent<improbable::unreal::%s>(improbable::unreal::%s::Data{}, UnrealWorkerWritePermission)"),
		*GetSchemaCompleteDataName(Class), *GetSchemaCompleteDataName(Class)));
	SourceWriter.Print(TEXT(".Build();"));
	SourceWriter.Outdent();
	SourceWriter.Outdent();
	SourceWriter.Print(TEXT("}"));
}

void GenerateCompleteSchemaFromClass(const FString& SchemaPath, const FString& ForwardingCodePath, int ComponentId, UClass* Class)
{
	FCodeWriter OutputSchema;
	FCodeWriter OutputHeader;
	FCodeWriter OutputSource;

	FString SchemaFilename = FString::Printf(TEXT("Unreal%s"), *Class->GetName());
	FString TypeBindingFilename = FString::Printf(TEXT("SpatialTypeBinding_%s"), *Class->GetName());

	FPropertyLayout Layout = CreatePropertyLayout(Class);

	// Generate schema.
	GenerateSchemaFromLayout(OutputSchema, ComponentId, Class, Layout);
	OutputSchema.WriteToFile(FString::Printf(TEXT("%s%s.schema"), *SchemaPath, *SchemaFilename));

	// Generate forwarding code.
	GenerateForwardingCodeFromLayout(OutputHeader, OutputSource, SchemaFilename, TypeBindingFilename, Class, Layout);
	OutputHeader.WriteToFile(FString::Printf(TEXT("%s%s.h"), *ForwardingCodePath, *TypeBindingFilename));
	OutputSource.WriteToFile(FString::Printf(TEXT("%s%s.cpp"), *ForwardingCodePath, *TypeBindingFilename));
}
} // ::

int32 UGenerateSchemaCommandlet::Main(const FString& Params)
{
	FString CombinedSchemaPath = FPaths::Combine(*FPaths::GetPath(FPaths::GetProjectFilePath()), TEXT("../../../schema/unreal/generated/"));
	FString CombinedForwardingCodePath = FPaths::Combine(*FPaths::GetPath(FPaths::GetProjectFilePath()), TEXT("../../../workers/unreal/Game/Source/NUF/Generated/"));
	UE_LOG(LogTemp, Display, TEXT("Schema path %s - Forwarding code path %s"), *CombinedSchemaPath, *CombinedForwardingCodePath);

	TArray<FString> Classes = { TEXT("Character"), TEXT("PlayerController") };
	if (FPaths::CollapseRelativeDirectories(CombinedSchemaPath) && FPaths::CollapseRelativeDirectories(CombinedForwardingCodePath))
	{
		int ComponentId = 100000;
		for (auto& ClassName : Classes)
		{
			UClass* Class = FindObject<UClass>(ANY_PACKAGE, *ClassName);
			GenerateCompleteSchemaFromClass(CombinedSchemaPath, CombinedForwardingCodePath, ComponentId, Class);
			ComponentId += 3;
		}
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Path was invalid - schema not generated"));
	}

	return 0;
}
