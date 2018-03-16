// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TypeStructure.h"

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

FString GetLifetimeConditionAsString(ELifetimeCondition Condition)
{
	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("ELifetimeCondition"), true);
	if (!EnumPtr)
	{
		return FString("Invalid");
	}
	return EnumPtr->GetNameByValue((int64)Condition).ToString();
}

FString GetRepNotifyLifetimeConditionAsString(ELifetimeRepNotifyCondition Condition)
{
	switch (Condition)
	{
	case REPNOTIFY_OnChanged: return FString(TEXT("REPNOTIFY_OnChanged"));
	case REPNOTIFY_Always: return FString(TEXT("REPNOTIFY_Always"));
	default:
		checkNoEntry();
	}
	return FString();
}

TArray<EReplicatedPropertyGroup> GetAllReplicatedPropertyGroups()
{
	static TArray<EReplicatedPropertyGroup> Groups = {REP_SingleClient, REP_MultiClient};
	return Groups;
}

FString GetReplicatedPropertyGroupName(EReplicatedPropertyGroup Group)
{
	return Group == REP_SingleClient ? TEXT("SingleClient") : TEXT("MultiClient");
}

TArray<ERPCType> GetRPCTypes()
{
	static TArray<ERPCType> Groups = {RPC_Client, RPC_Server};
	return Groups;
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

FString GetRPCTypeName(ERPCType RPCType)
{
	switch (RPCType)
	{
	case ERPCType::RPC_Client:
		return "Client";
	case ERPCType::RPC_Server:
		return "Server";
	default:
		checkf(false, TEXT("RPCType is invalid!"));
		return "";
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

void VisitProperty(TArray<FPropertyInfo_OLD>& PropertyInfo, UObject* CDO, TArray<UProperty*> Stack, UProperty* Property)
{
	// Skip properties that make no sense to store in SpatialOS.
	if (Property->IsA<UMulticastDelegateProperty>())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s - multicast delegate property, skipping"), *Property->GetName());
		return;
	}
	if (Property->IsA<UArrayProperty>())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s - array property, skipping"), *Property->GetName());
		return;
	}
	if (Property->GetPropertyFlags() & CPF_Transient && !(Property->GetPropertyFlags() & CPF_Net))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s - transient and not replicated, skipping"), *Property->GetName());
		return;
	}

	// Get property type.
	ERepLayoutCmdType PropertyType = PropertyToRepLayoutType(Property);

	// If this property is a struct or object, we need to recurse into its properties.
	//
	// Usually, struct properties map directly to built in types like FString, or FPlane. However, custom structs map
	// directly to REPCMD_Property, so we need to make sure to check that it's a struct.
	UStruct* PropertyValueType = nullptr;
	if ((PropertyType == REPCMD_Property && Property->IsA<UStructProperty>()) ||
		(PropertyType == REPCMD_PropertyObject && Property->IsA<UObjectProperty>()))
	{
		// Get struct/class of property value.
		if (PropertyType != REPCMD_PropertyObject)
		{
			// Properties which are structs are always owned by its outer, as they're just inlined into the memory structure
			// of the object and by definition are not UObjects.
			PropertyValueType = Cast<UStructProperty>(Property)->Struct;
		}
		else
		{
			// Properties which are objects are more interesting. These can either be a subobject which is owned by its outer,
			// such as a Character owning a MovementComponent. However, some objects properties are in fact weak references to
			// other objects. An example for this is MovementBase inside Character (which stores the object the character is standing
			// on). In this case, we need to make sure to _not_ recurse into its properties but instead just generated a weak
			// reference field.

			UObjectProperty* ObjectProperty = Cast<UObjectProperty>(Property);
			UClass* PropertyValueTypeAsClass = nullptr;

			// If we have the CDO of this properties container type, we can try to figure out whether this object property is
			// a strong or weak reference.
			if (CDO)
			{
				// Obtain the properties actual value from the CDO, so we can figure out its true type (for example, to determine
				// that the property is a CharacterMovementComponent rather than just a MovementComponent).
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
						PropertyValueTypeAsClass = Value->GetClass();
					}
					else
					{
						// The values outer is not us, store as weak reference.
						UE_LOG(LogTemp, Warning, TEXT("%s - %s weak reference (outer not this)"), *Property->GetName(), *ObjectProperty->PropertyClass->GetName());
					}
				}
				else
				{
					// If value is just nullptr, then we clearly don't own it.
					UE_LOG(LogTemp, Warning, TEXT("%s - %s weak reference (null init)"), *Property->GetName(), *ObjectProperty->PropertyClass->GetName());
				}
			}
			else
			{
				// If we don't have a CDO for this properties container type, then the container is just a struct. Structs always store weak references to objects.
				UE_LOG(LogTemp, Warning, TEXT("%s - %s weak reference (object inside struct)"), *Property->GetName(), *ObjectProperty->PropertyClass->GetName());
			}

			// Once we have the class of this object, and it's not a weak reference. Skip it if it makes no sense to store in SpatialOS.
			if (PropertyValueTypeAsClass)
			{
				if (PropertyValueTypeAsClass->IsChildOf<UClass>())
				{
					return;
				}
				if (!(PropertyValueTypeAsClass->GetClassFlags() & CLASS_DefaultToInstanced))
				{
					UE_LOG(LogTemp, Warning, TEXT("%s - %s not instanced, skipping"), *Property->GetName(), *PropertyValueTypeAsClass->GetName());
					return;
				}
				if (PropertyValueTypeAsClass->IsChildOf(FTickFunction::StaticStruct()))
				{
					return;
				}
			}

			PropertyValueType = PropertyValueTypeAsClass;
		}

		// If not nullptr, then this property is an owning reference to the value.
		if (PropertyValueType)
		{
			// Instantiate CDO of this struct if it's a class.
			UObject* PropertyValueClassCDO = nullptr;
			if (Cast<UClass>(PropertyValueType) != nullptr)
			{
				PropertyValueClassCDO = Cast<UClass>(PropertyValueType)->GetDefaultObject();
			}

			// Recurse into properties.
			TArray<UProperty*> NewStack(Stack);
			NewStack.Add(Property);
			for (TFieldIterator<UProperty> It(PropertyValueType); It; ++It)
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
		PropertyValueType,
		Stack
	});
}

FPropertyLayout_OLD CreatePropertyLayout(UClass* Class)
{
	FPropertyLayout_OLD Layout;

	// Read the RepLayout for the class into a data structure.
	FRepLayout RepLayout;
	RepLayout.InitFromObjectClass(Class);
	TArray<TPair<uint16, FRepLayoutEntry_OLD>> RepLayoutProperties;
	for (int CmdIndex = 0; CmdIndex < RepLayout.Cmds.Num(); ++CmdIndex)
	{
		auto& Cmd = RepLayout.Cmds[CmdIndex];

		if (Cmd.Type == REPCMD_Return || Cmd.Type == REPCMD_DynamicArray)
		{
			continue;
		}

		if (Cmd.Property == nullptr)
		{
			continue;
		}

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
			PropertyChain = {ParentProperty, Property};
		}
		else
		{
			PropertyChain = {Property};
		}

		uint16 Handle = Cmd.RelativeHandle;
		RepLayoutProperties.Add(MakeTuple(Handle, FRepLayoutEntry_OLD{
			Property,
			ParentProperty,
			PropertyChain,
			RepLayout.Parents[Cmd.ParentIndex].Condition,
			RepLayout.Parents[Cmd.ParentIndex].RepNotifyCondition,
			(ERepLayoutCmdType)Cmd.Type,
			CmdIndex,
			Handle
		}));
	}

	// Recurse into class properties and build a complete property list.
	for (TFieldIterator<UProperty> It(Class); It; ++It)
	{
		VisitProperty(Layout.CompleteProperties, Class->GetDefaultObject(), {}, *It);
	}

	// Divide properties into replicated and complete properties.
	TArray<FReplicatedPropertyInfo_OLD> ReplicatedProperties;
	TArray<FPropertyInfo_OLD> CompletePropertiesToRemove;
	for (auto& Pair : RepLayoutProperties)
	{
		FRepLayoutEntry_OLD& Entry = Pair.Value;
		FReplicatedPropertyInfo_OLD Info{Entry, {}};

		// Search for all properties with a property chain that begins with either {Parent, Property} or {Property}.
		Info.PropertyList = Layout.CompleteProperties.FilterByPredicate([Entry](const FPropertyInfo_OLD& Property)
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
			Layout.RPCs[GetRPCTypeFromFunction(*RemoteFunction)].Emplace(FRPCDefinition_OLD{Class, *RemoteFunction, GetRPCTypeFromFunction(*RemoteFunction), bReliable});
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
				Layout.RPCs[GetRPCTypeFromFunction(*RemoteFunction)].Emplace(FRPCDefinition_OLD{CharacterMovementComponentClass, *RemoteFunction, GetRPCTypeFromFunction(*RemoteFunction), bReliable});
			}
		}
	}

	// Vehicle: Add WheeledVehicleMovementComponent.
	if (Class->GetName() == TEXT("WheeledVehicle"))
	{
		UClass* UWheeledVehicleMovementComponent = FindObject<UClass>(ANY_PACKAGE, TEXT("WheeledVehicleMovementComponent"));
		for (TFieldIterator<UFunction> RemoteFunction(UWheeledVehicleMovementComponent); RemoteFunction; ++RemoteFunction)
		{
			if (RemoteFunction->FunctionFlags & FUNC_NetClient ||
				RemoteFunction->FunctionFlags & FUNC_NetServer)
			{
				bool bReliable = (RemoteFunction->FunctionFlags & FUNC_NetReliable) != 0;
				Layout.RPCs[GetRPCTypeFromFunction(*RemoteFunction)].Emplace(FRPCDefinition_OLD{UWheeledVehicleMovementComponent, *RemoteFunction, GetRPCTypeFromFunction(*RemoteFunction), bReliable});
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

void VisitAllObjects(TSharedPtr<FUnrealType> TypeNode, TFunction<bool(TSharedPtr<FUnrealType>)> Visitor, bool bRecurseIntoSubobjects)
{
	bool bShouldRecurseFurther = Visitor(TypeNode);
	for (auto& PropertyPair : TypeNode->Properties)
	{
		if (bShouldRecurseFurther && PropertyPair.Value->Type.IsValid())
		{
			// Either recurse into subobjects if they're structs or bRecurseIntoSubobjects is true.
			if (bRecurseIntoSubobjects || PropertyPair.Value->Property->IsA<UStructProperty>())
			{
				VisitAllObjects(PropertyPair.Value->Type, Visitor, bRecurseIntoSubobjects);
			}
		}
	}
}

void VisitAllProperties(TSharedPtr<FUnrealType> TypeNode, TFunction<bool(TSharedPtr<FUnrealProperty>)> Visitor, bool bRecurseIntoSubobjects)
{
	for (auto& PropertyPair : TypeNode->Properties)
	{
		bool bShouldRecurseFurther = Visitor(PropertyPair.Value);
		if (bShouldRecurseFurther && PropertyPair.Value->Type.IsValid())
		{
			// Either recurse into subobjects if they're structs or bRecurseIntoSubobjects is true.
			if (bRecurseIntoSubobjects || PropertyPair.Value->Property->IsA<UStructProperty>())
			{
				VisitAllProperties(PropertyPair.Value->Type, Visitor, bRecurseIntoSubobjects);
			}
		}
	}
}

void VisitAllProperties(TSharedPtr<FUnrealRPC> RPC, TFunction<bool(TSharedPtr<FUnrealProperty>)> Visitor, bool bRecurseIntoSubobjects)
{
	for (auto& PropertyPair : RPC->Parameters)
	{
		bool bShouldRecurseFurther = Visitor(PropertyPair.Value);
		if (bShouldRecurseFurther && PropertyPair.Value->Type.IsValid())
		{
			// Either recurse into subobjects if they're structs or bRecurseIntoSubobjects is true.
			if (bRecurseIntoSubobjects || PropertyPair.Value->Property->IsA<UStructProperty>())
			{
				VisitAllProperties(PropertyPair.Value->Type, Visitor, bRecurseIntoSubobjects);
			}
		}
	}
}

TSharedPtr<FUnrealType> CreateUnrealTypeInfo(UStruct* Type)
{
	// Struct types will set this to nullptr.
	UClass* Class = Cast<UClass>(Type);

	// Create type node.
	TSharedPtr<FUnrealType> TypeNode = MakeShared<FUnrealType>();
	TypeNode->Type = Type;

	// Iterate through each property in the struct.
	for (TFieldIterator<UProperty> It(Type); It; ++It)
	{
		UProperty* Property = *It;

		// TODO(David): Should we still be skipping this?
		if (Property->IsA<UMulticastDelegateProperty>())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s - multicast delegate property, skipping"), *Property->GetName());
			continue;
		}
		
		// Create property node and add it to the AST.
		TSharedPtr<FUnrealProperty> PropertyNode = MakeShared<FUnrealProperty>();
		PropertyNode->Property = Property;
		PropertyNode->ContainerType = TypeNode;
		TypeNode->Properties.Add(Property, PropertyNode);

		// If this property not a struct or object (which can contain more properties), stop here.
		if (!Property->IsA<UStructProperty>() && !Property->IsA<UObjectProperty>())
		{
			continue;
		}

		// If this is a struct property, then get the struct type and recurse into it.
		if (Property->IsA<UStructProperty>())
		{
			UStructProperty* StructProperty = Cast<UStructProperty>(Property);
			PropertyNode->Type = CreateUnrealTypeInfo(StructProperty->Struct);
			PropertyNode->Type->ParentProperty = PropertyNode;
			continue;
		}

		// If this is an object property, then we need to do two things:
		//	 1) Determine whether this property is a strong or weak reference to the object. Some subobjects (such as the CharacterMovementComponent)
		//		are in fact owned by the character, and can be stored in the same entity as the character itself. Some subobjects (such as the Controller
		//		field in AActor) is a weak reference, and should just store a reference to the real object. We inspect the CDO to determine whether
		//		the owner of the property value is equal to itself. As structs don't have CDOs, we assume that all object properties in structs are
		//		weak references.
		//
		//   2) Obtain the concrete object type stored in this property. For example, the property containing the CharacterMovementComponent
		//      might be a property which stores a MovementComponent pointer, so we'd need to somehow figure out the real type being stored there
		//		during runtime. This is determined by getting the CDO of this class to determine what is stored in that property.
		UObjectProperty* ObjectProperty = Cast<UObjectProperty>(Property);
		check(ObjectProperty);

		// If this is a property of a struct, assume it's a weak reference.
		if (!Class)
		{
			continue;
		}
		
		UObject* ContainerCDO = Class->GetDefaultObject();
		check(ContainerCDO);

		// Obtain the properties actual value from the CDO, so we can figure out its true type.
		UObject* Value = ObjectProperty->GetPropertyValue_InContainer(ContainerCDO);
		if (Value)
		{
			// If this is an editor-only property, skip it. As we've already added to the property list at this stage, just remove it.
			if (Value->IsEditorOnly())
			{
				UE_LOG(LogTemp, Warning, TEXT("%s - editor only, skipping"), *Property->GetName());
				TypeNode->Properties.Remove(Property);
				continue;
			}

			// Check whether the owner of this value is the CDO itself.
			if (Value->GetOuter() == ContainerCDO)
			{
				UE_LOG(LogTemp, Warning, TEXT("Property Class: %s Instance Class: %s"), *ObjectProperty->PropertyClass->GetName(), *Value->GetClass()->GetName());

				// This property is definitely a strong reference, recurse into it.
				PropertyNode->Type = CreateUnrealTypeInfo(ObjectProperty->PropertyClass);
				PropertyNode->Type->ParentProperty = PropertyNode;
			}
			else
			{
				// The values outer is not us, store as weak reference.
				UE_LOG(LogTemp, Warning, TEXT("%s - %s weak reference (outer not this)"), *Property->GetName(), *ObjectProperty->PropertyClass->GetName());
			}
		}
		else
		{
			// If value is just nullptr, then we clearly don't own it.
			UE_LOG(LogTemp, Warning, TEXT("%s - %s weak reference (null init)"), *Property->GetName(), *ObjectProperty->PropertyClass->GetName());
		}
	}

	// If this is not a class, exit now, as structs cannot have RPCs or replicated properties.
	if (!Class)
	{
		return TypeNode;
	}

	// Iterate through each RPC in the class.
	for (TFieldIterator<UFunction> RemoteFunction(Class); RemoteFunction; ++RemoteFunction)
	{
		if (RemoteFunction->FunctionFlags & FUNC_NetClient ||
			RemoteFunction->FunctionFlags & FUNC_NetServer)
		{
			TSharedPtr<FUnrealRPC> RPCNode = MakeShared<FUnrealRPC>();
			RPCNode->CallerType = Class;
			RPCNode->Function = *RemoteFunction;
			RPCNode->Type = GetRPCTypeFromFunction(*RemoteFunction);
			RPCNode->bReliable = (RemoteFunction->FunctionFlags & FUNC_NetReliable) != 0;
			TypeNode->RPCs.Add(*RemoteFunction, RPCNode);

			// Fill out parameters.
			for (TFieldIterator<UProperty> It(*RemoteFunction); It; ++It)
			{
				UProperty* Parameter = *It;

				TSharedPtr<FUnrealProperty> PropertyNode = MakeShared<FUnrealProperty>();
				PropertyNode->Property = Parameter;
				RPCNode->Parameters.Add(Parameter, PropertyNode);

				// If this RPC parameter is a struct, recurse into it.
				UStructProperty* StructParameter = Cast<UStructProperty>(Parameter);
				if (StructParameter)
				{
					PropertyNode->Type = CreateUnrealTypeInfo(StructParameter->Struct);
					PropertyNode->Type->ParentProperty = PropertyNode;
				}
			}
		}
	}

	// Set up replicated properties by reading the rep layout and matching the properties with the ones in the type node.
	// Based on inspection in InitFromObjectClass, the RepLayout will always replicate object properties using NetGUIDs, regardless of
	// ownership. However, the rep layout will recurse into structs and allocate rep handles for their properties, unless the condition
	// "Struct->StructFlags & STRUCT_NetSerializeNative" is true. In this case, the entire struct is replicated as a whole.
	FRepLayout RepLayout;
	RepLayout.InitFromObjectClass(Class);
	for (int CmdIndex = 0; CmdIndex < RepLayout.Cmds.Num(); ++CmdIndex)
	{
		FRepLayoutCmd& Cmd = RepLayout.Cmds[CmdIndex];
		FRepParentCmd& Parent = RepLayout.Parents[Cmd.ParentIndex];

		if (Cmd.Type == REPCMD_Return || Cmd.Property == nullptr)
		{
			continue;
		}

		if (Cmd.Type == REPCMD_DynamicArray || Parent.Property->IsA<UArrayProperty>())
		{
			// Skip dynamic array commands.
			continue;
		}

		// In a FRepLayout, all the root level replicated properties in a class are stored in the Parents array.
		// The Cmds array is an expanded version of the Parents array. This usually maps 1:1 with the Parents array (as most properties
		// don't contain other properties). The main exception are structs which don't have a native serialize function. In this case
		// multiple Cmds map to the structs properties, but they all have the same ParentIndex (which points to the root replicated property
		// which contains them.
		//
		// This might be problematic if we have a property which is inside a struct, nested in another struct which is replicated. For example:
		//
		//	class Foo
		//	{
		//		struct Bar
		//		{
		// 			struct Baz
		// 			{
		// 				int Nested;
		// 			} Baz;
		// 		} Bar;
		//	}
		//
		// The parents array will contain "Bar", and the cmds array will contain "Nested", but we have no reference to "Baz" anywhere in the RepLayout.
		// What we do here is recurse into all of Bar's properties in the AST until we find Baz.

		TSharedPtr<FUnrealProperty> PropertyNode = nullptr;

		// Simple case: Cmd is a root property in the object.
		if (Parent.Property == Cmd.Property)
		{
			PropertyNode = TypeNode->Properties[Cmd.Property];
		}
		else
		{
			// Here, the Cmd is some property inside the Parent property. We need to find it in the AST.
			TSharedPtr<FUnrealProperty> RootProperty = TypeNode->Properties[Parent.Property];
			checkf(RootProperty->Type.IsValid(), TEXT("Properties in the AST which are parent properties in the rep layout must have child properties"));
			VisitAllProperties(RootProperty->Type, [&PropertyNode, &Cmd](TSharedPtr<FUnrealProperty> Property)
			{
				if (Property->Property == Cmd.Property)
				{
					checkf(!PropertyNode.IsValid(), TEXT("We've already found a previous property node with the same property. This indicates that we have a 'diamond of death' style situation."))
					PropertyNode = Property;
				}
				return true;
			}, false);
			checkf(PropertyNode.IsValid(), TEXT("Couldn't find the Cmd property inside the Parent's sub-properties. This shouldn't happen."));
		}
		
		// We now have the right property node. Fill in the rep data.
		TSharedPtr<FUnrealRepData> RepDataNode = MakeShared<FUnrealRepData>();
		RepDataNode->RepLayoutType = (ERepLayoutCmdType)Cmd.Type;
		RepDataNode->Condition = Parent.Condition;
		RepDataNode->RepNotifyCondition = Parent.RepNotifyCondition;
		RepDataNode->CmdIndex = CmdIndex;
		RepDataNode->Handle = Cmd.RelativeHandle;
		PropertyNode->ReplicationData = RepDataNode;
	}

	return TypeNode;
}

TMap<EReplicatedPropertyGroup, TMap<uint16, TSharedPtr<FUnrealProperty>>> GetFlatRepData(TSharedPtr<FUnrealType> TypeInfo)
{
	TMap<EReplicatedPropertyGroup, TMap<uint16, TSharedPtr<FUnrealProperty>>> RepData;
	RepData.Add(REP_MultiClient);
	RepData.Add(REP_SingleClient);

	VisitAllProperties(TypeInfo, [&RepData](TSharedPtr<FUnrealProperty> Property)
	{
		if (Property->ReplicationData.IsValid())
		{
			EReplicatedPropertyGroup Group = REP_MultiClient;
			switch (Property->ReplicationData->Condition)
			{
			case COND_AutonomousOnly:
			case COND_OwnerOnly:
				Group = REP_SingleClient;
				break;
			}
			RepData[Group].Add(Property->ReplicationData->Handle, Property);
		}
		return true;
	}, false);

	// Sort by replication handle.
	RepData[REP_MultiClient].KeySort([](uint16 A, uint16 B)
	{
		return A < B;
	});
	RepData[REP_SingleClient].KeySort([](uint16 A, uint16 B)
	{
		return A < B;
	});
	return RepData;
}

TMap<ERPCType, TArray<TSharedPtr<FUnrealRPC>>> GetAllRPCsByType(TSharedPtr<FUnrealType> TypeInfo)
{
	TMap<ERPCType, TArray<TSharedPtr<FUnrealRPC>>> RPCsByType;
	RPCsByType.Add(RPC_Client);
	RPCsByType.Add(RPC_Server);
	VisitAllObjects(TypeInfo, [&RPCsByType](TSharedPtr<FUnrealType> Type)
	{
		for (auto& RPC : Type->RPCs)
		{
			RPCsByType.FindOrAdd(RPC.Value->Type).Add(RPC.Value);
		}
		return true;
	}, true);
	return RPCsByType;
}
