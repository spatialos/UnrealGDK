// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GenerateInteropCodeCommandlet.h"

#include <functional>

#include "CodeWriter.h"
#include "Net/DataReplication.h"
#include "GameFramework/Character.h"
#include "Misc/FileHelper.h"
#include "Components/ArrowComponent.h"
#include "ComponentIdGenerator.h"
#include "Net/RepLayout.h"

namespace
{
struct FPropertyInfo
{
	UProperty* Property;
	ERepLayoutCmdType Type;
	UStruct* PropertyStruct; // If Struct or Object, this will be set to the UClass*
	// Properties that were traversed to reach this property, including the property itself.
	TArray<UProperty*> Chain;

	bool operator==(const FPropertyInfo& Other) const
	{
		return Property == Other.Property && Type == Other.Type && Chain == Other.Chain;
	}
};

struct FFunctionSignature
{
	FString Type;
	FString Name;
	FString Params;

	FString Declaration()
	{
		return FString::Printf(TEXT("%s %s%s;"), *Type, *Name, *Params);
	}

	FString Definition(const FString& TypeName)
	{
		return FString::Printf(TEXT("%s %s::%s%s"), *Type, *TypeName, *Name, *Params);
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

// RPC_Client means RPCs which are destined for the client (such as ClientAckGoodMove etc).
enum ERPCType
{
	RPC_Client,
	RPC_Server,
	RPC_Unknown
};

struct FRPCDefinition
{
	UClass* CallerType;
	UFunction* Function;
	bool bReliable;
};

struct FPropertyLayout
{
	TMap<EReplicatedPropertyGroup, TArray<FReplicatedPropertyInfo>> ReplicatedProperties;
	TArray<FPropertyInfo> CompleteProperties;
	TMap<ERPCType, TArray<FRPCDefinition>> RPCs;
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

TArray<ERPCType> GetRPCTypes()
{
	static TArray<ERPCType> Groups = {RPC_Client, RPC_Server};
	return Groups;
}

FString GetReplicatedPropertyGroupName(EReplicatedPropertyGroup Group)
{
	return Group == REP_SingleClient ? TEXT("SingleClient") : TEXT("MultiClient");
}

ERPCType GetRPCTypeFromFunction(UFunction* Function)
{
	if (Function->FunctionFlags & EFunctionFlags::FUNC_NetClient)
	{
		return ERPCType::RPC_Client;
	}
	if (Function->FunctionFlags & EFunctionFlags::FUNC_NetServer)
	{
		return ERPCType::RPC_Server;
	}
	else
	{
		checkNoEntry()
		return ERPCType::RPC_Unknown;
	}
}

FString GetRPCTypeString(ERPCType RPCType)
{
	switch (RPCType)
	{
		case ERPCType::RPC_Client:
			return "Client";
		case ERPCType::RPC_Server:
			return "Server";
		default:
			checkf(false, TEXT("RPCType is invalid!"))
			return "";
	}
}

FString PropertyNameToSchemaName(FString Name)
{
	Name.ReplaceInline(TEXT("."), TEXT("_"));
	Name.ReplaceInline(SUBOBJECT_DELIMITER, TEXT("_"));
	Name.ToLowerInline();
	return Name;
}

FString PropertyGeneratedName(UProperty* Property)
{
	FString SchemaName = PropertyNameToSchemaName(Property->GetFullGroupName(false));
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

FString GetFullCPPName(UClass* Class)
{
	if (Class->IsChildOf(AActor::StaticClass()))
	{
		return FString::Printf(TEXT("A%s"), *Class->GetName());
	}
	else
	{
		return FString::Printf(TEXT("U%s"), *Class->GetName());
	}
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

FString GetSchemaRPCComponentName(ERPCType RpcType, UStruct* Type)
{
	return FString::Printf(TEXT("Unreal%s%sRPCs"), *Type->GetName(), *GetRPCTypeString(RpcType));
}

FString GetCommandNameFromFunction(UFunction* Function)
{
	FString Lower = Function->GetName().ToLower();
	Lower[0] = FChar::ToUpper(Lower[0]);
	return Lower;
}

FString GetSchemaRPCRequestTypeFromUnreal(UFunction* Func)
{
	return TEXT("Unreal") + Func->GetName() + TEXT("Request");
}

FString GetSchemaRPCResponseTypeFromUnreal(UFunction* Func)
{
	return TEXT("Unreal") + Func->GetName() + TEXT("Response");
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
	UStruct* PropertyValueStruct = nullptr;
	if ((PropertyType == REPCMD_Property && Property->IsA<UStructProperty>()) ||
		(PropertyType == REPCMD_PropertyObject && Property->IsA<UObjectProperty>()))
	{
		// Get struct/class of property value.
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
		PropertyValueStruct,
		Stack
		});
}

// Generates code to copy an Unreal PropertyValue into a SpatialOS component update.
void GenerateUnrealToSchemaConversion(
	FCodeWriter& Writer,
	const FString& Update,
	TArray<UProperty*> PropertyChain,
	const FString& PropertyValue,
	const bool bIsUpdate,
	//int32 Handle,
	TFunction<void(const FString&)> ObjectResolveFailureGenerator)
{
	// Get result type.
	UProperty* Property = PropertyChain[PropertyChain.Num() - 1];
	FString SpatialValueSetter = Update + TEXT(".set_") + GetFullyQualifiedName(PropertyChain);

	if (UEnumProperty* EnumProperty = Cast<UEnumProperty>(Property))
	{
		Writer.Printf("// UNSUPPORTED UEnumProperty - %s = %s;", *SpatialValueSetter, *PropertyValue);
		//Writer.Print(FString::Printf(TEXT("auto Underlying = %s.GetValue()"), *PropertyValue));
		//return GenerateUnrealToSchemaConversion(Writer, EnumProperty->GetUnderlyingProperty(), TEXT("Underlying"), ResultName, Handle);
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
			Writer.Printf("%s(improbable::Vector3f(%s.X, %s.Y, %s.Z));", *SpatialValueSetter, *PropertyValue, *PropertyValue, *PropertyValue);
		}
		else if (Struct->GetFName() == NAME_Rotator)
		{
			Writer.Printf("%s(improbable::unreal::UnrealFRotator(%s.Yaw, %s.Pitch, %s.Roll));", *SpatialValueSetter, *PropertyValue, *PropertyValue, *PropertyValue);
		}
		else if (Struct->GetFName() == NAME_Plane)
		{
			Writer.Printf("%s(improbable::unreal::UnrealFPlane(%s.X, %s.Y, %s.Z, %s.W));", *SpatialValueSetter, *PropertyValue, *PropertyValue, *PropertyValue, *PropertyValue);
		}
		else if (Struct->GetName() == TEXT("RepMovement") ||
				 Struct->GetName() == TEXT("UniqueNetIdRepl"))
		{
			Writer.Print("{").Indent();
			Writer.Printf(R"""(
				TArray<uint8> ValueData;
				FMemoryWriter ValueDataWriter(ValueData);
				bool Success;
				%s.NetSerialize(ValueDataWriter, nullptr, Success);
				%s(std::string((char*)ValueData.GetData(), ValueData.Num()));)""", *PropertyValue, *SpatialValueSetter);
			Writer.Outdent().Print("}");
		}
		else
		{
			// If this is not a struct that we handle explicitly, instead recurse in to its properties
			for (TFieldIterator<UProperty> It(Struct); It; ++It)
			{
				TArray<UProperty*> NewChain = PropertyChain;
				NewChain.Add(*It);
				GenerateUnrealToSchemaConversion(Writer, Update, NewChain, PropertyValue + TEXT(".") + (*It)->GetNameCPP(), bIsUpdate, ObjectResolveFailureGenerator);
			}
		}
	}
	else if (Property->IsA(UBoolProperty::StaticClass()))
	{
		Writer.Printf("%s(%s);", *SpatialValueSetter, *PropertyValue);
	}
	else if (Property->IsA(UFloatProperty::StaticClass()))
	{
		Writer.Printf("%s(%s);", *SpatialValueSetter, *PropertyValue);
	}
	else if (Property->IsA(UIntProperty::StaticClass()))
	{
		Writer.Printf("%s(%s);", *SpatialValueSetter, *PropertyValue);
	}
	else if (Property->IsA(UByteProperty::StaticClass()))
	{
		Writer.Printf("%s(uint32_t(%s));", *SpatialValueSetter, *PropertyValue);
	}
	else if (Property->IsA(UClassProperty::StaticClass()))
	{
		// todo David: UClasses are yet to be implemented. 
		// this is above UObjectProperty to make sure it isn't caught there.
		Writer.Print("// UNSUPPORTED UClass");
	}
	else if (Property->IsA(UObjectPropertyBase::StaticClass()))
	{
		Writer.Printf("if (%s != nullptr)", *PropertyValue);
		Writer.Print("{").Indent();
		Writer.Printf(R"""(
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(%s);
			improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
			if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF))""", *PropertyValue);
		Writer.Print("{").Indent();
		ObjectResolveFailureGenerator(*PropertyValue);
		Writer.Outdent().Print("}");
		Writer.Printf(R"""(
			else
			{
				%s(ObjectRef);
			})""", *SpatialValueSetter);
		Writer.Outdent().Print("}");
		Writer.Printf(R"""(
			else
			{
				%s(SpatialConstants::NULL_OBJECT_REF);
			})""", *SpatialValueSetter);
	}
	else if (Property->IsA(UNameProperty::StaticClass()))
	{
		Writer.Printf("%s(TCHAR_TO_UTF8(*%s.ToString()));", *SpatialValueSetter, *PropertyValue);
	}
	else if (Property->IsA(UUInt32Property::StaticClass()))
	{
		Writer.Printf("%s(uint32_t(%s));", *SpatialValueSetter, *PropertyValue);
	}
	else if (Property->IsA(UUInt64Property::StaticClass()))
	{
		Writer.Printf("%s(uint64_t(%s));", *SpatialValueSetter, *PropertyValue);
	}
	else if (Property->IsA(UStrProperty::StaticClass()))
	{
		Writer.Printf("%s(TCHAR_TO_UTF8(*%s));", *SpatialValueSetter, *PropertyValue);
	}
	else
	{
		Writer.Print("// UNSUPPORTED");
	}
}

// Given a 'chained' list of UProperties, this class will generate the code to extract values from flattened schema and apply them to the corresponding Unreal data structure 
void GeneratePropertyToUnrealConversion(
	FCodeWriter& Writer,
	const FString& Update,
	TArray<UProperty*> PropertyChain,
	const FString& PropertyValue,
	const bool bIsUpdate,
	const FString& PropertyType,
	TFunction<void(const FString&)> ObjectResolveFailureGenerator)
{
	FString SpatialValue;

	// This bool is used to differentiate between property updates and command arguments. Unlike command arguments, all property updates are optionals and must be accessed through .data()
	if (bIsUpdate)
	{
		SpatialValue = FString::Printf(TEXT("(*%s.%s().data())"), *Update, *GetFullyQualifiedName(PropertyChain));
	}	
	else
	{
		SpatialValue = FString::Printf(TEXT("%s.%s()"), *Update, *GetFullyQualifiedName(PropertyChain));
	}

	// Get result type.
	UProperty* Property = PropertyChain[PropertyChain.Num() - 1];
	if (UEnumProperty* EnumProperty = Cast<UEnumProperty>(Property))
	{
		Writer.Printf("// UNSUPPORTED (Enum) - %s %s;", *PropertyValue, *SpatialValue);
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
			Writer.Print("{").Indent();
			Writer.Printf("auto& Vector = %s;", *SpatialValue);
			Writer.Printf("%s.X = Vector.x();", *PropertyValue);
			Writer.Printf("%s.Y = Vector.y();", *PropertyValue);
			Writer.Printf("%s.Z = Vector.z();", *PropertyValue);
			Writer.Outdent().Print("}");
		}
		else if (Struct->GetFName() == NAME_Rotator)
		{
			Writer.Print("{").Indent();
			Writer.Printf("auto& Rotator = %s;", *SpatialValue);
			Writer.Printf("%s.Yaw = Rotator.yaw();", *PropertyValue);
			Writer.Printf("%s.Pitch = Rotator.pitch();", *PropertyValue);
			Writer.Printf("%s.Roll = Rotator.roll();", *PropertyValue);
			Writer.Outdent().Print("}");
		}
		else if (Struct->GetFName() == NAME_Plane)
		{
			Writer.Print("{").Indent();
			Writer.Printf("auto& Plane = %s;", *SpatialValue);
			Writer.Printf("%s.X = Plane.x();", *PropertyValue);
			Writer.Printf("%s.Y = Plane.y();", *PropertyValue);
			Writer.Printf("%s.Z = Plane.z();", *PropertyValue);
			Writer.Printf("%s.W = Plane.w();", *PropertyValue);
			Writer.Outdent().Print("}");
		}
		else if (Struct->GetName() == TEXT("RepMovement") ||
				Struct->GetName() == TEXT("UniqueNetIdRepl"))
		{
			Writer.Print("{").Indent();
			Writer.Print(FString::Printf(TEXT(R"""(
				auto& ValueDataStr = %s;
				TArray<uint8> ValueData;
				ValueData.Append((uint8*)ValueDataStr.data(), ValueDataStr.size());
				FMemoryReader ValueDataReader(ValueData);
				bool bSuccess;
				%s.NetSerialize(ValueDataReader, nullptr, bSuccess);)"""), *SpatialValue, *PropertyValue));
			Writer.Outdent().Print("}");
		}
		else
		{
			for (TFieldIterator<UProperty> It(Struct); It; ++It)
			{
				TArray<UProperty*> NewChain = PropertyChain;
				NewChain.Add(*It);
				GeneratePropertyToUnrealConversion(Writer, Update, NewChain, PropertyValue + TEXT(".") + (*It)->GetNameCPP(), bIsUpdate, (*It)->GetCPPType(), ObjectResolveFailureGenerator);
			}
		}
	}
	else if (Property->IsA(UBoolProperty::StaticClass()))
	{
		Writer.Printf("%s = %s;", *PropertyValue, *SpatialValue);
		if (Property->GetCPPType() != TEXT("bool"))
		{
			Writer.Printf("// Because Unreal will look for a specific bit in the serialization function below, we simply set all bits.");
			Writer.Printf("// We will soon move away from using bunches when receiving Spatial updates.");
			Writer.Printf("if (%s)", *PropertyValue);
			Writer.Printf("{").Indent();
			Writer.Printf("%s = 0xFF;", *PropertyValue);
			Writer.Outdent().Print("}");
		}
	}
	else if (Property->IsA(UFloatProperty::StaticClass()))
	{
		Writer.Printf("%s = %s;", *PropertyValue, *SpatialValue);
	}
	else if (Property->IsA(UIntProperty::StaticClass()))
	{
		Writer.Printf("%s = %s;", *PropertyValue, *SpatialValue);
	}
	else if (Property->IsA(UByteProperty::StaticClass()))
	{
		// Byte properties are weird, because they can also be an enum in the form TEnumAsByte<...>. Therefore, the code generator needs to cast to either
		// TEnumAsByte<...> or uint8. However, as TEnumAsByte<...> only has a uint8 constructor, we need to cast the SpatialOS value into uint8 first
		// which causes "uint8(uint8(...))" to be generated for non enum bytes.
		Writer.Printf("%s = %s(uint8(%s));", *PropertyValue, *PropertyType, *SpatialValue);
	}
	else if (Property->IsA(UClassProperty::StaticClass()))
	{
		// todo David: UClasses are yet to be implemented. 
		// this is above UObjectProperty to make sure it isn't caught there.
		Writer.Print("// UNSUPPORTED UClass");
	}
	else if (Property->IsA(UObjectPropertyBase::StaticClass()))
	{
		Writer.Print("{").Indent();
		Writer.Printf(R"""(
			improbable::unreal::UnrealObjectRef ObjectRef = %s;
			check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
			if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
			{
				%s = nullptr;
			})""", *SpatialValue, *PropertyValue);
		Writer.Print("else");
		Writer.Print("{").Indent();
		Writer.Printf(R"""(
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
			if (NetGUID.IsValid())
			{
				%s = static_cast<%s>(PackageMap->GetObjectFromNetGUID(NetGUID, true));
			})""", *PropertyValue, *PropertyType);
		Writer.Print("else");
		Writer.Print("{").Indent();
		ObjectResolveFailureGenerator(*PropertyValue);
		Writer.Outdent().Print("}");
		Writer.Outdent().Print("}");
		Writer.Outdent().Print("}");
	}
	else if (Property->IsA(UNameProperty::StaticClass()))
	{
		Writer.Printf("%s = FName((%s).data());", *PropertyValue, *SpatialValue);
	}
	else if (Property->IsA(UUInt32Property::StaticClass()))
	{
		Writer.Printf("%s = uint32(%s);", *PropertyValue, *SpatialValue);
	}
	else if (Property->IsA(UUInt64Property::StaticClass()))
	{
		Writer.Printf("%s = uint64(%s);", *PropertyValue, *SpatialValue);
	}
	else if (Property->IsA(UStrProperty::StaticClass()))
	{
		Writer.Printf("%s = FString(UTF8_TO_TCHAR(%s.c_str()));", *PropertyValue, *SpatialValue);
	}
	else
	{
		Writer.Print("// UNSUPPORTED");
	}
}

FString GeneratePropertyReader(UProperty* Property)
{
	// This generates the appropriate macro for Unreal to read values from an FFrame. This is the same method as Unreal uses in .generated.h files
	if (Property->IsA(UBoolProperty::StaticClass()))
	{
		return FString::Printf(TEXT("P_GET_UBOOL(%s);"), *Property->GetName());
	}
	else if (Property->IsA(UObjectProperty::StaticClass()))
	{
		return FString::Printf(TEXT("P_GET_OBJECT(%s, %s);"),
			*GetFullCPPName(Cast<UObjectProperty>(Property)->PropertyClass),
			*Property->GetName());
	}
	else if (Property->IsA(UStructProperty::StaticClass()))
	{
		return FString::Printf(TEXT("P_GET_STRUCT(%s, %s)"),
			*Cast<UStructProperty>(Property)->Struct->GetStructCPPName(),
			*Property->GetName());
	}
	return FString::Printf(TEXT("P_GET_PROPERTY(%s, %s);"),
		*GetFullCPPName(Property->GetClass()),
		*Property->GetName());
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

	for (auto Group : GetRPCTypes())
	{
		Layout.RPCs.Add(Group);
	}

	for (TFieldIterator<UFunction> RemoteFunction(Class); RemoteFunction; ++RemoteFunction)
	{
		if (RemoteFunction->FunctionFlags & FUNC_NetClient ||
			RemoteFunction->FunctionFlags & FUNC_NetServer)
		{
			bool bReliable = (RemoteFunction->FunctionFlags & FUNC_NetReliable) != 0;
			Layout.RPCs[GetRPCTypeFromFunction(*RemoteFunction)].Emplace(FRPCDefinition{Class, *RemoteFunction, bReliable});
		}
	}

	// Character: Add CharacterMovementComponent.
	if (Class->GetName() == TEXT("Character"))
	{
		UClass* CharacterMovementComponentClass = FindObject<UClass>(ANY_PACKAGE, TEXT("CharacterMovementComponent"));
		for (TFieldIterator<UFunction> RemoteFunction(CharacterMovementComponentClass); RemoteFunction; ++RemoteFunction)
		{
			if (RemoteFunction->FunctionFlags & FUNC_NetClient ||
				RemoteFunction->FunctionFlags & FUNC_NetServer)
			{
				bool bReliable = (RemoteFunction->FunctionFlags & FUNC_NetReliable) != 0;
				Layout.RPCs[GetRPCTypeFromFunction(*RemoteFunction)].Emplace(FRPCDefinition{CharacterMovementComponentClass, *RemoteFunction, bReliable});
			}
		}
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

int GenerateSchemaFromLayout(FCodeWriter& Writer, int ComponentId, UClass* Class, const FPropertyLayout& Layout)
{
	FComponentIdGenerator IdGenerator(ComponentId);

	Writer.Print(R"""(
		// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
		// Note that this file has been generated automatically
		package improbable.unreal;

		import "improbable/vector3.schema";
		import "unreal/core_types.schema";)""");
	Writer.Print();

	TArray<EReplicatedPropertyGroup> RepPropertyGroups;
	Layout.ReplicatedProperties.GetKeys(RepPropertyGroups);

	// Replicated properties.
	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		Writer.Printf("component %s {", *GetSchemaReplicatedDataName(Group, Class));
		Writer.Indent();
		Writer.Printf("id = %d;", IdGenerator.GetNextAvailableId());
		int FieldCounter = 0;
		for (auto& RepProp : Layout.ReplicatedProperties[Group])
		{
			for (auto& Prop : RepProp.PropertyList)
			{
				FieldCounter++;
				Writer.Printf("%s %s = %d; // %s",
					*RepLayoutTypeToSchemaType(Prop.Type),
					*GetFullyQualifiedName(Prop.Chain),
					FieldCounter,
					*GetLifetimeConditionAsString(RepProp.Entry.Condition)
				);
			}
		}
		Writer.Outdent().Print("}");
	}

	// Complete properties.
	Writer.Printf("component %s {", *GetSchemaCompleteDataName(Class));
	Writer.Indent();
	Writer.Printf("id = %d;", IdGenerator.GetNextAvailableId());

	int FieldCounter = 0;
	for (auto& Prop : Layout.CompleteProperties)
	{
		FieldCounter++;
		Writer.Printf("%s %s = %d;",
			*RepLayoutTypeToSchemaType(Prop.Type),
			*GetFullyQualifiedName(Prop.Chain),
			FieldCounter
		);
	}
	Writer.Outdent().Print("}");

	for (auto Group : GetRPCTypes())
	{
		// Generate schema RPC command types
		for (auto& RPC : (Layout.RPCs[Group]))
		{
			FString TypeStr = GetSchemaRPCRequestTypeFromUnreal(RPC.Function);

			Writer.Printf("type %s {", *TypeStr);
			Writer.Indent();

			// Recurse into class properties and build a complete property list.
			TArray<FPropertyInfo> ParamList;
			for (TFieldIterator<UProperty> It(RPC.Function); It; ++It)
			{
				VisitProperty(ParamList, nullptr, {}, *It);
			}

			// RPC target subobject offset.
			Writer.Printf("uint32 target_subobject_offset = 1;");
			FieldCounter = 1;
			for (auto& Param : ParamList)
			{
				FieldCounter++;
				Writer.Printf("%s %s = %d;",
					*RepLayoutTypeToSchemaType(Param.Type),
					*GetFullyQualifiedName(Param.Chain),
					FieldCounter
				);
			}
			Writer.Outdent().Print("}");

			// RPC responses don't contain any parameters
			Writer.Printf("type %s {}", *GetSchemaRPCResponseTypeFromUnreal(RPC.Function));
		}
	}
	Writer.Print();

	for (auto Group : GetRPCTypes())
	{
		// Generate ClientRPCs component
		Writer.Printf("component %s {", *GetSchemaRPCComponentName(Group, Class));
		Writer.Indent();
		Writer.Printf("id = %i;", IdGenerator.GetNextAvailableId());
		for (auto& RPC : Layout.RPCs[Group])
		{		
			Writer.Printf("command %s %s(%s);",
				*GetSchemaRPCResponseTypeFromUnreal(RPC.Function),
				*PropertyNameToSchemaName(RPC.Function->GetName()),
				*GetSchemaRPCRequestTypeFromUnreal(RPC.Function));
		}
		Writer.Outdent().Print("}");
	}

	return IdGenerator.GetNumUsedIds();
}

void GenerateForwardingCodeFromLayout(
	FCodeWriter& HeaderWriter,
	FCodeWriter& SourceWriter,
	FString SchemaFilename,
	FString InteropFilename,
	UClass* Class,
	const FPropertyLayout& Layout)
{
	TArray<EReplicatedPropertyGroup> RepPropertyGroups;
	Layout.ReplicatedProperties.GetKeys(RepPropertyGroups);

	FString TypeBindingName = FString::Printf(TEXT("USpatialTypeBinding_%s"), *Class->GetName());

	// Create helper function signatures.
	FFunctionSignature BuildComponentUpdateSignature;
	TMap<EReplicatedPropertyGroup, FFunctionSignature> UnrealToSpatialSignatureByGroup;
	TMap<EReplicatedPropertyGroup, FFunctionSignature> SpatialToUnrealSignatureByGroup;

	BuildComponentUpdateSignature.Type = TEXT("void");
	BuildComponentUpdateSignature.Name = TEXT("BuildSpatialComponentUpdate");
	BuildComponentUpdateSignature.Params = TEXT("(\n\tconst FPropertyChangeState& Changes,\n\tUSpatialActorChannel* Channel");
	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		BuildComponentUpdateSignature.Params += FString::Printf(TEXT(",\n\timprobable::unreal::%s::Update& %sUpdate,\n\tbool& b%sUpdateChanged"),
			*GetSchemaReplicatedDataName(Group, Class),
			*GetReplicatedPropertyGroupName(Group),
			*GetReplicatedPropertyGroupName(Group),
			*GetReplicatedPropertyGroupName(Group));
	}
	BuildComponentUpdateSignature.Params += TEXT(") const");
	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		UnrealToSpatialSignatureByGroup.Add(Group, {
			TEXT("void"),
			FString::Printf(TEXT("ServerSendUpdate_%s"), *GetReplicatedPropertyGroupName(Group)),
			FString::Printf(TEXT("(\n\tconst uint8* RESTRICT Data,\n\tint32 Handle,\n\tUProperty* Property,\n\tUSpatialActorChannel* Channel,\n\timprobable::unreal::%s::Update& OutUpdate) const"),
				*GetSchemaReplicatedDataName(Group, Class))
		});
		SpatialToUnrealSignatureByGroup.Add(Group, {
			TEXT("void"),
			FString::Printf(TEXT("ClientReceiveUpdate_%s"), *GetReplicatedPropertyGroupName(Group)),
			FString::Printf(TEXT("(\n\tUSpatialActorChannel* ActorChannel,\n\tconst improbable::unreal::%s::Update& Update) const"),
				*GetSchemaReplicatedDataName(Group, Class))
		});
	}

	// Forwarding code header file.
	// ===========================================
	HeaderWriter.Printf(R"""(
		// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
		// Note that this file has been generated automatically
		#pragma once

		#include <improbable/worker.h>
		#include <improbable/view.h>
		#include <unreal/generated/%s.h>
		#include <unreal/core_types.h>
		#include "SpatialHandlePropertyMap.h"
		#include "SpatialTypeBinding.h"
		#include "SpatialTypeBinding_%s.generated.h")""", *SchemaFilename, *Class->GetName());
	HeaderWriter.Print();

	// Type binding class.
	HeaderWriter.Print("UCLASS()");
	HeaderWriter.Printf("class %s : public USpatialTypeBinding", *TypeBindingName);
	HeaderWriter.Print("{").Indent();
	HeaderWriter.Print("GENERATED_BODY()");
	HeaderWriter.Print();
	HeaderWriter.Outdent().Print("public:").Indent();
	HeaderWriter.Print(R"""(
		static const FRepHandlePropertyMap& GetHandlePropertyMap();

		void Init(USpatialInterop* InInterop, USpatialPackageMapClient* InPackageMap) override;
		void BindToView() override;
		void UnbindFromView() override;
		worker::ComponentId GetReplicatedGroupComponentId(EReplicatedPropertyGroup Group) const override;

		worker::Entity CreateActorEntity(const FString& ClientWorkerId, const FVector& Position, const FString& Metadata, const FPropertyChangeState& InitialChanges, USpatialActorChannel* Channel) const override;
		void SendComponentUpdates(const FPropertyChangeState& Changes, USpatialActorChannel* Channel, const worker::EntityId& EntityId) const override;
		void SendRPCCommand(UObject* TargetObject, const UFunction* const Function, FFrame* const Frame) override;

		void ApplyQueuedStateToChannel(USpatialActorChannel* ActorChannel) override;)""");
	HeaderWriter.Print();
	HeaderWriter.Outdent().Print("private:").Indent();
	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		HeaderWriter.Printf("worker::Dispatcher::CallbackKey %sAddCallback;", *GetReplicatedPropertyGroupName(Group));
		HeaderWriter.Printf("worker::Dispatcher::CallbackKey %sUpdateCallback;", *GetReplicatedPropertyGroupName(Group));
	}
	HeaderWriter.Print();
	HeaderWriter.Print("// Pending updates.");
	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		HeaderWriter.Printf("TMap<worker::EntityId, improbable::unreal::%s::Data> Pending%sData;",
			*GetSchemaReplicatedDataName(Group, Class),
			*GetReplicatedPropertyGroupName(Group));
	}
	HeaderWriter.Print();
	HeaderWriter.Printf(R"""(
		// RPC sender and receiver callbacks.
		using FRPCSender = void (%s::*)(worker::Connection* const, struct FFrame* const, UObject*);
		TMap<FName, FRPCSender> RPCToSenderMap;
		TArray<worker::Dispatcher::CallbackKey> RPCReceiverCallbacks;)""", *TypeBindingName);
	HeaderWriter.Print();
	HeaderWriter.Print("// Component update helper functions.");
	HeaderWriter.Print(*BuildComponentUpdateSignature.Declaration());
	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		HeaderWriter.Print(UnrealToSpatialSignatureByGroup[Group].Declaration());
	}
	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		HeaderWriter.Print(SpatialToUnrealSignatureByGroup[Group].Declaration());
	}

	HeaderWriter.Print();
	HeaderWriter.Print("// RPC sender functions.");
	for (auto Group : GetRPCTypes())
	{
		// Command receiver function signatures
		for (auto& RPC : Layout.RPCs[Group])
		{
			HeaderWriter.Printf("void %s_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);",
				*RPC.Function->GetName());
		}
	}

	HeaderWriter.Print();
	HeaderWriter.Print("// RPC sender response functions.");
	for (auto Group : GetRPCTypes())
	{
		// Command receiver function signatures
		for (auto& RPC : Layout.RPCs[Group])
		{
			HeaderWriter.Printf("void %s_Sender_Response(const worker::CommandResponseOp<improbable::unreal::%s::Commands::%s>& Op);",
				*RPC.Function->GetName(),
				*GetSchemaRPCComponentName(Group, Class),
				*GetCommandNameFromFunction(RPC.Function));
		}
	}

	HeaderWriter.Print();
	HeaderWriter.Print("// RPC receiver functions.");
	for(auto Group : GetRPCTypes())
	{
		// Command receiver function signatures
		for (auto& RPC : Layout.RPCs[Group])
		{
			HeaderWriter.Printf("void %s_Receiver(const worker::CommandRequestOp<improbable::unreal::%s::Commands::%s>& Op);",
				*RPC.Function->GetName(),
				*GetSchemaRPCComponentName(Group, Class),
				*GetCommandNameFromFunction(RPC.Function));
		}
	}
	HeaderWriter.Outdent();
	HeaderWriter.Print("};");

	// Forwarding code source file.
	// ===========================================
	SourceWriter.Printf(R"""(
		// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
		// Note that this file has been generated automatically

		#include "%s.h"
		#include "SpatialOS.h"
		#include "Engine.h"
		#include "SpatialActorChannel.h"
		#include "EntityBuilder.h"
		#include "SpatialPackageMapClient.h"
		#include "SpatialNetDriver.h"
		#include "SpatialConstants.h"
		#include "SpatialInterop.h")""", *InteropFilename);

	// Handle to Property map.
	// ===========================================
	SourceWriter.Print();
	SourceWriter.Printf("const FRepHandlePropertyMap& %s::GetHandlePropertyMap()", *TypeBindingName);
	SourceWriter.Print("{");
	SourceWriter.Indent();
	SourceWriter.Print("static FRepHandlePropertyMap HandleToPropertyMap;");
	SourceWriter.Print("if (HandleToPropertyMap.Num() == 0)");
	SourceWriter.Print("{").Indent();
	SourceWriter.Printf("UClass* Class = FindObject<UClass>(ANY_PACKAGE, TEXT(\"%s\"));", *Class->GetName());

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
			SourceWriter.Printf("HandleToPropertyMap.Add(%d, FRepHandleData{Class->FindPropertyByName(\"%s\"), nullptr, %s});",
				Handle, *RepProp.Entry.Parent->GetName(), *GetLifetimeConditionAsString(RepProp.Entry.Condition));
			SourceWriter.Printf("HandleToPropertyMap[%d].Property = Cast<UStructProperty>(HandleToPropertyMap[%d].Parent)->Struct->FindPropertyByName(\"%s\");",
				Handle, Handle, *RepProp.Entry.Property->GetName());
		}
		else
		{
			SourceWriter.Printf("HandleToPropertyMap.Add(%d, FRepHandleData{nullptr, Class->FindPropertyByName(\"%s\"), %s});",
				Handle, *RepProp.Entry.Property->GetName(), *GetLifetimeConditionAsString(RepProp.Entry.Condition));
		}
	}
	SourceWriter.Outdent().Print("}");
	SourceWriter.Print("return HandleToPropertyMap;");
	SourceWriter.Outdent();
	SourceWriter.Print("}");

	// Init
	// ===========================================
	SourceWriter.Print();
	SourceWriter.Printf("void %s::Init(USpatialInterop* InInterop, USpatialPackageMapClient* InPackageMap)", *TypeBindingName);
	SourceWriter.Print("{");
	SourceWriter.Indent();
	SourceWriter.Print("Super::Init(InInterop, InPackageMap);");
	SourceWriter.Print();
	for (auto Group : GetRPCTypes())
	{
		for (auto& RPC : Layout.RPCs[Group])
		{
			SourceWriter.Printf("RPCToSenderMap.Emplace(\"%s\", &%s::%s_Sender);", *RPC.Function->GetName(), *TypeBindingName, *RPC.Function->GetName());
		}
	}

	SourceWriter.Outdent();
	SourceWriter.Print("}");

	// BindToView
	// ===========================================
	SourceWriter.Print();
	SourceWriter.Printf("void %s::BindToView()", *TypeBindingName);
	SourceWriter.Print("{");
	SourceWriter.Indent();
	SourceWriter.Print("TSharedPtr<worker::View> View = Interop->GetSpatialOS()->GetView().Pin();");
	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		// OnAddComponent.
		SourceWriter.Printf("%sAddCallback = View->OnAddComponent<improbable::unreal::%s>([this](",
			*GetReplicatedPropertyGroupName(Group),
			*GetSchemaReplicatedDataName(Group, Class));
		SourceWriter.Indent();
		SourceWriter.Printf("const worker::AddComponentOp<improbable::unreal::%s>& Op)",
			*GetSchemaReplicatedDataName(Group, Class));
		SourceWriter.Outdent();
		SourceWriter.Print("{");
		SourceWriter.Indent();
		SourceWriter.Printf("auto Update = improbable::unreal::%s::Update::FromInitialData(Op.Data);",
			*GetSchemaReplicatedDataName(Group, Class));
		SourceWriter.Printf(R"""(
			USpatialActorChannel* ActorChannel = Interop->GetClientActorChannel(Op.EntityId);
			if (ActorChannel)
			{
				ClientReceiveUpdate_%s(ActorChannel, Update);
			}
			else
			{
				Pending%sData.Add(Op.EntityId, Op.Data);
			})""",
			*GetReplicatedPropertyGroupName(Group),
			*GetReplicatedPropertyGroupName(Group));
		SourceWriter.Outdent();
		SourceWriter.Print("});");

		// OnComponentUpdate.
		SourceWriter.Printf("%sUpdateCallback = View->OnComponentUpdate<improbable::unreal::%s>([this](",
			*GetReplicatedPropertyGroupName(Group),
			*GetSchemaReplicatedDataName(Group, Class));
		SourceWriter.Indent();
		SourceWriter.Printf("const worker::ComponentUpdateOp<improbable::unreal::%s>& Op)",
			*GetSchemaReplicatedDataName(Group, Class));
		SourceWriter.Outdent();
		SourceWriter.Print("{");
		SourceWriter.Indent();
		SourceWriter.Printf(R"""(
			USpatialActorChannel* ActorChannel = Interop->GetClientActorChannel(Op.EntityId);
			if (ActorChannel)
			{
				ClientReceiveUpdate_%s(ActorChannel, Op.Update);
			}
			else
			{
				Op.Update.ApplyTo(Pending%sData.FindOrAdd(Op.EntityId));
			})""",
			*GetReplicatedPropertyGroupName(Group),
			*GetReplicatedPropertyGroupName(Group));
		SourceWriter.Outdent();
		SourceWriter.Print("});");
	}

	for (auto Group : GetRPCTypes())
	{
		// Ensure that this class contains RPCs of the type specified by group (eg, Server or Client) so that we don't generate code for missing components
		if (Layout.RPCs.Contains(Group) && Layout.RPCs[Group].Num() > 0)
		{
			SourceWriter.Printf("using %sRPCCommandTypes = improbable::unreal::%s::Commands;",
				*GetRPCTypeString(Group),
				*GetSchemaRPCComponentName(Group, Class));
			for (auto& RPC : Layout.RPCs[Group])
			{
				SourceWriter.Printf("RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<%sRPCCommandTypes::%s>(std::bind(&%s::%s_Receiver, this, std::placeholders::_1)));",
					*GetRPCTypeString(Group),
					*GetCommandNameFromFunction(RPC.Function),
					*TypeBindingName,
					*RPC.Function->GetName());
			}
			for (auto& RPC : Layout.RPCs[Group])
			{
				SourceWriter.Printf("RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<%sRPCCommandTypes::%s>(std::bind(&%s::%s_Sender_Response, this, std::placeholders::_1)));",
					*GetRPCTypeString(Group),
					*GetCommandNameFromFunction(RPC.Function),
					*TypeBindingName,
					*RPC.Function->GetName());
			}
		}
	}

	SourceWriter.Outdent();
	SourceWriter.Print("}");

	// UnbindFromView
	// ===========================================
	SourceWriter.Print();
	SourceWriter.Printf("void %s::UnbindFromView()", *TypeBindingName);
	SourceWriter.Print("{");
	SourceWriter.Indent();
	SourceWriter.Print("TSharedPtr<worker::View> View = Interop->GetSpatialOS()->GetView().Pin();");
	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		SourceWriter.Printf("View->Remove(%sAddCallback);", *GetReplicatedPropertyGroupName(Group));
		SourceWriter.Printf("View->Remove(%sUpdateCallback);", *GetReplicatedPropertyGroupName(Group));
	}
	SourceWriter.Print(R"""(
		for (auto& Callback : RPCReceiverCallbacks)
		{
			View->Remove(Callback);
		})""");

	SourceWriter.Outdent();
	SourceWriter.Print("}");

	// GetReplicatedGroupComponentId
	// ===========================================
	SourceWriter.Print();
	SourceWriter.Printf("worker::ComponentId %s::GetReplicatedGroupComponentId(EReplicatedPropertyGroup Group) const", *TypeBindingName);
	SourceWriter.Print("{");
	SourceWriter.Indent();
	SourceWriter.Print("switch (Group)");
	SourceWriter.Print("{");
	SourceWriter.Indent();
	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		SourceWriter.Outdent();
		SourceWriter.Printf("case GROUP_%s:", *GetReplicatedPropertyGroupName(Group));
		SourceWriter.Indent();
		SourceWriter.Printf("return improbable::unreal::%s::ComponentId;", *GetSchemaReplicatedDataName(Group, Class));
	}
	SourceWriter.Outdent().Print("default:").Indent();
	SourceWriter.Print("checkNoEntry();");
	SourceWriter.Print("return 0;");
	SourceWriter.Outdent();
	SourceWriter.Print("}");
	SourceWriter.Outdent();
	SourceWriter.Print("}");

	// CreateActorEntity
	// ===========================================
	SourceWriter.Print();
	SourceWriter.Printf("worker::Entity %s::CreateActorEntity(const FString& ClientWorkerId, const FVector& Position, const FString& Metadata, const FPropertyChangeState& InitialChanges, USpatialActorChannel* Channel) const", *TypeBindingName);
	SourceWriter.Print("{");
	SourceWriter.Indent();
	
	// Set up initial data.
	SourceWriter.Print(TEXT("// Setup initial data."));
	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		SourceWriter.Printf("improbable::unreal::%s::Data %sData;",
			*GetSchemaReplicatedDataName(Group, Class),
			*GetReplicatedPropertyGroupName(Group));
		SourceWriter.Printf("improbable::unreal::%s::Update %sUpdate;",
			*GetSchemaReplicatedDataName(Group, Class),
			*GetReplicatedPropertyGroupName(Group));
		SourceWriter.Printf("bool b%sUpdateChanged = false;", *GetReplicatedPropertyGroupName(Group));
	}
	// Need to introduce scope here as "BuildUpdateArgs" is redefined below.
	{
		TArray<FString> BuildUpdateArgs;
		for (EReplicatedPropertyGroup Group : RepPropertyGroups)
		{
			BuildUpdateArgs.Add(FString::Printf(TEXT("%sUpdate"), *GetReplicatedPropertyGroupName(Group)));
			BuildUpdateArgs.Add(FString::Printf(TEXT("b%sUpdateChanged"), *GetReplicatedPropertyGroupName(Group)));
		}
		SourceWriter.Printf("BuildSpatialComponentUpdate(InitialChanges, Channel, %s);", *FString::Join(BuildUpdateArgs, TEXT(", ")));
	}
	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		SourceWriter.Printf("%sUpdate.ApplyTo(%sData);",
			*GetReplicatedPropertyGroupName(Group),
			*GetReplicatedPropertyGroupName(Group));
	}

	// Create Entity.
	SourceWriter.Print();
	SourceWriter.Print(R"""(
		// Create entity.
		std::string ClientWorkerIdString = TCHAR_TO_UTF8(*ClientWorkerId);

		improbable::WorkerAttributeSet WorkerAttribute{{worker::List<std::string>{"UnrealWorker"}}};
		improbable::WorkerAttributeSet ClientAttribute{{worker::List<std::string>{"UnrealClient"}}};
		improbable::WorkerAttributeSet OwningClientAttribute{{"workerId:" + ClientWorkerIdString}};

		improbable::WorkerRequirementSet WorkersOnly{{WorkerAttribute}};
		improbable::WorkerRequirementSet ClientsOnly{{ClientAttribute}};
		improbable::WorkerRequirementSet OwningClientOnly{{OwningClientAttribute}};
		improbable::WorkerRequirementSet AnyUnrealWorkerOrClient{{WorkerAttribute, ClientAttribute}};
		improbable::WorkerRequirementSet AnyUnrealWorkerOrOwningClient{{WorkerAttribute, OwningClientAttribute}};

		const improbable::Coordinates SpatialPosition = USpatialOSConversionFunctionLibrary::UnrealCoordinatesToSpatialOsCoordinatesCast(Position);)""");
	SourceWriter.Print("return improbable::unreal::FEntityBuilder::Begin()");
	SourceWriter.Indent();
	SourceWriter.Printf(R"""(
		.AddPositionComponent(improbable::Position::Data{SpatialPosition}, WorkersOnly)
		.AddMetadataComponent(improbable::Metadata::Data{TCHAR_TO_UTF8(*Metadata)})
		.SetPersistence(true)
		.SetReadAcl(%s))""",
		Class->GetName() == TEXT("PlayerController") ? TEXT("AnyUnrealWorkerOrOwningClient") : TEXT("AnyUnrealWorkerOrClient"));
	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		SourceWriter.Printf(".AddComponent<improbable::unreal::%s>(%sData, WorkersOnly)",
			*GetSchemaReplicatedDataName(Group, Class), *GetReplicatedPropertyGroupName(Group));
	}
	SourceWriter.Printf(".AddComponent<improbable::unreal::%s>(improbable::unreal::%s::Data{}, WorkersOnly)",
		*GetSchemaCompleteDataName(Class), *GetSchemaCompleteDataName(Class));
	SourceWriter.Printf(".AddComponent<improbable::unreal::%s>(improbable::unreal::%s::Data{}, OwningClientOnly)",
		*GetSchemaRPCComponentName(ERPCType::RPC_Client, Class), *GetSchemaRPCComponentName(ERPCType::RPC_Client, Class));
	SourceWriter.Printf(".AddComponent<improbable::unreal::%s>(improbable::unreal::%s::Data{}, WorkersOnly)",
		*GetSchemaRPCComponentName(ERPCType::RPC_Server, Class), *GetSchemaRPCComponentName(ERPCType::RPC_Server, Class));
	SourceWriter.Print(".Build();");
	SourceWriter.Outdent();
	SourceWriter.Outdent();
	SourceWriter.Print("}");

	// SendComponentUpdates
	// ===========================================
	SourceWriter.Print();
	SourceWriter.Printf("void %s::SendComponentUpdates(const FPropertyChangeState& Changes, USpatialActorChannel* Channel, const worker::EntityId& EntityId) const",
		*TypeBindingName);
	SourceWriter.Print("{");
	SourceWriter.Indent();

	SourceWriter.Print("// Build SpatialOS updates.");
	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		SourceWriter.Printf("improbable::unreal::%s::Update %sUpdate;",
			*GetSchemaReplicatedDataName(Group, Class),
			*GetReplicatedPropertyGroupName(Group));
		SourceWriter.Printf("bool b%sUpdateChanged = false;", *GetReplicatedPropertyGroupName(Group));
	}

	// Need to introduce scope here as "BuildUpdateArgs" is defined above.
	{
		TArray<FString> BuildUpdateArgs;
		for (EReplicatedPropertyGroup Group : RepPropertyGroups)
		{
			BuildUpdateArgs.Add(FString::Printf(TEXT("%sUpdate"), *GetReplicatedPropertyGroupName(Group)));
			BuildUpdateArgs.Add(FString::Printf(TEXT("b%sUpdateChanged"), *GetReplicatedPropertyGroupName(Group)));
		}
		SourceWriter.Printf("BuildSpatialComponentUpdate(Changes, Channel, %s);", *FString::Join(BuildUpdateArgs, TEXT(", ")));
	}

	SourceWriter.Print();
	SourceWriter.Print("// Send SpatialOS updates if anything changed.");
	SourceWriter.Print("TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();");
	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		SourceWriter.Printf(R"""(
			if (b%sUpdateChanged)
			{
				Connection->SendComponentUpdate<improbable::unreal::%s>(EntityId, %sUpdate);
			})""",
			*GetReplicatedPropertyGroupName(Group),
			*GetSchemaReplicatedDataName(Group, Class),
			*GetReplicatedPropertyGroupName(Group));
	}

	SourceWriter.Outdent();
	SourceWriter.Print("}");

	// SendRPCCommand
	// ===========================================
	SourceWriter.Print();
	SourceWriter.Printf("void %s::SendRPCCommand(UObject* TargetObject, const UFunction* const Function, FFrame* const Frame)",
		*TypeBindingName);
	SourceWriter.Print("{");
	SourceWriter.Indent();
	SourceWriter.Print(R"""(
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		auto SenderFuncIterator = RPCToSenderMap.Find(Function->GetFName());
		checkf(*SenderFuncIterator, TEXT("Sender for %s has not been registered with RPCToSenderMap."), *Function->GetFName().ToString());
		(this->*(*SenderFuncIterator))(Connection.Get(), Frame, TargetObject);)""");
	SourceWriter.Outdent().Print("}");

	// ApplyQueuedStateToChannel
	// ==================================
	SourceWriter.Print();
	SourceWriter.Printf("void %s::ApplyQueuedStateToChannel(USpatialActorChannel* ActorChannel)", *TypeBindingName);
	SourceWriter.Print("{");
	SourceWriter.Indent();
	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		SourceWriter.Printf(R"""(
			improbable::unreal::%s::Data* %sData = Pending%sData.Find(ActorChannel->GetEntityId());
			if (%sData)
			{
				auto Update = improbable::unreal::%s::Update::FromInitialData(*%sData);
				Pending%sData.Remove(ActorChannel->GetEntityId());
				ClientReceiveUpdate_%s(ActorChannel, Update);
			})""",
			*GetSchemaReplicatedDataName(Group, Class),
			*GetReplicatedPropertyGroupName(Group),
			*GetReplicatedPropertyGroupName(Group),
			*GetReplicatedPropertyGroupName(Group),
			*GetSchemaReplicatedDataName(Group, Class),
			*GetReplicatedPropertyGroupName(Group),
			*GetReplicatedPropertyGroupName(Group),
			*GetReplicatedPropertyGroupName(Group));
	}
	SourceWriter.Outdent();
	SourceWriter.Print("}");

	// BuildSpatialComponentUpdate
	// ===========================================
	SourceWriter.Print();
	SourceWriter.Print(BuildComponentUpdateSignature.Definition(TypeBindingName));
	SourceWriter.Print("{");
	SourceWriter.Indent();
	SourceWriter.Print(R"""(
		// Build up SpatialOS component updates.
		auto& PropertyMap = GetHandlePropertyMap();
		FChangelistIterator ChangelistIterator(Changes.Changed, 0);
		FRepHandleIterator HandleIterator(ChangelistIterator, Changes.Cmds, Changes.BaseHandleToCmdIndex, 0, 1, 0, Changes.Cmds.Num() - 1);
		while (HandleIterator.NextHandle()))""");
	SourceWriter.Print("{");
	SourceWriter.Indent();
	SourceWriter.Print(R"""(
		const FRepLayoutCmd& Cmd = Changes.Cmds[HandleIterator.CmdIndex];
		const uint8* Data = Changes.SourceData + HandleIterator.ArrayOffset + Cmd.Offset;
		auto& PropertyMapData = PropertyMap[HandleIterator.Handle];
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending property update. actor %s (%lld), property %s (handle %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*Channel->Actor->GetName(),
			Channel->GetEntityId(),
			*Cmd.Property->GetName(),
			HandleIterator.Handle);)""");

	SourceWriter.Print("switch (GetGroupFromCondition(PropertyMapData.Condition))");
	SourceWriter.Print("{");
	SourceWriter.Indent();
	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		SourceWriter.Outdent();
		SourceWriter.Printf("case GROUP_%s:", *GetReplicatedPropertyGroupName(Group));
		SourceWriter.Indent();
		SourceWriter.Printf("ServerSendUpdate_%s(Data, HandleIterator.Handle, Cmd.Property, Channel, %sUpdate);",
			*GetReplicatedPropertyGroupName(Group),
			*GetReplicatedPropertyGroupName(Group));
		SourceWriter.Printf("b%sUpdateChanged = true;",
			*GetReplicatedPropertyGroupName(Group));
		SourceWriter.Print("break;");
	}
	SourceWriter.Outdent();
	SourceWriter.Print("}");
	SourceWriter.Outdent();
	SourceWriter.Print("}");
	SourceWriter.Outdent();
	SourceWriter.Print("}");

	// Unreal to Spatial conversion helper functions.
	// ===========================================
	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		SourceWriter.Print();
		SourceWriter.Print(UnrealToSpatialSignatureByGroup[Group].Definition(TypeBindingName));
		SourceWriter.Print("{");
		SourceWriter.Indent();
		if (Layout.ReplicatedProperties[Group].Num() > 0)
		{
			SourceWriter.Print("switch (Handle)");
			SourceWriter.Print("{").Indent();
			for (auto& RepProp : Layout.ReplicatedProperties[Group])
			{
				auto Handle = RepProp.Entry.Handle;
				UProperty* Property = RepProp.Entry.Property;

				SourceWriter.Printf("case %d: // %s", Handle, *GetFullyQualifiedName(RepProp.Entry.Chain));
				SourceWriter.Print("{");
				SourceWriter.Indent();

				// Get unreal data by deserialising from the reader, convert and set the corresponding field in the update object.
				FString PropertyValueName = TEXT("Value");
				FString PropertyCppType = Property->GetClass()->GetFName().ToString();
				FString PropertyValueCppType = Property->GetCPPType();
				FString PropertyName = TEXT("Property");
				//todo-giray: The reinterpret_cast below is ugly and we believe we can do this more gracefully using Property helper functions.
				if (Property->IsA<UBoolProperty>())
				{
					SourceWriter.Printf("bool %s = static_cast<UBoolProperty*>(Property)->GetPropertyValue(Data);", *PropertyValueName);
				}
				else
				{
					SourceWriter.Printf("%s %s = *(reinterpret_cast<%s const*>(Data));", *PropertyValueCppType, *PropertyValueName, *PropertyValueCppType);
				}
				SourceWriter.Print();
				GenerateUnrealToSchemaConversion(
					SourceWriter, "OutUpdate", RepProp.Entry.Chain, PropertyValueName, true,
					[&SourceWriter, Handle](const FString& PropertyValue)
					{
						SourceWriter.Printf("Interop->AddPendingOutgoingObjectRefUpdate(%s, Channel, %d);", *PropertyValue, Handle);
					});
				SourceWriter.Print("break;");
				SourceWriter.Outdent();
				SourceWriter.Print("}");
			}
			SourceWriter.Outdent().Print("default:");
			SourceWriter.Indent();
			SourceWriter.Print("checkf(false, TEXT(\"Unknown replication handle %d encountered when creating a SpatialOS update.\"));");
			SourceWriter.Print("break;");
			SourceWriter.Outdent();
			SourceWriter.Print("}");
		}
		SourceWriter.Outdent();
		SourceWriter.Print("}");
	}

	// Spatial to Unreal conversion helper functions.
	// ===========================================
	for (EReplicatedPropertyGroup Group : RepPropertyGroups)
	{
		SourceWriter.Print();
		SourceWriter.Print(SpatialToUnrealSignatureByGroup[Group].Definition(TypeBindingName));
		SourceWriter.Print("{");
		SourceWriter.Indent();
		SourceWriter.Printf(R"""(
			FBunchPayloadWriter OutputWriter(PackageMap);

			auto& HandleToPropertyMap = GetHandlePropertyMap();
			const bool bAutonomousProxy = ActorChannel->IsClientAutonomousProxy(improbable::unreal::%s::ComponentId);
			ConditionMapFilter ConditionMap(ActorChannel, bAutonomousProxy);)""",
			*GetSchemaRPCComponentName(ERPCType::RPC_Client, Class));
		for (auto& RepProp : Layout.ReplicatedProperties[Group])
		{
			auto Handle = RepProp.Entry.Handle;
			UProperty* Property = RepProp.Entry.Property;

			// Check if only the first property is in the property list. This implies that the rest is also in the update, as
			// they are sent together atomically.
			SourceWriter.Printf("if (!Update.%s().empty())", *GetFullyQualifiedName(RepProp.PropertyList[0].Chain));
			SourceWriter.Print("{");
			SourceWriter.Indent();

			// Check if the property is relevant.
			SourceWriter.Printf("// %s", *GetFullyQualifiedName(RepProp.Entry.Chain));
			SourceWriter.Printf("uint32 Handle = %d;", Handle);
			SourceWriter.Print("const FRepHandleData& Data = HandleToPropertyMap[Handle];");
			SourceWriter.Print("if (ConditionMap.IsRelevant(Data.Condition))\n{");
			SourceWriter.Indent();

			if (Property->IsA<UObjectPropertyBase>())
			{
				SourceWriter.Print("bool bWriteObjectProperty = true;");
			}

			// Convert update data to the corresponding Unreal type and serialize to OutputWriter.
			FString PropertyValueName = TEXT("Value");
			FString PropertyValueCppType = Property->GetCPPType();
			FString PropertyName = TEXT("Data.Property");
			SourceWriter.Printf("%s %s;", *PropertyValueCppType, *PropertyValueName);
			SourceWriter.Print();
			GeneratePropertyToUnrealConversion(
				SourceWriter, TEXT("Update"), RepProp.Entry.Chain, PropertyValueName, true, PropertyValueCppType,
				[&SourceWriter](const FString& PropertyValue)
				{
					SourceWriter.Print(R"""(
						UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received unresolved object property. Value: (entity: %llu, offset: %u). actor %s (%lld), property %s (handle %d)"),
							*Interop->GetSpatialOS()->GetWorkerId(),
							ObjectRef.entity(),
							ObjectRef.offset(),
							*ActorChannel->Actor->GetName(),
							ActorChannel->GetEntityId(),
							*Data.Property->GetName(),
							Handle);)""");
					SourceWriter.Print("bWriteObjectProperty = false;");
					SourceWriter.Print("Interop->AddPendingIncomingObjectRefUpdate(ObjectRef, ActorChannel, Cast<UObjectPropertyBase>(Data.Property), Handle);");
				});

			// If this is RemoteRole (which will get swapped to Role when the bunch is processed), make sure to downgrade if bAutonomousProxy is false.
			if (Property->GetFName() == NAME_RemoteRole)
			{
				SourceWriter.Print();
				SourceWriter.Print(R"""(
					// Downgrade role from AutonomousProxy to SimulatedProxy if we aren't authoritative over
					// the server RPCs component.
					if (Value == ROLE_AutonomousProxy && !bAutonomousProxy)
					{
						Value = ROLE_SimulatedProxy;
					})""");
			}

			SourceWriter.Print();

			if (Property->IsA<UObjectPropertyBase>())
			{
				SourceWriter.Print("if (bWriteObjectProperty)");
				SourceWriter.Print("{").Indent();
			}

			SourceWriter.Printf("OutputWriter.SerializeProperty(Handle, %s, &%s);", *PropertyName, *PropertyValueName);
			SourceWriter.Print(R"""(
				UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%lld), property %s (handle %d)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*ActorChannel->Actor->GetName(),
					ActorChannel->GetEntityId(),
					*Data.Property->GetName(),
					Handle);)""");

			if (Property->IsA<UObjectPropertyBase>())
			{
				SourceWriter.Outdent().Print("}");
			}

			// End condition map check block.
			SourceWriter.Outdent();
			SourceWriter.Print("}");

			// End property block.
			SourceWriter.Outdent();
			SourceWriter.Print("}");
		}
		SourceWriter.Print("Interop->ReceiveSpatialUpdate(ActorChannel, OutputWriter.GetNetBitWriter());");
		SourceWriter.Outdent();
		SourceWriter.Print("}");
	}

	// RPC sender functions.
	// ===========================================
	for (auto Group : GetRPCTypes())
	{
		for (auto& RPC : Layout.RPCs[Group])
		{
			SourceWriter.Print();
			SourceWriter.Printf("void %s::%s_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)",
				*TypeBindingName,
				*RPC.Function->GetName());
			SourceWriter.Print("{").Indent();

			// Extract RPC arguments from the stack.
			if (RPC.Function->NumParms > 0)
			{
				// Note that macros returned by GeneratePropertyReader require this FFrame variable to be named "Stack"
				SourceWriter.Print("FFrame& Stack = *RPCFrame;");
				for (TFieldIterator<UProperty> Param(RPC.Function); Param; ++Param)
				{
					SourceWriter.Print(*GeneratePropertyReader(*Param));
				}
				SourceWriter.Print();
			}

			// Build closure to send the command request.
			TArray<FString> CapturedArguments;
			CapturedArguments.Add(TEXT("TargetObject"));
			for (TFieldIterator<UProperty> Param(RPC.Function); Param; ++Param)
			{
				CapturedArguments.Add((*Param)->GetName());
			}
			SourceWriter.Printf("auto Sender = [this, Connection, %s]() mutable -> FRPCRequestResult", *FString::Join(CapturedArguments, TEXT(", ")));
			SourceWriter.Print("{").Indent();
			SourceWriter.Printf(R"""(
				// Resolve TargetObject.
				improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
				if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
				{
					UE_LOG(LogSpatialOSInterop, Log, TEXT("%%s: RPC %s queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
					return FRPCRequestResult{TargetObject};
				})""", *RPC.Function->GetName());
			SourceWriter.Print();
			SourceWriter.Print("// Build request.");
			SourceWriter.Printf("improbable::unreal::%s Request;", *GetSchemaRPCRequestTypeFromUnreal(RPC.Function));
			for (TFieldIterator<UProperty> Param(RPC.Function); Param; ++Param)
			{
				TArray<UProperty*> NewChain = {*Param};
				GenerateUnrealToSchemaConversion(
					SourceWriter, "Request", NewChain, *Param->GetNameCPP(), false,
					[&SourceWriter, &RPC](const FString& PropertyValue)
					{
						SourceWriter.Printf("UE_LOG(LogSpatialOSInterop, Log, TEXT(\"%%s: RPC %s queued. %s is unresolved.\"), *Interop->GetSpatialOS()->GetWorkerId());",
							*RPC.Function->GetName(),
							*PropertyValue);
						SourceWriter.Printf("return FRPCRequestResult{%s};", *PropertyValue);
					});
			}
			SourceWriter.Print();
			SourceWriter.Printf(R"""(
				// Send command request.
				Request.set_target_subobject_offset(TargetObjectRef.offset());
				UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%%s: Sending RPC: %s, target: %%s (entity ID %%lld, offset: %%d)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*TargetObject->GetName(),
					TargetObjectRef.entity(),
					TargetObjectRef.offset());
				auto RequestId = Connection->SendCommandRequest<improbable::unreal::%s::Commands::%s>(TargetObjectRef.entity(), Request, 0);
				return FRPCRequestResult{RequestId.Id};)""",
				*RPC.Function->GetName(),
				*GetSchemaRPCComponentName(Group, Class),
				*GetCommandNameFromFunction(RPC.Function));
			SourceWriter.Outdent().Print("};");
			SourceWriter.Printf("Interop->SendCommandRequest(Sender, %s);", RPC.bReliable ? TEXT("/*bReliable*/ true") : TEXT("/*bReliable*/ false"));
			SourceWriter.Outdent().Print("}");
		}
	}

	// RPC sender response functions.
	// ===========================================
	for (auto Group : GetRPCTypes())
	{
		for (auto& RPC : Layout.RPCs[Group])
		{
			SourceWriter.Print();
			SourceWriter.Printf("void %s::%s_Sender_Response(const worker::CommandResponseOp<improbable::unreal::%s::Commands::%s>& Op)",
				*TypeBindingName,
				*RPC.Function->GetName(),
				*GetSchemaRPCComponentName(Group, Class),
				*GetCommandNameFromFunction(RPC.Function));
			SourceWriter.Print("{").Indent();
			SourceWriter.Printf("Interop->HandleCommandResponse(TEXT(\"%s\"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));",
				*RPC.Function->GetName());
			SourceWriter.Outdent().Print("}");
		}
	}

	// RPC receiver functions.
	// ===========================================
	for (auto Group : GetRPCTypes())
	{
		for (auto& RPC : Layout.RPCs[Group])
		{
			SourceWriter.Print();
			SourceWriter.Printf("void %s::%s_Receiver(const worker::CommandRequestOp<improbable::unreal::%s::Commands::%s>& Op)",
				*TypeBindingName,
				*RPC.Function->GetName(),
				*GetSchemaRPCComponentName(Group, Class),
				*GetCommandNameFromFunction(RPC.Function));
			SourceWriter.Print("{");
			SourceWriter.Indent();

			auto ObjectResolveFailureGenerator = [&SourceWriter, &RPC, Group, Class](const FString& PropertyName, const FString& ObjectRef)
			{
				SourceWriter.Printf(R"""(
					UE_LOG(LogSpatialOSInterop, Log, TEXT("%%s: %s_Receiver: %s (entity id %%lld, offset %%d) is not resolved on this worker. Sending command failure."),
						*Interop->GetSpatialOS()->GetWorkerId(),
						%s.entity(),
						%s.offset());
					SendRPCResponse<improbable::unreal::%s::Commands::%s>(Op, false, TEXT("%s is unresolved on the target worker"));
					return;)""",
					*RPC.Function->GetName(),
					*PropertyName,
					*ObjectRef,
					*ObjectRef,
					*GetSchemaRPCComponentName(Group, Class),
					*GetCommandNameFromFunction(RPC.Function),
					*PropertyName);
			};

			// Get the target object.
			SourceWriter.Printf(R"""(
				improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
				FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
				if (!TargetNetGUID.IsValid()))""");
			SourceWriter.Print("{").Indent();
			ObjectResolveFailureGenerator("Target object", "TargetObjectRef");
			SourceWriter.Outdent().Print("}");
			SourceWriter.Printf(R"""(
				UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
				%s* TargetObject = Cast<%s>(TargetObjectUntyped);
				checkf(TargetObjectUntyped, TEXT("%%s: %s_Receiver: Object Ref (entity: %%llu, offset: %%u) (NetGUID %%s) does not correspond to a UObject."),
					*Interop->GetSpatialOS()->GetWorkerId(),
					TargetObjectRef.entity(),
					TargetObjectRef.offset(),
					*TargetNetGUID.ToString());
				checkf(TargetObject, TEXT("%%s: %s_Receiver: Object Ref (entity: %%llu, offset: %%u) (NetGUID %%s) is the wrong type. Name: %%s"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					TargetObjectRef.entity(),
					TargetObjectRef.offset(),
					*TargetNetGUID.ToString(),
					*TargetObjectUntyped->GetName());)""",
				*GetFullCPPName(RPC.CallerType),
				*GetFullCPPName(RPC.CallerType),
				*RPC.Function->GetName(),
				*RPC.Function->GetName());

			// Grab RPC arguments.
			TArray<FString> RPCParameters;
			for (TFieldIterator<UProperty> Param(RPC.Function); Param; ++Param)
			{
				FString PropertyValueName = Param->GetNameCPP();
				FString PropertyValueCppType = Param->GetCPPType();
				FString PropertyName = TEXT("Data.Property");

				// Extract parameter.
				SourceWriter.Print();
				SourceWriter.Printf("// Extract %s", *Param->GetName());
				SourceWriter.Printf("%s %s;", *PropertyValueCppType, *PropertyValueName);
				GeneratePropertyToUnrealConversion(
					SourceWriter, "Op.Request", {*Param}, PropertyValueName, false, PropertyValueCppType,
					std::bind(ObjectResolveFailureGenerator, std::placeholders::_1, "ObjectRef"));

				// Append to parameter list.
				RPCParameters.Add(Param->GetNameCPP());
			}
			SourceWriter.Print();
			SourceWriter.Print("// Call implementation and send command response.");
			SourceWriter.Printf(R"""(
				UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%%s: Received RPC: %s, target: %%s (entity: %%llu, offset: %%u)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*TargetObject->GetName(),
					TargetObjectRef.entity(),
					TargetObjectRef.offset());)""",
				*RPC.Function->GetName());
			SourceWriter.Printf("TargetObject->%s_Implementation(%s);",
				*RPC.Function->GetName(), *FString::Join(RPCParameters, TEXT(", ")));
			SourceWriter.Printf("SendRPCResponse<improbable::unreal::%s::Commands::%s>(Op, true, FString());",
				*GetSchemaRPCComponentName(Group, Class),
				*GetCommandNameFromFunction(RPC.Function));
			SourceWriter.Outdent().Print(TEXT("}"));
		}
	}
}

int GenerateCompleteSchemaFromClass(const FString& SchemaPath, const FString& ForwardingCodePath, int ComponentId, UClass* Class)
{
	FCodeWriter OutputSchema;
	FCodeWriter OutputHeader;
	FCodeWriter OutputSource;

	FString SchemaFilename = FString::Printf(TEXT("Unreal%s"), *Class->GetName());
	FString TypeBindingFilename = FString::Printf(TEXT("SpatialTypeBinding_%s"), *Class->GetName());

	FPropertyLayout Layout = CreatePropertyLayout(Class);

	// Generate schema.
	int NumComponents = GenerateSchemaFromLayout(OutputSchema, ComponentId, Class, Layout);
	OutputSchema.WriteToFile(FString::Printf(TEXT("%s%s.schema"), *SchemaPath, *SchemaFilename));

	// Generate forwarding code.
	GenerateForwardingCodeFromLayout(OutputHeader, OutputSource, SchemaFilename, TypeBindingFilename, Class, Layout);
	OutputHeader.WriteToFile(FString::Printf(TEXT("%s%s.h"), *ForwardingCodePath, *TypeBindingFilename));
	OutputSource.WriteToFile(FString::Printf(TEXT("%s%s.cpp"), *ForwardingCodePath, *TypeBindingFilename));
	return NumComponents;
}
} // ::

int32 UGenerateInteropCodeCommandlet::Main(const FString& Params)
{
	FString CombinedSchemaPath = FPaths::Combine(*FPaths::GetPath(FPaths::GetProjectFilePath()), TEXT("../../../schema/unreal/generated/"));
	FString CombinedForwardingCodePath = FPaths::Combine(*FPaths::GetPath(FPaths::GetProjectFilePath()), TEXT("../../../workers/unreal/Game/Source/NUF/Generated/"));
	UE_LOG(LogTemp, Display, TEXT("Schema path %s - Forwarding code path %s"), *CombinedSchemaPath, *CombinedForwardingCodePath);

	TArray<FString> Classes = {TEXT("Character"), TEXT("PlayerController"), TEXT("PlayerState"), TEXT("GameStateBase")};
	if (FPaths::CollapseRelativeDirectories(CombinedSchemaPath) && FPaths::CollapseRelativeDirectories(CombinedForwardingCodePath))
	{
		int ComponentId = 100000;
		for (auto& ClassName : Classes)
		{
			UClass* Class = FindObject<UClass>(ANY_PACKAGE, *ClassName);
			ComponentId += GenerateCompleteSchemaFromClass(CombinedSchemaPath, CombinedForwardingCodePath, ComponentId, Class);
		}
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Path was invalid - schema not generated"));
	}

	return 0;
}
