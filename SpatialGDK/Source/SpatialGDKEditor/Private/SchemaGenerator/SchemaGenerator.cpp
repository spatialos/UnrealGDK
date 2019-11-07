// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SchemaGenerator.h"

#include "Algo/Reverse.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SCS_Node.h"
#include "UObject/TextProperty.h"

#include "Interop/SpatialClassInfoManager.h"
#include "SpatialGDKSettings.h"
#include "Utils/CodeWriter.h"
#include "Utils/ComponentIdGenerator.h"
#include "Utils/DataTypeUtilities.h"
#include "SpatialGDKEditorSchemaGenerator.h"

using namespace SpatialGDKEditor::Schema;

DEFINE_LOG_CATEGORY(LogSchemaGenerator);

ESchemaComponentType PropertyGroupToSchemaComponentType(EReplicatedPropertyGroup Group)
{
	if (Group == REP_MultiClient)
	{
		return SCHEMA_Data;
	}
	else if (Group == REP_SingleClient)
	{
		return SCHEMA_OwnerOnly;
	}
	else
	{
		checkNoEntry();
		return SCHEMA_Invalid;
	}
}

// Given a RepLayout cmd type (a data type supported by the replication system). Generates the corresponding
// type used in schema.
FString PropertyToSchemaType(UProperty* Property)
{
	FString DataType;

	if (Property->IsA(UStructProperty::StaticClass()))
	{
		UStructProperty* StructProp = Cast<UStructProperty>(Property);
		UScriptStruct* Struct = StructProp->Struct;
		DataType = TEXT("bytes");
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
		DataType = TEXT("int32");
	}
	else if (Property->IsA(UInt16Property::StaticClass()))
	{
		DataType = TEXT("int32");
	}
	else if (Property->IsA(UIntProperty::StaticClass()))
	{
		DataType = TEXT("int32");
	}
	else if (Property->IsA(UInt64Property::StaticClass()))
	{
		DataType = TEXT("int64");
	}
	else if (Property->IsA(UByteProperty::StaticClass()))
	{
		DataType = TEXT("uint32"); // uint8 not supported in schema.
	}
	else if (Property->IsA(UUInt16Property::StaticClass()))
	{
		DataType = TEXT("uint32");
	}
	else if (Property->IsA(UUInt32Property::StaticClass()))
	{
		DataType = TEXT("uint32");
	}
	else if (Property->IsA(UUInt64Property::StaticClass()))
	{
		DataType = TEXT("uint64");
	}
	else if (Property->IsA(UNameProperty::StaticClass()) || Property->IsA(UStrProperty::StaticClass()) || Property->IsA(UTextProperty::StaticClass()))
	{
		DataType = TEXT("string");
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
	else if (Property->IsA(UEnumProperty::StaticClass()))
	{
		DataType = GetEnumDataType(Cast<UEnumProperty>(Property));
	}
	else
	{
		DataType = TEXT("bytes");
	}

	return DataType;
}

void WriteSchemaRepField(FCodeWriter& Writer, const TSharedPtr<FUnrealProperty> RepProp, const int FieldCounter)
{
	Writer.Printf("{0} {1} = {2};",
		*PropertyToSchemaType(RepProp->Property),
		*SchemaFieldName(RepProp),
		FieldCounter
	);
}

void WriteSchemaHandoverField(FCodeWriter& Writer, const TSharedPtr<FUnrealProperty> HandoverProp, const int FieldCounter)
{
	Writer.Printf("{0} {1} = {2};",
		*PropertyToSchemaType(HandoverProp->Property),
		*SchemaFieldName(HandoverProp),
		FieldCounter
	);
}

void GenerateSubobjectSchema(FComponentIdGenerator& IdGenerator, UClass* Class, TSharedPtr<FUnrealType> TypeInfo, FString SchemaPath)
{
	FCodeWriter Writer;

	Writer.Printf(R"""(
		// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
		// Note that this file has been generated automatically
		package unreal.generated;)""");

	bool bShouldIncludeCoreTypes = false;

	// Only include core types if the subobject has replicated references to other UObjects
	FUnrealFlatRepData RepData = GetFlatRepData(TypeInfo);
	for (auto& PropertyGroup : RepData)
	{
		for (auto& PropertyPair : PropertyGroup.Value)
		{
			UProperty* Property = PropertyPair.Value->Property;
			if (Property->IsA<UObjectPropertyBase>())
			{
				bShouldIncludeCoreTypes = true;
			}

			if (Property->IsA<UArrayProperty>())
			{
				if (Cast<UArrayProperty>(Property)->Inner->IsA<UObjectPropertyBase>())
				{
					bShouldIncludeCoreTypes = true;
				}
			}
		}
	}

	if (bShouldIncludeCoreTypes)
	{
		Writer.PrintNewLine();
		Writer.Printf("import \"unreal/gdk/core_types.schema\";");
	}

	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		// Since it is possible to replicate subobjects which have no replicated properties.
		// We need to generate a schema component for every subobject. So if we have no replicated
		// properties, we only don't generate a schema component if we are REP_SingleClient
		if (RepData[Group].Num() == 0 && Group == REP_SingleClient)
		{
			continue;
		}

		// If this class is an Actor Component, it MUST have bReplicates at field ID 1.
		if (Group == REP_MultiClient && Class->IsChildOf<UActorComponent>())
		{
			TSharedPtr<FUnrealProperty> ExpectedReplicatesPropData = RepData[Group].FindRef(SpatialConstants::ACTOR_COMPONENT_REPLICATES_ID);
			const UProperty* ReplicatesProp = UActorComponent::StaticClass()->FindPropertyByName("bReplicates");

			if (!(ExpectedReplicatesPropData.IsValid() && ExpectedReplicatesPropData->Property == ReplicatesProp))
			{
				UE_LOG(LogSchemaGenerator, Error, TEXT("Did not find ActorComponent->bReplicates at field %d for class %s. Modifying the base Actor Component class is currently not supported."),
					SpatialConstants::ACTOR_COMPONENT_REPLICATES_ID,
					*Class->GetName());
			}
		}

		Writer.PrintNewLine();
		Writer.Printf("type {0} {", *SchemaReplicatedDataName(Group, Class));
		Writer.Indent();
		for (auto& RepProp : RepData[Group])
		{
			WriteSchemaRepField(Writer,
				RepProp.Value,
				RepProp.Value->ReplicationData->Handle);
		}
		Writer.Outdent().Print("}");
	}

	FCmdHandlePropertyMap HandoverData = GetFlatHandoverData(TypeInfo);
	if (HandoverData.Num() > 0)
	{
		Writer.PrintNewLine();

		Writer.Printf("type {0} {", *SchemaHandoverDataName(Class));
		Writer.Indent();
		int FieldCounter = 0;
		for (auto& Prop : HandoverData)
		{
			FieldCounter++;
			WriteSchemaHandoverField(Writer,
				Prop.Value,
				FieldCounter);
		}
		Writer.Outdent().Print("}");
	}

	// Use the max number of dynamically attached subobjects per class to generate
	// that many schema components for this subobject.
	const uint32 DynamicComponentsPerClass = GetDefault<USpatialGDKSettings>()->MaxDynamicallyAttachedSubobjectsPerClass;

	FSubobjectSchemaData SubobjectSchemaData;

	// Use previously generated component IDs when possible.
	const FSubobjectSchemaData* const ExistingSchemaData = SubobjectClassPathToSchema.Find(Class->GetPathName());
	if (ExistingSchemaData != nullptr && !ExistingSchemaData->GeneratedSchemaName.IsEmpty()
		&& ExistingSchemaData->GeneratedSchemaName != ClassPathToSchemaName[Class->GetPathName()])
	{
		UE_LOG(LogSchemaGenerator, Error, TEXT("Saved generated schema name does not match in-memory version for class %s - schema %s : %s"),
			*Class->GetPathName(), *ExistingSchemaData->GeneratedSchemaName, *ClassPathToSchemaName[Class->GetPathName()]);
		UE_LOG(LogSchemaGenerator, Error, TEXT("Schema generation may have resulted in component name clash, recommend you perform a full schema generation"));
	}

	for (uint32 i = 1; i <= DynamicComponentsPerClass; i++)
	{
		FDynamicSubobjectSchemaData DynamicSubobjectComponents;

		for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
		{
			// Since it is possible to replicate subobjects which have no replicated properties.
			// We need to generate a schema component for every subobject. So if we have no replicated
			// properties, we only don't generate a schema component if we are REP_SingleClient
			if (RepData[Group].Num() == 0 && Group == REP_SingleClient)
			{
				continue;
			}

			Writer.PrintNewLine();

			Worker_ComponentId ComponentId = 0;
			if (ExistingSchemaData != nullptr)
			{
				ComponentId = ExistingSchemaData->GetDynamicSubobjectComponentId(i - 1, PropertyGroupToSchemaComponentType(Group));
			}

			if (ComponentId == 0)
			{
				ComponentId = IdGenerator.Next();
			}
			FString ComponentName = SchemaReplicatedDataName(Group, Class) + TEXT("Dynamic") + FString::FromInt(i);

			Writer.Printf("component {0} {", *ComponentName);
			Writer.Indent();
			Writer.Printf("id = {0};", ComponentId);
			Writer.Printf("data {0};", *SchemaReplicatedDataName(Group, Class));
			Writer.Outdent().Print("}");

			DynamicSubobjectComponents.SchemaComponents[PropertyGroupToSchemaComponentType(Group)] = ComponentId;
		}

		if (HandoverData.Num() > 0)
		{
			Writer.PrintNewLine();

			Worker_ComponentId ComponentId = 0;
			if (ExistingSchemaData != nullptr)
			{
				ComponentId = ExistingSchemaData->GetDynamicSubobjectComponentId(i - 1, SCHEMA_Handover);
			}

			if (ComponentId == 0)
			{
				ComponentId = IdGenerator.Next();
			}
			FString ComponentName = SchemaHandoverDataName(Class) + TEXT("Dynamic") + FString::FromInt(i);

			Writer.Printf("component {0} {", *ComponentName);
			Writer.Indent();
			Writer.Printf("id = {0};", ComponentId);
			Writer.Printf("data {0};", *SchemaHandoverDataName(Class));
			Writer.Outdent().Print("}");

			DynamicSubobjectComponents.SchemaComponents[SCHEMA_Handover] = ComponentId;
		}

		SubobjectSchemaData.DynamicSubobjectComponents.Add(MoveTemp(DynamicSubobjectComponents));
	}

	Writer.WriteToFile(FString::Printf(TEXT("%s%s.schema"), *SchemaPath, *ClassPathToSchemaName[Class->GetPathName()]));
	SubobjectSchemaData.GeneratedSchemaName = ClassPathToSchemaName[Class->GetPathName()];
	SubobjectClassPathToSchema.Add(Class->GetPathName(), SubobjectSchemaData);
}

void GenerateActorSchema(FComponentIdGenerator& IdGenerator, UClass* Class, TSharedPtr<FUnrealType> TypeInfo, FString SchemaPath)
{
	const FActorSchemaData* const SchemaData = ActorClassPathToSchema.Find(Class->GetPathName());

	FCodeWriter Writer;

	Writer.Printf(R"""(
		// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
		// Note that this file has been generated automatically
		package unreal.generated.{0};)""",
		*ClassPathToSchemaName[Class->GetPathName()].ToLower());

	// Will always be included since AActor has replicated pointers to other actors
	Writer.PrintNewLine();
	Writer.Printf("import \"unreal/gdk/core_types.schema\";");

	FActorSchemaData ActorSchemaData;
	ActorSchemaData.GeneratedSchemaName = ClassPathToSchemaName[Class->GetPathName()];

	FUnrealFlatRepData RepData = GetFlatRepData(TypeInfo);

	// Client-server replicated properties.
	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		if (RepData[Group].Num() == 0)
		{
			continue;
		}

		// If this class is an Actor, it MUST have bTearOff at field ID 3.	
		if (Group == REP_MultiClient && Class->IsChildOf<AActor>())
		{
			TSharedPtr<FUnrealProperty> ExpectedReplicatesPropData = RepData[Group].FindRef(SpatialConstants::ACTOR_TEAROFF_ID);
			const UProperty* ReplicatesProp = AActor::StaticClass()->FindPropertyByName("bTearOff");

			if (!(ExpectedReplicatesPropData.IsValid() && ExpectedReplicatesPropData->Property == ReplicatesProp))
			{
				UE_LOG(LogSchemaGenerator, Error, TEXT("Did not find Actor->bTearOff at field %d for class %s. Modifying the base Actor class is currently not supported."),
					SpatialConstants::ACTOR_TEAROFF_ID,
					*Class->GetName());
			}
		}

		Worker_ComponentId ComponentId = 0;
		if (SchemaData != nullptr && SchemaData->SchemaComponents[PropertyGroupToSchemaComponentType(Group)] != 0)
		{
			ComponentId = SchemaData->SchemaComponents[PropertyGroupToSchemaComponentType(Group)];
		}
		else
		{
			ComponentId = IdGenerator.Next();
		}

		Writer.PrintNewLine();

		Writer.Printf("component {0} {", *SchemaReplicatedDataName(Group, Class));
		Writer.Indent();
		Writer.Printf("id = {0};", ComponentId);

		ActorSchemaData.SchemaComponents[PropertyGroupToSchemaComponentType(Group)] = ComponentId;

		int FieldCounter = 0;
		for (auto& RepProp : RepData[Group])
		{
			FieldCounter++;
			WriteSchemaRepField(Writer,
				RepProp.Value,
				RepProp.Value->ReplicationData->Handle);
		}

		Writer.Outdent().Print("}");
	}

	FCmdHandlePropertyMap HandoverData = GetFlatHandoverData(TypeInfo);
	if (HandoverData.Num() > 0)
	{
		Worker_ComponentId ComponentId = 0;
		if (SchemaData != nullptr && SchemaData->SchemaComponents[ESchemaComponentType::SCHEMA_Handover] != 0)
		{
			ComponentId = SchemaData->SchemaComponents[ESchemaComponentType::SCHEMA_Handover];
		}
		else
		{
			ComponentId = IdGenerator.Next();
		}

		Writer.PrintNewLine();

		// Handover (server to server) replicated properties.
		Writer.Printf("component {0} {", *SchemaHandoverDataName(Class));
		Writer.Indent();
		Writer.Printf("id = {0};", ComponentId);

		ActorSchemaData.SchemaComponents[ESchemaComponentType::SCHEMA_Handover] = ComponentId;

		int FieldCounter = 0;
		for (auto& Prop : HandoverData)
		{
			FieldCounter++;
			WriteSchemaHandoverField(Writer,
				Prop.Value,
				FieldCounter);
		}
		Writer.Outdent().Print("}");
	}

	GenerateSubobjectSchemaForActor(IdGenerator, Class, TypeInfo, SchemaPath, ActorSchemaData, ActorClassPathToSchema.Find(Class->GetPathName()));

	ActorClassPathToSchema.Add(Class->GetPathName(), ActorSchemaData);

	Writer.WriteToFile(FString::Printf(TEXT("%s%s.schema"), *SchemaPath, *ClassPathToSchemaName[Class->GetPathName()]));
}

FActorSpecificSubobjectSchemaData GenerateSchemaForStaticallyAttachedSubobject(FCodeWriter& Writer, FComponentIdGenerator& IdGenerator, FString PropertyName, TSharedPtr<FUnrealType>& TypeInfo, UClass* ComponentClass, UClass* ActorClass, int MapIndex, const FActorSpecificSubobjectSchemaData* ExistingSchemaData)
{
	FUnrealFlatRepData RepData = GetFlatRepData(TypeInfo);

	FActorSpecificSubobjectSchemaData SubobjectData;
	SubobjectData.ClassPath = ComponentClass->GetPathName();

	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		// Since it is possible to replicate subobjects which have no replicated properties.
		// We need to generate a schema component for every subobject. So if we have no replicated
		// properties, we only don't generate a schema component if we are REP_SingleClient
		if (RepData[Group].Num() == 0 && Group == REP_SingleClient)
		{
			continue;
		}

		Worker_ComponentId ComponentId = 0;
		if (ExistingSchemaData != nullptr && ExistingSchemaData->SchemaComponents[PropertyGroupToSchemaComponentType(Group)] != 0)
		{
			ComponentId = ExistingSchemaData->SchemaComponents[PropertyGroupToSchemaComponentType(Group)];
		}
		else
		{
			ComponentId = IdGenerator.Next();
		}

		Writer.PrintNewLine();

		FString ComponentName = PropertyName + GetReplicatedPropertyGroupName(Group);
		Writer.Printf("component {0} {", *ComponentName);
		Writer.Indent();
		Writer.Printf("id = {0};", ComponentId);
		Writer.Printf("data unreal.generated.{0};", *SchemaReplicatedDataName(Group, ComponentClass));
		Writer.Outdent().Print("}");

		SubobjectData.SchemaComponents[PropertyGroupToSchemaComponentType(Group)] = ComponentId;
	}

	FCmdHandlePropertyMap HandoverData = GetFlatHandoverData(TypeInfo);
	if (HandoverData.Num() > 0)
	{
		Worker_ComponentId ComponentId = 0;
		if (ExistingSchemaData != nullptr && ExistingSchemaData->SchemaComponents[ESchemaComponentType::SCHEMA_Handover] != 0)
		{
			ComponentId = ExistingSchemaData->SchemaComponents[ESchemaComponentType::SCHEMA_Handover];
		}
		else
		{
			ComponentId = IdGenerator.Next();
		}

		Writer.PrintNewLine();

		// Handover (server to server) replicated properties.
		Writer.Printf("component {0} {", *(PropertyName + TEXT("Handover")));
		Writer.Indent();
		Writer.Printf("id = {0};", ComponentId);
		Writer.Printf("data unreal.generated.{0};", *SchemaHandoverDataName(ComponentClass));
		Writer.Outdent().Print("}");

		SubobjectData.SchemaComponents[ESchemaComponentType::SCHEMA_Handover] = ComponentId;
	}

	return SubobjectData;
}

void GenerateSubobjectSchemaForActor(FComponentIdGenerator& IdGenerator, UClass* ActorClass, TSharedPtr<FUnrealType> TypeInfo, FString SchemaPath, FActorSchemaData& ActorSchemaData, const FActorSchemaData* ExistingSchemaData)
{
	FCodeWriter Writer;

	Writer.Printf(R"""(
		// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
		// Note that this file has been generated automatically
		package unreal.generated.{0}.subobjects;)""",
		*ClassPathToSchemaName[ActorClass->GetPathName()].ToLower());

	Writer.PrintNewLine();

	GenerateSubobjectSchemaForActorIncludes(Writer, TypeInfo);

	FSubobjectMap Subobjects = GetAllSubobjects(TypeInfo);

	bool bHasComponents = false;

	for (auto& It : Subobjects)
	{
		TSharedPtr<FUnrealType>& SubobjectTypeInfo = It.Value;
		UClass* SubobjectClass = Cast<UClass>(SubobjectTypeInfo->Type);

		FActorSpecificSubobjectSchemaData SubobjectData;

		if (SchemaGeneratedClasses.Contains(SubobjectClass))
		{
			bHasComponents = true;

			const FActorSpecificSubobjectSchemaData* ExistingSubobjectSchemaData = nullptr;
			if (ExistingSchemaData != nullptr)
			{
				for (auto& SubobjectIt : ExistingSchemaData->SubobjectData)
				{
					if (SubobjectIt.Value.Name == SubobjectTypeInfo->Name)
					{
						ExistingSubobjectSchemaData = &SubobjectIt.Value;
						break;
					}
				}
			}
			SubobjectData = GenerateSchemaForStaticallyAttachedSubobject(Writer, IdGenerator, UnrealNameToSchemaComponentName(SubobjectTypeInfo->Name.ToString()), SubobjectTypeInfo, SubobjectClass, ActorClass, 0, ExistingSubobjectSchemaData);
		}
		else
		{
			continue;
		}

		SubobjectData.Name = SubobjectTypeInfo->Name;
		uint32 SubobjectOffset = SubobjectData.SchemaComponents[SCHEMA_Data];
		check(SubobjectOffset != 0);
		ActorSchemaData.SubobjectData.Add(SubobjectOffset, SubobjectData);
	}

	if (bHasComponents)
	{
		Writer.WriteToFile(FString::Printf(TEXT("%s%sComponents.schema"), *SchemaPath, *ClassPathToSchemaName[ActorClass->GetPathName()]));
	}
}

void GenerateSubobjectSchemaForActorIncludes(FCodeWriter& Writer, TSharedPtr<FUnrealType>& TypeInfo)
{
	TSet<UStruct*> AlreadyImported;

	for (auto& PropertyPair : TypeInfo->Properties)
	{
		UProperty* Property = PropertyPair.Key;
		UObjectProperty* ObjectProperty = Cast<UObjectProperty>(Property);

		TSharedPtr<FUnrealType>& PropertyTypeInfo = PropertyPair.Value->Type;

		if (ObjectProperty && PropertyTypeInfo.IsValid())
		{
			UObject* Value = PropertyTypeInfo->Object;

			if (Value != nullptr && IsSupportedClass(Value->GetClass()))
			{
				UClass* Class = Value->GetClass();
				if (!AlreadyImported.Contains(Class) && SchemaGeneratedClasses.Contains(Class))
				{
					Writer.Printf("import \"unreal/generated/Subobjects/{0}.schema\";", *ClassPathToSchemaName[Class->GetPathName()]);
					AlreadyImported.Add(Class);
				}
			}
		}
	}
}

FString GetRPCFieldPrefix(ESchemaComponentType RPCType)
{
	switch (RPCType)
	{
	case SCHEMA_ClientReliableRPC:
		return TEXT("server_to_client_reliable");
	case SCHEMA_ClientUnreliableRPC:
		return TEXT("server_to_client_unreliable");
	case SCHEMA_ServerReliableRPC:
		return TEXT("client_to_server_reliable");
	case SCHEMA_ServerUnreliableRPC:
		return TEXT("client_to_server_unreliable");
	case SCHEMA_NetMulticastRPC:
		return TEXT("multicast");
	default:
		checkNoEntry();
	}

	return FString();
}

void GenerateRPCEndpoint(FCodeWriter& Writer, FString EndpointName, Worker_ComponentId ComponentId, TArray<ESchemaComponentType> SentRPCTypes, TArray<ESchemaComponentType> AckedRPCTypes)
{
	Writer.PrintNewLine();
	Writer.Printf("component Unreal{0}RPCEndpoint {", *EndpointName).Indent();
	Writer.Printf("id = {0};", ComponentId);

	Schema_FieldId FieldId = 1;
	for (ESchemaComponentType SentRPCType : SentRPCTypes)
	{
		uint32 RingBufferSize = GetDefault<USpatialGDKSettings>()->GetRPCRingBufferSize(SentRPCType);

		for (uint32 i = 1; i <= RingBufferSize; i++)
		{
			Writer.Printf("option<UnrealRPCPayload> {0}_rpc_{1} = {2};", GetRPCFieldPrefix(SentRPCType), i, FieldId++);
		}
		Writer.Printf("uint64 last_sent_{0}_rpc = {1};", GetRPCFieldPrefix(SentRPCType), FieldId++);

		// TODO: Check that this matches expected field IDs.
	}

	for (ESchemaComponentType AckedRPCType : AckedRPCTypes)
	{
		Writer.Printf("uint64 last_executed_{0}_rpc = {1};", GetRPCFieldPrefix(AckedRPCType), FieldId++);
	}

	if (ComponentId == SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID_RB)
	{
		// CrossServer RPC uses commands, only exists on ServerRPCEndpoint
		Writer.Print("command Void server_to_server_rpc_command(UnrealRPCPayload);");
	}
	else if (ComponentId == SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID_RB)
	{
		// Indicate last executed multicast RPC to avoid executing them multiple times.
		Writer.Printf("uint64 last_executed_multicast_rpc = {0};", FieldId++);
	}

	Writer.Outdent().Print("}");
}

void GenerateSchemaForRPCEndpoints(FString SchemaPath)
{
	FCodeWriter Writer;

	Writer.Print(R"""(
		// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
		// Note that this file has been generated automatically
		package unreal.generated;)""");
	Writer.PrintNewLine();
	Writer.Print("import \"unreal/gdk/core_types.schema\";");
	Writer.Print("import \"unreal/gdk/rpc_components.schema\";");
	//Writer.Print("import \"unreal/gdk/rpc_payload.schema\";");

	GenerateRPCEndpoint(Writer, TEXT("Client"), SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID_RB, { SCHEMA_ServerReliableRPC, SCHEMA_ServerUnreliableRPC }, { SCHEMA_ClientReliableRPC, SCHEMA_ClientUnreliableRPC });
	GenerateRPCEndpoint(Writer, TEXT("Server"), SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID_RB, { SCHEMA_ClientReliableRPC, SCHEMA_ClientUnreliableRPC }, { SCHEMA_ServerReliableRPC, SCHEMA_ServerUnreliableRPC });
	GenerateRPCEndpoint(Writer, TEXT("Multicast"), SpatialConstants::NETMULTICAST_RPCS_COMPONENT_ID_RB, { SCHEMA_NetMulticastRPC }, {});

	Writer.WriteToFile(FString::Printf(TEXT("%srpc_endpoints.schema"), *SchemaPath));
}
