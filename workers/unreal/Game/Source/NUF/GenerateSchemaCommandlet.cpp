#include "GenerateSchemaCommandlet.h"

// For GenerateSchemaFromClass
#include "Net/DataReplication.h"
#include "Net/RepLayout.h"
#include "GameFramework/Character.h"
#include "Misc/FileHelper.h"
#include "Components/ArrowComponent.h"

CodeWriter::CodeWriter() : Scope(0) 
{
}

CodeWriter& CodeWriter::Print() {
	OutputSource += TEXT("\n");
	return *this;
}

CodeWriter& CodeWriter::Print(const FString& String) {
	TArray<FString> Lines;
	String.ParseIntoArray(Lines, TEXT("\n"), false);
	for (auto& Line : Lines) {
		FString ScopeIdent;
		for (int ScopeLevel = 0; ScopeLevel < Scope; ++ScopeLevel) {
			ScopeIdent += FString(TEXT("\t"));
		}
		OutputSource += ScopeIdent + Line + TEXT("\n");
	}
	return *this;
}

void CodeWriter::WriteToFile(const FString& Filename) {
	check(Scope == 0);
	FFileHelper::SaveStringToFile(OutputSource, *Filename);
}

void CodeWriter::Dump() {
	UE_LOG(LogTemp, Warning, TEXT("%s"), *OutputSource);
}

CodeWriter& CodeWriter::Indent() {
	Scope++;
	return *this;
}

CodeWriter& CodeWriter::Outdent() {
	check(Scope > 0);
	Scope--;
	return *this;
}

namespace {
FString PropertySchemaName(UProperty* Property) {
	FString FullPath = Property->GetFullGroupName(false);
	FullPath.ReplaceInline(TEXT("."), TEXT("_"));
	FullPath.ReplaceInline(SUBOBJECT_DELIMITER, TEXT("_"));
	FullPath.ToLowerInline();
	return FullPath;
}

FString PropertyGeneratedName(UProperty* Property) {
	FString SchemaName = PropertySchemaName(Property);
	SchemaName[0] = FChar::ToUpper(SchemaName[0]);
	return SchemaName;
}

FString PropertyGeneratedName(const FString& SchemaName) {
	TArray<FString> ScopeNames;
	SchemaName.ParseIntoArray(ScopeNames, TEXT("_"), false);
	FString GeneratedName;
	for (auto Scope : ScopeNames) {
		Scope[0] = FChar::ToUpper(Scope[0]);
		GeneratedName += Scope;
	}
	return GeneratedName;
}

ERepLayoutCmdType PropertyToRepLayoutType(UProperty* Property) {
	UProperty * UnderlyingProperty = Property;
	if (UEnumProperty * EnumProperty = Cast< UEnumProperty >(Property)) {
		UnderlyingProperty = EnumProperty->GetUnderlyingProperty();
	}

	// Try to special case to custom types we know about
	if (UnderlyingProperty->IsA(UStructProperty::StaticClass())) {
		UStructProperty * StructProp = Cast< UStructProperty >(UnderlyingProperty);
		UScriptStruct * Struct = StructProp->Struct;
		if (Struct->GetFName() == NAME_Vector) {
			return REPCMD_PropertyVector;
		} else if (Struct->GetFName() == NAME_Rotator) {
			return REPCMD_PropertyRotator;
		} else if (Struct->GetFName() == NAME_Plane) {
			return  REPCMD_PropertyPlane;
		} else if (Struct->GetName() == TEXT("Vector_NetQuantize100")) {
			return REPCMD_PropertyVector100;
		} else if (Struct->GetName() == TEXT("Vector_NetQuantize10")) {
			return REPCMD_PropertyVector10;
		} else if (Struct->GetName() == TEXT("Vector_NetQuantizeNormal")) {
			return REPCMD_PropertyVectorNormal;
		} else if (Struct->GetName() == TEXT("Vector_NetQuantize")) {
			return REPCMD_PropertyVectorQ;
		} else if (Struct->GetName() == TEXT("UniqueNetIdRepl")) {
			return REPCMD_PropertyNetId;
		} else if (Struct->GetName() == TEXT("RepMovement")) {
			return REPCMD_RepMovement;
		} else {
			return REPCMD_Property;
		}
	} else if (UnderlyingProperty->IsA(UBoolProperty::StaticClass())) {
		return REPCMD_PropertyBool;
	} else if (UnderlyingProperty->IsA(UFloatProperty::StaticClass())) {
		return REPCMD_PropertyFloat;
	} else if (UnderlyingProperty->IsA(UIntProperty::StaticClass())) {
		return REPCMD_PropertyInt;
	} else if (UnderlyingProperty->IsA(UByteProperty::StaticClass())) {
		return REPCMD_PropertyByte;
	} else if (UnderlyingProperty->IsA(UObjectPropertyBase::StaticClass())) {
		return REPCMD_PropertyObject;
	} else if (UnderlyingProperty->IsA(UNameProperty::StaticClass())) {
		return REPCMD_PropertyName;
	} else if (UnderlyingProperty->IsA(UUInt32Property::StaticClass())) {
		return REPCMD_PropertyUInt32;
	} else if (UnderlyingProperty->IsA(UUInt64Property::StaticClass())) {
		return REPCMD_PropertyUInt64;
	} else if (UnderlyingProperty->IsA(UStrProperty::StaticClass())) {
		return REPCMD_PropertyString;
	} else {
		return REPCMD_Property;
	}
}

FString RepLayoutTypeToSchemaType(ERepLayoutCmdType Type) {
	FString DataType;
	switch (Type) {
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

FString SchemaHeader(const FString& PackageName) {
	return
		FString::Printf(TEXT("package %s;\n"), *PackageName) +
		TEXT("import \"improbable/vector3.schema\";\n")
		TEXT("type UnrealFRotator { float pitch = 1; float yaw = 2; float roll = 3; }\n")
		TEXT("type UnrealFPlane { float x = 1; float y = 2; float z = 3; float w = 4; }\n")
		TEXT("type UnrealObjectRef { EntityId entity = 1; uint32 offset = 2; }");
}

FString GetUnrealCompleteTypeName(UStruct* Type) {
    FString Prefix;
    if (Type->IsChildOf<AActor>()) {
        // Actor.
        Prefix = TEXT("A");
    } else if (Type->IsChildOf<UObject>()) {
        // Object.
        Prefix = TEXT("U");
    } else {
        // Struct.
        Prefix = TEXT("F");
    }
    return Prefix + Type->GetName();
}

FString GetSchemaReplicatedTypeFromUnreal(UStruct* Type) {
    return TEXT("Unreal") + GetUnrealCompleteTypeName(Type) + TEXT("Replicated");
}

FString GetSchemaCompleteTypeFromUnreal(UStruct* Type) {
    return TEXT("Unreal") + GetUnrealCompleteTypeName(Type) + TEXT("Complete");
}

FString GetSchemaReplicatedComponentFromUnreal(UStruct* Type) {
    return GetSchemaReplicatedTypeFromUnreal(Type) + TEXT("Data");
}

FString GetSchemaCompleteDataComponentFromUnreal(UStruct* Type) {
	return GetSchemaCompleteTypeFromUnreal(Type) + TEXT("Data");
}

FString GetFullyQualifiedName(TArray<UProperty*> Chain) {
	TArray<FString> ChainNames;
	for (auto Prop : Chain) {
		ChainNames.Add(Prop->GetName().ToLower());
	}
	// Prefix is required to disambiguate between properties in the generated code and UActorComponent/UObject properties
	// which the generated code extends :troll:.
	return TEXT("field_") + FString::Join(ChainNames, TEXT("_"));
}

FString GetFullyQualifiedCppName(TArray<UProperty*> Chain) {
	TArray<FString> ChainNames;
	for (auto Prop : Chain) {
		FString Name = Prop->GetName().ToLower();
		Name[0] = FChar::ToUpper(Name[0]);
		ChainNames.Add(Name);
	}
	// Prefix is required to disambiguate between properties in the generated code and UActorComponent/UObject properties
	// which the generated code extends :troll:.
	return TEXT("Field") + FString::Join(ChainNames, TEXT(""));
}

FString GetUnrealFieldLvalue(TArray<UProperty*> Chain) {
	TArray<FString> ChainNames;
	for (auto Prop : Chain) {
		ChainNames.Add(Prop->GetName());
	}
	return TEXT("this->") + FString::Join(ChainNames, TEXT("->"));
}

struct PropertyInfo {
	UProperty* Property;
	ERepLayoutCmdType Type;
	// Properties that were traversed to reach this property, including the property itself.
	TArray<UProperty*> Chain;
};

// CDO - Class default object which contains Property
void VisitProperty(TArray<PropertyInfo>& PropertyInfo, UObject* CDO, TArray<UProperty*> Stack, UProperty* Property) {
	// Skip properties that make no sense to store in SpatialOS.
	if (Property->IsA<UMulticastDelegateProperty>()) {
		UE_LOG(LogTemp, Warning, TEXT("%s - multicast delegate, skipping"), *Property->GetName());
		return;
	}
	if (Property->GetPropertyFlags() & CPF_Transient && !(Property->GetPropertyFlags() & CPF_Net)) {
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
		(PropertyType == REPCMD_PropertyObject && Property->IsA<UObjectProperty>())) {
		// Get struct/class of property value.
		UStruct* PropertyValueStruct;
		if (PropertyType != REPCMD_PropertyObject) { // UStruct property
			PropertyValueStruct = Cast<UStructProperty>(Property)->Struct;
		} else { // UObject property
			UObjectProperty* ObjectProperty = Cast<UObjectProperty>(Property);
			UClass* PropertyValueClass = nullptr;

			// Obtain the class of the UObject property value.
			if (CDO) {
				// If we have the CDO, resolve the actual type pointed to by the pointer (to deal with polymorphism).
				UObject* Value = ObjectProperty->GetPropertyValue_InContainer(CDO);
				if (Value) {
					// If this is an editor-only property, skip it.
					if (Value->IsEditorOnly()) {
						UE_LOG(LogTemp, Warning, TEXT("%s - editor only, skipping"), *Property->GetName());
						return;
					}

					// Make sure the owner of the property value is the CDO, otherwise this is a weak reference.
					if (Value->GetOuter() == CDO) {
						UE_LOG(LogTemp, Warning, TEXT("Property Class: %s Instance Class: %s"), *ObjectProperty->PropertyClass->GetName(), *Value->GetClass()->GetName());
						PropertyValueClass = Value->GetClass();
					} else {
						// The values outer is not us, store as weak reference.
						UE_LOG(LogTemp, Warning, TEXT("%s - %s weak reference (outer not this)"), *Property->GetName(), *ObjectProperty->PropertyClass->GetName());
					}
				} else {
					// If value is not set, then we clearly don't own it.
					UE_LOG(LogTemp, Warning, TEXT("%s - %s weak reference (null init)"), *Property->GetName(), *ObjectProperty->PropertyClass->GetName());
				}
			} else {
				// If we don't have a CDO, then this is a struct with a UObject property. In this case, the Struct will never be the owner.
				UE_LOG(LogTemp, Warning, TEXT("%s - %s weak reference (object inside struct)"), *Property->GetName(), *ObjectProperty->PropertyClass->GetName());
			}

			// Skip this property if it make no sense to store in SpatialOS.
			if (PropertyValueClass) {
				if (PropertyValueClass->IsChildOf<UClass>()) {
					return;
				}
				if (!(PropertyValueClass->GetClassFlags() & CLASS_DefaultToInstanced)) {
					UE_LOG(LogTemp, Warning, TEXT("%s - %s not instanced, skipping"), *Property->GetName(), *PropertyValueClass->GetName());
					return;
				}
				if (PropertyValueClass->IsChildOf(FTickFunction::StaticStruct())) {
					return;
				}
			}

			// Upcast property value type to struct.
			PropertyValueStruct = PropertyValueClass;
		}

		// If PropertyValueStruct is not nullptr, this property is an owning reference to the value.
		if (PropertyValueStruct) {
			// Instantiate CDO of this struct if it's a class.
			UObject* PropertyValueClassCDO = nullptr;
			if (Cast<UClass>(PropertyValueStruct) != nullptr) {
				PropertyValueClassCDO = Cast<UClass>(PropertyValueStruct)->GetDefaultObject();
			}

			// Recurse into properties.
			TArray<UProperty*> NewStack(Stack);
			NewStack.Add(Property);
			for (TFieldIterator<UProperty> It(PropertyValueStruct); It; ++It) {
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

void GenerateUnpackedStructUnrealToSchemaConversion(CodeWriter& Writer, TArray<UProperty*> PropertyChain, UStruct* Struct) {
}

// Returns the output expression to assign to the schema value.
void GenerateUnrealToSchemaConversion(CodeWriter& Writer, const FString& ReplicatedData, TArray<UProperty*> PropertyChain, const FString& PropertyValue) {
	// Get result type.
	UProperty* Property = PropertyChain[PropertyChain.Num() - 1];
	FString SchemaPropertyName = ReplicatedData + TEXT("->") + GetFullyQualifiedCppName(PropertyChain);

	if (UEnumProperty* EnumProperty = Cast<UEnumProperty>(Property)) {
		Writer.Print(TEXT("// UNSUPPORTED"));
		//Writer.Print(FString::Printf(TEXT("auto Underlying = %s.GetValue()"), *PropertyValue));
		//return GenerateUnrealToSchemaConversion(Writer, EnumProperty->GetUnderlyingProperty(), TEXT("Underlying"), ResultName);
	}

	// Try to special case to custom types we know about
	if (Property->IsA(UStructProperty::StaticClass())) {
		UStructProperty * StructProp = Cast<UStructProperty>(Property);
		UScriptStruct * Struct = StructProp->Struct;
		if (Struct->GetFName() == NAME_Vector) {
			Writer.Print(FString::Printf(TEXT("%s = %s;"), *SchemaPropertyName, *PropertyValue));
		} else if (Struct->GetFName() == NAME_Rotator) {
			FString ResultName = TEXT("Result");
			Writer.Print(FString::Printf(TEXT("auto& Rotator = %s;"), *SchemaPropertyName));
			Writer.Print(FString::Printf(TEXT("Rotator->SetPitch(%s.Pitch);"), *PropertyValue));
			Writer.Print(FString::Printf(TEXT("Rotator->SetYaw(%s.Yaw);"), *PropertyValue));
			Writer.Print(FString::Printf(TEXT("Rotator->SetRoll(%s.Roll);"), *PropertyValue));
		} else if (Struct->GetFName() == NAME_Plane) {
			Writer.Print(FString::Printf(TEXT("auto& Plane = %s;"), *SchemaPropertyName));
			Writer.Print(FString::Printf(TEXT("Plane->SetX(%s.X);"), *PropertyValue));
			Writer.Print(FString::Printf(TEXT("Plane->SetY(%s.Y);"), *PropertyValue));
			Writer.Print(FString::Printf(TEXT("Plane->SetZ(%s.Z);"), *PropertyValue));
			Writer.Print(FString::Printf(TEXT("Plane->SetW(%s.W);"), *PropertyValue));
		} else if (Struct->GetName() == TEXT("Vector_NetQuantize100")) {
			Writer.Print(FString::Printf(TEXT("%s = %s;"), *SchemaPropertyName, *PropertyValue));
		} else if (Struct->GetName() == TEXT("Vector_NetQuantize10")) {
			Writer.Print(FString::Printf(TEXT("%s = %s;"), *SchemaPropertyName, *PropertyValue));
		} else if (Struct->GetName() == TEXT("Vector_NetQuantizeNormal")) {
			Writer.Print(FString::Printf(TEXT("%s = %s;"), *SchemaPropertyName, *PropertyValue));
		} else if (Struct->GetName() == TEXT("Vector_NetQuantize")) {
			Writer.Print(FString::Printf(TEXT("%s = %s;"), *SchemaPropertyName, *PropertyValue));
		} else if (Struct->GetName() == TEXT("UniqueNetIdRepl")) {
			Writer.Print(TEXT("// UNSUPPORTED"));
		} else if (Struct->GetName() == TEXT("RepMovement")) {
			Writer.Print(FString::Printf(TEXT("TArray<uint8> Data;\nFMemoryWriter Writer(Data);\nbool Success;\n%s.NetSerialize(Writer, nullptr, Success);"), *PropertyValue));
			Writer.Print(FString::Printf(TEXT("%s = FBase64::Encode(Data);"), *SchemaPropertyName));
		} else {
			for (TFieldIterator<UProperty> It(Struct); It; ++It) {
				Writer.Print(TEXT("{")).Indent();
				TArray<UProperty*> NewChain = PropertyChain;
				NewChain.Add(*It);
				//GenerateUnrealToSchemaConversion(Writer, ReplicatedData, NewChain, PropertyValue + TEXT(".") + (*It)->GetNameCPP());
				Writer.Outdent().Print(TEXT("}"));
			}
		}
	} else if (Property->IsA(UBoolProperty::StaticClass())) {
		Writer.Print(FString::Printf(TEXT("%s = %s != 0;"), *SchemaPropertyName, *PropertyValue));
	} else if (Property->IsA(UFloatProperty::StaticClass())) {
		Writer.Print(FString::Printf(TEXT("%s = %s;"), *SchemaPropertyName, *PropertyValue));
	} else if (Property->IsA(UIntProperty::StaticClass())) {
		Writer.Print(FString::Printf(TEXT("%s = %s;"), *SchemaPropertyName, *PropertyValue));
	} else if (Property->IsA(UByteProperty::StaticClass())) {
		Writer.Print(FString::Printf(TEXT("%s = int(%s);"), *SchemaPropertyName, *PropertyValue));
	} else if (Property->IsA(UObjectPropertyBase::StaticClass())) {
		Writer.Print(FString::Printf(TEXT("auto UObjectRef = NewObject<UUnrealObjectRef>();\nUObjectRef->SetEntity(FEntityId((int64(PackageMap->GetNetGUIDFromObject(%s).Value))));\n%s = UObjectRef;"), *PropertyValue, *SchemaPropertyName));
	} else if (Property->IsA(UNameProperty::StaticClass())) {
		Writer.Print(FString::Printf(TEXT("%s = %s.ToString();"), *SchemaPropertyName, *PropertyValue));
	} else if (Property->IsA(UUInt32Property::StaticClass())) {
		Writer.Print(FString::Printf(TEXT("%s = int(%s);"), *SchemaPropertyName, *PropertyValue));
	} else if (Property->IsA(UUInt64Property::StaticClass())) {
		Writer.Print(FString::Printf(TEXT("%s = int(%s);"), *SchemaPropertyName, *PropertyValue));
	} else if (Property->IsA(UStrProperty::StaticClass())) {
		Writer.Print(FString::Printf(TEXT("// UNSUPPORTED - %s = %s;"), *SchemaPropertyName, *PropertyValue));
	} else {
		Writer.Print(TEXT("// UNSUPPORTED"));
	}
}

void GenerateCompleteSchemaFromClass(const FString& SchemaPath, const FString& ForwardingCodePath, UClass* Class) {
	CodeWriter OutputSchema;
	CodeWriter OutputForwardingCode;
	CodeWriter OutputForwardingCodeHeader;

    // Parse RepLayout.
    FRepLayout RepLayout;
    RepLayout.InitFromObjectClass(Class);
    TArray<TPair<int, PropertyInfo>> RepLayoutProperties;
    for (int cmdIndex = 0; cmdIndex < RepLayout.Cmds.Num(); ++cmdIndex) {
        if (RepLayout.Cmds[cmdIndex].Type == REPCMD_Return || RepLayout.Cmds[cmdIndex].Type == REPCMD_DynamicArray)
            continue;

        if (RepLayout.Cmds[cmdIndex].Property == nullptr)
            continue;

        // Get property and parent property from RepLayout.
        UProperty* Property = RepLayout.Cmds[cmdIndex].Property;
        UProperty* ParentProperty = RepLayout.Parents[RepLayout.Cmds[cmdIndex].ParentIndex].Property;
        if (ParentProperty == Property) {
            ParentProperty = nullptr;
        }
        TArray<UProperty*> PropertyChain;
        if (ParentProperty) {
            PropertyChain = { ParentProperty, Property };
        } else {
            PropertyChain = { Property };
        }

        RepLayoutProperties.Add(MakeTuple(cmdIndex, PropertyInfo{
            Property, (ERepLayoutCmdType)RepLayout.Cmds[cmdIndex].Type, PropertyChain
        }));
    }

	// Recurse into class properties.
	TArray<PropertyInfo> Properties;
	for (TFieldIterator<UProperty> It(Class); It; ++It) {
		VisitProperty(Properties, Class->GetDefaultObject(), {}, *It);
	}

    // Divide properties into replicated and non-replicated properties.
    TArray<PropertyInfo> ReplicatedProperties, CompleteProperties;
    for (auto& RepLayoutProperty : RepLayoutProperties) {
        ReplicatedProperties.Add(RepLayoutProperty.Value);
    }
    for (auto& Property : Properties) {
        // Skip properties which are already marked as replicated.
        for (auto& ReplicatedProperty : ReplicatedProperties) {
            if (ReplicatedProperty.Chain == Property.Chain) {
                continue;
            }
        }
        CompleteProperties.Add(Property);
    }

	// Schema.
	OutputSchema.Print(SchemaHeader(TEXT("improbable.unreal")));
    OutputSchema.Print(FString::Printf(TEXT("type %s {"), *GetSchemaReplicatedTypeFromUnreal(Class)));
    OutputSchema.Indent();
    int FieldCounter = 0;
    for (auto& Prop : ReplicatedProperties) {
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
    OutputSchema.Print(FString::Printf(TEXT("type %s {"), *GetSchemaCompleteTypeFromUnreal(Class)));
    OutputSchema.Indent();
    for (auto& Prop : CompleteProperties) {
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
	OutputSchema.WriteToFile(SchemaPath + TEXT("UnrealNative.schema"));

	// Forwarding code function signatures.
	FString CmdIndexToPropertyMapSignature = FString::Printf(
		TEXT("TMap<int, TPair<UProperty*, UProperty*>> CreateCmdIndexToPropertyMap_%s()"),
		*Class->GetName());
	FString ForwardingFunctionSignature = FString::Printf(
		TEXT("void ApplyUpdateToSpatial_%s(AActor* Actor, int CmdIndex, UProperty* ParentProperty, UProperty* Property, U%sComponent* ReplicatedData, USpatialPackageMapClient* PackageMap)"),
		*Class->GetName(),
		*GetSchemaReplicatedComponentFromUnreal(Class));
	FString SpatialToUnrealSignature = FString::Printf(
		TEXT("void ReceiveUpdateFromSpatial_%s(AActor* Actor, TMap<int, TPair<UProperty*, UProperty*>>& CmdIndexToPropertyMap, U%sComponentUpdate* Update)"),
		*Class->GetName(),
		*GetSchemaReplicatedComponentFromUnreal(Class));

	// Forwarding code header file.
	OutputForwardingCodeHeader.Print(TEXT("#include \"GameFramework/Actor.h\""));
	OutputForwardingCodeHeader.Print(FString::Printf(TEXT("#include \"%sComponent.h\""), *GetSchemaReplicatedComponentFromUnreal(Class)));
	OutputForwardingCodeHeader.Print();
	OutputForwardingCodeHeader.Print(CmdIndexToPropertyMapSignature + TEXT(";"));
	OutputForwardingCodeHeader.Print();
	OutputForwardingCodeHeader.Print(ForwardingFunctionSignature + TEXT(";"));
	OutputForwardingCodeHeader.Print();
	OutputForwardingCodeHeader.Print(SpatialToUnrealSignature + TEXT(";"));
	OutputForwardingCodeHeader.WriteToFile(ForwardingCodePath + FString::Printf(TEXT("SpatialInterop%s.h"), *Class->GetName()));

	// Forwarding code source file.
	OutputForwardingCode.Print(FString::Printf(TEXT("#include \"SpatialInterop%s.h\""), *Class->GetName()));
	OutputForwardingCode.Print(TEXT("#include \"CoreMinimal.h\""));
	OutputForwardingCode.Print(TEXT("#include \"Misc/Base64.h\""));
	OutputForwardingCode.Print(TEXT("#include \"Engine/PackageMapClient.h\""));
	OutputForwardingCode.Print();

	// Cmd Index to Property map.
	OutputForwardingCode.Print(CmdIndexToPropertyMapSignature);
	OutputForwardingCode.Print(TEXT("{"));
	OutputForwardingCode.Indent();
	OutputForwardingCode.Print(FString::Printf(TEXT("UClass* Class = %s::StaticClass();"), *GetUnrealCompleteTypeName(Class)));
	OutputForwardingCode.Print(TEXT("TMap<int, TPair<UProperty*, UProperty*>> Properties;"));
	for (auto& RepLayoutPair : RepLayoutProperties) {
		auto CmdIndex = RepLayoutPair.Key;
		PropertyInfo& PropertyInfo = RepLayoutPair.Value;
		// Parent case.
		if (PropertyInfo.Chain.Num() > 1) {
			OutputForwardingCode.Print(FString::Printf(TEXT("Properties[%d] = MakeTuple<UProperty*, UProperty*>(Class->FindPropertyByName(\"%s\"), nullptr);"),
				CmdIndex, *PropertyInfo.Chain[0]->GetName()));
			OutputForwardingCode.Print(FString::Printf(TEXT("Properties[%d].Value = Cast<UStructProperty>(Properties[%d].Key)->Struct->FindPropertyByName(\"%s\");"),
				CmdIndex, CmdIndex, *PropertyInfo.Chain[1]->GetName()));
		} else {
			OutputForwardingCode.Print(FString::Printf(TEXT("Properties[%d] = MakeTuple<UProperty*, UProperty*>(nullptr, Class->FindPropertyByName(\"%s\"));"),
				CmdIndex, *PropertyInfo.Property->GetName()));
		}
	}
	OutputForwardingCode.Print(TEXT("return Properties;"));
	OutputForwardingCode.Outdent();
	OutputForwardingCode.Print(TEXT("}"));
	OutputForwardingCode.Print();

	// Unreal -> Spatial.
	OutputForwardingCode.Print(ForwardingFunctionSignature);
	OutputForwardingCode.Print(TEXT("{"));
	OutputForwardingCode.Indent();
	OutputForwardingCode.Print(TEXT("UObject* Container = Actor;"));
	OutputForwardingCode.Print(TEXT("switch (CmdIndex)\n{"));
	OutputForwardingCode.Indent();
	for (auto& RepLayoutPair : RepLayoutProperties) {
		auto CmdIndex = RepLayoutPair.Key;
		PropertyInfo& PropertyInfo = RepLayoutPair.Value;

		// Output conversion code.
		OutputForwardingCode.Print(FString::Printf(TEXT("case %d:"), CmdIndex));
		OutputForwardingCode.Print(TEXT("{"));
		OutputForwardingCode.Indent();

		// Value expression.
		FString Container = TEXT("Container");
		FString PropertyValueName = TEXT("Value");
		FString FieldValueName = PropertyValueName;
		FString PropertyValueCppType = PropertyInfo.Property->GetCPPType();
		FString PropertyName = TEXT("Property");
		if (PropertyInfo.Chain.Num() > 1) {
			PropertyName = TEXT("ParentProperty");
			PropertyValueCppType = PropertyInfo.Chain[0]->GetCPPType();
			FieldValueName += TEXT(".") + PropertyInfo.Property->GetNameCPP();
		}
		OutputForwardingCode.Print(FString::Printf(TEXT("auto& %s = *%s->ContainerPtrToValuePtr<%s>(%s);"), *PropertyValueName, *PropertyName, *PropertyValueCppType, *Container));

		// Schema conversion.
		GenerateUnrealToSchemaConversion(OutputForwardingCode, TEXT("ReplicatedData"), PropertyInfo.Chain, FieldValueName);

		OutputForwardingCode.Print(TEXT("break;"));
		OutputForwardingCode.Outdent();
		OutputForwardingCode.Print(TEXT("}"));
	}
	OutputForwardingCode.Outdent();
	OutputForwardingCode.Print(TEXT("}"));
	OutputForwardingCode.Outdent();
	OutputForwardingCode.Print(TEXT("}"));

	// Spatial -> Unreal.
	OutputForwardingCode.Print(SpatialToUnrealSignature);
	OutputForwardingCode.Print(TEXT("{"));
	OutputForwardingCode.Indent();
	OutputForwardingCode.Print(TEXT("UObject* Container = Actor;"));
	for (auto& RepLayoutPair : RepLayoutProperties) {
		auto CmdIndex = RepLayoutPair.Key;
		PropertyInfo& PropertyInfo = RepLayoutPair.Value;

		OutputForwardingCode.Print(FString::Printf(TEXT("if (Update->Has%s())\n{"), *GetFullyQualifiedCppName(PropertyInfo.Chain)));
		OutputForwardingCode.Indent();
		OutputForwardingCode.Print(FString::Printf(TEXT("UProperty* ParentProperty = CmdIndexToPropertyMap[%d].Key;"), CmdIndex));
		OutputForwardingCode.Print(FString::Printf(TEXT("UProperty* Property = CmdIndexToPropertyMap[%d].Value;"), CmdIndex));

		// Value expression.
		FString Container = TEXT("Container");
		FString PropertyValueName = TEXT("Value");
		FString FieldValueName = PropertyValueName;
		FString PropertyValueCppType = PropertyInfo.Property->GetCPPType();
		FString PropertyName = TEXT("Property");
		if (PropertyInfo.Chain.Num() > 1) {
			PropertyName = TEXT("ParentProperty");
			PropertyValueCppType = PropertyInfo.Chain[0]->GetCPPType();
			FieldValueName += TEXT(".") + PropertyInfo.Property->GetNameCPP();
		}
		OutputForwardingCode.Print(FString::Printf(TEXT("auto& %s = *%s->ContainerPtrToValuePtr<%s>(%s);"), *PropertyValueName, *PropertyName, *PropertyValueCppType, *Container));

		// Schema -> Unreal conversion.
		//GenerateUnrealToSchemaConversion(OutputForwardingCode, TEXT("ReplicatedData"), PropertyInfo.Chain, FieldValueName);

		OutputForwardingCode.Outdent();
		OutputForwardingCode.Print(TEXT("}"));
	}
	OutputForwardingCode.Outdent();
	OutputForwardingCode.Print(TEXT("}"));

	OutputForwardingCode.WriteToFile(ForwardingCodePath + FString::Printf(TEXT("SpatialInterop%s.cpp"), *Class->GetName()));
}
}

UGenerateSchemaCommandlet::UGenerateSchemaCommandlet() {
}

UGenerateSchemaCommandlet::~UGenerateSchemaCommandlet() {
}

int32 UGenerateSchemaCommandlet::Main(const FString& Params) {
	FString CombinedSchemaPath =
		FPaths::Combine(*FPaths::GetPath(FPaths::GetProjectFilePath()), TEXT("../../../schema/generated/"));
	FString CombinedForwardingCodePath =
		FPaths::Combine(*FPaths::GetPath(FPaths::GetProjectFilePath()), TEXT("../../../workers/unreal/Game/Source/NUF/Generated/"));
	UE_LOG(LogTemp, Display, TEXT("Schema path %s - Forwarding code path %s"), *CombinedSchemaPath, *CombinedForwardingCodePath);
	if (FPaths::CollapseRelativeDirectories(CombinedSchemaPath) && FPaths::CollapseRelativeDirectories(CombinedForwardingCodePath)) {
		UE_LOG(LogTemp, Warning, TEXT("================================================================================="));
		GenerateCompleteSchemaFromClass(CombinedSchemaPath, CombinedForwardingCodePath, ACharacter::StaticClass());
		UE_LOG(LogTemp, Warning, TEXT("================================================================================="));
	} else {
		UE_LOG(LogTemp, Display, TEXT("Path was invalid - schema not generated"));
	}

	return 0;
}
