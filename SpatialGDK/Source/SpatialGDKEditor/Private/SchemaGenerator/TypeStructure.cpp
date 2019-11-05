// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TypeStructure.h"

#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SCS_Node.h"
#include "SpatialGDKEditorSchemaGenerator.h"
#include "Utils/RepLayoutUtils.h"

using namespace SpatialGDKEditor::Schema;

TArray<EReplicatedPropertyGroup> GetAllReplicatedPropertyGroups()
{
	static TArray<EReplicatedPropertyGroup> Groups = {REP_MultiClient, REP_SingleClient};
	return Groups;
}

FString GetReplicatedPropertyGroupName(EReplicatedPropertyGroup Group)
{
	return Group == REP_SingleClient ? TEXT("OwnerOnly") : TEXT("");
}

void VisitAllObjects(TSharedPtr<FUnrealType> TypeNode, TFunction<bool(TSharedPtr<FUnrealType>)> Visitor)
{
	bool bShouldRecurseFurther = Visitor(TypeNode);
	for (auto& PropertyPair : TypeNode->Properties)
	{
		if (bShouldRecurseFurther && PropertyPair.Value->Type.IsValid())
		{
			// Recurse into subobjects.
			VisitAllObjects(PropertyPair.Value->Type, Visitor);
		}
	}
}

void VisitAllProperties(TSharedPtr<FUnrealType> TypeNode, TFunction<bool(TSharedPtr<FUnrealProperty>)> Visitor)
{
	for (auto& PropertyPair : TypeNode->Properties)
	{
		bool bShouldRecurseFurther = Visitor(PropertyPair.Value);
		if (bShouldRecurseFurther && PropertyPair.Value->Type.IsValid())
		{
			// Recurse into properties if they're structs.
			if (PropertyPair.Value->Property->IsA<UStructProperty>())
			{
				VisitAllProperties(PropertyPair.Value->Type, Visitor);
			}
		}
	}
}

// GenerateChecksum is a method which replicates how Unreal generates it's own CompatibleChecksum for RepLayout Cmds.
// The original code can be found in the Unreal Engine's RepLayout. We use this to ensure we have the correct property at run-time.
uint32 GenerateChecksum(UProperty* Property, uint32 ParentChecksum, int32 StaticArrayIndex)
{
	uint32 Checksum = 0;
	Checksum = FCrc::StrCrc32(*Property->GetName().ToLower(), ParentChecksum);            // Evolve checksum on name
	Checksum = FCrc::StrCrc32(*Property->GetCPPType(nullptr, 0).ToLower(), Checksum);     // Evolve by property type
	Checksum = FCrc::MemCrc32(&StaticArrayIndex, sizeof(StaticArrayIndex), Checksum);     // Evolve by StaticArrayIndex (to make all unrolled static array elements unique)
	return Checksum;
}

TSharedPtr<FUnrealProperty> CreateUnrealProperty(TSharedPtr<FUnrealType> TypeNode, UProperty* Property, uint32 ParentChecksum, uint32 StaticArrayIndex)
{
	TSharedPtr<FUnrealProperty> PropertyNode = MakeShared<FUnrealProperty>();
	PropertyNode->Property = Property;
	PropertyNode->ContainerType = TypeNode;
	PropertyNode->ParentChecksum = ParentChecksum;
	PropertyNode->StaticArrayIndex = StaticArrayIndex;

	// Generate a checksum for this PropertyNode to be used to match properties with the RepLayout Cmds later.
	PropertyNode->CompatibleChecksum = GenerateChecksum(Property, ParentChecksum, StaticArrayIndex);
	TypeNode->Properties.Add(Property, PropertyNode);
	return PropertyNode;
}

TSharedPtr<FUnrealType> CreateUnrealTypeInfo(UStruct* Type, uint32 ParentChecksum, int32 StaticArrayIndex)
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
		TSharedPtr<FUnrealProperty> PropertyNode = CreateUnrealProperty(TypeNode, Property, ParentChecksum, StaticArrayIndex);

		// If this property not a struct or object (which can contain more properties), stop here.
		if (!Property->IsA<UStructProperty>() && !Property->IsA<UObjectProperty>())
		{
			for (int i = 1; i < Property->ArrayDim; i++)
			{
				CreateUnrealProperty(TypeNode, Property, ParentChecksum, i);
			}
			continue;
		}

		// If this is a struct property, then get the struct type and recurse into it.
		if (Property->IsA<UStructProperty>())
		{
			UStructProperty* StructProperty = Cast<UStructProperty>(Property);

			// This is the property for the 0th struct array member.
			uint32 ParentPropertyNodeChecksum = PropertyNode->CompatibleChecksum;
			PropertyNode->Type = CreateUnrealTypeInfo(StructProperty->Struct, ParentPropertyNodeChecksum, 0);
			PropertyNode->Type->ParentProperty = PropertyNode;

			// For static arrays we need to make a new struct array member node.
			for (int i = 1; i < Property->ArrayDim; i++)
			{
				// Create a new PropertyNode.
				TSharedPtr<FUnrealProperty> StaticStructArrayPropertyNode = CreateUnrealProperty(TypeNode, Property, ParentChecksum, i);

				// Generate Type information on the inner struct.
				// Note: The parent checksum of the properties within a struct that is a member of a static struct array, is the checksum for the struct itself after index modification.
				StaticStructArrayPropertyNode->Type = CreateUnrealTypeInfo(StructProperty->Struct, StaticStructArrayPropertyNode->CompatibleChecksum, 0);
				StaticStructArrayPropertyNode->Type->ParentProperty = StaticStructArrayPropertyNode;
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
				UE_LOG(LogSpatialGDKSchemaGenerator, Verbose, TEXT("%s - editor only, skipping"), *Property->GetName());
				TypeNode->Properties.Remove(Property);
				continue;
			}

			// Check whether the outer is the CDO of the class we're generating for
			// or the CDO of any of its parent classes.
			// (this also covers generating schema for a Blueprint derived from the outer's class)
			UObject* Outer = Value->GetOuter();
			if ((Outer != nullptr) &&
				Outer->HasAnyFlags(RF_ClassDefaultObject) &&
				ContainerCDO->IsA(Outer->GetClass()))
			{
				UE_LOG(LogSpatialGDKSchemaGenerator, Verbose, TEXT("Property Class: %s Instance Class: %s"), *ObjectProperty->PropertyClass->GetName(), *Value->GetClass()->GetName());

				// This property is definitely a strong reference, recurse into it.
				PropertyNode->Type = CreateUnrealTypeInfo(Value->GetClass(), ParentChecksum, 0);
				PropertyNode->Type->ParentProperty = PropertyNode;
				PropertyNode->Type->Object = Value;
				PropertyNode->Type->Name = Value->GetFName();

				// For static arrays we need to make a new object array member node.
				for (int i = 1; i < Property->ArrayDim; i++)
				{
					TSharedPtr<FUnrealProperty> StaticObjectArrayPropertyNode = CreateUnrealProperty(TypeNode, Property, ParentChecksum, i);

					// Note: The parent checksum of static arrays of strong object references will be the parent checksum of this class.
					StaticObjectArrayPropertyNode->Type = CreateUnrealTypeInfo(Value->GetClass(), ParentChecksum, 0);
					StaticObjectArrayPropertyNode->Type->ParentProperty = StaticObjectArrayPropertyNode;
				}
				bHandleStaticArrayProperties = false;
			}
			else
			{
				// The values outer is not us, store as weak reference.
				UE_LOG(LogSpatialGDKSchemaGenerator, Verbose, TEXT("%s - %s weak reference (outer not this)"), *Property->GetName(), *ObjectProperty->PropertyClass->GetName());
			}
		}
		else
		{
			// If value is just nullptr, then we clearly don't own it.
			UE_LOG(LogSpatialGDKSchemaGenerator, Verbose, TEXT("%s - %s weak reference (null init)"), *Property->GetName(), *ObjectProperty->PropertyClass->GetName());
		}

		// Weak reference static arrays are handled as a single UObjectRef per static array member.
		if (bHandleStaticArrayProperties)
		{
			for (int i = 1; i < Property->ArrayDim; i++)
			{
				CreateUnrealProperty(TypeNode, Property, ParentChecksum, i);
			}
		}
	} // END TFieldIterator<UProperty>

	// Blueprint components don't exist on the CDO so we need to iterate over the
	// BlueprintGeneratedClass (and all of its blueprint parents) to find all blueprint components
	UClass* BlueprintClass = Class;
	while (UBlueprintGeneratedClass* BGC = Cast<UBlueprintGeneratedClass>(BlueprintClass))
	{
		if (USimpleConstructionScript* SCS = BGC->SimpleConstructionScript)
		{
			for (USCS_Node* Node : SCS->GetAllNodes())
			{
				if (Node->ComponentTemplate == nullptr)
				{
					continue;
				}

				for (auto& PropertyPair : TypeNode->Properties)
				{
					UObjectProperty* ObjectProperty = Cast<UObjectProperty>(PropertyPair.Key);
					if (ObjectProperty == nullptr) continue;
					TSharedPtr<FUnrealProperty> PropertyNode = PropertyPair.Value;

					if (ObjectProperty->GetName().Equals(Node->GetVariableName().ToString()))
					{
						PropertyNode->Type = CreateUnrealTypeInfo(ObjectProperty->PropertyClass, ParentChecksum, 0);
						PropertyNode->Type->ParentProperty = PropertyNode;
						PropertyNode->Type->Object = Node->ComponentTemplate;
						PropertyNode->Type->Name = ObjectProperty->GetFName();
					}
				}
			}
		}

		BlueprintClass = BlueprintClass->GetSuperClass();
	}

	// If this is not a class, exit now, as structs cannot have replicated properties.
	if (!Class)
	{
		return TypeNode;
	}

	// Set up replicated properties by reading the rep layout and matching the properties with the ones in the type node.
	// Based on inspection in InitFromObjectClass, the RepLayout will always replicate object properties using NetGUIDs, regardless of
	// ownership. However, the rep layout will recurse into structs and allocate rep handles for their properties, unless the condition
	// "Struct->StructFlags & STRUCT_NetSerializeNative" is true. In this case, the entire struct is replicated as a whole.
#if ENGINE_MINOR_VERSION <= 22
	FRepLayout RepLayout;
	RepLayout.InitFromObjectClass(Class);
#else
	TSharedPtr<FRepLayout> RepLayoutPtr = FRepLayout::CreateFromClass(Class, nullptr/*ServerConnection*/, ECreateRepLayoutFlags::None);
	FRepLayout& RepLayout = *RepLayoutPtr.Get();
#endif
	for (int CmdIndex = 0; CmdIndex < RepLayout.Cmds.Num(); ++CmdIndex)
	{
		FRepLayoutCmd& Cmd = RepLayout.Cmds[CmdIndex];
		if (Cmd.Type == ERepLayoutCmdType::Return || Cmd.Property == nullptr)
		{
			continue;
		}

		// Jump over invalid replicated property types
		if (Cmd.Property->IsA<UDelegateProperty>() || Cmd.Property->IsA<UMulticastDelegateProperty>() || Cmd.Property->IsA<UInterfaceProperty>())
		{
			continue;
		}

		FRepParentCmd& Parent = RepLayout.Parents[Cmd.ParentIndex];

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
				if (PropertyPair.Value->CompatibleChecksum == Cmd.CompatibleChecksum)
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

			for (TSharedPtr<FUnrealProperty>& RootProperty : RootProperties)
			{
				checkf(RootProperty->Type.IsValid(), TEXT("Properties in the AST which are parent properties in the rep layout must have child properties"));
				VisitAllProperties(RootProperty->Type, [&PropertyNode, &Cmd](TSharedPtr<FUnrealProperty> Property)
				{
					if (Property->CompatibleChecksum == Cmd.CompatibleChecksum)
					{
						checkf(!PropertyNode.IsValid(), TEXT("We've already found a previous property node with the same property. This indicates that we have a 'diamond of death' style situation."))
						PropertyNode = Property;
					}
					return true;
				});
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

		if (Cmd.Type == ERepLayoutCmdType::DynamicArray)
		{
			// Bypass the inner properties and null terminator cmd when processing dynamic arrays.
			CmdIndex = Cmd.EndCmd - 1;
		}
	} // END CMD FOR LOOP

	// Find the handover properties.
	uint16 HandoverDataHandle = 1;
	VisitAllProperties(TypeNode, [&HandoverDataHandle, &Class](TSharedPtr<FUnrealProperty> PropertyInfo)
	{
		if (PropertyInfo->Property->PropertyFlags & CPF_Handover)
		{
			if (UStructProperty* StructProp = Cast<UStructProperty>(PropertyInfo->Property))
			{
				if (StructProp->Struct->StructFlags & STRUCT_NetDeltaSerializeNative)
				{
					// Warn about delta serialization
					UE_LOG(LogSpatialGDKSchemaGenerator, Warning, TEXT("%s in %s uses delta serialization. " \
						"This is not supported and standard serialization will be used instead."), *PropertyInfo->Property->GetName(), *Class->GetName());
				}
			}
			PropertyInfo->HandoverData = MakeShared<FUnrealHandoverData>();
			PropertyInfo->HandoverData->Handle = HandoverDataHandle++;
		}
		return true;
	});

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
	});

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

FCmdHandlePropertyMap GetFlatHandoverData(TSharedPtr<FUnrealType> TypeInfo)
{
	FCmdHandlePropertyMap HandoverData;
	VisitAllProperties(TypeInfo, [&HandoverData](TSharedPtr<FUnrealProperty> PropertyInfo)
	{
		if (PropertyInfo->HandoverData.IsValid())
		{
			HandoverData.Add(PropertyInfo->HandoverData->Handle, PropertyInfo);
		}
		return true;
	});

	// Sort by property handle.
	HandoverData.KeySort([](uint16 A, uint16 B)
	{
		return A < B;
	});
	return HandoverData;
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

FSubobjectMap GetAllSubobjects(TSharedPtr<FUnrealType> TypeInfo)
{
	FSubobjectMap Subobjects;

	TSet<UObject*> SeenComponents;
	uint32 CurrentOffset = 1;

	for (auto& PropertyPair : TypeInfo->Properties)
	{
		UProperty* Property = PropertyPair.Key;
		TSharedPtr<FUnrealType>& PropertyTypeInfo = PropertyPair.Value->Type;

		if (Property->IsA<UObjectProperty>() && PropertyTypeInfo.IsValid())
		{
			UObject* Value = PropertyTypeInfo->Object;

			if (Value != nullptr && IsSupportedClass(Value->GetClass()))
			{
				if (!SeenComponents.Contains(Value))
				{
					SeenComponents.Add(Value);
					Subobjects.Add(CurrentOffset, PropertyTypeInfo);
				}

				CurrentOffset++;
			}
		}
	}

	return Subobjects;
}
