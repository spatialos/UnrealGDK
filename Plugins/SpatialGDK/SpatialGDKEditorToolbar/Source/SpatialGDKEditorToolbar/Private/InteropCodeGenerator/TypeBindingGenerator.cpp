// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TypeBindingGenerator.h"
#include "SchemaGenerator.h"
#include "TypeStructure.h"

#include "Utils/CodeWriter.h"

// Needed for Algo::Transform
#include "Algo/Transform.h"

// Needed for std::bind.
#include <functional>

FString TypeBindingName(UClass* Class)
{
	return FString::Printf(TEXT("USpatialTypeBinding_%s"), *Class->GetName());
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

FString PropertyToWorkerSDKType(UProperty* Property)
{
	FString DataType;

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
			DataType = TEXT("improbable::Vector3f");
		}
		else if (Struct->GetFName() == NAME_Rotator)
		{
			DataType = TEXT("improbable::unreal::UnrealFRotator");
		}
		else if (Struct->GetFName() == NAME_Plane)
		{
			DataType = TEXT("improbable::unreal::UnrealFPlane");
		}
		else
		{
			DataType = TEXT("std::string"); //this includes RepMovement and UniqueNetId
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
		DataType = TEXT("std::int32_t");
	}
	else if (Property->IsA(UByteProperty::StaticClass()))
	{
		DataType = TEXT("std::uint32_t"); // uint8 not supported in schema.
	}
	else if (Property->IsA(UNameProperty::StaticClass()) || Property->IsA(UStrProperty::StaticClass()))
	{
		DataType = TEXT("std::string");
	}
	else if (Property->IsA(UUInt32Property::StaticClass()))
	{
		DataType = TEXT("uint32");
	}
	else if (Property->IsA(UUInt64Property::StaticClass()))
	{
		DataType = TEXT("std::string");
	}
	else if (Property->IsA(UObjectPropertyBase::StaticClass()))
	{
		DataType = TEXT("improbable::unreal::UnrealObjectRef");
	}
	else if (Property->IsA(UArrayProperty::StaticClass()))
	{
		DataType = PropertyToWorkerSDKType(Cast<UArrayProperty>(Property)->Inner);
		DataType = FString::Printf(TEXT("::worker::List<%s>"), *DataType);
	}
	else
	{
		DataType = TEXT("std::string");
	}

	return DataType;
}

void GenerateUnrealToSchemaConversion(FCodeWriter& Writer, const FString& Update, UProperty* Property, const FString& PropertyValue, const bool bIsUpdate, TFunction<void(const FString&)> ObjectResolveFailureGenerator)
{
	// Get result type.
	if (UEnumProperty* EnumProperty = Cast<UEnumProperty>(Property))
	{
		Writer.Printf("// UNSUPPORTED UEnumProperty %s(%s);", *Update, *PropertyValue);
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
			Writer.Printf("%s(improbable::Vector3f(%s.X, %s.Y, %s.Z));", *Update, *PropertyValue, *PropertyValue, *PropertyValue);
		}
		else if (Struct->GetFName() == NAME_Rotator)
		{
			Writer.Printf("%s(improbable::unreal::UnrealFRotator(%s.Yaw, %s.Pitch, %s.Roll));", *Update, *PropertyValue, *PropertyValue, *PropertyValue);
		}
		else if (Struct->GetFName() == NAME_Plane)
		{
			Writer.Printf("%s(improbable::unreal::UnrealFPlane(%s.X, %s.Y, %s.Z, %s.W));", *Update, *PropertyValue, *PropertyValue, *PropertyValue, *PropertyValue);
		}
		else if (Struct->StructFlags & STRUCT_NetSerializeNative)
		{
			// If user has implemented NetSerialize for custom serialization, we use that. Core structs like RepMovement or UniqueNetIdRepl also go through this path.
			Writer.BeginScope();
			Writer.Printf(R"""(
				TArray<uint8> ValueData;
				FMemoryWriter ValueDataWriter(ValueData);
				bool Success;
				(const_cast<%s&>(%s)).NetSerialize(ValueDataWriter, PackageMap, Success);
				%s(std::string(reinterpret_cast<char*>(ValueData.GetData()), ValueData.Num()));)""", *Struct->GetStructCPPName(), *PropertyValue, *Update);
			Writer.End();
		}
		else
		{
			// We do a basic binary serialization for the generic struct.
			Writer.Printf(R"""(
				TArray<uint8> ValueData;
				FMemoryWriter ValueDataWriter(ValueData);
				%s::StaticStruct()->SerializeBin(ValueDataWriter, reinterpret_cast<void*>(const_cast<%s*>(&%s)));
				%s(std::string(reinterpret_cast<char*>(ValueData.GetData()), ValueData.Num()));)""", *Property->GetCPPType(), *Property->GetCPPType(), *PropertyValue, *Update);
		}
	}
	else if (Property->IsA(UBoolProperty::StaticClass()))
	{
		Writer.Printf("%s(%s);", *Update, *PropertyValue);
	}
	else if (Property->IsA(UFloatProperty::StaticClass()))
	{
		Writer.Printf("%s(%s);", *Update, *PropertyValue);
	}
	else if (Property->IsA(UIntProperty::StaticClass()))
	{
		Writer.Printf("%s(%s);", *Update, *PropertyValue);
	}
	else if (Property->IsA(UByteProperty::StaticClass()))
	{
		Writer.Printf("%s(uint32_t(%s));", *Update, *PropertyValue);
	}
	else if (Property->IsA(UClassProperty::StaticClass()))
	{
		Writer.Printf("%s(PackageMap->GetHashFromStaticClass(%s));", *Update, *PropertyValue);
	}
	else if (Property->IsA(UObjectPropertyBase::StaticClass()))
	{
		Writer.Printf("if (%s != nullptr)", *PropertyValue);
		Writer.BeginScope();
		Writer.Printf(R"""(
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(%s);
			improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
			if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF))""", *PropertyValue);
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
	else if (Property->IsA(UUInt32Property::StaticClass()))
	{
		Writer.Printf("%s(uint32_t(%s));", *Update, *PropertyValue);
	}
	else if (Property->IsA(UUInt64Property::StaticClass()))
	{
		Writer.Printf("%s(uint64_t(%s));", *Update, *PropertyValue);
	}
	else if (Property->IsA(UStrProperty::StaticClass()))
	{
		Writer.Printf("%s(TCHAR_TO_UTF8(*%s));", *Update, *PropertyValue);
	}
	else if (Property->IsA(UArrayProperty::StaticClass()))
	{
		UArrayProperty* ArrayProperty = Cast<UArrayProperty>(Property);

		Writer.BeginScope();

		Writer.Printf("::worker::List<%s> List;", *PropertyToWorkerSDKType(ArrayProperty->Inner));

		Writer.Printf("for(int i = 0; i < %s.Num(); i++)", *PropertyValue);
		Writer.BeginScope();

		GenerateUnrealToSchemaConversion(Writer, "List.emplace_back", ArrayProperty->Inner, FString::Printf(TEXT("%s[i]"), *PropertyValue), bIsUpdate, ObjectResolveFailureGenerator);

		Writer.End();

		Writer.Printf("%s(List);", *Update);

		Writer.End();
	}
	else
	{
		Writer.Printf("// UNSUPPORTED U%s (unhandled) %s(%s)", *Property->GetClass()->GetName(), *Update, *PropertyValue);
	}
}

void GeneratePropertyToUnrealConversion(FCodeWriter& Writer, const FString& Update, const UProperty* Property, const FString& PropertyValue, const bool bIsUpdate, TFunction<void(const FString&)> ObjectResolveFailureGenerator)
{
	FString PropertyType = Property->GetCPPType();

	// Get result type.
	if (const UEnumProperty* EnumProperty = Cast<UEnumProperty>(Property))
	{
		Writer.Printf("// UNSUPPORTED UEnumProperty %s %s", *PropertyValue, *Update);
	}

	// Try to special case to custom types we know about
	if (Property->IsA(UStructProperty::StaticClass()))
	{
		const UStructProperty * StructProp = Cast<UStructProperty>(Property);
		UScriptStruct * Struct = StructProp->Struct;
		if (Struct->GetFName() == NAME_Vector ||
			Struct->GetName() == TEXT("Vector_NetQuantize100") ||
			Struct->GetName() == TEXT("Vector_NetQuantize10") ||
			Struct->GetName() == TEXT("Vector_NetQuantizeNormal") ||
			Struct->GetName() == TEXT("Vector_NetQuantize"))
		{
			Writer.Print("{").Indent();
			Writer.Printf("auto& Vector = %s;", *Update);
			Writer.Printf("%s.X = Vector.x();", *PropertyValue);
			Writer.Printf("%s.Y = Vector.y();", *PropertyValue);
			Writer.Printf("%s.Z = Vector.z();", *PropertyValue);
			Writer.Outdent().Print("}");
		}
		else if (Struct->GetFName() == NAME_Rotator)
		{
			Writer.Print("{").Indent();
			Writer.Printf("auto& Rotator = %s;", *Update);
			Writer.Printf("%s.Yaw = Rotator.yaw();", *PropertyValue);
			Writer.Printf("%s.Pitch = Rotator.pitch();", *PropertyValue);
			Writer.Printf("%s.Roll = Rotator.roll();", *PropertyValue);
			Writer.Outdent().Print("}");
		}
		else if (Struct->GetFName() == NAME_Plane)
		{
			Writer.Print("{").Indent();
			Writer.Printf("auto& Plane = %s;", *Update);
			Writer.Printf("%s.X = Plane.x();", *PropertyValue);
			Writer.Printf("%s.Y = Plane.y();", *PropertyValue);
			Writer.Printf("%s.Z = Plane.z();", *PropertyValue);
			Writer.Printf("%s.W = Plane.w();", *PropertyValue);
			Writer.Outdent().Print("}");
		}
		else if (Struct->StructFlags & STRUCT_NetSerializeNative)
		{
			// If user has implemented NetSerialize for custom serialization, we use that. Core structs like RepMovement or UniqueNetIdRepl also go through this path.
			Writer.BeginScope();
			Writer.Printf(R"""(
				auto& ValueDataStr = %s;
				TArray<uint8> ValueData;
				ValueData.Append(reinterpret_cast<const uint8*>(ValueDataStr.data()), ValueDataStr.size());
				FMemoryReader ValueDataReader(ValueData);
				bool bSuccess;
				%s.NetSerialize(ValueDataReader, PackageMap, bSuccess);)""", *Update, *PropertyValue);
			Writer.End();
		}
		else
		{
			Writer.Printf(R"""(
				auto& ValueDataStr = %s;
				TArray<uint8> ValueData;
				ValueData.Append(reinterpret_cast<const uint8*>(ValueDataStr.data()), ValueDataStr.size());
				FMemoryReader ValueDataReader(ValueData);
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
	else if (Property->IsA(UIntProperty::StaticClass()))
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
	else if (Property->IsA(UClassProperty::StaticClass()))
	{
		Writer.Printf("%s = PackageMap->GetStaticClassFromHash(%s);", *PropertyValue, *Update);
	}
	else if (Property->IsA(UObjectPropertyBase::StaticClass()))
	{
		Writer.BeginScope();
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
				%s = dynamic_cast<%s>(Object_Raw);
				checkf(%s, TEXT("Object ref %%s maps to object %%s with the wrong class."), *ObjectRefToString(ObjectRef), *Object_Raw->GetFullName());
			})""", *PropertyValue, *PropertyType, *PropertyValue);
		Writer.Print("else");
		Writer.BeginScope();
		ObjectResolveFailureGenerator(*PropertyValue);
		Writer.End();
		Writer.End();
		Writer.End();
	}
	else if (Property->IsA(UNameProperty::StaticClass()))
	{
		Writer.Printf("%s = FName((%s).data());", *PropertyValue, *Update);
	}
	else if (Property->IsA(UUInt32Property::StaticClass()))
	{
		Writer.Printf("%s = uint32(%s);", *PropertyValue, *Update);
	}
	else if (Property->IsA(UUInt64Property::StaticClass()))
	{
		Writer.Printf("%s = uint64(%s);", *PropertyValue, *Update);
	}
	else if (Property->IsA(UStrProperty::StaticClass()))
	{
		Writer.Printf("%s = FString(UTF8_TO_TCHAR(%s.c_str()));", *PropertyValue, *Update);
	}
	else if (Property->IsA(UArrayProperty::StaticClass())) {
		const UArrayProperty* ArrayProperty = Cast<UArrayProperty>(Property);

		Writer.BeginScope();
		Writer.Printf("auto& List = %s;", *Update);
		Writer.Printf("%s.SetNum(List.size());", *PropertyValue);
		Writer.Print("for(int i = 0; i < List.size(); i++)");
		Writer.BeginScope();
		GeneratePropertyToUnrealConversion(Writer, "List[i]", ArrayProperty->Inner, FString::Printf(TEXT("%s[i]"), *PropertyValue), true, ObjectResolveFailureGenerator);
		Writer.End();
		Writer.End();
	}
	else
	{
		Writer.Printf("// UNSUPPORTED U%s (unhandled) %s %s", *Property->GetClass()->GetName(), *PropertyValue, *Update);
	}
}

void GenerateRPCArgumentsStruct(FCodeWriter& Writer, const TSharedPtr<FUnrealRPC>& RPC, const FString& StructName)
{
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
		#include <improbable/unreal/core_types.h>
		#include <improbable/unreal/unreal_metadata.h>
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
		void BindToView() override;
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

	HeaderWriter.Print("// Component update helper functions.");
	FFunctionSignature BuildComponentUpdateSignature;
	BuildComponentUpdateSignature.Type = "void";
	BuildComponentUpdateSignature.NameAndParams = "BuildSpatialComponentUpdate(\n\tconst FPropertyChangeState& Changes,\n\tUSpatialActorChannel* Channel,";
	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		BuildComponentUpdateSignature.NameAndParams += FString::Printf(TEXT("\n\timprobable::unreal::%s::Update& %sUpdate,\n\tbool& b%sUpdateChanged,"),
			*SchemaReplicatedDataName(Group, Class),
			*GetReplicatedPropertyGroupName(Group),
			*GetReplicatedPropertyGroupName(Group),
			*GetReplicatedPropertyGroupName(Group));
	}
	BuildComponentUpdateSignature.NameAndParams += FString::Printf(TEXT("\n\timprobable::unreal::%s::Update& MigratableDataUpdate,\n\tbool& bMigratableDataUpdateChanged"),
		*SchemaMigratableDataName(Class));
	BuildComponentUpdateSignature.NameAndParams += ") const";
	HeaderWriter.Print(BuildComponentUpdateSignature.Declaration());
	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		HeaderWriter.Printf("void ServerSendUpdate_%s(const uint8* RESTRICT Data, int32 Handle, UProperty* Property, USpatialActorChannel* Channel, improbable::unreal::%s::Update& OutUpdate) const;",
			*GetReplicatedPropertyGroupName(Group),
			*SchemaReplicatedDataName(Group, Class));
	}
	HeaderWriter.Printf("void ServerSendUpdate_Migratable(const uint8* RESTRICT Data, int32 Handle, UProperty* Property, USpatialActorChannel* Channel, improbable::unreal::%s::Update& OutUpdate) const;",
		*SchemaMigratableDataName(Class));
	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		HeaderWriter.Printf("void ReceiveUpdate_%s(USpatialActorChannel* ActorChannel, const improbable::unreal::%s::Update& Update) const;",
			*GetReplicatedPropertyGroupName(Group),
			*SchemaReplicatedDataName(Group, Class));
	}
	HeaderWriter.Printf("void ReceiveUpdate_Migratable(USpatialActorChannel* ActorChannel, const improbable::unreal::%s::Update& Update) const;",
		*SchemaMigratableDataName(Class));

	// RPCs.
	FUnrealRPCsByType RPCsByType = GetAllRPCsByType(TypeInfo);

	HeaderWriter.PrintNewLine();
	HeaderWriter.Print("// RPC command sender functions.");
	for (auto Group : GetRPCTypes())
	{
		for (auto& RPC : RPCsByType[Group])
		{
			HeaderWriter.Printf("void %s_SendCommand(worker::Connection* const Connection, void* Params, UObject* TargetObject);",
				*RPC->Function->GetName());
		}
	}

	HeaderWriter.PrintNewLine();
	HeaderWriter.Print("// RPC command request handler functions.");
	for (auto Group : GetRPCTypes())
	{
		for (auto& RPC : RPCsByType[Group])
		{
			HeaderWriter.Printf("void %s_OnCommandRequest(const worker::CommandRequestOp<improbable::unreal::%s::Commands::%s>& Op);",
				*RPC->Function->GetName(),
				*SchemaRPCComponentName(Group, Class),
				*CPPCommandClassName(RPC->Function));
		}
	}

	HeaderWriter.PrintNewLine();
	HeaderWriter.Print("// RPC command response handler functions.");
	for (auto Group : GetRPCTypes())
	{
		// Command response receiver function signatures
		for (auto& RPC : RPCsByType[Group])
		{
			HeaderWriter.Printf("void %s_OnCommandResponse(const worker::CommandResponseOp<improbable::unreal::%s::Commands::%s>& Op);",
				*RPC->Function->GetName(),
				*SchemaRPCComponentName(Group, Class),
				*CPPCommandClassName(RPC->Function));
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
	TMap<uint16, TSharedPtr<FUnrealProperty>> MigratableData = GetFlatMigratableData(TypeInfo);
	FUnrealRPCsByType RPCsByType = GetAllRPCsByType(TypeInfo);

	// Generate methods implementations

	SourceWriter.PrintNewLine();
	GenerateFunction_GetRepHandlePropertyMap(SourceWriter, Class, RepData);

	SourceWriter.PrintNewLine();
	GenerateFunction_GetMigratableHandlePropertyMap(SourceWriter, Class, MigratableData);

	SourceWriter.PrintNewLine();
	GenerateFunction_GetBoundClass(SourceWriter, Class);

	SourceWriter.PrintNewLine();
	GenerateFunction_Init(SourceWriter, Class, RPCsByType);

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

	for (auto Group : GetRPCTypes())
	{
		for (auto& RPC : RPCsByType[Group])
		{
			SourceWriter.PrintNewLine();
			GenerateFunction_RPCSendCommand(SourceWriter, Class, RPC);
		}
	}

	for (auto Group : GetRPCTypes())
	{
		for (auto& RPC : RPCsByType[Group])
		{
			SourceWriter.PrintNewLine();
			GenerateFunction_RPCOnCommandRequest(SourceWriter, Class, RPC);
		}
	}

	for (auto Group : GetRPCTypes())
	{
		for (auto& RPC : RPCsByType[Group])
		{
			SourceWriter.PrintNewLine();
			GenerateFunction_RPCOnCommandResponse(SourceWriter, Class, RPC);
		}
	}
}

void GenerateFunction_GetRepHandlePropertyMap(FCodeWriter& SourceWriter, UClass* Class, const FUnrealFlatRepData& RepData)
{
	SourceWriter.BeginFunction({"const FRepHandlePropertyMap&", "GetRepHandlePropertyMap() const"}, TypeBindingName(Class));

	SourceWriter.Print("static FRepHandlePropertyMap HandleToPropertyMap;");
	SourceWriter.Print("if (HandleToPropertyMap.Num() == 0)");
	SourceWriter.BeginScope();

	// Reduce into single list of properties.
	TMap<uint16, TSharedPtr<FUnrealProperty>> ReplicatedProperties;
	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		ReplicatedProperties.Append(RepData[Group]);
	}
	ReplicatedProperties.KeySort([](uint16 A, uint16 B)
	{
		return A < B;
	});

	// Get class.
	SourceWriter.Printf("UClass* Class = FindObject<UClass>(ANY_PACKAGE, TEXT(\"%s\"));", *Class->GetName());

	// Populate HandleToPropertyMap.
	for (auto& RepProp : ReplicatedProperties)
	{
		auto Handle = RepProp.Key;

		// Create property chain initialiser list.
		FString PropertyChainInitList;
		TArray<FString> PropertyChainNames;
		Algo::Transform(GetPropertyChain(RepProp.Value), PropertyChainNames, [](const TSharedPtr<FUnrealProperty>& Property) -> FString
		{
			return TEXT("\"") + Property->Property->GetFName().ToString() + TEXT("\"");
		});
		PropertyChainInitList = FString::Join(PropertyChainNames, TEXT(", "));

		// Populate HandleToPropertyMap.
		SourceWriter.Printf("HandleToPropertyMap.Add(%d, FRepHandleData(Class, {%s}, %s, %s));",
			Handle,
			*PropertyChainInitList,
			*GetLifetimeConditionAsString(RepProp.Value->ReplicationData->Condition),
			*GetRepNotifyLifetimeConditionAsString(RepProp.Value->ReplicationData->RepNotifyCondition));
	}

	SourceWriter.End();

	SourceWriter.Print("return HandleToPropertyMap;");
	SourceWriter.End();
}

void GenerateFunction_GetMigratableHandlePropertyMap(FCodeWriter& SourceWriter, UClass* Class, const TMap<uint16, TSharedPtr<FUnrealProperty>>& MigratableData)
{
	SourceWriter.BeginFunction({"const FMigratableHandlePropertyMap&", "GetMigratableHandlePropertyMap() const"}, TypeBindingName(Class));

	SourceWriter.Print("static FMigratableHandlePropertyMap HandleToPropertyMap;");

	if (MigratableData.Num() > 0)
	{
		SourceWriter.Print("if (HandleToPropertyMap.Num() == 0)");
		SourceWriter.BeginScope();

		// Get class.
		SourceWriter.Printf("UClass* Class = FindObject<UClass>(ANY_PACKAGE, TEXT(\"%s\"));", *Class->GetName());

		// Populate HandleToPropertyMap.
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

			// Populate HandleToPropertyMap.
			SourceWriter.Printf("HandleToPropertyMap.Add(%d, FMigratableHandleData(Class, {%s}));",
				Handle,
				*PropertyChainInitList);
		}

		SourceWriter.End();
	}

	SourceWriter.Print("return HandleToPropertyMap;");
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

void GenerateFunction_Init(FCodeWriter& SourceWriter, UClass* Class, const FUnrealRPCsByType& RPCsByType)
{
	SourceWriter.BeginFunction({"void", "Init(USpatialInterop* InInterop, USpatialPackageMapClient* InPackageMap)"}, TypeBindingName(Class));

	SourceWriter.Print("Super::Init(InInterop, InPackageMap);");
	SourceWriter.PrintNewLine();
	for (auto Group : GetRPCTypes())
	{
		for (auto& RPC : RPCsByType[Group])
		{
			SourceWriter.Printf("RPCToSenderMap.Emplace(\"%s\", &%s::%s_SendCommand);", *RPC->Function->GetName(), *TypeBindingName(Class), *RPC->Function->GetName());
		}
	}

	SourceWriter.End();
}

void GenerateFunction_BindToView(FCodeWriter& SourceWriter, UClass* Class, const FUnrealRPCsByType& RPCsByType)
{
	SourceWriter.BeginFunction({"void", "BindToView()"}, TypeBindingName(Class));

	SourceWriter.Print("TSharedPtr<worker::View> View = Interop->GetSpatialOS()->GetView().Pin();");
	SourceWriter.Print("ViewCallbacks.Init(View);");
	SourceWriter.PrintNewLine();
	SourceWriter.Print("if (Interop->GetNetDriver()->GetNetMode() == NM_Client)");
	SourceWriter.BeginScope();
	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		SourceWriter.Printf("ViewCallbacks.Add(View->OnComponentUpdate<improbable::unreal::%s>([this](",
			*SchemaReplicatedDataName(Group, Class));
		SourceWriter.Indent();
		SourceWriter.Printf("const worker::ComponentUpdateOp<improbable::unreal::%s>& Op)", *SchemaReplicatedDataName(Group, Class));
		SourceWriter.Outdent();
		SourceWriter.Print("{");
		SourceWriter.Indent();
		SourceWriter.Printf(R"""(
			// TODO: Remove this check once we can disable component update short circuiting. This will be exposed in 14.0. See TIG-137.
			if (HasComponentAuthority(Interop->GetSpatialOS()->GetView(), Op.EntityId, improbable::unreal::%s::ComponentId))
			{
				return;
			})""", *SchemaReplicatedDataName(Group, Class));
		SourceWriter.Printf(R"""(
			USpatialActorChannel* ActorChannel = Interop->GetActorChannelByEntityId(Op.EntityId);
			check(ActorChannel);
			ReceiveUpdate_%s(ActorChannel, Op.Update);)""",
			*GetReplicatedPropertyGroupName(Group));
		SourceWriter.Outdent();
		SourceWriter.Print("}));");
	}
	SourceWriter.Printf("ViewCallbacks.Add(View->OnComponentUpdate<improbable::unreal::%s>([this](", *SchemaMigratableDataName(Class));
	SourceWriter.Indent();
	SourceWriter.Printf("const worker::ComponentUpdateOp<improbable::unreal::%s>& Op)", *SchemaMigratableDataName(Class));
	SourceWriter.Outdent();
	SourceWriter.Print("{");
	SourceWriter.Indent();
	SourceWriter.Printf(R"""(
		// TODO: Remove this check once we can disable component update short circuiting. This will be exposed in 14.0. See TIG-137.
		if (HasComponentAuthority(Interop->GetSpatialOS()->GetView(), Op.EntityId, improbable::unreal::%s::ComponentId))
		{
			return;
		})""", *SchemaMigratableDataName(Class));
	SourceWriter.Print(R"""(
		USpatialActorChannel* ActorChannel = Interop->GetActorChannelByEntityId(Op.EntityId);
		check(ActorChannel);
		ReceiveUpdate_Migratable(ActorChannel, Op.Update);)""");
	SourceWriter.Outdent();
	SourceWriter.Print("}));");
	SourceWriter.End();

	for (auto Group : GetRPCTypes())
	{
		// Ensure that this class contains RPCs of the type specified by group (eg, Server or Client) so that we don't generate code for missing components
		if (RPCsByType.Contains(Group) && RPCsByType[Group].Num() > 0)
		{
			SourceWriter.PrintNewLine();
			SourceWriter.Printf("using %sRPCCommandTypes = improbable::unreal::%s::Commands;",
				*GetRPCTypeName(Group),
				*SchemaRPCComponentName(Group, Class));
			for (auto& RPC : RPCsByType[Group])
			{
				SourceWriter.Printf("ViewCallbacks.Add(View->OnCommandRequest<%sRPCCommandTypes::%s>(std::bind(&%s::%s_OnCommandRequest, this, std::placeholders::_1)));",
					*GetRPCTypeName(Group),
					*CPPCommandClassName(RPC->Function),
					*TypeBindingName(Class),
					*RPC->Function->GetName());
			}
			for (auto& RPC : RPCsByType[Group])
			{
				SourceWriter.Printf("ViewCallbacks.Add(View->OnCommandResponse<%sRPCCommandTypes::%s>(std::bind(&%s::%s_OnCommandResponse, this, std::placeholders::_1)));",
					*GetRPCTypeName(Group),
					*CPPCommandClassName(RPC->Function),
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
		SourceWriter.Printf("improbable::unreal::%s::Data %sData;",
			*SchemaReplicatedDataName(Group, Class),
			*GetReplicatedPropertyGroupName(Group));
		SourceWriter.Printf("improbable::unreal::%s::Update %sUpdate;",
			*SchemaReplicatedDataName(Group, Class),
			*GetReplicatedPropertyGroupName(Group));
		SourceWriter.Printf("bool b%sUpdateChanged = false;", *GetReplicatedPropertyGroupName(Group));
	}
	SourceWriter.Printf("improbable::unreal::%s::Data MigratableData;", *SchemaMigratableDataName(Class));
	SourceWriter.Printf("improbable::unreal::%s::Update MigratableDataUpdate;", *SchemaMigratableDataName(Class));
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
	SourceWriter.Printf(R"""(
		.AddPositionComponent(improbable::Position::Data{SpatialPosition}, WorkersOnly)
		.AddMetadataComponent(improbable::Metadata::Data{TCHAR_TO_UTF8(*Metadata)})
		.SetPersistence(true)
		.SetReadAcl(%s)
		.AddComponent<improbable::unreal::UnrealMetadata>(UnrealMetadata, WorkersOnly))""",
		Class->GetName() == TEXT("PlayerController") ? TEXT("AnyUnrealWorkerOrOwningClient") : TEXT("AnyUnrealWorkerOrClient"));
	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		SourceWriter.Printf(".AddComponent<improbable::unreal::%s>(%sData, WorkersOnly)",
			*SchemaReplicatedDataName(Group, Class), *GetReplicatedPropertyGroupName(Group));
	}
	SourceWriter.Printf(".AddComponent<improbable::unreal::%s>(MigratableData, WorkersOnly)", *SchemaMigratableDataName(Class));
	SourceWriter.Printf(".AddComponent<improbable::unreal::%s>(improbable::unreal::%s::Data{}, OwningClientOnly)",
		*SchemaRPCComponentName(ERPCType::RPC_Client, Class), *SchemaRPCComponentName(ERPCType::RPC_Client, Class));
	SourceWriter.Printf(".AddComponent<improbable::unreal::%s>(improbable::unreal::%s::Data{}, WorkersOnly)",
		*SchemaRPCComponentName(ERPCType::RPC_Server, Class), *SchemaRPCComponentName(ERPCType::RPC_Server, Class));

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
		SourceWriter.Printf("improbable::unreal::%s::Update %sUpdate;",
			*SchemaReplicatedDataName(Group, Class),
			*GetReplicatedPropertyGroupName(Group));
		SourceWriter.Printf("bool b%sUpdateChanged = false;", *GetReplicatedPropertyGroupName(Group));
	}
	SourceWriter.Printf("improbable::unreal::%s::Update MigratableDataUpdate;", *SchemaMigratableDataName(Class));
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
		SourceWriter.Printf("Connection->SendComponentUpdate<improbable::unreal::%s>(EntityId.ToSpatialEntityId(), %sUpdate);",
			*SchemaReplicatedDataName(Group, Class),
			*GetReplicatedPropertyGroupName(Group));
		SourceWriter.End();
	}
	SourceWriter.Printf("if (bMigratableDataUpdateChanged)");
	SourceWriter.BeginScope();
	SourceWriter.Printf("Connection->SendComponentUpdate<improbable::unreal::%s>(EntityId.ToSpatialEntityId(), MigratableDataUpdate);",
		*SchemaMigratableDataName(Class));
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
		checkf(*SenderFuncIterator, TEXT("Sender for %s has not been registered with RPCToSenderMap."), *Function->GetFName().ToString());
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
				auto Update = improbable::unreal::%s::Update::FromInitialData(*%sAddOp->Data.data());
				ReceiveUpdate_%s(Channel, Update);
			})""",
			*GetReplicatedPropertyGroupName(Group),
			*SchemaReplicatedDataName(Group, Class),
			*GetReplicatedPropertyGroupName(Group),
			*SchemaReplicatedDataName(Group, Class),
			*GetReplicatedPropertyGroupName(Group),
			*GetReplicatedPropertyGroupName(Group));
	}
	SourceWriter.Printf(R"""(
		auto* MigratableDataAddOp = Cast<U%sAddComponentOp>(AddComponentOp);
		if (MigratableDataAddOp)
		{
			auto Update = improbable::unreal::%s::Update::FromInitialData(*MigratableDataAddOp->Data.data());
			ReceiveUpdate_Migratable(Channel, Update);
		})""",
		*SchemaMigratableDataName(Class),
		*SchemaMigratableDataName(Class));
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
				Interest.emplace(improbable::unreal::%s::ComponentId, worker::InterestOverride{false});
			}
			Interest.emplace(improbable::unreal::%s::ComponentId, worker::InterestOverride{false});
		}
		return Interest;)""",
		*SchemaReplicatedDataName(REP_SingleClient, Class),
		*SchemaMigratableDataName(Class));
	SourceWriter.End();
}

void GenerateFunction_BuildSpatialComponentUpdate(FCodeWriter& SourceWriter, UClass* Class)
{
	FFunctionSignature BuildComponentUpdateSignature;
	BuildComponentUpdateSignature.Type = "void";
	BuildComponentUpdateSignature.NameAndParams = "BuildSpatialComponentUpdate(\n\tconst FPropertyChangeState& Changes,\n\tUSpatialActorChannel* Channel,";
	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		BuildComponentUpdateSignature.NameAndParams += FString::Printf(TEXT("\n\timprobable::unreal::%s::Update& %sUpdate,\n\tbool& b%sUpdateChanged,"),
			*SchemaReplicatedDataName(Group, Class),
			*GetReplicatedPropertyGroupName(Group),
			*GetReplicatedPropertyGroupName(Group),
			*GetReplicatedPropertyGroupName(Group));
	}
	BuildComponentUpdateSignature.NameAndParams += FString::Printf(TEXT("\n\timprobable::unreal::%s::Update& MigratableDataUpdate,\n\tbool& bMigratableDataUpdateChanged"),
		*SchemaMigratableDataName(Class));
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
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending property update. actor %s (%lld), property %s (handle %d)"),
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
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending migratable property update. actor %s (%lld), property %s (handle %d)"),
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
		FString::Printf(TEXT("ServerSendUpdate_%s(const uint8* RESTRICT Data, int32 Handle, UProperty* Property, USpatialActorChannel* Channel, improbable::unreal::%s::Update& OutUpdate) const"),
			*GetReplicatedPropertyGroupName(Group),
			*SchemaReplicatedDataName(Group, Class))
	};
	SourceWriter.BeginFunction(ServerSendUpdateSignature, TypeBindingName(Class));

	if (RepData[Group].Num() > 0)
	{
		SourceWriter.Print("switch (Handle)");
		SourceWriter.BeginScope();

		for (auto& RepProp : RepData[Group])
		{
			auto Handle = RepProp.Key;
			UProperty* Property = RepProp.Value->Property;

			SourceWriter.Printf("case %d: // %s", Handle, *SchemaFieldName(RepProp.Value));
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
			else if (Property->IsA<UArrayProperty>())
			{
				UArrayProperty* ArrayProperty = Cast<UArrayProperty>(Property);
				SourceWriter.Printf("const TArray<%s>& %s = *(reinterpret_cast<TArray<%s> const*>(Data));", *ArrayProperty->Inner->GetCPPType(), *PropertyValueName, *ArrayProperty->Inner->GetCPPType());
			}
			else if (Property->IsA<UStructProperty>())
			{
				SourceWriter.Printf("const %s& %s = *(reinterpret_cast<%s const*>(Data));", *PropertyValueCppType, *PropertyValueName, *PropertyValueCppType);
			}
			else
			{
				SourceWriter.Printf("%s %s = *(reinterpret_cast<%s const*>(Data));", *PropertyValueCppType, *PropertyValueName, *PropertyValueCppType);
			}

			FString SpatialValueSetter = TEXT("OutUpdate.set_") + SchemaFieldName(RepProp.Value);

			SourceWriter.PrintNewLine();
			GenerateUnrealToSchemaConversion(
				SourceWriter, SpatialValueSetter, RepProp.Value->Property, PropertyValueName, true,
				[&SourceWriter, Handle](const FString& PropertyValue)
			{
				SourceWriter.Printf("Interop->QueueOutgoingObjectRepUpdate_Internal(%s, Channel, %d);", *PropertyValue, Handle);
			});
			SourceWriter.Print("break;");
			SourceWriter.End();
		}
		SourceWriter.Outdent().Print("default:");
		SourceWriter.Indent();
		SourceWriter.Print("checkf(false, TEXT(\"Unknown replication handle %d encountered when creating a SpatialOS update.\"));");
		SourceWriter.Print("break;");

		SourceWriter.End();
	}

	SourceWriter.End();
}

void GenerateFunction_ServerSendUpdate_MigratableData(FCodeWriter& SourceWriter, UClass* Class, const TMap<uint16, TSharedPtr<FUnrealProperty>>& MigratableData)
{
	FFunctionSignature ServerSendUpdateSignature{"void",
		FString::Printf(TEXT("ServerSendUpdate_Migratable(const uint8* RESTRICT Data, int32 Handle, UProperty* Property, USpatialActorChannel* Channel, improbable::unreal::%s::Update& OutUpdate) const"),
			*SchemaMigratableDataName(Class))
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
				SourceWriter, SpatialValueSetter, MigProp.Value->Property, PropertyValueName, true,
				[&SourceWriter, Handle](const FString& PropertyValue)
			{
				SourceWriter.Printf("Interop->QueueOutgoingObjectMigUpdate_Internal(%s, Channel, %d);", *PropertyValue, Handle);
			});
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
		FString::Printf(TEXT("ReceiveUpdate_%s(USpatialActorChannel* ActorChannel, const improbable::unreal::%s::Update& Update) const"),
			*GetReplicatedPropertyGroupName(Group),
			*SchemaReplicatedDataName(Group, Class))
	};
	SourceWriter.BeginFunction(ReceiveUpdateSignature, TypeBindingName(Class));

	SourceWriter.Printf(R"""(
			Interop->PreReceiveSpatialUpdate(ActorChannel);)""");
	if (RepData[Group].Num() > 0)
	{
		SourceWriter.Printf(R"""(
				TSet<UProperty*> RepNotifies;

				const bool bIsServer = Interop->GetNetDriver()->IsServer();
				const bool bAutonomousProxy = ActorChannel->IsClientAutonomousProxy(improbable::unreal::%s::ComponentId);
				const FRepHandlePropertyMap& HandleToPropertyMap = GetRepHandlePropertyMap();
				FSpatialConditionMapFilter ConditionMap(ActorChannel, bAutonomousProxy);)""",
			*SchemaRPCComponentName(ERPCType::RPC_Client, Class));
		SourceWriter.PrintNewLine();
		for (auto& RepProp : RepData[Group])
		{
			auto Handle = RepProp.Key;
			UProperty* Property = RepProp.Value->Property;

			// Check if this property is in the update.
			SourceWriter.Printf("if (!Update.%s().empty())", *SchemaFieldName(RepProp.Value));
			SourceWriter.BeginScope();

			// Check if the property is relevant on the client.
			SourceWriter.Printf("// %s", *SchemaFieldName(RepProp.Value));
			SourceWriter.Printf("uint16 Handle = %d;", Handle);
			SourceWriter.Print("const FRepHandleData* RepData = &HandleToPropertyMap[Handle];");
			SourceWriter.Print("if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))");

			SourceWriter.BeginScope();

			if (Property->IsA<UObjectPropertyBase>())
			{
				SourceWriter.Print("bool bWriteObjectProperty = true;");
			}

			// If the property is Role or RemoteRole, ensure to swap on the client.
			if (RepProp.Value->ReplicationData->RoleSwapHandle != -1)
			{
				SourceWriter.Printf(R"""(
					// On the client, we need to swap Role/RemoteRole.
					if (!bIsServer)
					{
						Handle = %d;
						RepData = &HandleToPropertyMap[Handle];
					})""", RepProp.Value->ReplicationData->RoleSwapHandle);
				SourceWriter.PrintNewLine();
			}

			// Convert update data to the corresponding Unreal type and serialize to OutputWriter.
			FString PropertyValueName = TEXT("Value");
			FString PropertyValueCppType = Property->GetCPPType();
			FString PropertyName = TEXT("RepData->Property");
			//todo-giray: The reinterpret_cast below is ugly and we believe we can do this more gracefully using Property helper functions.
			SourceWriter.Printf("uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));");
			if (Property->IsA<UBoolProperty>())
			{
				SourceWriter.Printf("bool %s = static_cast<UBoolProperty*>(%s)->GetPropertyValue(PropertyData);", *PropertyValueName, *PropertyName);
			}
			else if (Property->IsA<UArrayProperty>()) {
				UArrayProperty* ArrayProperty = Cast<UArrayProperty>(Property);
				SourceWriter.Printf("TArray<%s> %s = *(reinterpret_cast<TArray<%s> *>(PropertyData));", *(ArrayProperty->Inner->GetCPPType()), *PropertyValueName, *(ArrayProperty->Inner->GetCPPType()));
			}
			else
			{
				SourceWriter.Printf("%s %s = *(reinterpret_cast<%s const*>(PropertyData));", *PropertyValueCppType, *PropertyValueName, *PropertyValueCppType);
			}
			SourceWriter.PrintNewLine();

			FString SpatialValue = FString::Printf(TEXT("(*%s.%s().data())"), TEXT("Update"), *SchemaFieldName(RepProp.Value));

			GeneratePropertyToUnrealConversion(
				SourceWriter, SpatialValue, RepProp.Value->Property, PropertyValueName, true,
				[&SourceWriter](const FString& PropertyValue)
			{
				SourceWriter.Print(R"""(
					UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received unresolved object property. Value: %s. actor %s (%lld), property %s (handle %d)"),
						*Interop->GetSpatialOS()->GetWorkerId(),
						*ObjectRefToString(ObjectRef),
						*ActorChannel->Actor->GetName(),
						ActorChannel->GetEntityId().ToSpatialEntityId(),
						*RepData->Property->GetName(),
						Handle);)""");
				SourceWriter.Print("bWriteObjectProperty = false;");
				SourceWriter.Print("Interop->QueueIncomingObjectRepUpdate_Internal(ObjectRef, ActorChannel, RepData);");
			});

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
				UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
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

void GenerateFunction_ReceiveUpdate_MigratableData(FCodeWriter& SourceWriter, UClass* Class, const TMap<uint16, TSharedPtr<FUnrealProperty>>& MigratableData)
{
	FFunctionSignature ReceiveUpdateSignature{"void",
		FString::Printf(TEXT("ReceiveUpdate_Migratable(USpatialActorChannel* ActorChannel, const improbable::unreal::%s::Update& Update) const"),
			*SchemaMigratableDataName(Class))
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
				SourceWriter, SpatialValue, MigProp.Value->Property, PropertyValueName, true,
				[&SourceWriter](const FString& PropertyValue)
			{
				SourceWriter.Print(R"""(
					UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received unresolved object property. Value: %s. actor %s (%lld), property %s (handle %d)"),
						*Interop->GetSpatialOS()->GetWorkerId(),
						*ObjectRefToString(ObjectRef),
						*ActorChannel->Actor->GetName(),
						ActorChannel->GetEntityId().ToSpatialEntityId(),
						*MigratableData->Property->GetName(),
						Handle);)""");
				SourceWriter.Print("bWriteObjectProperty = false;");
				SourceWriter.Print("Interop->QueueIncomingObjectMigUpdate_Internal(ObjectRef, ActorChannel, MigratableData);");
			});

			SourceWriter.PrintNewLine();

			if (Property->IsA<UObjectPropertyBase>())
			{
				SourceWriter.Print("if (bWriteObjectProperty)");
				SourceWriter.BeginScope();
			}

			SourceWriter.Print("ApplyIncomingMigratablePropertyUpdate(*MigratableData, ActorChannel->Actor, static_cast<const void*>(&Value));");
			SourceWriter.PrintNewLine();

			SourceWriter.Print(R"""(
				UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received migratable property update. actor %s (%lld), property %s (handle %d)"),
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

void GenerateFunction_RPCSendCommand(FCodeWriter& SourceWriter, UClass* Class, const TSharedPtr<FUnrealRPC> RPC)
{
	FFunctionSignature SendCommandSignature{
		"void",
		FString::Printf(TEXT("%s_SendCommand(worker::Connection* const Connection, void* Params, UObject* TargetObject)"),
			*RPC->Function->GetName())
	};
	SourceWriter.BeginFunction(SendCommandSignature, TypeBindingName(Class));

	// Extract RPC arguments from the stack.
	if (RPC->Function->NumParms > 0)
	{
		UClass* RPCOwnerClass = RPC->Function->GetOwnerClass();
		// The name of the struct is consistent with the one in .generated.h
		FString ParametersStructName = FString::Printf(TEXT("%s_event%s_Parms"), *RPCOwnerClass->GetName(), *RPC->Function->GetName());
		if (RPCOwnerClass->ClassGeneratedBy)
		{
			// This RPC is generated from a blueprint class, so we need to generate the Params struct based on the RPC arguments
			GenerateRPCArgumentsStruct(SourceWriter, RPC, ParametersStructName);
			SourceWriter.PrintNewLine();
		}

		SourceWriter.Printf("%s StructuredParams = *static_cast<%s*>(Params);", *ParametersStructName, *ParametersStructName);
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
	SourceWriter.Print("{").Indent();

	SourceWriter.Printf(R"""(
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%%s: RPC %s queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return {TargetObject};
		})""", *RPC->Function->GetName());
	SourceWriter.PrintNewLine();
	SourceWriter.Print("// Build request.");
	SourceWriter.Printf("improbable::unreal::%s Request;", *SchemaRPCRequestType(RPC->Function));

	TArray<TSharedPtr<FUnrealProperty>> RPCParameters = GetFlatRPCParameters(RPC);
	for (auto Param : RPCParameters)
	{
		FString SpatialValueSetter = TEXT("Request.set_") + SchemaFieldName(Param);

		GenerateUnrealToSchemaConversion(
			SourceWriter, SpatialValueSetter, Param->Property, FString::Printf(TEXT("StructuredParams.%s"), *CPPFieldName(Param)), false,
			[&SourceWriter, &RPC](const FString& PropertyValue)
		{
			SourceWriter.Printf("UE_LOG(LogSpatialOSInterop, Log, TEXT(\"%%s: RPC %s queued. %s is unresolved.\"), *Interop->GetSpatialOS()->GetWorkerId());",
				*RPC->Function->GetName(),
				*PropertyValue);
			SourceWriter.Printf("return {%s};", *PropertyValue);
		});
	}

	SourceWriter.PrintNewLine();
	SourceWriter.Printf(R"""(
		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%%s: Sending RPC: %s, target: %%s %%s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			*ObjectRefToString(TargetObjectRef));
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::%s::Commands::%s>(TargetObjectRef.entity(), Request, 0);
		return {RequestId.Id};)""",
		*RPC->Function->GetName(),
		*SchemaRPCComponentName(RPC->Type, Class),
		*CPPCommandClassName(RPC->Function));
	SourceWriter.Outdent().Print("};");
	SourceWriter.Printf("Interop->SendCommandRequest_Internal(Sender, %s);", RPC->bReliable ? TEXT("/*bReliable*/ true") : TEXT("/*bReliable*/ false"));

	SourceWriter.End();
}

void GenerateFunction_RPCOnCommandRequest(FCodeWriter& SourceWriter, UClass* Class, const TSharedPtr<FUnrealRPC> RPC)
{
	FString RequestFuncName = FString::Printf(TEXT("%s_OnCommandRequest(const worker::CommandRequestOp<improbable::unreal::%s::Commands::%s>& Op)"),
		*RPC->Function->GetName(),
		*SchemaRPCComponentName(RPC->Type, Class),
		*CPPCommandClassName(RPC->Function));
	SourceWriter.BeginFunction({"void", RequestFuncName}, TypeBindingName(Class));

	// Generate receiver function.
	SourceWriter.Print("auto Receiver = [this, Op]() mutable -> FRPCCommandResponseResult");
	SourceWriter.Print("{").Indent();

	auto ObjectResolveFailureGenerator = [&SourceWriter, &RPC, Class](const FString& PropertyName, const FString& ObjectRef)
	{
		SourceWriter.Printf(R"""(
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%%s: %s_OnCommandRequest: %s %%s is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ObjectRefToString(%s));
			return {%s};)""",
			*RPC->Function->GetName(),
			*PropertyName,
			*ObjectRef,
			*ObjectRef,
			*ObjectRef);
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
		UObject* TargetObject = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		checkf(TargetObject, TEXT("%%s: %s_OnCommandRequest: Object Ref %%s (NetGUID %%s) does not correspond to a UObject."),
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

		UClass* RPCOwnerClass = RPC->Function->GetOwnerClass();
		// The name of the struct is consistent with the one in .generated.h
		FString ParametersStructName = FString::Printf(TEXT("%s_event%s_Parms"), *RPCOwnerClass->GetName(), *RPC->Function->GetName());
		if (RPCOwnerClass->ClassGeneratedBy)
		{
			// This RPC is generated from a blueprint class, so we need to generate the Params struct based on the RPC arguments
			GenerateRPCArgumentsStruct(SourceWriter, RPC, ParametersStructName);
			SourceWriter.PrintNewLine();
		}

		SourceWriter.Printf("%s Parameters;", *ParametersStructName);

		// Extract RPC arguments from request data.
		SourceWriter.PrintNewLine();
		SourceWriter.Print("// Extract from request data.");

		for (auto Param : GetFlatRPCParameters(RPC))
		{
			FString SpatialValue = FString::Printf(TEXT("%s.%s()"), TEXT("Op.Request"), *SchemaFieldName(Param));

			GeneratePropertyToUnrealConversion(
				SourceWriter, SpatialValue, Param->Property, FString::Printf(TEXT("Parameters.%s"), *CPPFieldName(Param)), false,
				std::bind(ObjectResolveFailureGenerator, std::placeholders::_1, "ObjectRef"));
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
				UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%%s: Received RPC: %s, target: %%s %%s"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*TargetObject->GetName(),
					*ObjectRefToString(TargetObjectRef));)""",
		*RPC->Function->GetName());

	SourceWriter.PrintNewLine();
	SourceWriter.Printf(R"""(
		UFunction* Function = TargetObject->FindFunction(FName(TEXT("%s")));
		if (Function)
		{
			TargetObject->ProcessEvent(Function, %s);
		}
		else
		{
			UE_LOG(LogSpatialOSInterop, Error, TEXT("%%s: %s_OnCommandRequest: Function not found. Object: %%s, Function: %s."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*TargetObject->GetFullName());
		})""",
		*RPC->Function->GetName(),
		*RPCParametersStruct,
		*RPC->Function->GetName(),
		*RPC->Function->GetName());
	SourceWriter.PrintNewLine();

	SourceWriter.Print("// Send command response.");
	SourceWriter.Print("TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();");
	SourceWriter.Printf("Connection->SendCommandResponse<improbable::unreal::%s::Commands::%s>(Op.RequestId, {});",
		*SchemaRPCComponentName(RPC->Type, Class),
		*CPPCommandClassName(RPC->Function));
	SourceWriter.Print("return {};");
	SourceWriter.Outdent().Print("};");

	SourceWriter.Print("Interop->SendCommandResponse_Internal(Receiver);");

	SourceWriter.End();
}

void GenerateFunction_RPCOnCommandResponse(FCodeWriter& SourceWriter, UClass* Class, const TSharedPtr<FUnrealRPC> RPC)
{
	FString ResponseFuncName = FString::Printf(TEXT("%s_OnCommandResponse(const worker::CommandResponseOp<improbable::unreal::%s::Commands::%s>& Op)"),
		*RPC->Function->GetName(),
		*SchemaRPCComponentName(RPC->Type, Class),
		*CPPCommandClassName(RPC->Function));

	SourceWriter.BeginFunction({"void", ResponseFuncName}, TypeBindingName(Class));
	SourceWriter.Printf("Interop->HandleCommandResponse_Internal(TEXT(\"%s\"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));",
		*RPC->Function->GetName());
	SourceWriter.End();
}
