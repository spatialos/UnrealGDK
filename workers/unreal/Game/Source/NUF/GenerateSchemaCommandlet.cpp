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
	struct PropertyInfo
	{
		UProperty* Property;
		ERepLayoutCmdType Type;
		// Properties that were traversed to reach this property, including the property itself.
		TArray<UProperty*> Chain;

		bool operator==(const PropertyInfo& Other) const
		{
			return Property == Other.Property && Type == Other.Type && Chain == Other.Chain;
		}
	};

	struct RepLayoutEntry
	{
		UProperty* Property;
		UProperty* Parent;
		TArray<UProperty*> Chain;
		ERepLayoutCmdType Type;
		int32 Handle;
		int32 Offset;
	};

	struct RepLayoutPropertyInfo
	{
		RepLayoutEntry Entry;
		// Usually a singleton list containing the PropertyInfo this RepLayoutEntry refers to.
		// In some cases (such as a struct within a struct), this can refer to many properties in schema.
		TArray<PropertyInfo> PropertyList;
	};

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

	FString SchemaHeader(const FString& PackageName)
	{
		return FString::Printf(TEXT(R"""(package %s;
		import "improbable/vector3.schema";
		type UnrealFRotator { float pitch = 1; float yaw = 2; float roll = 3; }
		type UnrealFPlane { float x = 1; float y = 2; float z = 3; float w = 4; }
		type UnrealObjectRef { EntityId entity = 1; uint32 offset = 2; })"""), *PackageName);
	}

	FString GetUnrealCompleteTypeName(UStruct* Type)
	{
		FString Prefix;
		if (Type->IsChildOf<AActor>())
		{
			// Actor.
			Prefix = TEXT("A");
		}
		else if (Type->IsChildOf<UObject>())
		{
			// Object.
			Prefix = TEXT("U");
		}
		else
		{
			// Struct.
			Prefix = TEXT("F");
		}
		return Prefix + Type->GetName();
	}

	FString GetSchemaReplicatedTypeFromUnreal(UStruct* Type)
	{
		return TEXT("Unreal") + Type->GetName() + TEXT("Replicated");
	}

	FString GetSchemaCompleteTypeFromUnreal(UStruct* Type)
	{
		return TEXT("Unreal") + Type->GetName() + TEXT("Complete");
	}

	FString GetSchemaReplicatedComponentFromUnreal(UStruct* Type)
	{
		return GetSchemaReplicatedTypeFromUnreal(Type) + TEXT("Data");
	}

	FString GetSchemaCompleteDataComponentFromUnreal(UStruct* Type)
	{
		return GetSchemaCompleteTypeFromUnreal(Type) + TEXT("Data");
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
	void VisitProperty(TArray<PropertyInfo>& PropertyInfo, UObject* CDO, TArray<UProperty*> Stack, UProperty* Property)
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
			Writer.Print(FString::Printf(TEXT("// WEAK OBJECT REPLICATION - %s = %s;"), *SpatialValueSetter, *PropertyValue));
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
			Writer.Print(FString::Printf(TEXT("// UNSUPPORTED ObjectProperty - %s %s;"), *PropertyValue, *SpatialValue));
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

	void GenerateCompleteSchemaFromClass(const FString& SchemaPath, const FString& ForwardingCodePath, UClass* Class)
	{
		FCodeWriter OutputSchema;
		FCodeWriter OutputForwardingCode;
		FCodeWriter OutputForwardingCodeHeader;

		// Read the RepLayout for the class into a data structure.
		FRepLayout RepLayout;
		RepLayout.InitFromObjectClass(Class);
		TArray<TPair<int, RepLayoutEntry>> RepLayoutProperties;
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

			int32 Handle = CmdIndex + 1;
			RepLayoutProperties.Add(MakeTuple(Handle, RepLayoutEntry{
				Property, ParentProperty, PropertyChain, (ERepLayoutCmdType)Cmd.Type, Handle, Cmd.Offset
				}));
		}

		// Recurse into class properties and build a complete property list.
		TArray<PropertyInfo> Properties;
		for (TFieldIterator<UProperty> It(Class); It; ++It)
		{
			VisitProperty(Properties, Class->GetDefaultObject(), {}, *It);
		}

		// Divide properties into replicated and complete properties.
		TArray<RepLayoutPropertyInfo> ReplicatedProperties;
		TArray<PropertyInfo> CompleteProperties = Properties;
		TArray<PropertyInfo> CompletePropertiesToRemove;
		for (auto& Pair : RepLayoutProperties)
		{
			RepLayoutEntry& Entry = Pair.Value;
			RepLayoutPropertyInfo Info{ Entry,{} };

			// Search for all properties with a property chain that begins with either {Parent, Property} or {Property}.
			Info.PropertyList = CompleteProperties.FilterByPredicate([Entry](const PropertyInfo& Property)
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
			CompleteProperties.Remove(PropertyToRemove);
		}

		// Schema.
		OutputSchema.Print(SchemaHeader(TEXT("improbable.unreal")));
		OutputSchema.Print(FString::Printf(TEXT("type %s {"), *GetSchemaReplicatedTypeFromUnreal(Class)));
		OutputSchema.Indent();
		int FieldCounter = 0;
		for (auto& RepProp : ReplicatedProperties)
		{
			for (auto& Prop : RepProp.PropertyList)
			{
				FieldCounter++;
				OutputSchema.Print(
					FString::Printf(
						TEXT("%s %s = %d;"),
						*RepLayoutTypeToSchemaType(Prop.Type),
						*GetFullyQualifiedName(Prop.Chain),
						FieldCounter
					)
				);
			}
		}
		OutputSchema.Outdent().Print(TEXT("}"));
		OutputSchema.Print(FString::Printf(TEXT("type %s {"), *GetSchemaCompleteTypeFromUnreal(Class)));
		OutputSchema.Indent();
		FieldCounter = 0;
		for (auto& Prop : CompleteProperties)
		{
			FieldCounter++;
			OutputSchema.Print(
				FString::Printf(
					TEXT("%s %s = %d;"),
					*RepLayoutTypeToSchemaType(Prop.Type),
					*GetFullyQualifiedName(Prop.Chain),
					FieldCounter
				)
			);
		}
		OutputSchema.Outdent().Print(TEXT("}"));

		FString SchemaFile = FString::Printf(TEXT("Unreal%s"), *Class->GetName());
		FString UpdateInteropFile = FString::Printf(TEXT("SpatialUpdateInterop_%s"), *Class->GetName());

		// Components.
		OutputSchema.Print(FString::Printf(TEXT("component %s {"), *GetSchemaReplicatedComponentFromUnreal(Class)));
		OutputSchema.Indent();
		OutputSchema.Print(TEXT("id = 100000;"));
		OutputSchema.Print(FString::Printf(TEXT("data %s;"), *GetSchemaReplicatedTypeFromUnreal(Class)));
		OutputSchema.Outdent().Print(TEXT("}"));
		OutputSchema.Print(FString::Printf(TEXT("component %s {"), *GetSchemaCompleteDataComponentFromUnreal(Class)));
		OutputSchema.Indent();
		OutputSchema.Print(TEXT("id = 100001;"));
		OutputSchema.Print(FString::Printf(TEXT("data %s;"), *GetSchemaCompleteTypeFromUnreal(Class)));
		OutputSchema.Outdent().Print(TEXT("}"));
		OutputSchema.WriteToFile(FString::Printf(TEXT("%s%s.schema"), *SchemaPath, *SchemaFile));

		// Forwarding code function signatures.
		FString HandlePropertyMapReturnType = TEXT("const RepHandlePropertyMap&");
		FString HandlePropertyMapSignature = FString::Printf(
			TEXT("GetHandlePropertyMap_%s()"),
			*Class->GetName());
		FString UnrealToSpatialReturnType = TEXT("void");
		FString UnrealToSpatialSignature = FString::Printf(
			TEXT("ApplyUpdateToSpatial_%s(FArchive& Reader, int32 Handle, UProperty* Property, improbable::unreal::%s::Update& Update)"),
			*Class->GetName(),
			*GetSchemaReplicatedComponentFromUnreal(Class));
		FString SpatialToUnrealReturnType = TEXT("void");
		FString SpatialToUnrealSignature = FString::Printf(
			TEXT("ReceiveUpdateFromSpatial_%s(USpatialActorChannel* ActorChannel, const improbable::unreal::%s::Update& Update)"),
			*Class->GetName(),
			*GetSchemaReplicatedComponentFromUnreal(Class));

		// Forwarding code header file.
		OutputForwardingCodeHeader.Print(TEXT("// Copyright (c) Improbable Worlds Ltd, All Rights Reserved"));
		OutputForwardingCodeHeader.Print(TEXT("// Note that this file has been generated automatically"));
		OutputForwardingCodeHeader.Print();
		OutputForwardingCodeHeader.Print(TEXT("#pragma once"));
		OutputForwardingCodeHeader.Print();
		OutputForwardingCodeHeader.Print(FString::Printf(TEXT("#include <generated/%s.h>"), *SchemaFile));
		OutputForwardingCodeHeader.Print(TEXT("#include \"SpatialHandlePropertyMap.h\""));
		OutputForwardingCodeHeader.Print();
		OutputForwardingCodeHeader.Print(TEXT("class USpatialActorChannel;"));
		OutputForwardingCodeHeader.Print();
		OutputForwardingCodeHeader.Print(FString::Printf(TEXT("%s %s;"), *HandlePropertyMapReturnType, *HandlePropertyMapSignature));
		OutputForwardingCodeHeader.Print(FString::Printf(TEXT("%s %s;"), *UnrealToSpatialReturnType, *UnrealToSpatialSignature));
		OutputForwardingCodeHeader.Print(FString::Printf(TEXT("%s %s;"), *SpatialToUnrealReturnType, *SpatialToUnrealSignature));
		OutputForwardingCodeHeader.WriteToFile(ForwardingCodePath + FString::Printf(TEXT("%s.h"), *UpdateInteropFile));

		// Forwarding code source file.
		OutputForwardingCode.Print(TEXT("// Copyright (c) Improbable Worlds Ltd, All Rights Reserved"));
		OutputForwardingCode.Print(TEXT("// Note that this file has been generated automatically"));
		OutputForwardingCode.Print();
		OutputForwardingCode.Print(FString::Printf(TEXT(R"""(#include "%s.h"
		#include "CoreMinimal.h"
		#include "GameFramework/Character.h"
		#include "Serialization/MemoryReader.h"
		#include "Serialization/MemoryWriter.h"
		#include "SpatialActorChannel.h")"""), *UpdateInteropFile));

		// Handle to Property map.
		OutputForwardingCode.Print();
		OutputForwardingCode.Print(FString::Printf(TEXT("%s %s"), *HandlePropertyMapReturnType, *HandlePropertyMapSignature));
		OutputForwardingCode.Print(TEXT("{"));
		OutputForwardingCode.Indent();
		OutputForwardingCode.Print(FString::Printf(TEXT("UClass* Class = %s::StaticClass();"), *GetUnrealCompleteTypeName(Class)));
		OutputForwardingCode.Print(TEXT("static RepHandlePropertyMap* HandleToPropertyMapData = nullptr;"));
		OutputForwardingCode.Print(TEXT("if (HandleToPropertyMapData == nullptr)"));
		OutputForwardingCode.Print(TEXT("{")).Indent();
		OutputForwardingCode.Print(TEXT("HandleToPropertyMapData = new RepHandlePropertyMap();"));
		OutputForwardingCode.Print(TEXT("auto& HandleToPropertyMap = *HandleToPropertyMapData;"));
		for (auto& RepProp : ReplicatedProperties)
		{
			auto Handle = RepProp.Entry.Handle;
			if (RepProp.Entry.Parent)
			{
				OutputForwardingCode.Print(FString::Printf(TEXT("HandleToPropertyMap.Add(%d, RepHandleData{Class->FindPropertyByName(\"%s\"), nullptr, %d});"),
					Handle, *RepProp.Entry.Parent->GetName(), RepProp.Entry.Offset));
				OutputForwardingCode.Print(FString::Printf(TEXT("HandleToPropertyMap[%d].Property = Cast<UStructProperty>(HandleToPropertyMap[%d].Parent)->Struct->FindPropertyByName(\"%s\");"),
					Handle, Handle, *RepProp.Entry.Property->GetName()));
			}
			else
			{
				OutputForwardingCode.Print(FString::Printf(TEXT("HandleToPropertyMap.Add(%d, RepHandleData{nullptr, Class->FindPropertyByName(\"%s\"), %d});"),
					Handle, *RepProp.Entry.Property->GetName(), RepProp.Entry.Offset));
			}
		}
		OutputForwardingCode.Outdent().Print(TEXT("}"));
		OutputForwardingCode.Print(TEXT("return *HandleToPropertyMapData;"));
		OutputForwardingCode.Outdent();
		OutputForwardingCode.Print(TEXT("}"));

		// Unreal -> Spatial (replicated)
		OutputForwardingCode.Print();
		OutputForwardingCode.Print(FString::Printf(TEXT("%s %s"), *UnrealToSpatialReturnType, *UnrealToSpatialSignature));
		OutputForwardingCode.Print(TEXT("{"));
		OutputForwardingCode.Indent();
		OutputForwardingCode.Print(TEXT("switch (Handle)\n{"));
		OutputForwardingCode.Indent();
		for (auto& RepProp : ReplicatedProperties)
		{
			auto Handle = RepProp.Entry.Handle;
			UProperty* Property = RepProp.Entry.Property;

			if (Property->IsA(UObjectPropertyBase::StaticClass()))
			{
				OutputForwardingCode.Print(FString::Printf(TEXT("// case %d: - %s is an object reference, skipping."), Handle, *Property->GetName()));
				continue;
			}

			OutputForwardingCode.Print(FString::Printf(TEXT("case %d: // %s"), Handle, *GetFullyQualifiedName(RepProp.Entry.Chain)));
			OutputForwardingCode.Print(TEXT("{"));
			OutputForwardingCode.Indent();

			// Get unreal data by deserialising from the reader, convert and set the corresponding field in the update object.
			FString PropertyValueName = TEXT("Value");
			FString PropertyValueCppType = Property->GetCPPType();
			FString PropertyName = TEXT("Property");
			OutputForwardingCode.Print(FString::Printf(TEXT("%s %s;"), *PropertyValueCppType, *PropertyValueName));
			OutputForwardingCode.Print(FString::Printf(TEXT("check(%s->ElementSize == sizeof(%s));"), *PropertyName, *PropertyValueName));
			OutputForwardingCode.Print(FString::Printf(TEXT("%s->NetSerializeItem(Reader, nullptr, &%s);"), *PropertyName, *PropertyValueName));
			OutputForwardingCode.Print();
			GenerateUnrealToSchemaConversion(OutputForwardingCode, TEXT("Update"), RepProp.Entry.Chain, PropertyValueName);
			OutputForwardingCode.Print(TEXT("break;"));
			OutputForwardingCode.Outdent();
			OutputForwardingCode.Print(TEXT("}"));
		}
		OutputForwardingCode.Outdent();
		OutputForwardingCode.Print(TEXT("}"));
		OutputForwardingCode.Outdent();
		OutputForwardingCode.Print(TEXT("}"));

		// Spatial -> Unreal.
		OutputForwardingCode.Print();
		OutputForwardingCode.Print(FString::Printf(TEXT("%s %s"), *SpatialToUnrealReturnType, *SpatialToUnrealSignature));
		OutputForwardingCode.Print(TEXT("{"));
		OutputForwardingCode.Indent();
		OutputForwardingCode.Print(TEXT("FNetBitWriter OutputWriter(nullptr, 0); "));
		OutputForwardingCode.Print(FString::Printf(TEXT("auto& HandleToPropertyMap = GetHandlePropertyMap_%s();"), *Class->GetName()));
		for (auto& RepProp : ReplicatedProperties)
		{
			auto Handle = RepProp.Entry.Handle;
			UProperty* Property = RepProp.Entry.Property;

			// Check if only the first property is in the property list. This implies that the rest is also in the update, as
			// they are sent together atomically.
			OutputForwardingCode.Print(FString::Printf(TEXT("if (!Update.%s().empty())\n{"), *GetFullyQualifiedName(RepProp.PropertyList[0].Chain)));
			OutputForwardingCode.Indent();

			// Write handle.
			OutputForwardingCode.Print(FString::Printf(TEXT("// %s"), *GetFullyQualifiedName(RepProp.Entry.Chain)));
			OutputForwardingCode.Print(FString::Printf(TEXT("uint32 Handle = %d;"), Handle));
			OutputForwardingCode.Print(TEXT("OutputWriter.SerializeIntPacked(Handle);"));
			OutputForwardingCode.Print(TEXT("const RepHandleData& Data = HandleToPropertyMap[Handle];"));
			OutputForwardingCode.Print();

			// Convert update data to the corresponding Unreal type and serialize to OutputWriter.
			FString PropertyValueName = TEXT("Value");
			FString PropertyValueCppType = Property->GetCPPType();
			FString PropertyName = TEXT("Data.Property");
			OutputForwardingCode.Print(FString::Printf(TEXT("%s %s;"), *PropertyValueCppType, *PropertyValueName));
			OutputForwardingCode.Print(FString::Printf(TEXT("check(%s->ElementSize == sizeof(%s));"), *PropertyName, *PropertyValueName));
			OutputForwardingCode.Print();
			GenerateSchemaToUnrealConversion(OutputForwardingCode, TEXT("Update"), RepProp.Entry.Chain, PropertyValueName, PropertyValueCppType);
			OutputForwardingCode.Print();
			OutputForwardingCode.Print(FString::Printf(TEXT("%s->NetSerializeItem(OutputWriter, nullptr, &%s);"), *PropertyName, *PropertyValueName));
			OutputForwardingCode.Print(TEXT("UE_LOG(LogTemp, Log, TEXT(\"<- Handle: %d Property %s\"), Handle, *Data.Property->GetName());"));
			OutputForwardingCode.Outdent();
			OutputForwardingCode.Print(TEXT("}"));
		}
		OutputForwardingCode.Print(TEXT("ActorChannel->SpatialReceivePropertyUpdate(OutputWriter);"));
		OutputForwardingCode.Outdent();
		OutputForwardingCode.Print(TEXT("}"));

		OutputForwardingCode.WriteToFile(ForwardingCodePath + FString::Printf(TEXT("%s.cpp"), *UpdateInteropFile));
	}
} // ::

UGenerateSchemaCommandlet::UGenerateSchemaCommandlet()
{
}

UGenerateSchemaCommandlet::~UGenerateSchemaCommandlet()
{
}

int32 UGenerateSchemaCommandlet::Main(const FString& Params)
{
	FString CombinedSchemaPath =
		FPaths::Combine(*FPaths::GetPath(FPaths::GetProjectFilePath()), TEXT("../../../schema/generated/"));
	FString CombinedForwardingCodePath =
		FPaths::Combine(*FPaths::GetPath(FPaths::GetProjectFilePath()), TEXT("../../../workers/unreal/Game/Source/NUF/Generated/"));
	UE_LOG(LogTemp, Display, TEXT("Schema path %s - Forwarding code path %s"), *CombinedSchemaPath, *CombinedForwardingCodePath);
	if (FPaths::CollapseRelativeDirectories(CombinedSchemaPath) && FPaths::CollapseRelativeDirectories(CombinedForwardingCodePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("================================================================================="));
		GenerateCompleteSchemaFromClass(CombinedSchemaPath, CombinedForwardingCodePath, ACharacter::StaticClass());
		UE_LOG(LogTemp, Warning, TEXT("================================================================================="));
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Path was invalid - schema not generated"));
	}

	return 0;
}
