// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TypeStructure.h"
#include "SpatialGDKEditorInteropCodeGenerator.h"

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
	static TArray<ERPCType> Groups = {RPC_Client, RPC_Server, RPC_NetMulticast};
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
	if (Function->FunctionFlags & EFunctionFlags::FUNC_NetMulticast)
	{
		return ERPCType::RPC_NetMulticast;
	}
	else
	{
		checkNoEntry();
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
	case ERPCType::RPC_NetMulticast:
		return "NetMulticast";
	default:
		checkf(false, TEXT("RPCType is invalid!"));
		return "";
	}
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

void VisitAllProperties(TSharedPtr<FUnrealRPC> RPCNode, TFunction<bool(TSharedPtr<FUnrealProperty>)> Visitor, bool bRecurseIntoSubobjects)
{
	for (auto& PropertyPair : RPCNode->Parameters)
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

uint32 GenerateChecksum(UProperty* Property, uint32 ParentChecksum, int32 StaticArrayIndex)
{
	uint32 Checksum = 0;
	Checksum = FCrc::StrCrc32(*Property->GetName().ToLower(), ParentChecksum);            // Evolve checksum on name
	Checksum = FCrc::StrCrc32(*Property->GetCPPType(nullptr, 0).ToLower(), Checksum);     // Evolve by property type
	Checksum = FCrc::StrCrc32(*FString::Printf(TEXT("%i"), StaticArrayIndex), Checksum);  // Evolve by StaticArrayIndex
	return Checksum;
}

TSharedPtr<FUnrealType> CreateUnrealTypeInfo(UStruct* Type, const TArray<TArray<FName>>& MigratableProperties, uint32 ParentChecksum, int32 StaticArrayIndex, bool bIsRPC)
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
		
		// Create property node and add it to the AST.
		TSharedPtr<FUnrealProperty> PropertyNode = MakeShared<FUnrealProperty>();
		PropertyNode->Property = Property;
		PropertyNode->ContainerType = TypeNode;
		PropertyNode->ParentChecksum = ParentChecksum;
		PropertyNode->StaticArrayIndex = StaticArrayIndex;
		TypeNode->Properties.Add(Property, PropertyNode);

		// Add checksums to match properties with the RepLayout Cmds later.
		PropertyNode->CompatibleChecksum = GenerateChecksum(Property, ParentChecksum, StaticArrayIndex);

		// If this property not a struct or object (which can contain more properties), stop here.
		if (!Property->IsA<UStructProperty>() && !Property->IsA<UObjectProperty>()) // RPCs handle static arrays differently.
		{
			if (!bIsRPC) // RPCs handle static arrays differently
			{
				for (int i = 1; i < Property->ArrayDim; i++)
				{
					TSharedPtr<FUnrealProperty> StaticArrayPropertyNode = MakeShared<FUnrealProperty>();
					StaticArrayPropertyNode->Property = Property;
					StaticArrayPropertyNode->ContainerType = TypeNode;
					StaticArrayPropertyNode->StaticArrayIndex = i;

					// Generate a new checksum for the static array member;
					StaticArrayPropertyNode->ParentChecksum = ParentChecksum;
					uint32 StaticArrayChecksum = GenerateChecksum(Property, ParentChecksum, i);
					StaticArrayPropertyNode->CompatibleChecksum = StaticArrayChecksum;

					TypeNode->Properties.Add(Property, StaticArrayPropertyNode);
				}
			}
			continue;
		}

		// If this is a struct property, then get the struct type and recurse into it.
		if (Property->IsA<UStructProperty>())
		{
			UStructProperty* StructProperty = Cast<UStructProperty>(Property);

			// This is the property for the 0th struct array member.
			uint32 ParentPropertyNodeChecksum = PropertyNode->CompatibleChecksum;
			PropertyNode->Type = CreateUnrealTypeInfo(StructProperty->Struct, {}, ParentPropertyNodeChecksum, 0, bIsRPC);
			PropertyNode->Type->ParentProperty = PropertyNode;

			if (!bIsRPC) // RPCs handle static arrays differently
			{
				// For static arrays we need to make a new struct array member node.
				for (int i = 1; i < Property->ArrayDim; i++)
				{
					// Create a new PropertyNode.
					TSharedPtr<FUnrealProperty> StaticStructArrayPropertyNode = MakeShared<FUnrealProperty>();
					StaticStructArrayPropertyNode->Property = Property;
					StaticStructArrayPropertyNode->ContainerType = TypeNode;
					StaticStructArrayPropertyNode->ParentChecksum = ParentPropertyNodeChecksum;

					// Generate a new checksum based on the static array index.
					uint32 StaticArrayChecksum = GenerateChecksum(Property, ParentChecksum, i);
					StaticStructArrayPropertyNode->CompatibleChecksum = StaticArrayChecksum;

					// Generate Type information on the inner struct.
					// Note: The parent checksum of the properties within a struct that is a member of a static struct array, is the checksum for the struct itself after index modification.
					StaticStructArrayPropertyNode->Type = CreateUnrealTypeInfo(StructProperty->Struct, {}, StaticArrayChecksum, 0, bIsRPC);
					StaticStructArrayPropertyNode->StaticArrayIndex = i;
					StaticStructArrayPropertyNode->Type->ParentProperty = StaticStructArrayPropertyNode;

					// Add the new StaticStructArrayPropertyNode to the current TypeNode.
					TypeNode->Properties.Add(Property, StaticStructArrayPropertyNode);
				}
			}
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

		// This is to ensure we handle static array properties only once.
		bool bHandleStaticArrayProperties = true;

		// Obtain the properties actual value from the CDO, so we can figure out its true type.
		UObject* Value = ObjectProperty->GetPropertyValue_InContainer(ContainerCDO);
		if (Value)
		{
			// If this is an editor-only property, skip it. As we've already added to the property list at this stage, just remove it.
			if (Value->IsEditorOnly())
			{
				UE_LOG(LogSpatialGDKInteropCodeGenerator, Warning, TEXT("%s - editor only, skipping"), *Property->GetName());
				TypeNode->Properties.Remove(Property);
				continue;
			}

			// Check whether the owner of this value is the CDO itself.
			if (Value->GetOuter() == ContainerCDO)
			{
				UE_LOG(LogSpatialGDKInteropCodeGenerator, Warning, TEXT("Property Class: %s Instance Class: %s"), *ObjectProperty->PropertyClass->GetName(), *Value->GetClass()->GetName());

				// This property is definitely a strong reference, recurse into it.
				PropertyNode->Type = CreateUnrealTypeInfo(ObjectProperty->PropertyClass, {}, ParentChecksum, 0, bIsRPC);
				PropertyNode->Type->ParentProperty = PropertyNode;

				if (!bIsRPC) // RPCs handle static arrays differently
				{
					// For static arrays we need to make a new object array member node.
					for (int i = 1; i < Property->ArrayDim; i++)
					{
						// Create a new PropertyNode.
						TSharedPtr<FUnrealProperty> StaticObjectArrayPropertyNode = MakeShared<FUnrealProperty>();
						StaticObjectArrayPropertyNode->Property = Property;
						StaticObjectArrayPropertyNode->ContainerType = TypeNode;
						StaticObjectArrayPropertyNode->ParentChecksum = ParentChecksum;

						// Generate a new checksum based on the static array index.
						StaticObjectArrayPropertyNode->CompatibleChecksum = GenerateChecksum(Property, ParentChecksum, i);

						// Generate Type information on the inner struct.
						StaticObjectArrayPropertyNode->Type = CreateUnrealTypeInfo(ObjectProperty->PropertyClass, {}, ParentChecksum, 0, bIsRPC);
						StaticObjectArrayPropertyNode->StaticArrayIndex = i;
						StaticObjectArrayPropertyNode->Type->ParentProperty = StaticObjectArrayPropertyNode;

						// Add the new StaticObjectArrayPropertyNode to the current TypeNode.
						TypeNode->Properties.Add(Property, StaticObjectArrayPropertyNode);
					}
				}
				bHandleStaticArrayProperties = false;
			}
			else
			{
				// The values outer is not us, store as weak reference.
				UE_LOG(LogSpatialGDKInteropCodeGenerator, Warning, TEXT("%s - %s weak reference (outer not this)"), *Property->GetName(), *ObjectProperty->PropertyClass->GetName());
			}
		}
		else
		{
			// If value is just nullptr, then we clearly don't own it.
			UE_LOG(LogSpatialGDKInteropCodeGenerator, Warning, TEXT("%s - %s weak reference (null init)"), *Property->GetName(), *ObjectProperty->PropertyClass->GetName());
		}

		// Weak reference static arrays are handled as a single UObjectRef per static array member.
		if (!bIsRPC && bHandleStaticArrayProperties)
		{
			for (int i = 1; i < Property->ArrayDim; i++)
			{
				TSharedPtr<FUnrealProperty> StaticArrayPropertyNode = MakeShared<FUnrealProperty>();
				StaticArrayPropertyNode->Property = Property;
				StaticArrayPropertyNode->ContainerType = TypeNode;
				StaticArrayPropertyNode->StaticArrayIndex = i;

				// Generate a new checksum for the static array member;
				StaticArrayPropertyNode->ParentChecksum = ParentChecksum;
				uint32 StaticArrayChecksum = GenerateChecksum(Property, ParentChecksum, i);
				StaticArrayPropertyNode->CompatibleChecksum = StaticArrayChecksum;

				TypeNode->Properties.Add(Property, StaticArrayPropertyNode);
			}
		}

	} // END TFieldIterator<UProperty>

	// If this is not a class, exit now, as structs cannot have RPCs or replicated properties.
	if (!Class)
	{
		return TypeNode;
	}

	// Iterate through each RPC in the class.
	for (TFieldIterator<UFunction> RemoteFunction(Class); RemoteFunction; ++RemoteFunction)
	{
		if (RemoteFunction->FunctionFlags & FUNC_NetClient ||
			RemoteFunction->FunctionFlags & FUNC_NetServer ||
			RemoteFunction->FunctionFlags & FUNC_NetMulticast)
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

				// RPCs can't have static arrays as parameters so we don't have to special case for them here, however struct parameters can have static arrays in them.
				RPCNode->Parameters.Add(Parameter, PropertyNode);

				// If this RPC parameter is a struct, recurse into it.
				UStructProperty* StructParameter = Cast<UStructProperty>(Parameter);
				if (StructParameter)
				{
					uint32 StructChecksum = GenerateChecksum(Parameter, ParentChecksum, 0);
					PropertyNode->CompatibleChecksum = StructChecksum;
					PropertyNode->Type = CreateUnrealTypeInfo(StructParameter->Struct, {}, StructChecksum, 0 , true);
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
			// Make sure we have the correct property via the checksums.
			for (auto& PropertyPair : TypeNode->Properties)
			{
				if(PropertyPair.Value->CompatibleChecksum == Cmd.CompatibleChecksum)
				{
					PropertyNode = PropertyPair.Value;
				}
			}
		}
		else
		{
			// It's possible to have duplicate parent properties (they are distinguished by ArrayIndex), so we make sure to look at them all.
			TArray<TSharedPtr<FUnrealProperty>> RootProperties;
			TypeNode->Properties.MultiFind(Parent.Property, RootProperties);

			for(int i = 0; i < RootProperties.Num(); i++)
			{
				TSharedPtr<FUnrealProperty> RootProperty = RootProperties[i];

				checkf(RootProperty->Type.IsValid(), TEXT("Properties in the AST which are parent properties in the rep layout must have child properties"));
				VisitAllProperties(RootProperty->Type, [&PropertyNode, &Cmd](TSharedPtr<FUnrealProperty> Property)
				{
					if (Property->CompatibleChecksum == Cmd.CompatibleChecksum)
					{
						checkf(!PropertyNode.IsValid(), TEXT("We've already found a previous property node with the same property. This indicates that we have a 'diamond of death' style situation."))
						PropertyNode = Property;
					}
					return true;
				}, false);
			}
		}
		checkf(PropertyNode.IsValid(), TEXT("Couldn't find the Cmd property inside the Parent's sub-properties. This shouldn't happen."));

		// We now have the right property node. Fill in the rep data.
		TSharedPtr<FUnrealRepData> RepDataNode = MakeShared<FUnrealRepData>();
		RepDataNode->RepLayoutType = (ERepLayoutCmdType)Cmd.Type;
		RepDataNode->Condition = Parent.Condition;
		RepDataNode->RepNotifyCondition = Parent.RepNotifyCondition;
		RepDataNode->ArrayIndex = PropertyNode->StaticArrayIndex;
		if (Parent.RoleSwapIndex != -1)
		{
			const int32 SwappedCmdIndex = RepLayout.Parents[Parent.RoleSwapIndex].CmdStart;
			RepDataNode->RoleSwapHandle = static_cast<int32>(RepLayout.Cmds[SwappedCmdIndex].RelativeHandle);
		}
		else
		{
			RepDataNode->RoleSwapHandle = -1;
		}
		PropertyNode->ReplicationData = RepDataNode;
		PropertyNode->ReplicationData->Handle = Cmd.RelativeHandle;

		if (Cmd.Type == REPCMD_DynamicArray)
		{
			// Bypass the inner properties and null terminator cmd when processing dynamic arrays.
			CmdIndex = Cmd.EndCmd - 1;
		}
	} // END CMD FOR LOOP

	// Process the migratable properties list.
	uint16 MigratableDataHandle = 1;
	for (const TArray<FName>& PropertyNames : MigratableProperties)
	{
		// Find the property represented by this chain.
		TSharedPtr<FUnrealProperty> MigratableProperty = nullptr;
		TSharedPtr<FUnrealType> CurrentTypeNode = TypeNode;
		for (FName PropertyName : PropertyNames)
		{
			checkf(CurrentTypeNode.IsValid(), TEXT("A property in the chain (except the leaf) is not a struct property."));
			UProperty* NextProperty = CurrentTypeNode->Type->FindPropertyByName(PropertyName);
			checkf(NextProperty, TEXT("Cannot find property %s in container %s"), *PropertyName.ToString(), *CurrentTypeNode->Type->GetName());
			MigratableProperty = CurrentTypeNode->Properties.FindChecked(NextProperty);
			CurrentTypeNode = MigratableProperty->Type;
		}

		// Create migratable data.
		MigratableProperty->MigratableData = MakeShared<FUnrealMigratableData>();
		MigratableProperty->MigratableData->Handle = MigratableDataHandle++;
	}

	return TypeNode;
}

FUnrealFlatRepData GetFlatRepData(TSharedPtr<FUnrealType> TypeInfo)
{
	FUnrealFlatRepData RepData;
	RepData.Add(REP_MultiClient);
	RepData.Add(REP_SingleClient);

	VisitAllProperties(TypeInfo, [&RepData](TSharedPtr<FUnrealProperty> PropertyInfo)
	{
		if (PropertyInfo->ReplicationData.IsValid())
		{
			EReplicatedPropertyGroup Group = REP_MultiClient;
			switch (PropertyInfo->ReplicationData->Condition)
			{
			case COND_AutonomousOnly:
			case COND_OwnerOnly:
				Group = REP_SingleClient;
				break;
			}
			RepData[Group].Add(PropertyInfo->ReplicationData->Handle, PropertyInfo);
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

FCmdHandlePropertyMap GetFlatMigratableData(TSharedPtr<FUnrealType> TypeInfo)
{
	FCmdHandlePropertyMap MigratableData;
	VisitAllProperties(TypeInfo, [&MigratableData](TSharedPtr<FUnrealProperty> PropertyInfo)
	{
		if (PropertyInfo->MigratableData.IsValid())
		{
			MigratableData.Add(PropertyInfo->MigratableData->Handle, PropertyInfo);
		}
		return true;
	}, true);

	// Sort by property handle.
	MigratableData.KeySort([](uint16 A, uint16 B)
	{
		return A < B;
	});
	return MigratableData;
}

// Goes through all RPCs in the TypeInfo and returns a list of all the unique RPC source classes.
TArray<FString> GetRPCTypeOwners(TSharedPtr<FUnrealType> TypeInfo)
{
	TArray<FString> RPCTypeOwners;
	VisitAllObjects(TypeInfo, [&RPCTypeOwners](TSharedPtr<FUnrealType> Type)
	{
		for (auto& RPC : Type->RPCs)
		{
			FString RPCOwnerName = *RPC.Value->Function->GetOuter()->GetName();
			RPCTypeOwners.AddUnique(RPCOwnerName);
			UE_LOG(LogSpatialGDKInteropCodeGenerator, Log, TEXT("RPC Type Owner Found - %s ::  %s"), *RPCOwnerName, *RPC.Value->Function->GetName());
		}
		return true;
	}, true);
	return RPCTypeOwners;
}

FUnrealRPCsByType GetAllRPCsByType(TSharedPtr<FUnrealType> TypeInfo)
{
	FUnrealRPCsByType RPCsByType;
	RPCsByType.Add(RPC_Client);
	RPCsByType.Add(RPC_Server);
	RPCsByType.Add(RPC_NetMulticast);
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

TArray<TSharedPtr<FUnrealProperty>> GetFlatRPCParameters(TSharedPtr<FUnrealRPC> RPCNode)
{
	TArray<TSharedPtr<FUnrealProperty>> ParamList;
	VisitAllProperties(RPCNode, [&ParamList](TSharedPtr<FUnrealProperty> Property)
	{
		// If the property is a generic struct without NetSerialize, recurse further.
		if (Property->Property->IsA<UStructProperty>())
		{
			if (Cast<UStructProperty>(Property->Property)->Struct->StructFlags & STRUCT_NetSerializeNative)
			{
				// We want to skip recursing into structs which have NetSerialize implemented.
				// This is to prevent flattening their internal structure, they will be represented as 'bytes'.
				ParamList.Add(Property);
				return false;
			}

			// For static arrays we want to stop recursion and serialize the property.
			// Note: This will use NetSerialize or SerializeBin which is currently known to not recursively call NetSerialize on inner structs. UNR-333
			if (Property->Property->ArrayDim > 1)
			{
				ParamList.Add(Property);
				return false;
			}

			// Generic struct. Recurse further.
			return true;
		}

		// If the RepType is not a generic struct, such as Vector3f or Plane, add to ParamList and stop recursion.
		ParamList.Add(Property);
		return false;
	}, false);
	return ParamList;
}

TArray<TSharedPtr<FUnrealProperty>> GetPropertyChain(TSharedPtr<FUnrealProperty> LeafProperty)
{
	TArray<TSharedPtr<FUnrealProperty>> OutputChain;
	TSharedPtr<FUnrealProperty> CurrentProperty = LeafProperty;
	while (CurrentProperty.IsValid())
	{
		OutputChain.Add(CurrentProperty);
		if (CurrentProperty->ContainerType.IsValid())
		{
			TSharedPtr<FUnrealType> EnclosingType = CurrentProperty->ContainerType.Pin();
			CurrentProperty = EnclosingType->ParentProperty.Pin();
		}
		else
		{
			CurrentProperty.Reset();
		}
	}

	// As we started at the leaf property and worked our way up, we need to reverse the list at the end.
	Algo::Reverse(OutputChain);
	return OutputChain;
}
