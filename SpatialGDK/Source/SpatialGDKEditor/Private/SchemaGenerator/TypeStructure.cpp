// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TypeStructure.h"

#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SCS_Node.h"
#include "SpatialGDKEditorSchemaGenerator.h"
#include "Utils/DataTypeUtilities.h"
#include "Utils/GDKPropertyMacros.h"
#include "Utils/RepLayoutUtils.h"

#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

using namespace SpatialGDKEditor::Schema;

TArray<EReplicatedPropertyGroup> GetAllReplicatedPropertyGroups()
{
	static TArray<EReplicatedPropertyGroup> Groups = { REP_MultiClient, REP_SingleClient };
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
			if (PropertyPair.Value->Property->IsA<GDK_PROPERTY(StructProperty)>())
			{
				VisitAllProperties(PropertyPair.Value->Type, Visitor);
			}
		}
	}
}

void VisitAllObjects(TSharedPtr<FUnrealOfflineType> TypeNode, TFunction<bool(TSharedPtr<FUnrealOfflineType>)> Visitor)
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

void VisitAllProperties(TSharedPtr<FUnrealOfflineType> TypeNode, TFunction<bool(TSharedPtr<FUnrealOfflineProperty>)> Visitor)
{
	for (auto& PropertyPair : TypeNode->Properties)
	{
		bool bShouldRecurseFurther = Visitor(PropertyPair.Value);
		if (bShouldRecurseFurther && PropertyPair.Value->Type.IsValid())
		{
			// Recurse into properties if they're structs.
			// if (PropertyPair.Value->Property->IsA<GDK_PROPERTY(StructProperty)>())
			if (PropertyPair.Value->bIsStruct)
			{
				VisitAllProperties(PropertyPair.Value->Type, Visitor);
			}
		}
	}
}

// GenerateChecksum is a method which replicates how Unreal generates it's own CompatibleChecksum for RepLayout Cmds.
// The original code can be found in the Unreal Engine's RepLayout. We use this to ensure we have the correct property at run-time.
uint32 GenerateChecksum(GDK_PROPERTY(Property) * Property, uint32 ParentChecksum, int32 StaticArrayIndex)
{
	uint32 Checksum = 0;
	Checksum = FCrc::StrCrc32(*Property->GetName().ToLower(), ParentChecksum);		  // Evolve checksum on name
	Checksum = FCrc::StrCrc32(*Property->GetCPPType(nullptr, 0).ToLower(), Checksum); // Evolve by property type
	Checksum = FCrc::MemCrc32(&StaticArrayIndex, sizeof(StaticArrayIndex),
							  Checksum); // Evolve by StaticArrayIndex (to make all unrolled static array elements unique)
	return Checksum;
}

TSharedPtr<FUnrealProperty> CreateUnrealProperty(TSharedPtr<FUnrealType> TypeNode, GDK_PROPERTY(Property) * Property, uint32 ParentChecksum,
												 uint32 StaticArrayIndex)
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
	for (TFieldIterator<GDK_PROPERTY(Property)> It(Type); It; ++It)
	{
		GDK_PROPERTY(Property)* Property = *It;

		// Create property node and add it to the AST.
		TSharedPtr<FUnrealProperty> PropertyNode = CreateUnrealProperty(TypeNode, Property, ParentChecksum, StaticArrayIndex);

		// If this property not a struct or object (which can contain more properties), stop here.
		if (!Property->IsA<GDK_PROPERTY(StructProperty)>() && !Property->IsA<GDK_PROPERTY(ObjectProperty)>())
		{
			for (int i = 1; i < Property->ArrayDim; i++)
			{
				CreateUnrealProperty(TypeNode, Property, ParentChecksum, i);
			}
			continue;
		}

		// If this is a struct property, then get the struct type and recurse into it.
		if (Property->IsA<GDK_PROPERTY(StructProperty)>())
		{
			GDK_PROPERTY(StructProperty)* StructProperty = GDK_CASTFIELD<GDK_PROPERTY(StructProperty)>(Property);

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
				// Note: The parent checksum of the properties within a struct that is a member of a static struct array, is the checksum
				// for the struct itself after index modification.
				StaticStructArrayPropertyNode->Type =
					CreateUnrealTypeInfo(StructProperty->Struct, StaticStructArrayPropertyNode->CompatibleChecksum, 0);
				StaticStructArrayPropertyNode->Type->ParentProperty = StaticStructArrayPropertyNode;
			}
			continue;
		}

		// If this is an object property, then we need to do two things:
		//
		// 1) Determine whether this property is a strong or weak reference to the object. Some subobjects (such as the
		// CharacterMovementComponent) are in fact owned by the character, and can be stored in the same entity as the character itself.
		// Some subobjects (such as the Controller field in AActor) is a weak reference, and should just store a reference to the real
		// object. We inspect the CDO to determine whether the owner of the property value is equal to itself. As structs don't have CDOs,
		// we assume that all object properties in structs are weak references.
		//
		// 2) Obtain the concrete object type stored in this property. For example, the property containing the CharacterMovementComponent
		// might be a property which stores a MovementComponent pointer, so we'd need to somehow figure out the real type being stored
		// there during runtime. This is determined by getting the CDO of this class to determine what is stored in that property.
		GDK_PROPERTY(ObjectProperty)* ObjectProperty = GDK_CASTFIELD<GDK_PROPERTY(ObjectProperty)>(Property);
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
			if ((Outer != nullptr) && Outer->HasAnyFlags(RF_ClassDefaultObject) && ContainerCDO->IsA(Outer->GetClass()))
			{
				UE_LOG(LogSpatialGDKSchemaGenerator, Verbose, TEXT("Property Class: %s Instance Class: %s"),
					   *ObjectProperty->PropertyClass->GetName(), *Value->GetClass()->GetName());

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
				UE_LOG(LogSpatialGDKSchemaGenerator, Verbose, TEXT("%s - %s weak reference (outer not this)"), *Property->GetName(),
					   *ObjectProperty->PropertyClass->GetName());
			}
		}
		else
		{
			// If value is just nullptr, then we clearly don't own it.
			UE_LOG(LogSpatialGDKSchemaGenerator, Verbose, TEXT("%s - %s weak reference (null init)"), *Property->GetName(),
				   *ObjectProperty->PropertyClass->GetName());
		}

		// Weak reference static arrays are handled as a single UObjectRef per static array member.
		if (bHandleStaticArrayProperties)
		{
			for (int i = 1; i < Property->ArrayDim; i++)
			{
				CreateUnrealProperty(TypeNode, Property, ParentChecksum, i);
			}
		}
	} // END TFieldIterator<GDK_PROPERTY(Property)>

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
					GDK_PROPERTY(ObjectProperty)* ObjectProperty = GDK_CASTFIELD<GDK_PROPERTY(ObjectProperty)>(PropertyPair.Key);
					if (ObjectProperty == nullptr)
						continue;
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
	TSharedPtr<FRepLayout> RepLayoutPtr = FRepLayout::CreateFromClass(Class, nullptr /*ServerConnection*/, ECreateRepLayoutFlags::None);
	FRepLayout& RepLayout = *RepLayoutPtr.Get();
	for (int CmdIndex = 0; CmdIndex < RepLayout.Cmds.Num(); ++CmdIndex)
	{
		FRepLayoutCmd& Cmd = RepLayout.Cmds[CmdIndex];
		if (Cmd.Type == ERepLayoutCmdType::Return || Cmd.Property == nullptr)
		{
			continue;
		}

		// Jump over invalid replicated property types
		if (Cmd.Property->IsA<GDK_PROPERTY(DelegateProperty)>() || Cmd.Property->IsA<GDK_PROPERTY(MulticastDelegateProperty)>()
			|| Cmd.Property->IsA<GDK_PROPERTY(InterfaceProperty)>())
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
		// This might be problematic if we have a property which is inside a struct, nested in another struct which is replicated. For
		// example:
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
		// The parents array will contain "Bar", and the cmds array will contain "Nested", but we have no reference to "Baz" anywhere in the
		// RepLayout. What we do here is recurse into all of Bar's properties in the AST until we find Baz.

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
			// It's possible to have duplicate parent properties (they are distinguished by ArrayIndex), so we make sure to look at them
			// all.
			TArray<TSharedPtr<FUnrealProperty>> RootProperties;
			TypeNode->Properties.MultiFind(Parent.Property, RootProperties);

			for (TSharedPtr<FUnrealProperty>& RootProperty : RootProperties)
			{
				checkf(RootProperty->Type.IsValid(),
					   TEXT("Properties in the AST which are parent properties in the rep layout must have child properties"));
				VisitAllProperties(RootProperty->Type, [&PropertyNode, &Cmd](TSharedPtr<FUnrealProperty> Property) {
					if (Property->CompatibleChecksum == Cmd.CompatibleChecksum)
					{
						checkf(!PropertyNode.IsValid(), TEXT("We've already found a previous property node with the same property. This "
															 "indicates that we have a 'diamond of death' style situation."));
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
#if ENGINE_MINOR_VERSION >= 25
		if (Class->IsChildOf(AActor::StaticClass()))
		{
			// Uses the same pattern as ComponentReader::ApplySchemaObject and ReceivePropertyHelper
			if (UNLIKELY((int32)AActor::ENetFields_Private::RemoteRole == Cmd.ParentIndex))
			{
				const int32 SwappedCmdIndex = RepLayout.Parents[(int32)AActor::ENetFields_Private::Role].CmdStart;
				RepDataNode->RoleSwapHandle = static_cast<int32>(RepLayout.Cmds[SwappedCmdIndex].RelativeHandle);
			}
			else if (UNLIKELY((int32)AActor::ENetFields_Private::Role == Cmd.ParentIndex))
			{
				const int32 SwappedCmdIndex = RepLayout.Parents[(int32)AActor::ENetFields_Private::RemoteRole].CmdStart;
				RepDataNode->RoleSwapHandle = static_cast<int32>(RepLayout.Cmds[SwappedCmdIndex].RelativeHandle);
			}
		}
#else
		if (Parent.RoleSwapIndex != -1)
		{
			const int32 SwappedCmdIndex = RepLayout.Parents[Parent.RoleSwapIndex].CmdStart;
			RepDataNode->RoleSwapHandle = static_cast<int32>(RepLayout.Cmds[SwappedCmdIndex].RelativeHandle);
		}
#endif
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
	VisitAllProperties(TypeNode, [&HandoverDataHandle, &Class](TSharedPtr<FUnrealProperty> PropertyInfo) {
		if (PropertyInfo->Property->PropertyFlags & CPF_Handover)
		{
			if (GDK_PROPERTY(StructProperty)* StructProp = GDK_CASTFIELD<GDK_PROPERTY(StructProperty)>(PropertyInfo->Property))
			{
				if (StructProp->Struct->StructFlags & STRUCT_NetDeltaSerializeNative)
				{
					// Warn about delta serialization
					UE_LOG(LogSpatialGDKSchemaGenerator, Warning,
						   TEXT("%s in %s uses delta serialization. "
								"This is not supported and standard serialization will be used instead."),
						   *PropertyInfo->Property->GetName(), *Class->GetName());
				}
			}
			PropertyInfo->HandoverData = MakeShared<FUnrealHandoverData>();
			PropertyInfo->HandoverData->Handle = HandoverDataHandle++;
		}
		return true;
	});

	return TypeNode;
}

FUnrealFlatRepData GetFlatRepData(TSharedPtr<FUnrealOfflineType> TypeInfo)
{
	FUnrealFlatRepData RepData;
	RepData.Add(REP_MultiClient);
	RepData.Add(REP_SingleClient);

	VisitAllProperties(TypeInfo, [&RepData](TSharedPtr<FUnrealOfflineProperty> PropertyInfo) {
		if (PropertyInfo->ReplicationData.IsValid())
		{
			EReplicatedPropertyGroup Group = REP_MultiClient;
			switch (PropertyInfo->ReplicationData->Condition)
			{
			case COND_AutonomousOnly:
			case COND_ReplayOrOwner:
			case COND_OwnerOnly:
				Group = REP_SingleClient;
				break;
			}
			RepData[Group].Add(PropertyInfo->ReplicationData->Handle, PropertyInfo);
		}
		return true;
	});

	// Sort by replication handle.
	RepData[REP_MultiClient].KeySort([](uint16 A, uint16 B) {
		return A < B;
	});
	RepData[REP_SingleClient].KeySort([](uint16 A, uint16 B) {
		return A < B;
	});
	return RepData;
}

FCmdHandlePropertyMap GetFlatHandoverData(TSharedPtr<FUnrealOfflineType> TypeInfo)
{
	FCmdHandlePropertyMap HandoverData;
	VisitAllProperties(TypeInfo, [&HandoverData](TSharedPtr<FUnrealOfflineProperty> PropertyInfo) {
		if (PropertyInfo->HandoverData.IsValid())
		{
			HandoverData.Add(PropertyInfo->HandoverData->Handle, PropertyInfo);
		}
		return true;
	});

	// Sort by property handle.
	HandoverData.KeySort([](uint16 A, uint16 B) {
		return A < B;
	});
	return HandoverData;
}

TArray<TSharedPtr<FUnrealOfflineProperty>> GetPropertyChain(TSharedPtr<FUnrealOfflineProperty> LeafProperty)
{
	TArray<TSharedPtr<FUnrealOfflineProperty>> OutputChain;
	TSharedPtr<FUnrealOfflineProperty> CurrentProperty = LeafProperty;
	while (CurrentProperty.IsValid())
	{
		OutputChain.Add(CurrentProperty);
		if (CurrentProperty->ContainerType.IsValid())
		{
			TSharedPtr<FUnrealOfflineType> EnclosingType = CurrentProperty->ContainerType.Pin();
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

FSubobjectMap GetAllSubobjects(TSharedPtr<FUnrealOfflineType> TypeInfo)
{
	FSubobjectMap Subobjects;

	// TSet<UObject*> SeenComponents;
	TSet<uint32_t> SeenComponents;
	uint32 CurrentOffset = 1;

	for (auto& PropertyPair : TypeInfo->Properties)
	{
		// GDK_PROPERTY(Property)* Property = PropertyPair.Key;
		const TSharedPtr<FUnrealOfflineProperty>& Property = PropertyPair.Value;
		TSharedPtr<FUnrealOfflineType>& PropertyTypeInfo = PropertyPair.Value->Type;

		if (Property->bIsObjectProperty && PropertyTypeInfo.IsValid() && Property->SubobjectId != -1)
		{
			// UObject* Value = PropertyTypeInfo->Object;
			//
			// if (Value != nullptr && IsSupportedClass(Value->GetClass()))
			{
				if (!SeenComponents.Contains(Property->SubobjectId))
				{
					SeenComponents.Add(Property->SubobjectId);
					Subobjects.Add(CurrentOffset, PropertyTypeInfo);
				}

				CurrentOffset++;
			}
		}
	}

	return Subobjects;
}

struct OfflineConversionStackItem
{
	OfflineConversionStackItem(TSharedPtr<FUnrealType> InType, TWeakPtr<FUnrealOfflineType> InChildType, UClass* InClass)
		: Type(MoveTemp(InType))
		, ChildType(InChildType)
		, ClassToConsider(InClass)
	{
	}
	TSharedPtr<FUnrealType> Type;
	TWeakPtr<FUnrealOfflineType> ChildType;
	UClass* ClassToConsider;
};

TSharedPtr<FUnrealOfflineType> ConvertToOffline(const TSharedPtr<FUnrealType>& TypeInfo, bool StopAtRootPackage)
{
	TArray<TSharedPtr<FUnrealType>> TypeStack;
	TArray<TSharedPtr<FUnrealOfflineProperty>> PropStackOff;

	TMap<TSharedPtr<FUnrealType>, TSharedPtr<FUnrealOfflineType>> TypeMap;

	TMap<UObject*, uint32> SeenComponents;
	uint32 CurrentOffset = 1;

	UPackage* RootPackage = TypeInfo->Type->GetOutermost();

	TypeStack.Add(TypeInfo);
	while (TypeStack.Num() > 0)
	{
		TSharedPtr<FUnrealType> TypeToInspect = TypeStack.Pop();
		TSharedPtr<FUnrealOfflineType> ChildType;
		if (TSharedPtr<FUnrealOfflineType>* ExistingType = TypeMap.Find(TypeToInspect))
		{
			PropStackOff.Last()->Type = *ExistingType;
			PropStackOff.Pop();
			// PropStack.Pop();
		}
		else
		{
			UStruct* CurrentType = TypeToInspect->Type;

			while (CurrentType)
			{
				TSharedPtr<FUnrealOfflineType> NewType = MakeShared<FUnrealOfflineType>();

				if (ChildType)
				{
					ChildType->ParentType = NewType;
				}
				else
				{
					TypeMap.Add(TypeToInspect, NewType);
				}
				NewType->TypeName = CurrentType->GetName();
				NewType->TypePath = CurrentType->GetPathName();

				if (StopAtRootPackage && CurrentType->GetOutermost() != RootPackage)
				{
					break;
				}

				if (PropStackOff.Num() > 0)
				{
					NewType->ParentProperty = PropStackOff.Last();
					PropStackOff.Last()->Type = NewType;
					PropStackOff.Pop();
				}

				for (auto const& Prop : TypeToInspect->Properties)
				{
					const TSharedPtr<FUnrealProperty>& Property = Prop.Value;

					if (StopAtRootPackage && Property->Property->GetOwnerClass() != CurrentType)
					{
						continue;
					}

					TSharedPtr<FUnrealOfflineProperty> NewProperty = MakeShared<FUnrealOfflineProperty>();
					NewProperty->bIsStruct = Property->Property->IsA(GDK_PROPERTY(StructProperty)::StaticClass());
					NewProperty->bIsArrayProperty = Property->Property->IsA(GDK_PROPERTY(ArrayProperty)::StaticClass());
					GDK_PROPERTY(ObjectProperty)* ObjProp = nullptr;
					if (NewProperty->bIsArrayProperty)
					{
						GDK_PROPERTY(ArrayProperty)* ArrayProp = GDK_CASTFIELD<GDK_PROPERTY(ArrayProperty)>(Property->Property);
						ObjProp = GDK_CASTFIELD<GDK_PROPERTY(ObjectProperty)>(ArrayProp->Inner);
						NewProperty->bIsObjectProperty = ObjProp != nullptr;
					}
					else
					{
						ObjProp = GDK_CASTFIELD<GDK_PROPERTY(ObjectProperty)>(Property->Property);
						NewProperty->bIsObjectProperty = ObjProp != nullptr;
					}

					NewProperty->SubobjectId = -1;
					if (ObjProp)
					{
						if (Property->Type.IsValid())
						{
							UObject* Value = Property->Type->Object;
							if (Value != nullptr && IsSupportedClass(ObjProp->PropertyClass))
							{
								uint32* SubobjectId = SeenComponents.Find(Value);
								if (SubobjectId == nullptr)
								{
									SeenComponents.Add(Value, CurrentOffset++);
								}
								else
								{
									NewProperty->SubobjectId = *SubobjectId;
								}
							}
						}
					}

					NewProperty->ContainerType = NewType;
					NewProperty->PropertyName = Property->Property->GetName();
					NewProperty->PropertyPath = Property->Property->GetPathName();
					NewProperty->SchemaType = PropertyToSchemaType(Property->Property);

					NewProperty->ArrayDim = Property->Property->ArrayDim;
					NewProperty->CompatibleChecksum = Property->CompatibleChecksum;
					NewProperty->HandoverData = Property->HandoverData;
					NewProperty->ParentChecksum = Property->ParentChecksum;
					NewProperty->ReplicationData = Property->ReplicationData;
					NewProperty->StaticArrayIndex = Property->StaticArrayIndex;

					NewType->Properties.Add(Property->Property->GetName(), NewProperty);

					if (Property->Type)
					{
						// PropStack.Add(Property);
						PropStackOff.Add(NewProperty);
						TypeStack.Add(Property->Type);
					}
				}

				CurrentType = StopAtRootPackage ? CurrentType->GetSuperStruct() : nullptr;
				ChildType = NewType;
			}

			// if (CurrentClass && ChildType)
			//{
			//	ChildType->ParentTypeName = CurrentClass->GetName();
			//	ChildType->ParentTypePath = CurrentClass->GetPathName();
			//}
		}
	}

	return *TypeMap.Find(TypeInfo);
}

struct TypeImport
{
	FString Path;
	FString TypeName;
	uint32_t Index;
};

void WriteReplicationData(TSharedRef<TJsonWriter<>> Writer, const FUnrealRepData& Data)
{
	Writer->WriteValue(TEXT("RepLayoutType"), (int32)Data.RepLayoutType);
	Writer->WriteValue(TEXT("Condition"), (int32)Data.Condition);
	Writer->WriteValue(TEXT("RepNotifyCondition"), (int32)Data.RepNotifyCondition);
	Writer->WriteValue(TEXT("Handle"), Data.Handle);
	Writer->WriteValue(TEXT("RoleSwapHandle"), Data.RoleSwapHandle);
	Writer->WriteValue(TEXT("ArrayIndex"), Data.ArrayIndex);
}

TSharedPtr<FUnrealRepData> ReadReplicationData(TSharedRef<FJsonObject> Obj)
{
	TSharedPtr<FUnrealRepData> NewRepData = MakeShared<FUnrealRepData>();

	NewRepData->RepLayoutType = (ERepLayoutCmdType)Obj->GetIntegerField(TEXT("RepLayoutType"));
	NewRepData->Condition = (ELifetimeCondition)Obj->GetIntegerField(TEXT("Condition"));
	NewRepData->RepNotifyCondition = (ELifetimeRepNotifyCondition)Obj->GetIntegerField(TEXT("RepNotifyCondition"));
	NewRepData->Handle = Obj->GetIntegerField(TEXT("Handle"));
	NewRepData->RoleSwapHandle = Obj->GetIntegerField(TEXT("RoleSwapHandle"));
	NewRepData->ArrayIndex = Obj->GetIntegerField(TEXT("ArrayIndex"));

	return NewRepData;
}

void WriteOfflineInfo(const FString& Path, const FUnrealClassDesc& Desc, TSharedPtr<FUnrealOfflineType> TypeInfo)
{
	FString RootPath = TypeInfo->TypePath;

	TSet<FUnrealOfflineType*> TypesToWrite;
	TMap<FUnrealOfflineType*, TypeImport> Imports;

	TArray<TSharedPtr<FUnrealOfflineType>> TypeStack;

	TypeStack.Add(TypeInfo);
	while (TypeStack.Num() > 0)
	{
		TSharedPtr<FUnrealOfflineType> TypeToInspect = TypeStack.Pop();

		if (TypeToInspect->TypePath != RootPath)
		{
			TypeImport Import;
			Import.TypeName = TypeToInspect->TypeName;
			Import.Path = TypeToInspect->TypePath;
			Import.Index = Imports.Num();
			Imports.Add(TypeToInspect.Get(), Import);
		}
		else
		{
			TypesToWrite.Add(TypeToInspect.Get());
			if (TypeToInspect->ParentType)
			{
				TypeStack.Add(TypeToInspect->ParentType);
			}
			for (auto const& Prop : TypeToInspect->Properties)
			{
				const TSharedPtr<FUnrealOfflineProperty>& Property = Prop.Value;
				if (Property->Type && !TypesToWrite.Contains(Property->Type.Get()))
				{
					TypeStack.Add(Property->Type);
				}
			}
		}
	}

	FString Text;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Text);

	Writer->WriteObjectStart();
	Writer->WriteObjectStart("ClassDesc");
	Writer->WriteValue(TEXT("Name"), Desc.ClassName);
	Writer->WriteValue(TEXT("Path"), Desc.ClassPath);
	Writer->WriteValue(TEXT("IsActor"), Desc.bIsActor);
	Writer->WriteValue(TEXT("IsActorComponent"), Desc.bIsActorComponent);
	Writer->WriteValue(TEXT("NetCullDistance"), Desc.ClassNCD);
	Writer->WriteValue(TEXT("RootType"), TypeInfo->TypeName);
	Writer->WriteObjectEnd();

	Writer->WriteArrayStart(TEXT("Imports"));
	for (auto const& Import : Imports)
	{
		Writer->WriteObjectStart();
		Writer->WriteValue(TEXT("Name"), Import.Value.TypeName);
		Writer->WriteValue(TEXT("Path"), Import.Value.Path);
		Writer->WriteObjectEnd();
	}
	Writer->WriteArrayEnd();

	Writer->WriteArrayStart(TEXT("Types"));
	for (auto const& TypePtr : TypesToWrite)
	{
		FUnrealOfflineType& Type = *TypePtr;
		Writer->WriteObjectStart();
		Writer->WriteValue(TEXT("Name"), Type.Name.ToString());
		Writer->WriteValue(TEXT("TypeName"), Type.TypeName);
		Writer->WriteValue(TEXT("TypePath"), Type.TypePath);

		if (Type.ParentType)
		{
			if (TypesToWrite.Contains(Type.ParentType.Get()))
			{
				Writer->WriteValue(TEXT("ParentType"), Type.ParentType->TypeName);
			}
			else
			{
				const TypeImport* Import = Imports.Find(Type.ParentType.Get());
				check(Import);
				Writer->WriteValue(TEXT("ParentType"), FString::Printf(TEXT("*%i"), Import->Index));
			}
		}

		Writer->WriteArrayStart(TEXT("Properties"));
		for (auto const& PropPtr : Type.Properties)
		{
			const FUnrealOfflineProperty& Property = *PropPtr.Value;
			Writer->WriteObjectStart();
			Writer->WriteValue(TEXT("Key"), PropPtr.Key);
			if (TypesToWrite.Contains(Property.Type.Get()))
			{
				Writer->WriteValue(TEXT("Type"), Property.Type->TypeName);
			}
			else if (Property.Type)
			{
				const TypeImport* Import = Imports.Find(Property.Type.Get());
				check(Import);
				Writer->WriteValue(TEXT("Type"), FString::Printf(TEXT("*%i"), Import->Index));
			}

			Writer->WriteValue(TEXT("IsArrayProperty"), Property.bIsArrayProperty);
			Writer->WriteValue(TEXT("IsObjectProperty"), Property.bIsObjectProperty);
			Writer->WriteValue(TEXT("IsStruct"), Property.bIsStruct);

			//!!!! PATCH PARENT POINTER !!!!
			//!!!! THE MOST DERIVED PATCH MUST WIN !!!!
			Writer->WriteValue(TEXT("SubobjectId"), (int32)Property.SubobjectId);

			Writer->WriteValue(TEXT("PropertyName"), Property.PropertyName);
			Writer->WriteValue(TEXT("SchemaType"), Property.SchemaType);
			Writer->WriteValue(TEXT("ArrayDim"), (int32)Property.ArrayDim);
			Writer->WriteValue(TEXT("CompatibleChecksum"), (int32)Property.CompatibleChecksum);

			if (Property.HandoverData)
			{
				Writer->WriteObjectStart(TEXT("HandoverData"));
				Writer->WriteValue(TEXT("Handle"), Property.HandoverData->Handle);
				Writer->WriteObjectEnd();
			}

			Writer->WriteValue(TEXT("ParentCheckSum"), (int32)Property.ParentChecksum);

			if (Property.ReplicationData)
			{
				Writer->WriteObjectStart(TEXT("ReplicationData"));
				WriteReplicationData(Writer, *Property.ReplicationData);
				Writer->WriteObjectEnd();
			}

			Writer->WriteValue(TEXT("StaticArrayIndex"), Property.StaticArrayIndex);

			Writer->WriteObjectEnd();
		}
		Writer->WriteArrayEnd();
		Writer->WriteObjectEnd();
	}
	Writer->WriteArrayEnd();

	Writer->WriteObjectEnd();

	Writer->Close();

	FFileHelper::SaveStringToFile(Text, *Path);
}

TArray<TSharedPtr<FUnrealType>> GetTypeInfosForClasses(const TSet<UClass*>& Classes, bool bNativeClass)
{
	TArray<TSharedPtr<FUnrealType>> TypeInfos;
	TSet<UClass*> VisitedClasses;

	for (const auto& Class : Classes)
	{
		bool bIsUAsset = Class->GetOutermost()->GetPathName().StartsWith(TEXT("/Game"));
		if (bIsUAsset != !bNativeClass)
		{
			continue;
		}
		if (!IsSupportedClass(Class))
		{
			continue;
		}

		if (VisitedClasses.Contains(Class))
		{
			continue;
		}

		VisitedClasses.Add(Class);
		// Parent and static array index start at 0 for checksum calculations.
		TSharedPtr<FUnrealType> TypeInfo = CreateUnrealTypeInfo(Class, 0, 0);
		UE_LOG(LogTemp, Log, TEXT("ComputedTypeInfo for %s"), *TypeInfo->Name.ToString());
		TypeInfos.Add(TypeInfo);
		VisitAllObjects(TypeInfo, [&](TSharedPtr<FUnrealType> TypeNode) {
			if (UClass* NestedClass = Cast<UClass>(TypeNode->Type))
			{
				bool bIsUAsset = Class->GetOutermost()->GetPathName().StartsWith(TEXT("/Game"));
				if (bIsUAsset != !bNativeClass && !VisitedClasses.Contains(NestedClass) && IsSupportedClass(NestedClass))
				{
					TypeInfos.Add(CreateUnrealTypeInfo(NestedClass, 0, 0));
					VisitedClasses.Add(NestedClass);
				}
			}
			return true;
		});
	}

	return TypeInfos;
}

FUnrealClassDesc::FUnrealClassDesc(UClass* ZeClass)
{
	ClassName = ZeClass->GetName();
	ClassPath = ZeClass->GetPathName();
	bIsActor = ZeClass->IsChildOf<AActor>();
	bIsActorComponent = ZeClass->IsChildOf<UActorComponent>();
	if (bIsActor)
	{
		AActor* DefaultActor = Cast<AActor>(ZeClass->GetDefaultObject());
		check(DefaultActor);
		ClassNCD = DefaultActor->NetCullDistanceSquared;
	}
}

TypeInfoDatabase::TypeInfoDatabase()
{
	TArray<UObject*> AllClasses;
	GetObjectsOfClass(UClass::StaticClass(), AllClasses);

	TSet<UClass*> Classes;

	for (const auto& ClassIt : AllClasses)
	{
		UClass* SupportedClass = Cast<UClass>(ClassIt);

		Classes.Add(SupportedClass);
	}

	TArray<TSharedPtr<FUnrealType>> TypeInfos = GetTypeInfosForClasses(Classes, true);

	for (auto const& Info : TypeInfos)
	{
		ClassEntry NewEntry;

		UClass* ZeClass = Cast<UClass>(Info->Type);
		NewEntry.Class = FUnrealClassDesc(ZeClass);
		NewEntry.RootType = ConvertToOffline(Info, false);
		NewEntry.bIsNative = true;
		NativeEntries.Add(NewEntry.Class.ClassPath, MoveTemp(NewEntry));
	}

	for (auto& Entry : NativeEntries)
	{
		ComputeFlattenedType(Entry.Value);
	}
}

void TypeInfoDatabase::ReadInfo(const FString& Path)
{
	TSharedPtr<FJsonValue> TypeInfoJson;
	{
		TUniquePtr<FArchive> TypeInfoFile(IFileManager::Get().CreateFileReader(*Path));

		if (!TypeInfoFile)
		{
			return;
		}

		TSharedRef<TJsonReader<char>> JsonReader = TJsonReader<char>::Create(TypeInfoFile.Get());

		FJsonSerializer::Deserialize(*JsonReader, TypeInfoJson);
	}
	if (!TypeInfoJson)
	{
		return;
	}

	TSharedPtr<FJsonObject> RootObject = TypeInfoJson->AsObject();

	ClassEntry NewEntry;

	auto ClassObject = RootObject->GetObjectField(TEXT("ClassDesc"));

	NewEntry.Class.ClassName = ClassObject->GetStringField(TEXT("Name"));
	NewEntry.Class.ClassPath = ClassObject->GetStringField(TEXT("Path"));
	NewEntry.Class.bIsActor = ClassObject->GetBoolField(TEXT("IsActor"));
	NewEntry.Class.bIsActorComponent = ClassObject->GetBoolField(TEXT("IsActorComponent"));
	NewEntry.Class.ClassNCD = ClassObject->GetNumberField(TEXT("NetCullDIstance"));

	FString RootType = ClassObject->GetStringField(TEXT("RootType"));

	auto ImportsArray = RootObject->GetArrayField(TEXT("Imports"));

	for (auto const& ImportItem : ImportsArray)
	{
		auto ImportObject = ImportItem->AsObject();

		Import NewImport;
		NewImport.TypeName = ImportObject->GetStringField(TEXT("Name"));
		NewImport.TypePath = ImportObject->GetStringField(TEXT("Path"));
		NewEntry.Imports.Add(MoveTemp(NewImport));
	}

	TMap<FString, TArray<TSharedPtr<FUnrealOfflineType>*>> InnerImports;

	auto TypesArray = RootObject->GetArrayField(TEXT("Types"));
	for (auto const& TypeItem : TypesArray)
	{
		auto TypeObject = TypeItem->AsObject();
		TSharedPtr<FUnrealOfflineType> Type = MakeShared<FUnrealOfflineType>();
		Type->TypeName = TypeObject->GetStringField(TEXT("TypeName"));
		Type->TypePath = TypeObject->GetStringField(TEXT("TypePath"));
		NewEntry.Types.Add(Type->TypeName, Type);
		if (RootType == Type->TypeName)
		{
			NewEntry.RootType = Type;
		}

		{
			FString ParentRef;
			if (TypeObject->TryGetStringField(TEXT("ParentType"), ParentRef))
			{
				if (ParentRef.StartsWith(TEXT("*")))
				{
					ParentRef.LeftChop(1);
					int32 ImportIndex = TCString<TCHAR>::Atoi(*ParentRef);
					NewEntry.Imports[ImportIndex].RefToPatch.Add(&Type->ParentType);
				}
				else
				{
					InnerImports.FindOrAdd(ParentRef).Add(&Type->ParentType);
				}
			}
		}

		auto PropertyArray = TypeObject->GetArrayField(TEXT("Properties"));

		for (auto const& PropItem : PropertyArray)
		{
			auto PropObject = PropItem->AsObject();

			TSharedPtr<FUnrealOfflineProperty> Property = MakeShared<FUnrealOfflineProperty>();

			FString Key = PropObject->GetStringField(TEXT("Key"));

			Type->Properties.Add(Key, Property);
			{
				FString PropTypeRef;
				if (PropObject->TryGetStringField(TEXT("Type"), PropTypeRef))
				{
					if (PropTypeRef.StartsWith(TEXT("*")))
					{
						PropTypeRef.LeftChop(1);
						int32 ImportIndex = TCString<TCHAR>::Atoi(*PropTypeRef);
						NewEntry.Imports[ImportIndex].RefToPatch.Add(&Property->Type);
					}
					else
					{
						InnerImports.FindOrAdd(PropTypeRef).Add(&Type->ParentType);
					}
				}
			}

			Property->bIsArrayProperty = PropObject->GetBoolField(TEXT("IsArrayProperty"));
			Property->bIsObjectProperty = PropObject->GetBoolField(TEXT("IsObjectProperty"));
			Property->bIsStruct = PropObject->GetBoolField(TEXT("IsStruct"));

			//!!!! PATCH PARENT POINTER !!!!
			//!!!! THE MOST DERIVED PATCH MUST WIN !!!!
			Property->SubobjectId = PropObject->GetIntegerField(TEXT("SubobjectId"));

			Property->PropertyName = PropObject->GetStringField(TEXT("PropertyName"));
			Property->SchemaType = PropObject->GetStringField(TEXT("SchemaType"));
			Property->ArrayDim = PropObject->GetIntegerField(TEXT("ArrayDim"));
			Property->CompatibleChecksum = PropObject->GetIntegerField(TEXT("CompatibleChecksum"));

			{
				const TSharedPtr<FJsonObject>* HandoverData;
				if (PropObject->TryGetObjectField(TEXT("HandoverData"), HandoverData))
				{
					Property->HandoverData = MakeShared<FUnrealHandoverData>();
					Property->HandoverData->Handle = (*HandoverData)->GetIntegerField(TEXT("Handle"));
				}
			}

			Property->ParentChecksum = PropObject->GetIntegerField(TEXT("ParentCheckSum"));

			{
				const TSharedPtr<FJsonObject>* ReplicationData;
				if (PropObject->TryGetObjectField(TEXT("ReplicationData"), ReplicationData))
				{
					Property->ReplicationData = ReadReplicationData(ReplicationData->ToSharedRef());
				}
			}

			Property->StaticArrayIndex = PropObject->GetIntegerField(TEXT("StaticArrayIndex"));
		}
	}

	for (auto& InnerImport : InnerImports)
	{
		TSharedPtr<FUnrealOfflineType>& Type = NewEntry.Types.FindChecked(InnerImport.Key);
		for (auto& PtrToPatch : InnerImport.Value)
		{
			*PtrToPatch = Type;
		}
	}

	ClassEntries.Add(NewEntry.Class.ClassPath, MoveTemp(NewEntry));
}

const TypeInfoDatabase::ClassEntry* TypeInfoDatabase::GetEntryForAsset(const FString& TypePath)
{
	return ClassEntries.Find(TypePath);
}

const TypeInfoDatabase::ClassEntry* TypeInfoDatabase::GetNativeEntry(const FString& TypePath)
{
	return NativeEntries.Find(TypePath);
}

TSharedPtr<FUnrealOfflineType> TypeInfoDatabase::ComputeFlattenedType(const ClassEntry& iEntry)
{
	if (iEntry.FlattenedRootType)
	{
		return iEntry.FlattenedRootType;
	}

	if (iEntry.Imports.Num() == 0)
	{
		// Should deep copy anyway...
		const_cast<ClassEntry&>(iEntry).FlattenedRootType = iEntry.RootType;
		return iEntry.FlattenedRootType;
	}

	TArray<TSharedPtr<FUnrealOfflineType>> ResolvedImports;

	for (auto const& ImportToResolve : iEntry.Imports)
	{
		const ClassEntry* FoundImport = GetNativeEntry(ImportToResolve.TypePath);
		if (!FoundImport)
		{
			if (iEntry.bIsNative)
			{
				return iEntry.FlattenedRootType;
			}
			FoundImport = GetEntryForAsset(ImportToResolve.TypePath);
		}
		if (!FoundImport)
		{
			return iEntry.FlattenedRootType;
		}

		// const TSharedPtr<FUnrealOfflineTy1pe>* FoundType = FoundImport->Types.Find(ImportToResolve.TypeName);
		check(FoundImport->RootType->TypeName == ImportToResolve.TypeName);

		ResolvedImports.Add(ComputeFlattenedType(*FoundImport));
	}

	TMap<const TSharedPtr<FUnrealOfflineType>*, uint32> RevImportMap;

	for (int32 i = 0; i < iEntry.Imports.Num(); ++i)
	{
		auto& CurImport = iEntry.Imports[i];
		for (auto Ptr : CurImport.RefToPatch)
		{
			RevImportMap.Add(Ptr, i);
		}
	}

	TSharedPtr<FUnrealOfflineType> RootType = iEntry.RootType;
	TSharedPtr<FUnrealOfflineType> FlattenedRootType;

	TMap<TSharedPtr<FUnrealOfflineType>, TSharedPtr<FUnrealOfflineType>> CopyMap;
	TMap<TSharedPtr<FUnrealOfflineType>*, TSharedPtr<FUnrealOfflineType>*> PointerPatchMap;

	TArray<TSharedPtr<FUnrealOfflineType>> TypeStack;
	TArray<TSharedPtr<FUnrealOfflineProperty>> PropertyStack;

	TypeStack.Add(RootType);

	while (!TypeStack.Num() == 0)
	{
		TSharedPtr<FUnrealOfflineType> CurType = TypeStack.Pop();

		if (auto TypePtr = CopyMap.Find(CurType))
		{
			check(PropertyStack.Num() > 0);
			PropertyStack.Last()->Type = *TypePtr;
			PropertyStack.Pop();
			continue;
		}

		TSharedPtr<FUnrealOfflineType> FlattenedType = MakeShared<FUnrealOfflineType>();
		if (CurType == RootType)
		{
			FlattenedRootType = FlattenedType;
		}
		FlattenedType->TypeName = CurType->TypeName;
		FlattenedType->TypePath = CurType->TypePath;

		if (PropertyStack.Num() > 0)
		{
			PropertyStack.Last()->Type = FlattenedType;
			PropertyStack.Pop();
		}

		CopyMap.Add(CurType, FlattenedType);

		for (auto const& Prop : CurType->Properties)
		{
			const TSharedPtr<FUnrealOfflineProperty>& CurProperty = Prop.Value;
			TSharedPtr<FUnrealOfflineProperty> NewProperty = MakeShared<FUnrealOfflineProperty>();

			*NewProperty = *CurProperty;
			FlattenedType->Properties.Add(Prop.Key, NewProperty);

			if (CurProperty->Type)
			{
				PropertyStack.Add(NewProperty);
				TypeStack.Add(CurProperty->Type);
			}
			else if (uint32* Import = RevImportMap.Find(&CurProperty->Type))
			{
				CurProperty->Type = ResolvedImports[*Import];
			}
		}

		if (uint32* Import = RevImportMap.Find(&(CurType->ParentType)))
		{
			TSharedPtr<FUnrealOfflineType>& ResolvedParent = ResolvedImports[*Import];
			for (auto const& Prop : ResolvedParent->Properties)
			{
				FlattenedType->Properties.Add(Prop.Key, Prop.Value);
			}
		}
	}

	const_cast<ClassEntry&>(iEntry).FlattenedRootType = FlattenedRootType;
	return FlattenedRootType;
}
