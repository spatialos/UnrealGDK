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

void VisitProperty(TArray<FPropertyInfo>& PropertyInfo, UObject* CDO, TArray<UProperty*> Stack, UProperty* Property)
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

FPropertyLayout CreatePropertyLayout(UClass* Class)
{
	FPropertyLayout Layout;

	// Read the RepLayout for the class into a data structure.
	FRepLayout RepLayout;
	RepLayout.InitFromObjectClass(Class);
	TArray<TPair<uint16, FRepLayoutEntry>> RepLayoutProperties;
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
		RepLayoutProperties.Add(MakeTuple(Handle, FRepLayoutEntry{
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
	TArray<FReplicatedPropertyInfo> ReplicatedProperties;
	TArray<FPropertyInfo> CompletePropertiesToRemove;
	for (auto& Pair : RepLayoutProperties)
	{
		FRepLayoutEntry& Entry = Pair.Value;
		FReplicatedPropertyInfo Info{Entry, {}};

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
			Layout.RPCs[GetRPCTypeFromFunction(*RemoteFunction)].Emplace(FRPCDefinition{Class, *RemoteFunction, GetRPCTypeFromFunction(*RemoteFunction), bReliable});
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
				Layout.RPCs[GetRPCTypeFromFunction(*RemoteFunction)].Emplace(FRPCDefinition{CharacterMovementComponentClass, *RemoteFunction, GetRPCTypeFromFunction(*RemoteFunction), bReliable});
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
				Layout.RPCs[GetRPCTypeFromFunction(*RemoteFunction)].Emplace(FRPCDefinition{UWheeledVehicleMovementComponent, *RemoteFunction, GetRPCTypeFromFunction(*RemoteFunction), bReliable});
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
