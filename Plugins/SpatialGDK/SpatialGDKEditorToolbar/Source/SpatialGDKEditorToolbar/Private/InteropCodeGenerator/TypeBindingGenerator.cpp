// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TypeBindingGenerator.h"
#include "SchemaGenerator.h"
#include "TypeStructure.h"

#include "Utils/CodeWriter.h"
#include "Utils/DataTypeUtilities.h"

// Needed for Algo::Transform
#include "Algo/Transform.h"

// Needed for TLess
#include "Templates/Less.h"

// Needed for std::bind.
#include <functional>

// Given an Unreal class, generates the name of the type binding class.
// For example: USpatialTypeBinding_Character.
FString TypeBindingName(UClass* Class)
{
	return FString::Printf(TEXT("USpatialTypeBinding_%s"), *Class->GetName());
}

// Given an UObjectProperty, return the string of the first native class in the inheritance hierarchy.
FString GetNativeClassName(const UObjectPropertyBase* Property)
{
	const UClass* Class = Property->PropertyClass;
	while (!Class->HasAnyClassFlags(CLASS_Native))
	{
		Class = Class->GetSuperClass();
	}
	return FString::Printf(TEXT("%s%s"), Class->GetPrefixCPP(), *Class->GetName());
}

FString CPPFieldName(TSharedPtr<FUnrealProperty> Property)
{
	// Transform the property chain into a chain of C++ names, joined by either -> or . (UObject or UStruct respectively).
	FString CPPFieldName;
	TArray<TSharedPtr<FUnrealProperty>> Chain = GetPropertyChain(Property);
	for (int i = 0; i < Chain.Num(); ++i)
	{
		UProperty* Prop = Chain[i]->Property;
		CPPFieldName += Prop->GetNameCPP();
		if (i < (Chain.Num() - 1))
		{
			// If this property is a UObject, use the arrow operator. Otherwise, use the dot operator.
			if (Prop->IsA<UObjectProperty>())
			{
				CPPFieldName += "->";
			}
			else
			{
				CPPFieldName += ".";
			}
		}
	}
	return CPPFieldName;
}

FString PropertyToWorkerSDKType(UProperty* Property, bool bIsRPCProperty)
{
	FString DataType;

	// For RPC arguments we may wish to handle them differently.
	if (bIsRPCProperty)
	{
		if (Property->ArrayDim > 1) // Static arrays in RPC arguments are replicated as lists.
		{
			DataType = PropertyToWorkerSDKType(Property, false);
			DataType = FString::Printf(TEXT("::worker::List<%s>"), *DataType);
			return DataType;
		}
	}

	if (Property->IsA(UStructProperty::StaticClass()))
	{
		DataType = TEXT("std::string"); // All structs serialize to 'bytes' and so we use std::string for now.
	}
	else if (Property->IsA(UBoolProperty::StaticClass()))
	{
		DataType = TEXT("bool");
	}
	else if (Property->IsA(UFloatProperty::StaticClass()))
	{
		DataType = TEXT("float");
	}
	else if (Property->IsA(UDoubleProperty::StaticClass()))
	{
		DataType = TEXT("double");
	}
	else if (Property->IsA(UInt8Property::StaticClass()))
	{
		DataType = TEXT("std::int32_t");
	}
	else if (Property->IsA(UInt16Property::StaticClass()))
	{
		DataType = TEXT("std::int32_t");
	}
	else if (Property->IsA(UIntProperty::StaticClass()))
	{
		DataType = TEXT("std::int32_t");
	}
	else if (Property->IsA(UInt64Property::StaticClass()))
	{
		DataType = TEXT("std::int64_t");
	}
	else if (Property->IsA(UByteProperty::StaticClass()))
	{
		DataType = TEXT("std::uint32_t"); // uint8 not supported in schema.
	}
	else if (Property->IsA(UUInt16Property::StaticClass()))
	{
		DataType = TEXT("std::uint32_t");
	}
	else if (Property->IsA(UUInt32Property::StaticClass()))
	{
		DataType = TEXT("std::uint32_t");
	}
	else if (Property->IsA(UUInt64Property::StaticClass()))
	{
		DataType = TEXT("std::uint64_t");
	}
	else if (Property->IsA(UNameProperty::StaticClass()) || Property->IsA(UStrProperty::StaticClass()))
	{
		DataType = TEXT("std::string");
	}
	else if (Property->IsA(UObjectPropertyBase::StaticClass()))
	{
		DataType = TEXT("improbable::unreal::UnrealObjectRef");
	}
	else if (Property->IsA(UArrayProperty::StaticClass()))
	{
		DataType = PropertyToWorkerSDKType(Cast<UArrayProperty>(Property)->Inner, bIsRPCProperty);
		DataType = FString::Printf(TEXT("::worker::List<%s>"), *DataType);
	}
	else if (Property->IsA(UEnumProperty::StaticClass()))
	{
		DataType = GetEnumDataType(Cast<UEnumProperty>(Property));
	}
	else
	{
		DataType = TEXT("std::string");
	}

	return DataType;
}

void GenerateUnrealToSchemaConversion(FCodeWriter& Writer, const FString& Update, UProperty* Property, const FString& PropertyValue, TFunction<void(const FString&)> ObjectResolveFailureGenerator, bool bIsRPCProperty, bool bUnresolvedObjectsHandledOutside)
{
	// For RPC arguments we may wish to handle them differently.
	if (bIsRPCProperty)
	{
		if (Property->ArrayDim > 1) // Static arrays in RPC arguments are replicated as lists.
		{
			Writer.Printf("::worker::List<%s> List;", *PropertyToWorkerSDKType(Property, false));
			Writer.Printf("for(int i = 0; i < sizeof(%s) / sizeof(%s[0]); i++)", *PropertyValue, *PropertyValue);
			Writer.BeginScope();
			GenerateUnrealToSchemaConversion(Writer, "List.emplace_back", Property, FString::Printf(TEXT("%s[i]"), *PropertyValue), ObjectResolveFailureGenerator, false, bUnresolvedObjectsHandledOutside);
			Writer.End();
			Writer.Printf("%s(List);", *Update);
			return;
		}
	}

	// Try to special case to custom types we know about
	if (Property->IsA(UStructProperty::StaticClass()))
	{
		UStructProperty * StructProp = Cast<UStructProperty>(Property);
		UScriptStruct * Struct = StructProp->Struct;

		// Common parts that are generated for any struct property regardless of whether it's NetSerialized.
		if (!bUnresolvedObjectsHandledOutside)
		{
			// Unresolved objects are not handled outside, so we should declare a set to store the unresolved objects in the current scope.
			Writer.Print("TSet<const UObject*> UnresolvedObjects;");
		}
		Writer.Print(R"""(
			TArray<uint8> ValueData;
			FSpatialMemoryWriter ValueDataWriter(ValueData, PackageMap, UnresolvedObjects);)""");

		if (Struct->StructFlags & STRUCT_NetSerializeNative)
		{
			// If user has implemented NetSerialize for custom serialization, we use that. Core structs like RepMovement or UniqueNetIdRepl also go through this path.
			Writer.Printf(R"""(
				bool bSuccess = true;
				(const_cast<%s&>(%s)).NetSerialize(ValueDataWriter, PackageMap, bSuccess);
				checkf(bSuccess, TEXT("NetSerialize on %s failed."));)""", *Struct->GetStructCPPName(), *PropertyValue, *Struct->GetStructCPPName());
		}
		else
		{
			// We do a basic binary serialization for the generic struct.
			Writer.Printf("%s::StaticStruct()->SerializeBin(ValueDataWriter, reinterpret_cast<void*>(const_cast<%s*>(&%s)));", *Property->GetCPPType(), *Property->GetCPPType(), *PropertyValue);
		}

		Writer.Printf("%s(std::string(reinterpret_cast<char*>(ValueData.GetData()), ValueData.Num()));", *Update);
	}
	else if (Property->IsA(UBoolProperty::StaticClass()))
	{
		Writer.Printf("%s(%s);", *Update, *PropertyValue);
	}
	else if (Property->IsA(UFloatProperty::StaticClass()))
	{
		Writer.Printf("%s(%s);", *Update, *PropertyValue);
	}
	else if (Property->IsA(UDoubleProperty::StaticClass()))
	{
		Writer.Printf("%s(%s);", *Update, *PropertyValue);
	}
	else if (Property->IsA(UInt8Property::StaticClass()))
	{
		Writer.Printf("%s(int32_t(%s));", *Update, *PropertyValue);
	}
	else if (Property->IsA(UInt16Property::StaticClass()))
	{
		Writer.Printf("%s(int32_t(%s));", *Update, *PropertyValue);
	}
	else if (Property->IsA(UIntProperty::StaticClass()))
	{
		Writer.Printf("%s(int32_t(%s));", *Update, *PropertyValue);
	}
	else if (Property->IsA(UInt64Property::StaticClass()))
	{
		Writer.Printf("%s(int64_t(%s));", *Update, *PropertyValue);
	}
	else if (Property->IsA(UByteProperty::StaticClass()))
	{
		Writer.Printf("%s(uint32_t(%s));", *Update, *PropertyValue);
	}
	else if (Property->IsA(UUInt16Property::StaticClass()))
	{
		Writer.Printf("%s(uint32_t(%s));", *Update, *PropertyValue);
	}
	else if (Property->IsA(UUInt32Property::StaticClass()))
	{
		Writer.Printf("%s(uint32_t(%s));", *Update, *PropertyValue);
	}
	else if (Property->IsA(UUInt64Property::StaticClass()))
	{
		Writer.Printf("%s(uint64_t(%s));", *Update, *PropertyValue);
	}
	else if (Property->IsA(UObjectPropertyBase::StaticClass()))
	{
		Writer.Printf("if (%s != nullptr)", *PropertyValue);
		Writer.BeginScope();
		Writer.Printf(R"""(
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(%s);
			if (!NetGUID.IsValid())
			{
				if (%s->IsFullNameStableForNetworking())
				{
					NetGUID = PackageMap->ResolveStablyNamedObject(%s);
				}
			}
			improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
			if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF))""", *PropertyValue, *PropertyValue, *PropertyValue);
		Writer.BeginScope();
		ObjectResolveFailureGenerator(*PropertyValue);
		Writer.End();
		Writer.Printf(R"""(
			else
			{
				%s(ObjectRef);
			})""", *Update);
		Writer.End();
		Writer.Printf(R"""(
			else
			{
				%s(SpatialConstants::NULL_OBJECT_REF);
			})""", *Update);
	}
	else if (Property->IsA(UNameProperty::StaticClass()))
	{
		Writer.Printf("%s(TCHAR_TO_UTF8(*%s.ToString()));", *Update, *PropertyValue);
	}
	else if (Property->IsA(UStrProperty::StaticClass()))
	{
		Writer.Printf("%s(TCHAR_TO_UTF8(*%s));", *Update, *PropertyValue);
	}
	else if (Property->IsA(UTextProperty::StaticClass()))
	{
		Writer.Printf("%s(TCHAR_TO_UTF8(*%s.ToString()));", *Update, *PropertyValue);
	}
	else if (Property->IsA(UArrayProperty::StaticClass()))
	{
		UArrayProperty* ArrayProperty = Cast<UArrayProperty>(Property);

		Writer.Printf("::worker::List<%s> List;", *PropertyToWorkerSDKType(ArrayProperty->Inner, bIsRPCProperty));

		Writer.Printf("for(int i = 0; i < %s.Num(); i++)", *PropertyValue);
		Writer.BeginScope();

		GenerateUnrealToSchemaConversion(Writer, "List.emplace_back", ArrayProperty->Inner, FString::Printf(TEXT("%s[i]"), *PropertyValue), ObjectResolveFailureGenerator, bIsRPCProperty, bUnresolvedObjectsHandledOutside);

		Writer.End();

		Writer.Printf("%s(List);", *Update);
	} 
	else if (Property->IsA(UEnumProperty::StaticClass()))
	{
		FString DataType = GetEnumDataType(Cast<UEnumProperty>(Property));
		Writer.Printf("%s(%s(%s));", *Update, *DataType, *PropertyValue);
	}
	else
	{
		Writer.Printf("// UNSUPPORTED U%s (unhandled) %s(%s)", *Property->GetClass()->GetName(), *Update, *PropertyValue);
	}
}

void GeneratePropertyToUnrealConversion(FCodeWriter& Writer, const FString& Update, const UProperty* Property, const FString& PropertyValue, TFunction<void(const FString&)> ObjectResolveFailureGenerator, bool bIsRPCProperty)
{
	FString PropertyType = Property->GetCPPType();

	// For RPC arguments we may wish to handle them differently.
	if (bIsRPCProperty)
	{
		if (Property->ArrayDim > 1) // Static arrays in RPC arguments are replicated as lists.
		{
			Writer.Printf("auto& List = %s;", *Update);
			Writer.Print("for(int i = 0; i < List.size(); i++)");
			Writer.BeginScope();
			GeneratePropertyToUnrealConversion(Writer, "List[i]", Property, FString::Printf(TEXT("%s[i]"), *PropertyValue), ObjectResolveFailureGenerator, false);
			Writer.End();
			return;
		}
	}

	// Try to special case to custom types we know about
	if (Property->IsA(UStructProperty::StaticClass()))
	{
		const UStructProperty * StructProp = Cast<UStructProperty>(Property);
		UScriptStruct * Struct = StructProp->Struct;

		if (Struct->StructFlags & STRUCT_NetSerializeNative)
		{
			// If user has implemented NetSerialize for custom serialization, we use that. Core structs like RepMovement or UniqueNetIdRepl also go through this path.
			Writer.Printf(R"""(
				auto& ValueDataStr = %s;
				TArray<uint8> ValueData;
				ValueData.Append(reinterpret_cast<const uint8*>(ValueDataStr.data()), ValueDataStr.size());
				FSpatialMemoryReader ValueDataReader(ValueData, PackageMap);
				bool bSuccess = true;
				%s.NetSerialize(ValueDataReader, PackageMap, bSuccess);
				checkf(bSuccess, TEXT("NetSerialize on %s failed."));)""", *Update, *PropertyValue, *PropertyType);
		}
		else
		{
			Writer.Printf(R"""(
				auto& ValueDataStr = %s;
				TArray<uint8> ValueData;
				ValueData.Append(reinterpret_cast<const uint8*>(ValueDataStr.data()), ValueDataStr.size());
				FSpatialMemoryReader ValueDataReader(ValueData, PackageMap);
				%s::StaticStruct()->SerializeBin(ValueDataReader, reinterpret_cast<void*>(&%s));)""", *Update, *PropertyType, *PropertyValue);
		}
	}
	else if (Property->IsA(UBoolProperty::StaticClass()))
	{
		Writer.Printf("%s = %s;", *PropertyValue, *Update);
	}
	else if (Property->IsA(UFloatProperty::StaticClass()))
	{
		Writer.Printf("%s = %s;", *PropertyValue, *Update);
	}
	else if (Property->IsA(UDoubleProperty::StaticClass()))
	{
		Writer.Printf("%s = %s;", *PropertyValue, *Update);
	}
	else if (Property->IsA(UInt8Property::StaticClass()))
	{
		Writer.Printf("%s = int8(%s);", *PropertyValue, *Update);
	}
	else if (Property->IsA(UInt16Property::StaticClass()))
	{
		Writer.Printf("%s = int16(%s);", *PropertyValue, *Update);
	}
	else if (Property->IsA(UIntProperty::StaticClass()))
	{
		Writer.Printf("%s = %s;", *PropertyValue, *Update);
	}
	else if (Property->IsA(UInt64Property::StaticClass()))
	{
		Writer.Printf("%s = %s;", *PropertyValue, *Update);
	}
	else if (Property->IsA(UByteProperty::StaticClass()))
	{
		// Byte properties are weird, because they can also be an enum in the form TEnumAsByte<...>. Therefore, the code generator needs to cast to either
		// TEnumAsByte<...> or uint8. However, as TEnumAsByte<...> only has a uint8 constructor, we need to cast the SpatialOS value into uint8 first
		// which causes "uint8(uint8(...))" to be generated for non enum bytes.
		Writer.Printf("%s = %s(uint8(%s));", *PropertyValue, *PropertyType, *Update);
	}
	else if (Property->IsA(UUInt16Property::StaticClass()))
	{
		Writer.Printf("%s = uint16(%s);", *PropertyValue, *Update);
	}
	else if (Property->IsA(UUInt32Property::StaticClass()))
	{
		Writer.Printf("%s = %s;", *PropertyValue, *Update);
	}
	else if (Property->IsA(UUInt64Property::StaticClass()))
	{
		Writer.Printf("%s = %s;", *PropertyValue, *Update);
	}
	else if (Property->IsA(UObjectPropertyBase::StaticClass()))
	{
		Writer.Printf(R"""(
			improbable::unreal::UnrealObjectRef ObjectRef = %s;
			check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
			if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
			{
				%s = nullptr;
			})""", *Update, *PropertyValue);
		Writer.Print("else");
		Writer.BeginScope();
		Writer.Printf(R"""(
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
			if (NetGUID.IsValid())
			{
				UObject* Object_Raw = PackageMap->GetObjectFromNetGUID(NetGUID, true);
				checkf(Object_Raw, TEXT("An object ref %%s should map to a valid object."), *ObjectRefToString(ObjectRef));
				checkf(Cast<%s>(Object_Raw), TEXT("Object ref %%s maps to object %%s with the wrong class."), *ObjectRefToString(ObjectRef), *Object_Raw->GetFullName());
				%s = Cast<%s>(Object_Raw);
			})""", *GetNativeClassName(Cast<UObjectPropertyBase>(Property)), *PropertyValue, *GetNativeClassName(Cast<UObjectPropertyBase>(Property)));
		Writer.Print("else");
		Writer.BeginScope();
		ObjectResolveFailureGenerator(*PropertyValue);
		Writer.End();
		Writer.End();
	}
	else if (Property->IsA(UNameProperty::StaticClass()))
	{
		Writer.Printf("%s = FName((%s).data());", *PropertyValue, *Update);
	}
	else if (Property->IsA(UStrProperty::StaticClass()))
	{
		Writer.Printf("%s = FString(UTF8_TO_TCHAR(%s.c_str()));", *PropertyValue, *Update);
	}
	else if (Property->IsA(UTextProperty::StaticClass()))
	{
		Writer.Printf("%s = FText::FromString((%s).data());", *PropertyValue, *Update);
	}
	else if (Property->IsA(UArrayProperty::StaticClass())) {
		const UArrayProperty* ArrayProperty = Cast<UArrayProperty>(Property);

		Writer.Printf("auto& List = %s;", *Update);
		Writer.Printf("%s.SetNum(List.size());", *PropertyValue);
		Writer.Print("for(int i = 0; i < List.size(); i++)");
		Writer.BeginScope();
		GeneratePropertyToUnrealConversion(Writer, "List[i]", ArrayProperty->Inner, FString::Printf(TEXT("%s[i]"), *PropertyValue), ObjectResolveFailureGenerator, bIsRPCProperty);
		Writer.End();
	}
	else if (Property->IsA(UEnumProperty::StaticClass()))
	{
		Writer.Printf("%s = %s(%s);", *PropertyValue, *Property->GetCPPType(), *Update);
	}
	else
	{
		Writer.Printf("// UNSUPPORTED U%s (unhandled) %s %s", *Property->GetClass()->GetName(), *PropertyValue, *Update);
	}
}

void GenerateRPCArgumentsStruct(FCodeWriter& Writer, const TSharedPtr<FUnrealRPC>& RPC, FString& StructName)
{
	UClass* RPCOwnerClass = RPC->Function->GetOwnerClass();
	// The name of the struct is consistent with the one in .generated.h
	StructName = FString::Printf(TEXT("%s_event%s_Parms"), *RPCOwnerClass->GetName(), *RPC->Function->GetName());
	if (RPCOwnerClass->ClassGeneratedBy)
	{
		// This RPC is generated from a blueprint class, so we need to generate the parameters struct based on the RPC arguments
		Writer.Printf("struct %s", *StructName);
		Writer.Print("{").Indent();

		for (UProperty* Prop : TFieldRange<UProperty>(RPC->Function))
		{
			if (!(Prop->PropertyFlags & CPF_Parm))
			{
				continue;
			}

			FStringOutputDevice PropertyText;

			// COPY-PASTE: Copied from UnrealHeaderTool/Private/CodeGenerator.cpp:3587
			bool bEmitConst = Prop->HasAnyPropertyFlags(CPF_ConstParm) && Prop->IsA<UObjectProperty>();

			//@TODO: UCREMOVAL: This is awful code duplication to avoid a double-const
			{
				// export 'const' for parameters
				const bool bIsConstParam = (Prop->IsA(UInterfaceProperty::StaticClass()) && !Prop->HasAllPropertyFlags(CPF_OutParm)); //@TODO: This should be const once that flag exists
				const bool bIsOnConstClass = (Prop->IsA(UObjectProperty::StaticClass()) && static_cast<UObjectProperty*>(Prop)->PropertyClass != NULL && static_cast<UObjectProperty*>(Prop)->PropertyClass->HasAnyClassFlags(CLASS_Const));

				if (bIsConstParam || bIsOnConstClass)
				{
					bEmitConst = false; // ExportCppDeclaration will do it for us
				}
			}

			if (bEmitConst)
			{
				PropertyText.Logf(TEXT("const "));
			}
			// COPY-PASTE END

			Prop->ExportCppDeclaration(PropertyText, EExportedDeclaration::Local, nullptr);

			Writer.Printf("%s;", *PropertyText);
		}

		Writer.Outdent().Print("};");
		Writer.PrintNewLine();
	}
	else
	{
		// This RPC is declared in C++, so we use the parameters struct generated by Unreal
		Writer.Printf("// This struct is declared in %s.generated.h (in a macro that is then put in %s.h UCLASS macro)",
			*RPCOwnerClass->GetName(),
			*RPCOwnerClass->GetName());
	}
}

void GenerateTypeBindingHeader(FCodeWriter& HeaderWriter, FString SchemaFilename, FString InteropFilename, UClass* Class, const TSharedPtr<FUnrealType> TypeInfo)
{
	HeaderWriter.Printf(R"""(
		// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
		// Note that this file has been generated automatically
		#pragma once

		#include "CoreMinimal.h"
		#include <improbable/worker.h>
		#include <improbable/view.h>
		#include <improbable/unreal/gdk/core_types.h>
		#include <improbable/unreal/gdk/unreal_metadata.h>
		#include <improbable/unreal/generated/%s.h>
		#include "ScopedViewCallbacks.h"
		#include "SpatialTypeBinding.h"
		#include "SpatialTypeBinding_%s.generated.h")""", *SchemaFilename, *Class->GetName());
	HeaderWriter.PrintNewLine();

	// Type binding class.
	HeaderWriter.Print("UCLASS()");
	HeaderWriter.Printf("class %s : public USpatialTypeBinding", *TypeBindingName(Class));
	HeaderWriter.Print("{").Indent();
	HeaderWriter.Print("GENERATED_BODY()");
	HeaderWriter.PrintNewLine();
	HeaderWriter.Outdent().Print("public:").Indent();
	HeaderWriter.Print(R"""(
		const FRepHandlePropertyMap& GetRepHandlePropertyMap() const override;
		const FMigratableHandlePropertyMap& GetMigratableHandlePropertyMap() const override;
		UClass* GetBoundClass() const override;

		void Init(USpatialInterop* InInterop, USpatialPackageMapClient* InPackageMap) override;
		void BindToView(bool bIsClient) override;
		void UnbindFromView() override;

		worker::Entity CreateActorEntity(const FString& ClientWorkerId, const FVector& Position, const FString& Metadata, const FPropertyChangeState& InitialChanges, USpatialActorChannel* Channel) const override;
		void SendComponentUpdates(const FPropertyChangeState& Changes, USpatialActorChannel* Channel, const FEntityId& EntityId) const override;
		void SendRPCCommand(UObject* TargetObject, const UFunction* const Function, void* Parameters) override;

		void ReceiveAddComponent(USpatialActorChannel* Channel, UAddComponentOpWrapperBase* AddComponentOp) const override;
		worker::Map<worker::ComponentId, worker::InterestOverride> GetInterestOverrideMap(bool bIsClient, bool bAutonomousProxy) const override;)""");
	HeaderWriter.PrintNewLine();
	HeaderWriter.Outdent().Print("private:").Indent();
	HeaderWriter.Print("improbable::unreal::callbacks::FScopedViewCallbacks ViewCallbacks;");
	HeaderWriter.PrintNewLine();
	HeaderWriter.Printf(R"""(
		// RPC to sender map.
		using FRPCSender = void (%s::*)(worker::Connection* const, void*, UObject*);
		TMap<FName, FRPCSender> RPCToSenderMap;)""", *TypeBindingName(Class));
	HeaderWriter.PrintNewLine();
	HeaderWriter.Print("FRepHandlePropertyMap RepHandleToPropertyMap;");
	HeaderWriter.Print("FMigratableHandlePropertyMap MigratableHandleToPropertyMap;");
	HeaderWriter.PrintNewLine();

	HeaderWriter.Print("// Component update helper functions.");
	FFunctionSignature BuildComponentUpdateSignature;
	BuildComponentUpdateSignature.Type = "void";
	BuildComponentUpdateSignature.NameAndParams = "BuildSpatialComponentUpdate(\n\tconst FPropertyChangeState& Changes,\n\tUSpatialActorChannel* Channel,";
	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		BuildComponentUpdateSignature.NameAndParams += FString::Printf(TEXT("\n\t%s::Update& %sUpdate,\n\tbool& b%sUpdateChanged,"),
			*SchemaReplicatedDataName(Group, Class, true),
			*GetReplicatedPropertyGroupName(Group),
			*GetReplicatedPropertyGroupName(Group),
			*GetReplicatedPropertyGroupName(Group));
	}
	BuildComponentUpdateSignature.NameAndParams += FString::Printf(TEXT("\n\t%s::Update& MigratableDataUpdate,\n\tbool& bMigratableDataUpdateChanged"),
		*SchemaMigratableDataName(Class, true));
	BuildComponentUpdateSignature.NameAndParams += ") const";
	HeaderWriter.Print(BuildComponentUpdateSignature.Declaration());
	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		HeaderWriter.Printf("void ServerSendUpdate_%s(const uint8* RESTRICT Data, int32 Handle, UProperty* Property, USpatialActorChannel* Channel, %s::Update& OutUpdate) const;",
			*GetReplicatedPropertyGroupName(Group),
			*SchemaReplicatedDataName(Group, Class, true));
	}
	HeaderWriter.Printf("void ServerSendUpdate_Migratable(const uint8* RESTRICT Data, int32 Handle, UProperty* Property, USpatialActorChannel* Channel, %s::Update& OutUpdate) const;",
		*SchemaMigratableDataName(Class, true));
	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		HeaderWriter.Printf("void ReceiveUpdate_%s(USpatialActorChannel* ActorChannel, const %s::Update& Update) const;",
			*GetReplicatedPropertyGroupName(Group),
			*SchemaReplicatedDataName(Group, Class, true));
	}
	HeaderWriter.Printf("void ReceiveUpdate_Migratable(USpatialActorChannel* ActorChannel, const %s::Update& Update) const;",
		*SchemaMigratableDataName(Class, true));

	// RPCs.
	FUnrealRPCsByType RPCsByType = GetAllRPCsByType(TypeInfo);

	HeaderWriter.Printf("void ReceiveUpdate_NetMulticastRPCs(worker::EntityId EntityId, const %s::Update& Update);",
		*SchemaRPCComponentName(RPC_NetMulticast, Class, true));

	HeaderWriter.PrintNewLine();
	HeaderWriter.Print("// RPC command sender functions.");
	for (auto Group : GetRPCTypes())
	{
		for (auto& RPC : RPCsByType[Group])
		{
			HeaderWriter.Printf("void %s_SendRPC(worker::Connection* const Connection, void* Parameters, UObject* TargetObject);",
				*RPC->Function->GetName());
		}
	}

	HeaderWriter.PrintNewLine();
	HeaderWriter.Print("// RPC command request handler functions.");
	for (auto Group : GetRPCTypes())
	{
		for (auto& RPC : RPCsByType[Group])
		{
			if (Group == RPC_NetMulticast)
			{
				HeaderWriter.Printf("void %s_OnRPCPayload(const worker::EntityId EntityId, const %s& EventData);",			
					*RPC->Function->GetName(),
					*SchemaRPCRequestType(RPC->Function, true));
			}
			else
			{
				HeaderWriter.Printf("void %s_OnRPCPayload(const worker::CommandRequestOp<%s::Commands::%s>& Op);",
					*RPC->Function->GetName(),
					*SchemaRPCComponentName(Group, Class, true),
					*CPPCommandClassName(Class, RPC->Function));
			}
		}
	}

	HeaderWriter.PrintNewLine();
	HeaderWriter.Print("// RPC command response handler functions.");
	for (auto Group : GetRPCTypes())
	{
		// Multicast RPCs are skipped since they use events rather than commands, and events
		// don't support responses
		if (Group == RPC_NetMulticast)
		{
			continue;
		}

		// Command response receiver function signatures
		for (auto& RPC : RPCsByType[Group])
		{
			HeaderWriter.Printf("void %s_OnCommandResponse(const worker::CommandResponseOp<%s::Commands::%s>& Op);",
				*RPC->Function->GetName(),
				*SchemaRPCComponentName(Group, Class, true),
				*CPPCommandClassName(Class, RPC->Function));
		}
	}
	HeaderWriter.Outdent();
	HeaderWriter.Print("};");
}

void GenerateTypeBindingSource(FCodeWriter& SourceWriter, FString SchemaFilename, FString InteropFilename, UClass* Class, const TSharedPtr<FUnrealType>& TypeInfo, const TArray<FString>& TypeBindingHeaders)
{
	SourceWriter.Printf(R"""(
		// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
		// Note that this file has been generated automatically

		#include "%s.h"

		#include "GameFramework/PlayerState.h"
		#include "NetworkGuid.h"

		#include "SpatialOS.h"
		#include "EntityBuilder.h"

		#include "SpatialConstants.h"
		#include "SpatialConditionMapFilter.h"
		#include "SpatialUnrealObjectRef.h"
		#include "SpatialActorChannel.h"
		#include "SpatialPackageMapClient.h"
		#include "SpatialMemoryReader.h"
		#include "SpatialMemoryWriter.h"
		#include "SpatialNetDriver.h"
		#include "SpatialInterop.h")""", *InteropFilename);

	// Add the header files specified in DefaultEditorSpatialGDK.ini.
	for (const FString& Header : TypeBindingHeaders)
	{
		if (!Header.IsEmpty())
		{
			SourceWriter.Printf("#include \"%s\"", *Header);
		}
	}

	SourceWriter.PrintNewLine();
	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		SourceWriter.Printf("#include \"%sAddComponentOp.h\"", *SchemaReplicatedDataName(Group, Class));
	}
	SourceWriter.Printf("#include \"%sAddComponentOp.h\"", *SchemaMigratableDataName(Class));

	// Get replicated data and RPCs.
	FUnrealFlatRepData RepData = GetFlatRepData(TypeInfo);
	FCmdHandlePropertyMap MigratableData = GetFlatMigratableData(TypeInfo);
	FUnrealRPCsByType RPCsByType = GetAllRPCsByType(TypeInfo);

	// Generate methods implementations

	SourceWriter.PrintNewLine();
	GenerateFunction_GetRepHandlePropertyMap(SourceWriter, Class);

	SourceWriter.PrintNewLine();
	GenerateFunction_GetMigratableHandlePropertyMap(SourceWriter, Class);

	SourceWriter.PrintNewLine();
	GenerateFunction_GetBoundClass(SourceWriter, Class);

	SourceWriter.PrintNewLine();
	GenerateFunction_Init(SourceWriter, Class, RPCsByType, RepData, MigratableData);

	SourceWriter.PrintNewLine();
	GenerateFunction_BindToView(SourceWriter, Class, RPCsByType);

	SourceWriter.PrintNewLine();
	GenerateFunction_UnbindFromView(SourceWriter, Class);

	SourceWriter.PrintNewLine();
	GenerateFunction_CreateActorEntity(SourceWriter, Class);

	SourceWriter.PrintNewLine();
	GenerateFunction_SendComponentUpdates(SourceWriter, Class);

	SourceWriter.PrintNewLine();
	GenerateFunction_SendRPCCommand(SourceWriter, Class);

	SourceWriter.PrintNewLine();
	GenerateFunction_ReceiveAddComponent(SourceWriter, Class);

	SourceWriter.PrintNewLine();
	GenerateFunction_GetInterestOverrideMap(SourceWriter, Class);

	SourceWriter.PrintNewLine();
	GenerateFunction_BuildSpatialComponentUpdate(SourceWriter, Class);

	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		SourceWriter.PrintNewLine();
		GenerateFunction_ServerSendUpdate_RepData(SourceWriter, Class, RepData, Group);
	}

	SourceWriter.PrintNewLine();
	GenerateFunction_ServerSendUpdate_MigratableData(SourceWriter, Class, MigratableData);

	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		SourceWriter.PrintNewLine();
		GenerateFunction_ReceiveUpdate_RepData(SourceWriter, Class, RepData, Group);
	}

	SourceWriter.PrintNewLine();
	GenerateFunction_ReceiveUpdate_MigratableData(SourceWriter, Class, MigratableData);

	SourceWriter.PrintNewLine();
	GenerateFunction_ReceiveUpdate_MulticastRPCs(SourceWriter, Class, RPCsByType[RPC_NetMulticast]);

	for (auto Group : GetRPCTypes())
	{
		for (auto& RPC : RPCsByType[Group])
		{
			GenerateFunction_SendRPC(SourceWriter, Class, RPC);
		}
	}

	for (auto Group : GetRPCTypes())
	{
		for (auto& RPC : RPCsByType[Group])
		{
			GenerateFunction_OnRPCPayload(SourceWriter, Class, RPC);
		}
	}

	for (auto Group : GetRPCTypes())
	{
		// Multicast RPCs are skipped since they use events rather than commands, and events
		// don't support responses
		if (Group == ERPCType::RPC_NetMulticast)
		{
			continue;
		}

		for (auto& RPC : RPCsByType[Group])
		{
			SourceWriter.PrintNewLine();
			GenerateFunction_RPCOnCommandResponse(SourceWriter, Class, RPC);
		}
	}
}

void GenerateFunction_GetRepHandlePropertyMap(FCodeWriter& SourceWriter, UClass* Class)
{
	SourceWriter.BeginFunction({"const FRepHandlePropertyMap&", "GetRepHandlePropertyMap() const"}, TypeBindingName(Class));
	SourceWriter.Print("return RepHandleToPropertyMap;");
	SourceWriter.End();
}

void GenerateFunction_GetMigratableHandlePropertyMap(FCodeWriter& SourceWriter, UClass* Class)
{
	SourceWriter.BeginFunction({"const FMigratableHandlePropertyMap&", "GetMigratableHandlePropertyMap() const"}, TypeBindingName(Class));
	SourceWriter.Print("return MigratableHandleToPropertyMap;");
	SourceWriter.End();
}

void GenerateFunction_GetBoundClass(FCodeWriter& SourceWriter, UClass* Class)
{
	SourceWriter.BeginFunction({"UClass*", "GetBoundClass() const"}, TypeBindingName(Class));
	if (Class->ClassGeneratedBy)
	{
		// This is a blueprint class, so use Unreal's reflection to find UClass pointer at runtime.
		SourceWriter.Printf("return FindObject<UClass>(ANY_PACKAGE, TEXT(\"%s\"));", *Class->GetName());
	}
	else
	{
		SourceWriter.Printf("return %s::StaticClass();", *GetFullCPPName(Class));
	}
	SourceWriter.End();
}

void GenerateFunction_Init(FCodeWriter& SourceWriter, UClass* Class, const FUnrealRPCsByType& RPCsByType, const FUnrealFlatRepData& RepData, const FCmdHandlePropertyMap& MigratableData)
{
	SourceWriter.BeginFunction({"void", "Init(USpatialInterop* InInterop, USpatialPackageMapClient* InPackageMap)"}, TypeBindingName(Class));

	SourceWriter.Print("Super::Init(InInterop, InPackageMap);");
	SourceWriter.PrintNewLine();
	for (auto Group : GetRPCTypes())
	{
		for (auto& RPC : RPCsByType[Group])
		{
			SourceWriter.Printf("RPCToSenderMap.Emplace(\"%s\", &%s::%s_SendRPC);", *RPC->Function->GetName(), *TypeBindingName(Class), *RPC->Function->GetName());
		}
	}

	if (RepData.Num() > 0 || MigratableData.Num() > 0)
	{
		// Get class.
		SourceWriter.PrintNewLine();
		SourceWriter.Printf("UClass* Class = FindObject<UClass>(ANY_PACKAGE, TEXT(\"%s\"));", *Class->GetName());
	}

	// Populate RepHandleToPropertyMap.
	if (RepData.Num() > 0)
	{
		// Reduce into single list of properties.
		TMap<uint16, TSharedPtr<FUnrealProperty>> ReplicatedProperties;
		for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
		{
			ReplicatedProperties.Append(RepData[Group]);
		}
		ReplicatedProperties.KeySort(TLess<uint16>());

		SourceWriter.PrintNewLine();
		SourceWriter.Print("// Populate RepHandleToPropertyMap.");

		for (auto& RepProp : ReplicatedProperties)
		{
			// Create property chain initialiser list.
			FString PropertyChainInitList;
			FString PropertyChainIndicesInitList;
			TArray<FString> PropertyChainNames;
			TArray<FString> PropertyChainIndices;
			Algo::Transform(GetPropertyChain(RepProp.Value), PropertyChainNames, [](const TSharedPtr<FUnrealProperty>& PropertyInfo) -> FString
			{
				return TEXT("\"") + PropertyInfo->Property->GetFName().ToString() + TEXT("\"");
			});
			PropertyChainInitList = FString::Join(PropertyChainNames, TEXT(", "));

			Algo::Transform(GetPropertyChain(RepProp.Value), PropertyChainIndices, [](const TSharedPtr<FUnrealProperty>& PropertyInfo) -> FString
			{
				return FString::FromInt(PropertyInfo->StaticArrayIndex);
			});
			PropertyChainIndicesInitList = FString::Join(PropertyChainIndices, TEXT(", "));

			SourceWriter.Printf("RepHandleToPropertyMap.Add(%d, FRepHandleData(Class, {%s}, {%s}, %s, %s));",
				RepProp.Value->ReplicationData->Handle,
				*PropertyChainInitList,
				*PropertyChainIndicesInitList,
				*GetLifetimeConditionAsString(RepProp.Value->ReplicationData->Condition),
				*GetRepNotifyLifetimeConditionAsString(RepProp.Value->ReplicationData->RepNotifyCondition));
		}
	}

	// Populate MigratableHandleToPropertyMap.
	if (MigratableData.Num() > 0)
	{
		SourceWriter.PrintNewLine();
		SourceWriter.Print("// Populate MigratableHandleToPropertyMap.");

		for (auto& MigratableProp : MigratableData)
		{
			auto Handle = MigratableProp.Key;

			// Create property chain initialiser list.
			FString PropertyChainInitList;
			TArray<FString> PropertyChainNames;
			Algo::Transform(GetPropertyChain(MigratableProp.Value), PropertyChainNames, [](const TSharedPtr<FUnrealProperty>& Property) -> FString
			{
				return TEXT("\"") + Property->Property->GetFName().ToString() + TEXT("\"");
			});
			PropertyChainInitList = FString::Join(PropertyChainNames, TEXT(", "));

			// Add the handle data to the map.
			SourceWriter.Printf("MigratableHandleToPropertyMap.Add(%d, FMigratableHandleData(Class, {%s}));",
				Handle,
				*PropertyChainInitList);
		}
	}

	SourceWriter.End();
}

void GenerateFunction_BindToView(FCodeWriter& SourceWriter, UClass* Class, const FUnrealRPCsByType& RPCsByType)
{
	SourceWriter.BeginFunction({"void", "BindToView(bool bIsClient)"}, TypeBindingName(Class));

	SourceWriter.Print("TSharedPtr<worker::View> View = Interop->GetSpatialOS()->GetView().Pin();");
	SourceWriter.Print("ViewCallbacks.Init(View);");
	SourceWriter.PrintNewLine();
	SourceWriter.Print("if (Interop->GetNetDriver()->GetNetMode() == NM_Client)");
	SourceWriter.BeginScope();
	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		SourceWriter.Printf("ViewCallbacks.Add(View->OnComponentUpdate<%s>([this](",
			*SchemaReplicatedDataName(Group, Class, true));
		SourceWriter.Indent();
		SourceWriter.Printf("const worker::ComponentUpdateOp<%s>& Op)", *SchemaReplicatedDataName(Group, Class, true));
		SourceWriter.Outdent();
		SourceWriter.Print("{");
		SourceWriter.Indent();
		SourceWriter.Printf(R"""(
			// TODO: Remove this check once we can disable component update short circuiting. This will be exposed in 14.0. See TIG-137.
			if (HasComponentAuthority(Interop->GetSpatialOS()->GetView(), Op.EntityId, %s::ComponentId))
			{
				return;
			})""", *SchemaReplicatedDataName(Group, Class, true));
		SourceWriter.Printf(R"""(
			USpatialActorChannel* ActorChannel = Interop->GetActorChannelByEntityId(Op.EntityId);
			check(ActorChannel);
			ReceiveUpdate_%s(ActorChannel, Op.Update);)""",
			*GetReplicatedPropertyGroupName(Group));
		SourceWriter.Outdent();
		SourceWriter.Print("}));");
	}
	SourceWriter.Printf("if (!bIsClient)");
	SourceWriter.BeginScope();
	SourceWriter.Printf("ViewCallbacks.Add(View->OnComponentUpdate<%s>([this](", *SchemaMigratableDataName(Class, true));
	SourceWriter.Indent();
	SourceWriter.Printf("const worker::ComponentUpdateOp<%s>& Op)", *SchemaMigratableDataName(Class, true));
	SourceWriter.Outdent();
	SourceWriter.BeginScope();
	SourceWriter.Printf(R"""(
		// TODO: Remove this check once we can disable component update short circuiting. This will be exposed in 14.0. See TIG-137.
		if (HasComponentAuthority(Interop->GetSpatialOS()->GetView(), Op.EntityId, %s::ComponentId))
		{
			return;
		})""", *SchemaMigratableDataName(Class, true));
	SourceWriter.Print(R"""(
		USpatialActorChannel* ActorChannel = Interop->GetActorChannelByEntityId(Op.EntityId);
		check(ActorChannel);
		ReceiveUpdate_Migratable(ActorChannel, Op.Update);)""");
	SourceWriter.Outdent();
	SourceWriter.Print("}));");
	SourceWriter.End();
	SourceWriter.End();

	// Multicast RPCs
	SourceWriter.Printf("ViewCallbacks.Add(View->OnComponentUpdate<%s>([this](",
		*SchemaRPCComponentName(RPC_NetMulticast, Class, true));
	SourceWriter.Indent();
	SourceWriter.Printf("const worker::ComponentUpdateOp<%s>& Op)",
		*SchemaRPCComponentName(RPC_NetMulticast, Class, true));
	SourceWriter.Outdent();
	SourceWriter.Print("{");
	SourceWriter.Indent();
	SourceWriter.Printf(R"""(
		// TODO: Remove this check once we can disable component update short circuiting. This will be exposed in 14.0. See TIG-137.
		if (HasComponentAuthority(Interop->GetSpatialOS()->GetView(), Op.EntityId, %s::ComponentId))
		{
			return;
		})""",
		*SchemaRPCComponentName(RPC_NetMulticast, Class, true));
	SourceWriter.Printf(R"""(
		ReceiveUpdate_NetMulticastRPCs(Op.EntityId, Op.Update);)""",
		*GetRPCTypeName(RPC_NetMulticast));
	SourceWriter.Outdent();
	SourceWriter.Print("}));");

	for (auto Group : GetRPCTypes())
	{
		if (Group == RPC_NetMulticast)
		{
			continue;
		}

		// Ensure that this class contains RPCs of the type specified by group (eg, Server or
		// Client) so that we don't generate code for missing components
		if (RPCsByType.Contains(Group) && RPCsByType[Group].Num() > 0)
		{
			SourceWriter.PrintNewLine();
			SourceWriter.Printf("using %sRPCCommandTypes = %s::Commands;",
				*GetRPCTypeName(Group),
				*SchemaRPCComponentName(Group, Class, true));
			for (auto& RPC : RPCsByType[Group])
			{
				SourceWriter.Printf("ViewCallbacks.Add(View->OnCommandRequest<%sRPCCommandTypes::%s>(std::bind(&%s::%s_OnRPCPayload, this, std::placeholders::_1)));",
					*GetRPCTypeName(Group),
					*CPPCommandClassName(Class, RPC->Function),
					*TypeBindingName(Class),
					*RPC->Function->GetName());
			}
			for (auto& RPC : RPCsByType[Group])
			{
				SourceWriter.Printf("ViewCallbacks.Add(View->OnCommandResponse<%sRPCCommandTypes::%s>(std::bind(&%s::%s_OnCommandResponse, this, std::placeholders::_1)));",
					*GetRPCTypeName(Group),
					*CPPCommandClassName(Class, RPC->Function),
					*TypeBindingName(Class),
					*RPC->Function->GetName());
			}
		}
	}

	SourceWriter.End();
}

void GenerateFunction_UnbindFromView(FCodeWriter& SourceWriter, UClass* Class)
{
	SourceWriter.BeginFunction({"void", "UnbindFromView()"}, TypeBindingName(Class));
	SourceWriter.Print("ViewCallbacks.Reset();");
	SourceWriter.End();
}

void GenerateFunction_CreateActorEntity(FCodeWriter& SourceWriter, UClass* Class)
{
	SourceWriter.BeginFunction(
		{"worker::Entity", "CreateActorEntity(const FString& ClientWorkerId, const FVector& Position, const FString& Metadata, const FPropertyChangeState& InitialChanges, USpatialActorChannel* Channel) const"},
		TypeBindingName(Class));

	// Set up initial data.
	SourceWriter.Print(R"""(
		// Validate replication list.
		const uint16 RepHandlePropertyMapCount = GetRepHandlePropertyMap().Num();
		for (auto& Rep : InitialChanges.RepChanged)
		{
			checkf(Rep <= RepHandlePropertyMapCount, TEXT("Attempting to replicate a property with a handle that the type binding is not aware of. Have additional replicated properties been added in a non generated child object?"))
		}
		)""");

	SourceWriter.Print(TEXT("// Setup initial data."));
	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		SourceWriter.Printf("%s::Data %sData;",
			*SchemaReplicatedDataName(Group, Class, true),
			*GetReplicatedPropertyGroupName(Group));
		SourceWriter.Printf("%s::Update %sUpdate;",
			*SchemaReplicatedDataName(Group, Class, true),
			*GetReplicatedPropertyGroupName(Group));
		SourceWriter.Printf("bool b%sUpdateChanged = false;", *GetReplicatedPropertyGroupName(Group));
	}
	SourceWriter.Printf("%s::Data MigratableData;", *SchemaMigratableDataName(Class, true));
	SourceWriter.Printf("%s::Update MigratableDataUpdate;", *SchemaMigratableDataName(Class, true));
	SourceWriter.Print("bool bMigratableDataUpdateChanged = false;");
	TArray<FString> BuildUpdateArgs;
	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		BuildUpdateArgs.Add(FString::Printf(TEXT("%sUpdate"), *GetReplicatedPropertyGroupName(Group)));
		BuildUpdateArgs.Add(FString::Printf(TEXT("b%sUpdateChanged"), *GetReplicatedPropertyGroupName(Group)));
	}
	BuildUpdateArgs.Add("MigratableDataUpdate");
	BuildUpdateArgs.Add("bMigratableDataUpdateChanged");
	SourceWriter.Printf("BuildSpatialComponentUpdate(InitialChanges, Channel, %s);", *FString::Join(BuildUpdateArgs, TEXT(", ")));
	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		SourceWriter.Printf("%sUpdate.ApplyTo(%sData);",
			*GetReplicatedPropertyGroupName(Group),
			*GetReplicatedPropertyGroupName(Group));
	}
	SourceWriter.Print("MigratableDataUpdate.ApplyTo(MigratableData);");

	// Create Entity.
	SourceWriter.PrintNewLine();
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

		// Set up unreal metadata.
		improbable::unreal::UnrealMetadata::Data UnrealMetadata;
		if (Channel->Actor->IsFullNameStableForNetworking())
		{
			UnrealMetadata.set_static_path({std::string{TCHAR_TO_UTF8(*Channel->Actor->GetPathName(Channel->Actor->GetWorld()))}});
		}
		if (!ClientWorkerIdString.empty())
		{
			UnrealMetadata.set_owner_worker_id({ClientWorkerIdString});
		}

		uint32 CurrentOffset = 1;
		worker::Map<std::string, std::uint32_t> SubobjectNameToOffset;
		ForEachObjectWithOuter(Channel->Actor, [&UnrealMetadata, &CurrentOffset, &SubobjectNameToOffset](UObject* Object)
		{
			// Objects can only be allocated NetGUIDs if this is true.
			if (Object->IsSupportedForNetworking() && !Object->IsPendingKill() && !Object->IsEditorOnly())
			{
				SubobjectNameToOffset.emplace(TCHAR_TO_UTF8(*(Object->GetName())), CurrentOffset);
				CurrentOffset++;
			}
		});
		UnrealMetadata.set_subobject_name_to_offset(SubobjectNameToOffset);

		// Build entity.
		const improbable::Coordinates SpatialPosition = SpatialConstants::LocationToSpatialOSCoordinates(Position);)""");
	SourceWriter.Print("return improbable::unreal::FEntityBuilder::Begin()");
	SourceWriter.Indent();
	// If this is a APlayerController entity, ensure that only the owning client and workers have read ACL permissions. 
	// This ensures that only one APlayerController object is created per client.
	SourceWriter.Printf(R"""(
		.AddPositionComponent(improbable::Position::Data{SpatialPosition}, WorkersOnly)
		.AddMetadataComponent(improbable::Metadata::Data{TCHAR_TO_UTF8(*Metadata)})
		.SetPersistence(true)
		.SetReadAcl(%s)
		.AddComponent<improbable::unreal::UnrealMetadata>(UnrealMetadata, WorkersOnly))""",
		Class->IsChildOf(APlayerController::StaticClass()) ? TEXT("AnyUnrealWorkerOrOwningClient") : TEXT("AnyUnrealWorkerOrClient"));
	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		SourceWriter.Printf(".AddComponent<%s>(%sData, WorkersOnly)",
			*SchemaReplicatedDataName(Group, Class, true), *GetReplicatedPropertyGroupName(Group));
	}
	SourceWriter.Printf(".AddComponent<%s>(MigratableData, WorkersOnly)", *SchemaMigratableDataName(Class, true));
	SourceWriter.Printf(".AddComponent<%s>(%s::Data{}, OwningClientOnly)",
		*SchemaRPCComponentName(ERPCType::RPC_Client, Class, true), *SchemaRPCComponentName(ERPCType::RPC_Client, Class, true));
	SourceWriter.Printf(".AddComponent<%s>(%s::Data{}, WorkersOnly)",
		*SchemaRPCComponentName(ERPCType::RPC_Server, Class, true), *SchemaRPCComponentName(ERPCType::RPC_Server, Class, true));
	SourceWriter.Printf(".AddComponent<%s>(%s::Data{}, WorkersOnly)",
		*SchemaRPCComponentName(ERPCType::RPC_NetMulticast, Class, true), *SchemaRPCComponentName(ERPCType::RPC_NetMulticast, Class, true));

	SourceWriter.Print(".Build();");
	SourceWriter.Outdent();

	SourceWriter.End();
}

void GenerateFunction_SendComponentUpdates(FCodeWriter& SourceWriter, UClass* Class)
{
	SourceWriter.BeginFunction(
		{"void", "SendComponentUpdates(const FPropertyChangeState& Changes, USpatialActorChannel* Channel, const FEntityId& EntityId) const"},
		TypeBindingName(Class));

	SourceWriter.Print("// Build SpatialOS updates.");
	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		SourceWriter.Printf("%s::Update %sUpdate;",
			*SchemaReplicatedDataName(Group, Class, true),
			*GetReplicatedPropertyGroupName(Group));
		SourceWriter.Printf("bool b%sUpdateChanged = false;", *GetReplicatedPropertyGroupName(Group));
	}
	SourceWriter.Printf("%s::Update MigratableDataUpdate;", *SchemaMigratableDataName(Class, true));
	SourceWriter.Print("bool bMigratableDataUpdateChanged = false;");

	TArray<FString> BuildUpdateArgs;
	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		BuildUpdateArgs.Add(FString::Printf(TEXT("%sUpdate"), *GetReplicatedPropertyGroupName(Group)));
		BuildUpdateArgs.Add(FString::Printf(TEXT("b%sUpdateChanged"), *GetReplicatedPropertyGroupName(Group)));
	}
	BuildUpdateArgs.Add("MigratableDataUpdate");
	BuildUpdateArgs.Add("bMigratableDataUpdateChanged");
	SourceWriter.Printf("BuildSpatialComponentUpdate(Changes, Channel, %s);", *FString::Join(BuildUpdateArgs, TEXT(", ")));

	SourceWriter.PrintNewLine();
	SourceWriter.Print("// Send SpatialOS updates if anything changed.");
	SourceWriter.Print("TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();");
	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		SourceWriter.Printf("if (b%sUpdateChanged)", *GetReplicatedPropertyGroupName(Group));
		SourceWriter.BeginScope();
		SourceWriter.Printf("Connection->SendComponentUpdate<%s>(EntityId.ToSpatialEntityId(), %sUpdate);",
			*SchemaReplicatedDataName(Group, Class, true),
			*GetReplicatedPropertyGroupName(Group));
		SourceWriter.End();
	}
	SourceWriter.Printf("if (bMigratableDataUpdateChanged)");
	SourceWriter.BeginScope();
	SourceWriter.Printf("Connection->SendComponentUpdate<%s>(EntityId.ToSpatialEntityId(), MigratableDataUpdate);",
		*SchemaMigratableDataName(Class, true));
	SourceWriter.End();

	SourceWriter.End();
}

void GenerateFunction_SendRPCCommand(FCodeWriter& SourceWriter, UClass* Class)
{
	SourceWriter.BeginFunction(
		{"void", "SendRPCCommand(UObject* TargetObject, const UFunction* const Function, void* Parameters)"},
		TypeBindingName(Class));
	SourceWriter.Print(R"""(
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		auto SenderFuncIterator = RPCToSenderMap.Find(Function->GetFName());
		if (SenderFuncIterator == nullptr)
		{
			UE_LOG(LogSpatialGDKInterop, Error, TEXT("Sender for %s has not been registered with RPCToSenderMap."), *Function->GetFName().ToString());
			return;
		}
		checkf(*SenderFuncIterator, TEXT("Sender for %s has been registered as null."), *Function->GetFName().ToString());
		(this->*(*SenderFuncIterator))(Connection.Get(), Parameters, TargetObject);)""");
	SourceWriter.End();
}

void GenerateFunction_ReceiveAddComponent(FCodeWriter& SourceWriter, UClass* Class)
{
	SourceWriter.BeginFunction(
		{"void", "ReceiveAddComponent(USpatialActorChannel* Channel, UAddComponentOpWrapperBase* AddComponentOp) const"},
		TypeBindingName(Class));
	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		SourceWriter.Printf(R"""(
			auto* %sAddOp = Cast<U%sAddComponentOp>(AddComponentOp);
			if (%sAddOp)
			{
				auto Update = %s::Update::FromInitialData(*%sAddOp->Data.data());
				ReceiveUpdate_%s(Channel, Update);
			})""",
			*GetReplicatedPropertyGroupName(Group),
			*SchemaReplicatedDataName(Group, Class),
			*GetReplicatedPropertyGroupName(Group),
			*SchemaReplicatedDataName(Group, Class, true),
			*GetReplicatedPropertyGroupName(Group),
			*GetReplicatedPropertyGroupName(Group));
	}
	SourceWriter.Printf(R"""(
		auto* MigratableDataAddOp = Cast<U%sAddComponentOp>(AddComponentOp);
		if (MigratableDataAddOp)
		{
			auto Update = %s::Update::FromInitialData(*MigratableDataAddOp->Data.data());
			ReceiveUpdate_Migratable(Channel, Update);
		})""",
		*SchemaMigratableDataName(Class),
		*SchemaMigratableDataName(Class, true));
	SourceWriter.End();
}

void GenerateFunction_GetInterestOverrideMap(FCodeWriter& SourceWriter, UClass* Class)
{
	SourceWriter.BeginFunction(
		{"worker::Map<worker::ComponentId, worker::InterestOverride>", "GetInterestOverrideMap(bool bIsClient, bool bAutonomousProxy) const"},
		TypeBindingName(Class));
	SourceWriter.Printf(R"""(
		worker::Map<worker::ComponentId, worker::InterestOverride> Interest;
		if (bIsClient)
		{
			if (!bAutonomousProxy)
			{
				Interest.emplace(%s::ComponentId, worker::InterestOverride{false});
			}
			Interest.emplace(%s::ComponentId, worker::InterestOverride{false});
		}
		return Interest;)""",
		*SchemaReplicatedDataName(REP_SingleClient, Class, true),
		*SchemaMigratableDataName(Class, true));
	SourceWriter.End();
}

void GenerateFunction_BuildSpatialComponentUpdate(FCodeWriter& SourceWriter, UClass* Class)
{
	FFunctionSignature BuildComponentUpdateSignature;
	BuildComponentUpdateSignature.Type = "void";
	BuildComponentUpdateSignature.NameAndParams = "BuildSpatialComponentUpdate(\n\tconst FPropertyChangeState& Changes,\n\tUSpatialActorChannel* Channel,";
	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		BuildComponentUpdateSignature.NameAndParams += FString::Printf(TEXT("\n\t%s::Update& %sUpdate,\n\tbool& b%sUpdateChanged,"),
			*SchemaReplicatedDataName(Group, Class, true),
			*GetReplicatedPropertyGroupName(Group),
			*GetReplicatedPropertyGroupName(Group),
			*GetReplicatedPropertyGroupName(Group));
	}
	BuildComponentUpdateSignature.NameAndParams += FString::Printf(TEXT("\n\t%s::Update& MigratableDataUpdate,\n\tbool& bMigratableDataUpdateChanged"),
		*SchemaMigratableDataName(Class, true));
	BuildComponentUpdateSignature.NameAndParams += ") const";

	SourceWriter.BeginFunction(BuildComponentUpdateSignature, TypeBindingName(Class));
	SourceWriter.Print("const FRepHandlePropertyMap& RepPropertyMap = GetRepHandlePropertyMap();");
	SourceWriter.Print("const FMigratableHandlePropertyMap& MigPropertyMap = GetMigratableHandlePropertyMap();");
	SourceWriter.Print("if (Changes.RepChanged.Num() > 0)");
	SourceWriter.BeginScope();

	SourceWriter.Print(R"""(
		// Populate the replicated data component updates from the replicated property changelist.
		FChangelistIterator ChangelistIterator(Changes.RepChanged, 0);
		FRepHandleIterator HandleIterator(ChangelistIterator, Changes.RepCmds, Changes.RepBaseHandleToCmdIndex, 0, 1, 0, Changes.RepCmds.Num() - 1);
		while (HandleIterator.NextHandle()))""");
	SourceWriter.BeginScope();
	SourceWriter.Print(R"""(
		const FRepLayoutCmd& Cmd = Changes.RepCmds[HandleIterator.CmdIndex];
		const FRepHandleData& PropertyMapData = RepPropertyMap[HandleIterator.Handle];
		const uint8* Data = PropertyMapData.GetPropertyData(Changes.SourceData) + HandleIterator.ArrayOffset;
		UE_LOG(LogSpatialGDKInterop, Verbose, TEXT("%s: Sending property update. actor %s (%lld), property %s (handle %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*Channel->Actor->GetName(),
			Channel->GetEntityId().ToSpatialEntityId(),
			*Cmd.Property->GetName(),
			HandleIterator.Handle);)""");

	SourceWriter.Print("switch (GetGroupFromCondition(PropertyMapData.Condition))");
	SourceWriter.BeginScope();
	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
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
	SourceWriter.End();
	SourceWriter.Print(R"""(
		if (Cmd.Type == REPCMD_DynamicArray)
		{
			if (!HandleIterator.JumpOverArray())
			{
				break;
			}
		})""");
	SourceWriter.End();
	SourceWriter.End();

	SourceWriter.PrintNewLine();
	SourceWriter.Print(R"""(
		// Populate the migrated data component update from the migrated property changelist.
		for (uint16 ChangedHandle : Changes.MigChanged)
		{
			const FMigratableHandleData& PropertyMapData = MigPropertyMap[ChangedHandle];
			const uint8* Data = PropertyMapData.GetPropertyData(Changes.SourceData);
			UE_LOG(LogSpatialGDKInterop, Verbose, TEXT("%s: Sending migratable property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*Channel->Actor->GetName(),
				Channel->GetEntityId().ToSpatialEntityId(),
				*PropertyMapData.Property->GetName(),
				ChangedHandle);
			ServerSendUpdate_Migratable(Data, ChangedHandle, PropertyMapData.Property, Channel, MigratableDataUpdate);
			bMigratableDataUpdateChanged = true;
		})""");

	SourceWriter.End();
}

void GenerateFunction_ServerSendUpdate_RepData(FCodeWriter& SourceWriter, UClass* Class, const FUnrealFlatRepData& RepData, EReplicatedPropertyGroup Group)
{
	FFunctionSignature ServerSendUpdateSignature{"void",
		FString::Printf(TEXT("ServerSendUpdate_%s(const uint8* RESTRICT Data, int32 Handle, UProperty* Property, USpatialActorChannel* Channel, %s::Update& OutUpdate) const"),
			*GetReplicatedPropertyGroupName(Group),
			*SchemaReplicatedDataName(Group, Class, true))
	};
	SourceWriter.BeginFunction(ServerSendUpdateSignature, TypeBindingName(Class));

	if (RepData[Group].Num() > 0)
	{
		SourceWriter.Print("switch (Handle)");
		SourceWriter.BeginScope();

		for (auto& RepProp : RepData[Group])
		{
			check(RepProp.Value->ReplicationData->Handle > 0);

			GenerateBody_SendUpdate_RepDataProperty(SourceWriter,
				RepProp.Value->ReplicationData->Handle,
				RepProp.Value);
		}
		SourceWriter.Outdent().Print("default:");
		SourceWriter.Indent();
		SourceWriter.Print("checkf(false, TEXT(\"Unknown replication handle %d encountered when creating a SpatialOS update.\"));");
		SourceWriter.Print("break;");

		SourceWriter.End();
	}

	SourceWriter.End();
}

void GenerateBody_SendUpdate_RepDataProperty(FCodeWriter& SourceWriter, uint16 Handle, TSharedPtr<FUnrealProperty> PropertyInfo)
{
	UProperty* Property = PropertyInfo->Property;
	SourceWriter.Printf("case %d: // %s", Handle, *SchemaFieldName(PropertyInfo));
	SourceWriter.BeginScope();

	// Get unreal data by deserialising from the reader, convert and set the corresponding field in the update object.
	FString PropertyValueName = TEXT("Value");
	FString PropertyCppType = Property->GetClass()->GetFName().ToString();
	FString PropertyValueCppType = Property->GetCPPType();

	bool bHandleUnresolvedObjects = false;

	FString PropertyName = TEXT("Property");
	//todo-giray: The reinterpret_cast below is ugly and we believe we can do this more gracefully using Property helper functions.
	if (Property->IsA<UBoolProperty>())
	{
		SourceWriter.Printf("bool %s = static_cast<UBoolProperty*>(Property)->GetPropertyValue(Data);", *PropertyValueName);
	}
	else if (Property->IsA<UArrayProperty>())
	{
		UArrayProperty* ArrayProperty = Cast<UArrayProperty>(Property);
		UProperty* InnerProperty = ArrayProperty->Inner;
		SourceWriter.Printf("const TArray<%s>& %s = *(reinterpret_cast<TArray<%s> const*>(Data));", *InnerProperty->GetCPPType(), *PropertyValueName, *InnerProperty->GetCPPType());

		if (InnerProperty->IsA<UObjectPropertyBase>() || InnerProperty->IsA<UStructProperty>())
		{
			// Special case because we need to handle unresolved objects
			bHandleUnresolvedObjects = true;
		}
	}
	else if (Property->IsA<UStructProperty>())
	{
		SourceWriter.Printf("const %s& %s = *(reinterpret_cast<%s const*>(Data));", *PropertyValueCppType, *PropertyValueName, *PropertyValueCppType);
		// Structs may have UObject* inside which could be unresolved
		bHandleUnresolvedObjects = true;
	}
	else if (Property->IsA<UWeakObjectProperty>())
	{
		FString ClassName = GetNativeClassName(Cast<UObjectPropertyBase>(Property));
		SourceWriter.Printf("%s* %s = (reinterpret_cast<%s const*>(Data))->Get();", *ClassName, *PropertyValueName, *Property->GetCPPType());
	}
	else if (Property->IsA<UObjectPropertyBase>())
	{
		FString ClassName = GetNativeClassName(Cast<UObjectPropertyBase>(Property));
		SourceWriter.Printf("%s* %s = *(reinterpret_cast<%s* const*>(Data));", *ClassName, *PropertyValueName, *ClassName);
	}
	else
	{
		SourceWriter.Printf("%s %s = *(reinterpret_cast<%s const*>(Data));", *PropertyValueCppType, *PropertyValueName, *PropertyValueCppType);
	}

	FString SpatialValueSetter = TEXT("OutUpdate.set_") + SchemaFieldName(PropertyInfo);

	SourceWriter.PrintNewLine();

	if (bHandleUnresolvedObjects)
	{
		SourceWriter.Printf(R"""(
			Interop->ResetOutgoingArrayRepUpdate_Internal(Channel, %d);
			TSet<const UObject*> UnresolvedObjects;)""", Handle);

		FString ResultSetter = FString::Printf(TEXT("const %s& Result = "), *PropertyToWorkerSDKType(Property, false));
		GenerateUnrealToSchemaConversion(SourceWriter, ResultSetter, Property, PropertyValueName, [&SourceWriter](const FString& PropertyValue)
		{
			SourceWriter.Printf("UnresolvedObjects.Add(%s);", *PropertyValue);
		}, false, true);

		SourceWriter.Printf(R"""(
			if (UnresolvedObjects.Num() == 0)
			{
				%s(Result);
			}
			else
			{
				Interop->QueueOutgoingArrayRepUpdate_Internal(UnresolvedObjects, Channel, %d);
			})""", *SpatialValueSetter, Handle);
	}
	else
	{
		GenerateUnrealToSchemaConversion(SourceWriter, SpatialValueSetter, PropertyInfo->Property, PropertyValueName, [&SourceWriter, Handle](const FString& PropertyValue)
		{
			SourceWriter.Printf("// A legal static object reference should never be unresolved.");
			SourceWriter.Printf("check(!%s->IsFullNameStableForNetworking())", *PropertyValue);
			SourceWriter.Printf("Interop->QueueOutgoingObjectRepUpdate_Internal(%s, Channel, %d);", *PropertyValue, Handle);
		}, false, false);
	}
	SourceWriter.Print("break;");
	SourceWriter.End();
}

void GenerateFunction_ServerSendUpdate_MigratableData(FCodeWriter& SourceWriter, UClass* Class, const FCmdHandlePropertyMap& MigratableData)
{
	// TODO: Support fixed size arrays for migratable data. UNR-282.
	FFunctionSignature ServerSendUpdateSignature{"void",
		FString::Printf(TEXT("ServerSendUpdate_Migratable(const uint8* RESTRICT Data, int32 Handle, UProperty* Property, USpatialActorChannel* Channel, %s::Update& OutUpdate) const"),
			*SchemaMigratableDataName(Class, true))
	};
	SourceWriter.BeginFunction(ServerSendUpdateSignature, TypeBindingName(Class));

	if (MigratableData.Num() > 0)
	{
		SourceWriter.Print("switch (Handle)");
		SourceWriter.BeginScope();

		for (auto& MigProp : MigratableData)
		{
			auto Handle = MigProp.Key;
			UProperty* Property = MigProp.Value->Property;

			SourceWriter.Printf("case %d: // %s", Handle, *SchemaFieldName(MigProp.Value));
			SourceWriter.BeginScope();

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
			SourceWriter.PrintNewLine();

			FString SpatialValueSetter = TEXT("OutUpdate.set_") + SchemaFieldName(MigProp.Value);

			GenerateUnrealToSchemaConversion(
				SourceWriter, SpatialValueSetter, MigProp.Value->Property, PropertyValueName,
				[&SourceWriter, Handle](const FString& PropertyValue)
			{
				SourceWriter.Printf("Interop->QueueOutgoingObjectMigUpdate_Internal(%s, Channel, %d);", *PropertyValue, Handle);
			}, false, false);
			SourceWriter.Print("break;");
			SourceWriter.End();
		}
		SourceWriter.Outdent().Print("default:");
		SourceWriter.Indent();
		SourceWriter.Print("checkf(false, TEXT(\"Unknown migration property handle %d encountered when creating a SpatialOS update.\"));");
		SourceWriter.Print("break;");

		SourceWriter.End();
	}

	SourceWriter.End();
}

void GenerateFunction_ReceiveUpdate_RepData(FCodeWriter& SourceWriter, UClass* Class, const FUnrealFlatRepData& RepData, EReplicatedPropertyGroup Group)
{
	FFunctionSignature ReceiveUpdateSignature{"void",
		FString::Printf(TEXT("ReceiveUpdate_%s(USpatialActorChannel* ActorChannel, const %s::Update& Update) const"),
			*GetReplicatedPropertyGroupName(Group),
			*SchemaReplicatedDataName(Group, Class, true))
	};
	SourceWriter.BeginFunction(ReceiveUpdateSignature, TypeBindingName(Class));

	SourceWriter.Printf(R"""(
			Interop->PreReceiveSpatialUpdate(ActorChannel);)""");
	if (RepData[Group].Num() > 0)
	{
		SourceWriter.Printf(R"""(
				TSet<UProperty*> RepNotifies;

				const bool bIsServer = Interop->GetNetDriver()->IsServer();
				const bool bAutonomousProxy = ActorChannel->IsClientAutonomousProxy(%s::ComponentId);
				const FRepHandlePropertyMap& HandleToPropertyMap = GetRepHandlePropertyMap();
				FSpatialConditionMapFilter ConditionMap(ActorChannel, bAutonomousProxy);)""",
			*SchemaRPCComponentName(ERPCType::RPC_Client, Class, true));
		SourceWriter.PrintNewLine();
		for (auto& RepProp : RepData[Group])
		{
			check(RepProp.Value->ReplicationData->Handle > 0);

			GenerateBody_ReceiveUpdate_RepDataProperty(SourceWriter,
				RepProp.Value->ReplicationData->Handle,
				RepProp.Value);
		}
		SourceWriter.Print("Interop->PostReceiveSpatialUpdate(ActorChannel, RepNotifies.Array());");
	}
	else
	{
		SourceWriter.Print(R"""(
			TArray<UProperty*> RepNotifies;
			Interop->PostReceiveSpatialUpdate(ActorChannel, RepNotifies);)""");
	}

	SourceWriter.End();
}

void GenerateBody_ReceiveUpdate_RepDataProperty(FCodeWriter& SourceWriter, uint16 Handle, TSharedPtr<FUnrealProperty> PropertyInfo)
{
	UProperty* Property = PropertyInfo->Property;
	// Check if this property is in the update.
	SourceWriter.Printf("if (!Update.%s().empty())", *SchemaFieldName(PropertyInfo));
	SourceWriter.BeginScope();

	// Check if the property is relevant on the client.
	SourceWriter.Printf("// %s", *SchemaFieldName(PropertyInfo));
	SourceWriter.Printf("uint16 Handle = %d;", Handle);
	SourceWriter.Print("const FRepHandleData* RepData = &HandleToPropertyMap[Handle];");
	SourceWriter.Print("if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))");

	SourceWriter.BeginScope();

	if (Property->IsA<UObjectPropertyBase>())
	{
		SourceWriter.Print("bool bWriteObjectProperty = true;");
	}

	// If the property is Role or RemoteRole, ensure to swap on the client.
	if (PropertyInfo->ReplicationData->RoleSwapHandle != -1)
	{
		SourceWriter.Printf(R"""(
			// On the client, we need to swap Role/RemoteRole.
			if (!bIsServer)
			{
				Handle = %d;
				RepData = &HandleToPropertyMap[Handle];
			})""", PropertyInfo->ReplicationData->RoleSwapHandle);
		SourceWriter.PrintNewLine();
	}

	// Convert update data to the corresponding Unreal type and serialize to OutputWriter.
	FString PropertyValueName = TEXT("Value");
	FString PropertyValueCppType = Property->GetCPPType();

	// Special case for unresolved object refs inside an array.
	bool bIsArrayOfObjects = false;

	FString PropertyName = TEXT("RepData->Property");
	//todo-giray: The reinterpret_cast below is ugly and we believe we can do this more gracefully using Property helper functions.
	SourceWriter.Printf("uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));");
	if (Property->IsA<UBoolProperty>())
	{
		SourceWriter.Printf("bool %s = static_cast<UBoolProperty*>(%s)->GetPropertyValue(PropertyData);", *PropertyValueName, *PropertyName);
	}
	else if (Property->IsA<UArrayProperty>())
	{
		UArrayProperty* ArrayProperty = Cast<UArrayProperty>(Property);
		UProperty* InnerProperty = ArrayProperty->Inner;
		SourceWriter.Printf("TArray<%s> %s = *(reinterpret_cast<TArray<%s> *>(PropertyData));", *(InnerProperty->GetCPPType()), *PropertyValueName, *(InnerProperty->GetCPPType()));
		if (InnerProperty->IsA<UObjectPropertyBase>())
		{
			bIsArrayOfObjects = true;
		}
	}
	else if (Property->IsA<UWeakObjectProperty>())
	{
		FString ClassName = GetNativeClassName(Cast<UObjectPropertyBase>(Property));
		SourceWriter.Printf("auto WeakPtrData = *(reinterpret_cast<%s const*>(PropertyData));", *Property->GetCPPType());
		SourceWriter.Printf("%s* %s = WeakPtrData.Get();", *ClassName, *PropertyValueName);
	}
	else if (Property->IsA<UObjectPropertyBase>())
	{
		FString ClassName = GetNativeClassName(Cast<UObjectPropertyBase>(Property));
		SourceWriter.Printf("%s* %s = *(reinterpret_cast<%s* const*>(PropertyData));", *ClassName, *PropertyValueName, *ClassName);
	}
	else
	{
		SourceWriter.Printf("%s %s = *(reinterpret_cast<%s const*>(PropertyData));", *PropertyValueCppType, *PropertyValueName, *PropertyValueCppType);
	}
	SourceWriter.PrintNewLine();

	FString SpatialValue = FString::Printf(TEXT("(*%s.%s().data())"), TEXT("Update"), *SchemaFieldName(PropertyInfo));

	GeneratePropertyToUnrealConversion(
		SourceWriter, SpatialValue, PropertyInfo->Property, PropertyValueName,
		[&SourceWriter, bIsArrayOfObjects](const FString& PropertyValue)
	{
		if (bIsArrayOfObjects)
		{
			// This callback is called on the inner property of an array of objects. In the future, we need a way to track this property
			// as an unmapped object reference and update its value once the object is resolved. Until then, ignore this object ref.
			SourceWriter.Print(R"""(
				// Pre-alpha limitation: if a UObject* in an array property is unresolved, we currently don't have a way to update it once
				// it is resolved. It will remain null and will only be updated when the server replicates this array again (when it changes).
				UE_LOG(LogSpatialGDKInterop, Warning, TEXT("%s: Ignoring unresolved object property. Value: %s. actor %s (%lld), property %s (handle %d)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*ObjectRefToString(ObjectRef),
					*ActorChannel->Actor->GetName(),
					ActorChannel->GetEntityId().ToSpatialEntityId(),
					*RepData->Property->GetName(),
					Handle);)""");
			SourceWriter.Printf("%s = nullptr;", *PropertyValue);
		}
		else
		{
			SourceWriter.Print(R"""(
				UE_LOG(LogSpatialGDKInterop, Log, TEXT("%s: Received unresolved object property. Value: %s. actor %s (%lld), property %s (handle %d)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*ObjectRefToString(ObjectRef),
					*ActorChannel->Actor->GetName(),
					ActorChannel->GetEntityId().ToSpatialEntityId(),
					*RepData->Property->GetName(),
					Handle);)""");
			SourceWriter.Printf("// A legal static object reference should never be unresolved.");
			SourceWriter.Printf("check(ObjectRef.path().empty());");
			SourceWriter.Print("bWriteObjectProperty = false;");
			SourceWriter.Print("Interop->QueueIncomingObjectRepUpdate_Internal(ObjectRef, ActorChannel, RepData);");
		}
	}, false);

	// If this is RemoteRole, make sure to downgrade if bAutonomousProxy is false.
	if (Property->GetFName() == NAME_RemoteRole)
	{
		SourceWriter.PrintNewLine();
		SourceWriter.Print(R"""(
			// Downgrade role from AutonomousProxy to SimulatedProxy if we aren't authoritative over
			// the server RPCs component.
			if (!bIsServer && Value == ROLE_AutonomousProxy && !bAutonomousProxy)
			{
				Value = ROLE_SimulatedProxy;
			})""");
	}

	SourceWriter.PrintNewLine();

	if (Property->IsA<UObjectPropertyBase>())
	{
		SourceWriter.Print("if (bWriteObjectProperty)");
		SourceWriter.BeginScope();
	}

	SourceWriter.Print("ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);");
	SourceWriter.PrintNewLine();

	SourceWriter.Print(R"""(
		UE_LOG(LogSpatialGDKInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*ActorChannel->Actor->GetName(),
			ActorChannel->GetEntityId().ToSpatialEntityId(),
			*RepData->Property->GetName(),
			Handle);)""");

	if (Property->IsA<UObjectPropertyBase>())
	{
		SourceWriter.End();
	}

	SourceWriter.End();
	SourceWriter.End();
}

void GenerateFunction_ReceiveUpdate_MigratableData(FCodeWriter& SourceWriter, UClass* Class, const FCmdHandlePropertyMap& MigratableData)
{
	// TODO: Support fixed size arrays for migratable data. UNR-282.
	FFunctionSignature ReceiveUpdateSignature{"void",
		FString::Printf(TEXT("ReceiveUpdate_Migratable(USpatialActorChannel* ActorChannel, const %s::Update& Update) const"),
			*SchemaMigratableDataName(Class, true))
	};
	SourceWriter.BeginFunction(ReceiveUpdateSignature, TypeBindingName(Class));

	if (MigratableData.Num() > 0)
	{
		SourceWriter.Print("const FMigratableHandlePropertyMap& HandleToPropertyMap = GetMigratableHandlePropertyMap();");
		SourceWriter.PrintNewLine();
		for (auto& MigProp : MigratableData)
		{
			auto Handle = MigProp.Key;
			UProperty* Property = MigProp.Value->Property;

			// Check if this property is in the update.
			SourceWriter.Printf("if (!Update.%s().empty())", *SchemaFieldName(MigProp.Value));
			SourceWriter.BeginScope();

			SourceWriter.Printf("// %s", *SchemaFieldName(MigProp.Value));
			SourceWriter.Printf("uint16 Handle = %d;", Handle);
			SourceWriter.Print("const FMigratableHandleData* MigratableData = &HandleToPropertyMap[Handle];");

			if (Property->IsA<UObjectPropertyBase>())
			{
				SourceWriter.Print("bool bWriteObjectProperty = true;");
			}

			// Convert update data to the corresponding Unreal type and serialize to OutputWriter.
			FString PropertyValueName = TEXT("Value");
			FString PropertyValueCppType = Property->GetCPPType();
			FString PropertyName = TEXT("MigratableData->Property");
			//todo-giray: The reinterpret_cast below is ugly and we believe we can do this more gracefully using Property helper functions.
			SourceWriter.Printf("uint8* PropertyData = MigratableData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));");
			if (Property->IsA<UBoolProperty>())
			{
				SourceWriter.Printf("bool %s = static_cast<UBoolProperty*>(%s)->GetPropertyValue(PropertyData);", *PropertyValueName, *PropertyName);
			}
			else
			{
				SourceWriter.Printf("%s %s = *(reinterpret_cast<%s const*>(PropertyData));", *PropertyValueCppType, *PropertyValueName, *PropertyValueCppType);
			}
			SourceWriter.PrintNewLine();

			FString SpatialValue = FString::Printf(TEXT("(*%s.%s().data())"), TEXT("Update"), *SchemaFieldName(MigProp.Value));

			GeneratePropertyToUnrealConversion(
				SourceWriter, SpatialValue, MigProp.Value->Property, PropertyValueName,
				[&SourceWriter](const FString& PropertyValue)
			{
				SourceWriter.Print(R"""(
					UE_LOG(LogSpatialGDKInterop, Log, TEXT("%s: Received unresolved object property. Value: %s. actor %s (%lld), property %s (handle %d)"),
						*Interop->GetSpatialOS()->GetWorkerId(),
						*ObjectRefToString(ObjectRef),
						*ActorChannel->Actor->GetName(),
						ActorChannel->GetEntityId().ToSpatialEntityId(),
						*MigratableData->Property->GetName(),
						Handle);)""");
				SourceWriter.Print("bWriteObjectProperty = false;");
				SourceWriter.Print("Interop->QueueIncomingObjectMigUpdate_Internal(ObjectRef, ActorChannel, MigratableData);");
			}, false);

			SourceWriter.PrintNewLine();

			if (Property->IsA<UObjectPropertyBase>())
			{
				SourceWriter.Print("if (bWriteObjectProperty)");
				SourceWriter.BeginScope();
			}

			SourceWriter.Print("ApplyIncomingMigratablePropertyUpdate(*MigratableData, ActorChannel->Actor, static_cast<const void*>(&Value));");
			SourceWriter.PrintNewLine();

			SourceWriter.Print(R"""(
				UE_LOG(LogSpatialGDKInterop, Verbose, TEXT("%s: Received migratable property update. actor %s (%lld), property %s (handle %d)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*ActorChannel->Actor->GetName(),
					ActorChannel->GetEntityId().ToSpatialEntityId(),
					*MigratableData->Property->GetName(),
					Handle);)""");

			if (Property->IsA<UObjectPropertyBase>())
			{
				SourceWriter.End();
			}

			SourceWriter.End();
		}
	}

	SourceWriter.End();
}

void GenerateFunction_ReceiveUpdate_MulticastRPCs(FCodeWriter& SourceWriter, UClass* Class, const TArray <TSharedPtr<FUnrealRPC>> RPCs)
{
	FFunctionSignature ReceiveUpdateSignature{
		"void",
		FString::Printf(TEXT("ReceiveUpdate_NetMulticastRPCs(worker::EntityId EntityId, const %s::Update& Update)"),
			*SchemaRPCComponentName(RPC_NetMulticast, Class, true))};
	SourceWriter.BeginFunction(ReceiveUpdateSignature, TypeBindingName(Class));

	for (auto RPC : RPCs)
	{
		SourceWriter.Printf(R"""(
			for (auto& event : Update.%s())
			{
				%s_OnRPCPayload(EntityId, event);
			})""",
			*SchemaRPCName(Class, RPC->Function),
			*RPC->Function->GetName());

		SourceWriter.PrintNewLine();
	}

	SourceWriter.End();
}

void GenerateFunction_SendRPC(FCodeWriter& SourceWriter, UClass* Class, const TSharedPtr<FUnrealRPC> RPC)
{
	FFunctionSignature SendCommandSignature{
		"void",
		FString::Printf(TEXT("%s_SendRPC(worker::Connection* const Connection, void* Parameters, UObject* TargetObject)"),
			*RPC->Function->GetName())
	};
	SourceWriter.BeginFunction(SendCommandSignature, TypeBindingName(Class));
		
	// Extract RPC arguments from the stack.
	if (RPC->Function->NumParms > 0)
	{
		// Name of the struct is set from GenerateRPCArgumentsStruct
		FString ParametersStructName;
		GenerateRPCArgumentsStruct(SourceWriter, RPC, ParametersStructName);

		SourceWriter.Printf("%s StructuredParams = *static_cast<%s*>(Parameters);", *ParametersStructName, *ParametersStructName);
		SourceWriter.PrintNewLine();
	}

	// Build closure to send the command request.
	TArray<FString> CapturedArguments;
	CapturedArguments.Add(TEXT("TargetObject"));
	if (RPC->Function->NumParms > 0)
	{
		CapturedArguments.Add(TEXT("StructuredParams"));
	}
	SourceWriter.Printf("auto Sender = [this, Connection, %s]() mutable -> FRPCCommandRequestResult", *FString::Join(CapturedArguments, TEXT(", ")));
	SourceWriter.BeginScope();

	SourceWriter.Printf(R"""(
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialGDKInterop, Log, TEXT("%%s: RPC %s queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return {TargetObject};
		})""", *RPC->Function->GetName());
	SourceWriter.PrintNewLine();

	SourceWriter.Print("// Build RPC Payload.");
	SourceWriter.Printf("%s RPCPayload;", *SchemaRPCRequestType(RPC->Function, true));

	TArray<TSharedPtr<FUnrealProperty>> RPCParameters = GetFlatRPCParameters(RPC);
	for (auto Param : RPCParameters)
	{
		FString SpatialValueSetter = TEXT("RPCPayload.set_") + SchemaFieldName(Param);

		FString PropertyValue = FString::Printf(TEXT("StructuredParams.%s"), *CPPFieldName(Param));
		if (Param->Property->IsA(UWeakObjectProperty::StaticClass()))
		{
			// In the case of a weak ptr, we want to use the underlying UObject, not the TWeakObjPtr.
			PropertyValue += TEXT(".Get()");
		}

		SourceWriter.BeginScope();
		GenerateUnrealToSchemaConversion(
			SourceWriter, SpatialValueSetter, Param->Property, PropertyValue,
			[&SourceWriter, &RPC](const FString& PropertyValue)
		{
			SourceWriter.Printf("UE_LOG(LogSpatialGDKInterop, Log, TEXT(\"%%s: RPC %s queued. %s is unresolved.\"), *Interop->GetSpatialOS()->GetWorkerId());",
				*RPC->Function->GetName(),
				*PropertyValue);
			SourceWriter.Printf("return {Cast<UObject>(%s)};", *PropertyValue);
		}, true, false);
		SourceWriter.End();
	}


	FString RPCSendingMethod;
	if (RPC->Type == RPC_NetMulticast)
	{
		RPCSendingMethod = FString::Printf(TEXT(R"""(
			%s::Update Update;
			Update.add_%s(RPCPayload);
			checkf(Update.%s().size() == 1, TEXT("%s_SendCommand: More than one event being sent"));
			Connection->SendComponentUpdate<%s>(TargetObjectRef.entity(), Update);
			return {};)"""),
			*SchemaRPCComponentName(RPC->Type, Class, true),
			*SchemaRPCName(Class, RPC->Function),
			*SchemaRPCName(Class, RPC->Function),
			*RPC->Function->GetName(),
			*SchemaRPCComponentName(RPC->Type, Class, true));
	}
	else
	{
		RPCSendingMethod = FString::Printf(TEXT(R"""(
			auto RequestId = Connection->SendCommandRequest<%s::Commands::%s>(TargetObjectRef.entity(), RPCPayload, 0);
			return {RequestId.Id};)"""),
			*SchemaRPCComponentName(RPC->Type, Class, true),
			*CPPCommandClassName(Class, RPC->Function));
	}

	SourceWriter.PrintNewLine();
	SourceWriter.Printf(R"""(
		// Send RPC
		RPCPayload.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialGDKInterop, Verbose, TEXT("%%s: Sending RPC: %s, target: %%s %%s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			*ObjectRefToString(TargetObjectRef));
		%s)""",
		*RPC->Function->GetName(),
		*RPCSendingMethod);
	SourceWriter.Outdent().Print("};");
	SourceWriter.Printf("Interop->InvokeRPCSendHandler_Internal(Sender, %s);", RPC->bReliable ? TEXT("/*bReliable*/ true") : TEXT("/*bReliable*/ false"));

	SourceWriter.End();
}

void GenerateFunction_OnRPCPayload(FCodeWriter& SourceWriter, UClass* Class, const TSharedPtr<FUnrealRPC> RPC)
{
	FString FunctionParameters;
	if (RPC->Type == RPC_NetMulticast)
	{
		FunctionParameters = FString::Printf(TEXT("const worker::EntityId EntityId, const %s& EventData"),
			*SchemaRPCRequestType(RPC->Function, true));
	}
	else
	{
		FunctionParameters = FString::Printf(TEXT("const worker::CommandRequestOp<%s::Commands::%s>& Op"),
			*SchemaRPCComponentName(RPC->Type, Class, true),
			*CPPCommandClassName(Class, RPC->Function));
	}

	FString RequestFuncName = FString::Printf(TEXT("%s_OnRPCPayload(%s)"),
		*RPC->Function->GetName(), *FunctionParameters);

	SourceWriter.BeginFunction({ "void", RequestFuncName }, TypeBindingName(Class));

	FString LambdaParameters;
	if (RPC->Type == RPC_NetMulticast)
	{
		LambdaParameters = TEXT("EntityId, EventData");
	}
	else
	{
		LambdaParameters = TEXT("Op");
	}

	// Generate receiver function.
	SourceWriter.Printf("auto Receiver = [this, %s]() mutable -> FRPCCommandResponseResult", *LambdaParameters);
	SourceWriter.BeginScope();

	auto ObjectResolveFailureGenerator = [&SourceWriter, &RPC, Class](const FString& PropertyName, const FString& ObjectRef)
	{
		SourceWriter.Printf("// A legal static object reference should never be unresolved.");
		SourceWriter.Printf("checkf(%s.path().empty(), TEXT(\"A stably named object should not need resolution.\"));", *ObjectRef);
		SourceWriter.Printf(R"""(
			UE_LOG(LogSpatialGDKInterop, Log, TEXT("%%s: %s_OnRPCPayload: %s %%s is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ObjectRefToString(%s));
			return {%s};)""",
			*RPC->Function->GetName(),
			*PropertyName,
			*ObjectRef,
			*ObjectRef,
			*ObjectRef);
	};

	FString EntityId = (RPC->Type == RPC_NetMulticast) ? TEXT("EntityId") : TEXT("Op.EntityId");
	FString RPCPayload = (RPC->Type == RPC_NetMulticast) ? TEXT("EventData") : TEXT("Op.Request");

	// Get the target object.
	SourceWriter.Printf(R"""(
		improbable::unreal::UnrealObjectRef TargetObjectRef{%s, %s.target_subobject_offset(), {}, {}};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid()))""",
		*EntityId, *RPCPayload);
	SourceWriter.BeginScope();
	ObjectResolveFailureGenerator("Target object", "TargetObjectRef");
	SourceWriter.End();
	SourceWriter.Printf(R"""(
		UObject* TargetObject = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		checkf(TargetObject, TEXT("%%s: %s_OnRPCPayload: Object Ref %%s (NetGUID %%s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*ObjectRefToString(TargetObjectRef),
			*TargetNetGUID.ToString());)""",
		*RPC->Function->GetName());

	// Create RPC argument variables.
	FString RPCParametersStruct;
	if (RPC->Parameters.Num() > 0)
	{
		SourceWriter.PrintNewLine();
		SourceWriter.Print("// Declare parameters.");

		// Name of the struct is set from GenerateRPCArgumentsStruct
		FString ParametersStructName;
		GenerateRPCArgumentsStruct(SourceWriter, RPC, ParametersStructName);

		SourceWriter.Printf("%s Parameters;", *ParametersStructName);

		// Extract RPC arguments from request data.
		SourceWriter.PrintNewLine();
		SourceWriter.Print("// Extract from request data.");

		for (auto Param : GetFlatRPCParameters(RPC))
		{
			FString SpatialValue = FString::Printf(TEXT("%s.%s()"), *RPCPayload, *SchemaFieldName(Param));

			SourceWriter.BeginScope();
			GeneratePropertyToUnrealConversion(
				SourceWriter, SpatialValue, Param->Property, FString::Printf(TEXT("Parameters.%s"), *CPPFieldName(Param)),
				std::bind(ObjectResolveFailureGenerator, std::placeholders::_1, "ObjectRef"), true);
			SourceWriter.End();
		}

		RPCParametersStruct = FString(TEXT("&Parameters"));
	}
	else
	{
		RPCParametersStruct = FString(TEXT("nullptr"));
	}

	// Call implementation and send response.
	SourceWriter.PrintNewLine();
	SourceWriter.Print("// Call implementation.");
	SourceWriter.Printf(R"""(
				UE_LOG(LogSpatialGDKInterop, Verbose, TEXT("%%s: Received RPC: %s, target: %%s %%s"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*TargetObject->GetName(),
					*ObjectRefToString(TargetObjectRef));)""",
		*RPC->Function->GetName());

	SourceWriter.PrintNewLine();
	SourceWriter.Printf(R"""(
		if (UFunction* Function = TargetObject->FindFunction(FName(TEXT("%s"))))
		{
			TargetObject->ProcessEvent(Function, %s);
		}
		else
		{
			UE_LOG(LogSpatialGDKInterop, Error, TEXT("%%s: %s_OnRPCPayload: Function not found. Object: %%s, Function: %s."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*TargetObject->GetFullName());
		})""",
		*RPC->Function->GetName(),
		*RPCParametersStruct,
		*RPC->Function->GetName(),
		*RPC->Function->GetName());
	SourceWriter.PrintNewLine();

	if (RPC->Type != RPC_NetMulticast)
	{
		SourceWriter.Print("// Send command response.");
		SourceWriter.Print("TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();");
		SourceWriter.Printf("Connection->SendCommandResponse<%s::Commands::%s>(Op.RequestId, {});",
			*SchemaRPCComponentName(RPC->Type, Class, true),
			*CPPCommandClassName(Class, RPC->Function));
	}

	SourceWriter.Print("return {};");
	SourceWriter.Outdent().Print("};");

	SourceWriter.Print("Interop->InvokeRPCReceiveHandler_Internal(Receiver);");

	SourceWriter.End();
}

void GenerateFunction_RPCOnCommandResponse(FCodeWriter& SourceWriter, UClass* Class, const TSharedPtr<FUnrealRPC> RPC)
{
	FString ResponseFuncName = FString::Printf(TEXT("%s_OnCommandResponse(const worker::CommandResponseOp<%s::Commands::%s>& Op)"),
		*RPC->Function->GetName(),
		*SchemaRPCComponentName(RPC->Type, Class, true),
		*CPPCommandClassName(Class, RPC->Function));

	SourceWriter.BeginFunction({"void", ResponseFuncName}, TypeBindingName(Class));
	SourceWriter.Printf("Interop->HandleCommandResponse_Internal(TEXT(\"%s\"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));",
		*RPC->Function->GetName());
	SourceWriter.End();
}
