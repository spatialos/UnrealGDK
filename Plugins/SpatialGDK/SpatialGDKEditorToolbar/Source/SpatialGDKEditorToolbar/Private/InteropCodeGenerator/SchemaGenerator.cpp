// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SchemaGenerator.h"

#include "Algo/Reverse.h"
#include "Algo/Transform.h"

#include "Utils/CodeWriter.h"
#include "Utils/ComponentIdGenerator.h"

FString UnrealNameToSchemaTypeName(const FString& UnrealName)
{
	return UnrealName.Replace(TEXT("_"), TEXT(""));
}

FString SchemaReplicatedDataName(EReplicatedPropertyGroup Group, UStruct* Type)
{
	return FString::Printf(TEXT("Unreal%s%sRepData"), *UnrealNameToSchemaTypeName(Type->GetName()), *GetReplicatedPropertyGroupName(Group));
}

FString SchemaMigratableDataName(UStruct* Type)
{
	return FString::Printf(TEXT("Unreal%sMigratableData"), *UnrealNameToSchemaTypeName(Type->GetName()));
}

FString SchemaRPCComponentName(ERPCType RpcType, UStruct* Type)
{
	return FString::Printf(TEXT("Unreal%s%sRPCs"), *UnrealNameToSchemaTypeName(Type->GetName()), *GetRPCTypeName(RpcType));
}

FString SchemaRPCRequestType(UFunction* Function)
{
	return FString::Printf(TEXT("Unreal%sRequest"), *UnrealNameToSchemaTypeName(Function->GetName()));
}

FString SchemaRPCResponseType(UFunction* Function)
{
	return FString::Printf(TEXT("Unreal%sResponse"), *UnrealNameToSchemaTypeName(Function->GetName()));
}

FString SchemaFieldName(const TSharedPtr<FUnrealProperty> Property)
{
	// Transform the property chain into a chain of names.
	TArray<FString> ChainNames;
	Algo::Transform(GetPropertyChain(Property), ChainNames, [](const TSharedPtr<FUnrealProperty>& Property) -> FString
	{
		// Note: Removing underscores to avoid naming mismatch between how schema compiler and interop generator process schema identifiers.
		return Property->Property->GetName().ToLower().Replace(TEXT("_"), TEXT(""));
	});

	// Prefix is required to disambiguate between properties in the generated code and UActorComponent/UObject properties
	// which the generated code extends :troll:.
	return TEXT("field_") + FString::Join(ChainNames, TEXT("_"));
}

FString SchemaCommandName(UFunction* Function)
{
	// Note: Removing underscores to avoid naming mismatch between how schema compiler and interop generator process schema identifiers.
	return Function->GetName().ToLower().Replace(TEXT("_"), TEXT(""));
}

FString CPPCommandClassName(UFunction* Function)
{
	FString SchemaName = SchemaCommandName(Function);
	SchemaName[0] = FChar::ToUpper(SchemaName[0]);
	return SchemaName;
}

FString PropertyToSchemaType(UProperty* Property)
{
	FString DataType;

	if (Property->IsA(UStructProperty::StaticClass()))
	{
		UStructProperty* StructProp = Cast<UStructProperty>(Property);
		UScriptStruct* Struct = StructProp->Struct;
		if (Struct->GetFName() == NAME_Vector ||
			Struct->GetName() == TEXT("Vector_NetQuantize100") ||
			Struct->GetName() == TEXT("Vector_NetQuantize10") ||
			Struct->GetName() == TEXT("Vector_NetQuantizeNormal") ||
			Struct->GetName() == TEXT("Vector_NetQuantize"))
		{
			DataType = TEXT("improbable.Vector3f"); // not well supported
		}
		else if (Struct->GetFName() == NAME_Rotator)
		{
			DataType = TEXT("UnrealFRotator");
		}
		else if (Struct->GetFName() == NAME_Plane)
		{
			DataType = TEXT("UnrealFPlane");
		}
		else
		{
			DataType = TEXT("bytes"); //this includes RepMovement and UniqueNetId
		}
	}
	else if (Property->IsA(UBoolProperty::StaticClass()))
	{
		DataType = TEXT("bool");
	}
	else if (Property->IsA(UFloatProperty::StaticClass()))
	{
		DataType = TEXT("float");
	}
	else if (Property->IsA(UIntProperty::StaticClass()))
	{
		DataType = TEXT("int32");
	}
	else if (Property->IsA(UByteProperty::StaticClass()))
	{
		DataType = TEXT("uint32"); // uint8 not supported in schema.
	}
	else if (Property->IsA(UNameProperty::StaticClass()) || Property->IsA(UStrProperty::StaticClass()))
	{
		DataType = TEXT("string");
	}
	else if (Property->IsA(UUInt32Property::StaticClass()))
	{
		DataType = TEXT("uint32");
	}
	else if (Property->IsA(UUInt64Property::StaticClass()))
	{
		DataType = TEXT("bytes");
	}
	else if (Property->IsA(UClassProperty::StaticClass()))
	{
		DataType = TEXT("uint32");	// Note: We hash the static class path names to UClass objects.
	}
	else if (Property->IsA(UObjectPropertyBase::StaticClass()))
	{
		DataType = TEXT("UnrealObjectRef");
	}
	else if (Property->IsA(UArrayProperty::StaticClass()))
	{
		DataType = PropertyToSchemaType(Cast<UArrayProperty>(Property)->Inner);
		DataType = FString::Printf(TEXT("list<%s>"), *DataType);
	}
	else
	{
		DataType = TEXT("bytes");
	}

	return DataType;
}
void AppendName(FString& ClassDescription, UProperty* Property)
{
	auto OwnerClass = Property->GetOwnerClass();
	if (OwnerClass == NULL) {
		ClassDescription += Property->GetOwnerStruct()->GetName();
		//ParentClassName += FString::Printf(TEXT(" // %s"), *RepProp.Value->Property->GetOwnerStruct()->GetOwnerClass()->GetName());
		//auto Ref = RepProp.Value->ContainerType.Pin();
		//if (Ref.IsValid()) {
		//	ClassDescription += Ref->Type->GetOuter()->GetName();
		//}
	}
	else {
		ClassDescription += OwnerClass->GetName();
	}
}

int GenerateTypeBindingSchema(FCodeWriter& Writer, int ComponentId, UClass* Class, TSharedPtr<FUnrealType> TypeInfo)
{
	FComponentIdGenerator IdGenerator(ComponentId);

	Writer.Print(R"""(
		// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
		// Note that this file has been generated automatically
		package improbable.unreal;

		import "improbable/vector3.schema";
		import "improbable/unreal/core_types.schema";)""");
	Writer.PrintNewLine();

	FUnrealFlatRepData RepData = GetFlatRepData(TypeInfo);

	// Client-server replicated properties.
	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		Writer.Printf("component %s {", *SchemaReplicatedDataName(Group, Class));
		Writer.Indent();
		Writer.Printf("id = %d;", IdGenerator.GetNextAvailableId());
		int FieldCounter = 0;
		for (auto& RepProp : RepData[Group])
		{
			FieldCounter++;
			//auto OwnerClass = RepProp.Value->Property->GetOwnerClass();
			//if (OwnerClass == NULL) {
			//	ParentClassName = RepProp.Value->Property->GetOwnerStruct()->GetName();
			//	//ParentClassName += FString::Printf(TEXT(" // %s"), *RepProp.Value->Property->GetOwnerStruct()->GetOwnerClass()->GetName());
			//	auto Ref = RepProp.Value->ContainerType.Pin();
			//	if (Ref.IsValid()){
			//		ParentClassName += Ref->Type->GetOuter()->GetName();
			//	}
			//} else {
			//	ParentClassName = OwnerClass->GetName();
			//}

			//// A node which represents an unreal type, such as ACharacter or UCharacterMovementComponent.
			//struct FUnrealType
			//{
			//	UStruct* Type;
			//	TMap<UProperty*, TSharedPtr<FUnrealProperty>> Properties;
			//	TMap<UFunction*, TSharedPtr<FUnrealRPC>> RPCs;
			//	TWeakPtr<FUnrealProperty> ParentProperty;
			//};

			//// A node which represents a single property or parameter in an RPC.
			//struct FUnrealProperty
			//{
			//	UProperty* Property;
			//	TSharedPtr<FUnrealType> Type; // Only set if strong reference to object/struct property.
			//	TSharedPtr<FUnrealRepData> ReplicationData; // Only set if property is replicated.
			//	TSharedPtr<FUnrealMigratableData> MigratableData; // Only set if property is migratable (and not replicated).
			//	TWeakPtr<FUnrealType> ContainerType; // Not set if this property is an RPC parameter.
			//};

			// RepProp.Value == FUnrealProperty // The base property we are searching from.
			// RepProp.Value->Type == FUnrealType // The type this property came from.
			// RepProp.Value->Type->ParentProperty // The parent of this unreal type.

			//AppendName();

			//auto ParentProperty = RepProp.Value->Type->ParentProperty.Pin();
			//while (ParentProperty.IsValid()) 
			//{
			//	auto OwnerClass = ParentProperty->Property->GetOwnerClass();
			//	if (OwnerClass != NULL) {
			//		ParentClassName += FString::Printf(TEXT(" // %s"), *OwnerClass->GetName());
			//	}
			//	else
			//	{
			//		ParentClassName += FString::Printf(TEXT(" // NULL"));
			//	}
			//	ParentProperty = ParentProperty->ContainerType.Pin()->ParentProperty.Pin();
			//}

			//auto OwnerClass = RepProp.Value->Property->GetOwnerClass();
			//if (OwnerClass == NULL)
			//{
			//}

			// while FUnrealProperty->Type->ParentProperty exists          then we need to get the name of this object.
			// FUnrealProperty = FUnrealProperty->Type->ParentProperty     recurse intp parent

			// Then we get the FUnrealProperty->Propetty->GetOuter()       // This should be where property variable 'lives'

			FString ParentClassName = TEXT("");

			auto ThisProp = RepProp.Value;
			auto loop = true;
			while (loop) {

				if (ThisProp->Type.IsValid()) // If we have a defined unreal type
				{
					if (ThisProp->Type->ParentProperty.IsValid()) // If we have a parent property, this should be the 'truth'
					{
						ThisProp = ThisProp->Type->ParentProperty.Pin();
						ParentClassName += FString::Printf(TEXT(" %s ::"), *ThisProp->Type->Type->GetName());
						
						// Duplicate Code (Oscillation)
						if (ThisProp->ContainerType.Pin()->ParentProperty.Pin().IsValid()) // Check the ContainerType for a parent property.
						{
							//ParentClassName += FString::Printf(TEXT(" %s ::"), *ThisProp->ContainerType.Pin()->Type->GetName());
							ThisProp = ThisProp->ContainerType.Pin()->ParentProperty.Pin();
						}
						else {
							loop = false;
						}
					}

				}
				else { // If we do not have an unreal type

					if (ThisProp->ContainerType.Pin()->ParentProperty.Pin().IsValid()) // Check the ContainerType for a parent property.
					{
						ParentClassName += FString::Printf(TEXT(" %s ::"), *ThisProp->ContainerType.Pin()->Type->GetName());
						ThisProp = ThisProp->ContainerType.Pin()->ParentProperty.Pin();
					} else {
						loop = false;
					}

				}
			}

			auto MaybeTheOwner = ThisProp->Property->GetOuter();
			if(MaybeTheOwner != nullptr)
			{
				ParentClassName += FString::Printf(TEXT(" %s"), *MaybeTheOwner->GetName());
			}

			// Insert parent class name here. Data comes from the rep prop.
			Writer.Printf("%s %s = %d; // %s // %s",
				*PropertyToSchemaType(RepProp.Value->Property),
				*SchemaFieldName(RepProp.Value),
				FieldCounter,
				*GetLifetimeConditionAsString(RepProp.Value->ReplicationData->Condition),
				*ParentClassName
			);
		}
		Writer.Outdent().Print("}");
	}

	// Worker-worker replicated properties.
	Writer.Printf("component %s {", *SchemaMigratableDataName(Class));
	Writer.Indent();
	Writer.Printf("id = %d;", IdGenerator.GetNextAvailableId());
	int FieldCounter = 0;
	for (auto& Prop : GetFlatMigratableData(TypeInfo))
	{
		FieldCounter++;
		Writer.Printf("%s %s = %d;",
			*PropertyToSchemaType(Prop.Value->Property),
			*SchemaFieldName(Prop.Value),
			FieldCounter
		);
	}
	Writer.Outdent().Print("}");

	// RPC components.
	FUnrealRPCsByType RPCsByType = GetAllRPCsByType(TypeInfo);
	for (auto Group : GetRPCTypes())
	{
		// Generate schema RPC command types
		for (auto& RPC : RPCsByType[Group])
		{
			FString TypeStr = SchemaRPCRequestType(RPC->Function);

			Writer.Printf("type %s {", *TypeStr);
			// Add location of rpc here
			//Writer.Printf("// class %s", );

			Writer.Indent();

			// Recurse into functions properties and build a complete transitive property list.
			TArray<TSharedPtr<FUnrealProperty>> ParamList = GetFlatRPCParameters(RPC);

			// RPC target subobject offset.
			Writer.Printf("uint32 target_subobject_offset = 1;");
			FieldCounter = 1;
			for (auto& Param : ParamList)
			{
				FieldCounter++;
				Writer.Printf("%s %s = %d;",
					*PropertyToSchemaType(Param->Property),
					*SchemaFieldName(Param),
					FieldCounter
				);
			}
			Writer.Outdent().Print("}");
		}
	}
	Writer.PrintNewLine();

	for (auto Group : GetRPCTypes())
	{
		// Generate ClientRPCs component
		Writer.Printf("component %s {", *SchemaRPCComponentName(Group, Class));
		Writer.Indent();
		Writer.Printf("id = %i;", IdGenerator.GetNextAvailableId());
		for (auto& RPC : RPCsByType[Group])
		{
			Writer.Printf("command UnrealRPCCommandResponse %s(%s);",
				*SchemaCommandName(RPC->Function),
				*SchemaRPCRequestType(RPC->Function));
		}
		Writer.Outdent().Print("}");
	}

	return IdGenerator.GetNumUsedIds();
}
