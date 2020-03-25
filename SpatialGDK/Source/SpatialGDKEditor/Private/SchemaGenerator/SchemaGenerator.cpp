// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SchemaGenerator.h"

#include "Algo/Reverse.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SCS_Node.h"
#include "UObject/TextProperty.h"

#include "Interop/SpatialClassInfoManager.h"
#include "SpatialGDKEditorSchemaGenerator.h"
#include "SpatialGDKSettings.h"
#include "Utils/CodeWriter.h"
#include "Utils/ComponentIdGenerator.h"
#include "Utils/DataTypeUtilities.h"
#include "Utils/GDKPropertyMacros.h"

using namespace SpatialGDKEditor::Schema;

DEFINE_LOG_CATEGORY(LogSchemaGenerator);

SchemaGeneratorData::SchemaGeneratorData()
{
	ForAllSchemaComponentTypes([&](ESchemaComponentType Type) {
		SchemaComponentTypeToComponents.Add(Type, TSet<Worker_ComponentId>());
	});

	NextAvailableComponentId = SpatialConstants::STARTING_GENERATED_COMPONENT_ID;
}

namespace
{
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
FString PropertyToSchemaType(const FUnrealOfflineProperty& Property)
{
	return Property.SchemaType;
}

void WriteSchemaRepField(FCodeWriter& Writer, const TSharedPtr<FUnrealOfflineProperty> RepProp, const int FieldCounter)
{
	Writer.Printf("{0} {1} = {2};", *PropertyToSchemaType(*RepProp), *SchemaFieldName(RepProp), FieldCounter);
}

void WriteSchemaHandoverField(FCodeWriter& Writer, const TSharedPtr<FUnrealOfflineProperty> HandoverProp, const int FieldCounter)
{
	Writer.Printf("{0} {1} = {2};", *PropertyToSchemaType(*HandoverProp), *SchemaFieldName(HandoverProp), FieldCounter);
}

FString GetRPCFieldPrefix(ERPCType RPCType)
{
	switch (RPCType)
	{
	case ERPCType::ClientReliable:
		return TEXT("server_to_client_reliable");
	case ERPCType::ClientUnreliable:
		return TEXT("server_to_client_unreliable");
	case ERPCType::ServerReliable:
		return TEXT("client_to_server_reliable");
	case ERPCType::ServerUnreliable:
		return TEXT("client_to_server_unreliable");
	case ERPCType::NetMulticast:
		return TEXT("multicast");
	default:
		checkNoEntry();
	}

	return FString();
}
} // anonymous namespace

// Generates schema for a statically attached subobject on an Actor.
FActorSpecificSubobjectSchemaData SchemaGenerator::GenerateSchemaForStaticallyAttachedSubobject(
	FCodeWriter& Writer, FComponentIdGenerator& IdGenerator, FString PropertyName, TSharedPtr<FUnrealOfflineType>& TypeInfo,
	FString const& ComponentClassPath, FString const& ActorClassPath, int MapIndex,
	const FActorSpecificSubobjectSchemaData* ExistingSchemaData)
{
	FUnrealFlatRepData RepData = GetFlatRepData(TypeInfo);

	FActorSpecificSubobjectSchemaData SubobjectData;
	SubobjectData.ClassPath = ComponentClassPath;

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
		Writer.Printf("data unreal.generated.{0};", *SchemaReplicatedDataName(Data.ClassPathToSchemaName, Group, ComponentClassPath));
		Writer.Outdent().Print("}");

		AddComponentId(ComponentId, SubobjectData.SchemaComponents, PropertyGroupToSchemaComponentType(Group));
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
		FString ComponentName = PropertyName + TEXT("Handover");
		Writer.Printf("component {0} {", *ComponentName);
		Writer.Indent();
		Writer.Printf("id = {0};", ComponentId);
		Writer.Printf("data unreal.generated.{0};", *SchemaHandoverDataName(Data.ClassPathToSchemaName, ComponentClassPath));
		Writer.Outdent().Print("}");

		AddComponentId(ComponentId, SubobjectData.SchemaComponents, ESchemaComponentType::SCHEMA_Handover);
	}

	return SubobjectData;
}

// Output the includes required by this schema file.
void SchemaGenerator::GenerateSubobjectSchemaForActorIncludes(FCodeWriter& Writer, TSharedPtr<FUnrealOfflineType>& TypeInfo)
{
	// TSet<UStruct*> AlreadyImported;
	TSet<FString> AlreadyImported;

	for (auto& PropertyPair : TypeInfo->Properties)
	{
		// GDK_PROPERTY(Property)* Property = PropertyPair.Key;
		const TSharedPtr<FUnrealOfflineProperty>& Property = PropertyPair.Value;
		// GDK_PROPERTY(ObjectProperty)* ObjectProperty = GDK_CASTFIELD<GDK_PROPERTY(ObjectProperty)>(Property);

		TSharedPtr<FUnrealOfflineType>& PropertyTypeInfo = PropertyPair.Value->Type;

		if (Property->bIsObjectProperty && PropertyTypeInfo.IsValid())
		{
			// UObject* Value = PropertyTypeInfo->Object;
			//
			// if (Value != nullptr && IsSupportedClass(Value->GetClass()))
			{
				// UClass* Class = Value->GetClass();
				if (!AlreadyImported.Contains(PropertyTypeInfo->TypeName)
					&& Data.SchemaGeneratedClasses.Contains(PropertyTypeInfo->TypeName))
				{
					Writer.Printf("import \"unreal/generated/Subobjects/{0}.schema\";",
								  *Data.ClassPathToSchemaName[PropertyTypeInfo->TypePath]);
					AlreadyImported.Add(PropertyTypeInfo->TypeName);
				}
			}
		}
	}
}

// Generates schema for all statically attached subobjects on an Actor.
void SchemaGenerator::GenerateSubobjectSchemaForActor(FComponentIdGenerator& IdGenerator, FString const& ActorClassPath,
													  TSharedPtr<FUnrealOfflineType> TypeInfo, FCodeWriter& Writer,
													  FActorSchemaData& ActorSchemaData, const FActorSchemaData* ExistingSchemaData)
{
	GenerateSubobjectSchemaForActorIncludes(Writer, TypeInfo);

	FSubobjectMap Subobjects = GetAllSubobjects(TypeInfo);

	bool bHasComponents = false;

	for (auto& It : Subobjects)
	{
		TSharedPtr<FUnrealOfflineType>& SubobjectTypeInfo = It.Value;
		// UClass* SubobjectClass = Cast<UClass>(SubobjectTypeInfo->Type);
		const FString& SubobjectClassName = SubobjectTypeInfo->TypeName;
		const FString& SubobjectClassPath = SubobjectTypeInfo->TypePath;

		FActorSpecificSubobjectSchemaData SubobjectData;

		if (Data.SchemaGeneratedClasses.Contains(SubobjectClassName))
		{
			if (!bHasComponents)
			{
				Writer.Printf(R"""(
					// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
					// Note that this file has been generated automatically
					package unreal.generated.{0}.subobjects;)""",
							  *Data.ClassPathToSchemaName[ActorClassPath].ToLower());

				Writer.PrintNewLine();
			}
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
			SubobjectData = GenerateSchemaForStaticallyAttachedSubobject(
				Writer, IdGenerator, UnrealNameToSchemaComponentName(SubobjectTypeInfo->Name.ToString()), SubobjectTypeInfo,
				SubobjectClassPath, ActorClassPath, 0, ExistingSubobjectSchemaData);
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
}

void SchemaGenerator::GenerateRPCEndpoint(FCodeWriter& Writer, FString EndpointName, Worker_ComponentId ComponentId,
										  TArray<ERPCType> SentRPCTypes, TArray<ERPCType> AckedRPCTypes)
{
	FString ComponentName = TEXT("Unreal") + EndpointName;
	Writer.PrintNewLine();
	Writer.Printf("component {0} {", *ComponentName).Indent();
	Writer.Printf("id = {0};", ComponentId);

	Schema_FieldId FieldId = 1;
	for (ERPCType SentRPCType : SentRPCTypes)
	{
		uint32 RingBufferSize = GetDefault<USpatialGDKSettings>()->MaxRPCRingBufferSize;

		for (uint32 RingBufferIndex = 0; RingBufferIndex < RingBufferSize; RingBufferIndex++)
		{
			Writer.Printf("option<UnrealRPCPayload> {0}_rpc_{1} = {2};", GetRPCFieldPrefix(SentRPCType), RingBufferIndex, FieldId++);
		}
		Writer.Printf("uint64 last_sent_{0}_rpc_id = {1};", GetRPCFieldPrefix(SentRPCType), FieldId++);
	}

	for (ERPCType AckedRPCType : AckedRPCTypes)
	{
		Writer.Printf("uint64 last_acked_{0}_rpc_id = {1};", GetRPCFieldPrefix(AckedRPCType), FieldId++);
	}

	if (ComponentId == SpatialConstants::MULTICAST_RPCS_COMPONENT_ID)
	{
		// This counter is used to let clients execute initial multicast RPCs when entity is just getting created,
		// while ignoring existing multicast RPCs when an entity enters the interest range.
		Writer.Printf("uint32 initially_present_multicast_rpc_count = {0};", FieldId++);
	}

	Writer.Outdent().Print("}");
}

void SchemaGenerator::GenerateSubobjectSchema(FComponentIdGenerator& IdGenerator, const FUnrealClassDesc& Class,
											  TSharedPtr<FUnrealOfflineType> TypeInfo, FString SchemaPath)
{
	FCodeWriter Writer;

	GenerateSubobjectSchema(IdGenerator, Class, TypeInfo, Writer);

	Writer.WriteToFile(FString::Printf(TEXT("%s%s.schema"), *SchemaPath, *Data.ClassPathToSchemaName[Class.ClassPath]));
}

void SchemaGenerator::GenerateSubobjectSchema(FComponentIdGenerator& IdGenerator, const FUnrealClassDesc& Class,
											  TSharedPtr<FUnrealOfflineType> TypeInfo, FCodeWriter& Writer)
{
	Writer.Printf(R"""(
		// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
		// Note that this file has been generated automatically
		package unreal.generated;)""");

	bool bShouldIncludeCoreTypes = false;

	const FString& ClassPath = Class.ClassPath;

	// Only include core types if the subobject has replicated references to other UObjects
	FUnrealFlatRepData RepData = GetFlatRepData(TypeInfo);
	for (auto& PropertyGroup : RepData)
	{
		for (auto& PropertyPair : PropertyGroup.Value)
		{
			// GDK_PROPERTY(Property)* Property = PropertyPair.Value->Property;
			const TSharedPtr<FUnrealOfflineProperty>& Property = PropertyPair.Value;
			if (Property->bIsObjectProperty)
			{
				bShouldIncludeCoreTypes = true;
			}

			// if (Property->bIsArrayProperty)
			//{
			//	if (Property->bIsObjectProperty)
			//	{
			//		bShouldIncludeCoreTypes = true;
			//	}
			//}
		}
	}

	// Also check the HandoverData
	FCmdHandlePropertyMap HandoverData = GetFlatHandoverData(TypeInfo);
	for (auto& PropertyPair : HandoverData)
	{
		const TSharedPtr<FUnrealOfflineProperty>& Property = PropertyPair.Value;
		if (Property->bIsObjectProperty)
		{
			bShouldIncludeCoreTypes = true;
		}

		// if (Property->IsA<GDK_PROPERTY(ArrayProperty)>())
		//{
		//	if (GDK_CASTFIELD<GDK_PROPERTY(ArrayProperty)>(Property)->Inner->IsA<GDK_PROPERTY(ObjectPropertyBase)>())
		//	{
		//		bShouldIncludeCoreTypes = true;
		//	}
		//}
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
		if (Group == REP_MultiClient && Class.bIsActorComponent)
		{
			TSharedPtr<FUnrealOfflineProperty> ExpectedReplicatesPropData =
				RepData[Group].FindRef(SpatialConstants::ACTOR_COMPONENT_REPLICATES_ID);
			// const GDK_PROPERTY(Property)* ReplicatesProp = UActorComponent::StaticClass()->FindPropertyByName("bReplicates");

			if (!(ExpectedReplicatesPropData.IsValid() && ExpectedReplicatesPropData->PropertyName == TEXT("bReplicates")))
			{
				UE_LOG(LogSchemaGenerator, Error,
					   TEXT("Did not find ActorComponent->bReplicates at field %d for class %s. Modifying the base Actor Component class "
							"is currently not supported."),
					   SpatialConstants::ACTOR_COMPONENT_REPLICATES_ID, *Class.ClassName);
			}
		}

		Writer.PrintNewLine();
		Writer.Printf("type {0} {", *SchemaReplicatedDataName(Data.ClassPathToSchemaName, Group, ClassPath));
		Writer.Indent();
		for (auto& RepProp : RepData[Group])
		{
			WriteSchemaRepField(Writer, RepProp.Value, RepProp.Value->ReplicationData->Handle);
		}
		Writer.Outdent().Print("}");
	}

	if (HandoverData.Num() > 0)
	{
		Writer.PrintNewLine();

		Writer.Printf("type {0} {", *SchemaHandoverDataName(Data.ClassPathToSchemaName, ClassPath));
		Writer.Indent();
		int FieldCounter = 0;
		for (auto& Prop : HandoverData)
		{
			FieldCounter++;
			WriteSchemaHandoverField(Writer, Prop.Value, FieldCounter);
		}
		Writer.Outdent().Print("}");
	}

	// Use the max number of dynamically attached subobjects per class to generate
	// that many schema components for this subobject.
	const uint32 DynamicComponentsPerClass = GetDefault<USpatialGDKSettings>()->MaxDynamicallyAttachedSubobjectsPerClass;

	FSubobjectSchemaData SubobjectSchemaData;

	// Use previously generated component IDs when possible.
	const FSubobjectSchemaData* const ExistingSchemaData = Data.SubobjectClassPathToSchema.Find(ClassPath);
	if (ExistingSchemaData != nullptr && !ExistingSchemaData->GeneratedSchemaName.IsEmpty()
		&& ExistingSchemaData->GeneratedSchemaName != Data.ClassPathToSchemaName[ClassPath])
	{
		UE_LOG(LogSchemaGenerator, Error,
			   TEXT("Saved generated schema name does not match in-memory version for class %s - schema %s : %s"), *ClassPath,
			   *ExistingSchemaData->GeneratedSchemaName, *Data.ClassPathToSchemaName[ClassPath]);
		UE_LOG(LogSchemaGenerator, Error,
			   TEXT("Schema generation may have resulted in component name clash, recommend you perform a full schema generation"));
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
			FString ComponentName =
				SchemaReplicatedDataName(Data.ClassPathToSchemaName, Group, ClassPath) + TEXT("Dynamic") + FString::FromInt(i);

			Writer.Printf("component {0} {", *ComponentName);
			Writer.Indent();
			Writer.Printf("id = {0};", ComponentId);
			Writer.Printf("data {0};", *SchemaReplicatedDataName(Data.ClassPathToSchemaName, Group, ClassPath));
			Writer.Outdent().Print("}");

			AddComponentId(ComponentId, DynamicSubobjectComponents.SchemaComponents, PropertyGroupToSchemaComponentType(Group));
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
			FString ComponentName = SchemaHandoverDataName(Data.ClassPathToSchemaName, ClassPath) + TEXT("Dynamic") + FString::FromInt(i);

			Writer.Printf("component {0} {", *ComponentName);
			Writer.Indent();
			Writer.Printf("id = {0};", ComponentId);
			Writer.Printf("data {0};", *SchemaHandoverDataName(Data.ClassPathToSchemaName, ClassPath));
			Writer.Outdent().Print("}");

			AddComponentId(ComponentId, DynamicSubobjectComponents.SchemaComponents, ESchemaComponentType::SCHEMA_Handover);
		}

		SubobjectSchemaData.DynamicSubobjectComponents.Add(MoveTemp(DynamicSubobjectComponents));
	}

	SubobjectSchemaData.GeneratedSchemaName = Data.ClassPathToSchemaName[Class.ClassPath];
	Data.SubobjectClassPathToSchema.Add(Class.ClassPath, SubobjectSchemaData);
}

void SchemaGenerator::GenerateActorSchema(FComponentIdGenerator& IdGenerator, const FUnrealClassDesc& Class,
										  TSharedPtr<FUnrealOfflineType> TypeInfo, FString SchemaPath)
{
	FCodeWriter Writer;
	FCodeWriter SubobjectsWriter;

	GenerateActorSchema(IdGenerator, Class, TypeInfo, Writer, SubobjectsWriter);

	Writer.WriteToFile(FString::Printf(TEXT("%s%s.schema"), *SchemaPath, *Data.ClassPathToSchemaName[Class.ClassPath]));
	if (!SubobjectsWriter.GetOutput().IsEmpty())
	{
		SubobjectsWriter.WriteToFile(
			FString::Printf(TEXT("%s%sComponents.schema"), *SchemaPath, *Data.ClassPathToSchemaName[Class.ClassPath]));
	}
}

void SchemaGenerator::GenerateActorSchema(FComponentIdGenerator& IdGenerator, const FUnrealClassDesc& Class,
										  TSharedPtr<FUnrealOfflineType> TypeInfo, FCodeWriter& Writer, FCodeWriter& SubobjectsWriter)
{
	const FActorSchemaData* const SchemaData = Data.ActorClassPathToSchema.Find(Class.ClassPath);
	const FString& ClassPath = Class.ClassPath;

	Writer.Printf(R"""(
		// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
		// Note that this file has been generated automatically
		package unreal.generated.{0};)""",
				  *Data.ClassPathToSchemaName[ClassPath].ToLower());

	// Will always be included since AActor has replicated pointers to other actors
	Writer.PrintNewLine();
	Writer.Printf("import \"unreal/gdk/core_types.schema\";");

	FActorSchemaData ActorSchemaData;
	ActorSchemaData.GeneratedSchemaName = Data.ClassPathToSchemaName[ClassPath];

	FUnrealFlatRepData RepData = GetFlatRepData(TypeInfo);

	// Client-server replicated properties.
	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		if (RepData[Group].Num() == 0)
		{
			continue;
		}

		// If this class is an Actor, it MUST have bTearOff at field ID 3.
		if (Group == REP_MultiClient && Class.bIsActor)
		{
			TSharedPtr<FUnrealOfflineProperty> ExpectedReplicatesPropData = RepData[Group].FindRef(SpatialConstants::ACTOR_TEAROFF_ID);
			// const GDK_PROPERTY(Property)* ReplicatesProp = AActor::StaticClass()->FindPropertyByName("bTearOff");

			if (!(ExpectedReplicatesPropData.IsValid() && ExpectedReplicatesPropData->PropertyName == TEXT("bTearOff")))
			{
				UE_LOG(LogSchemaGenerator, Error,
					   TEXT("Did not find Actor->bTearOff at field %d for class %s. Modifying the base Actor class is currently not "
							"supported."),
					   SpatialConstants::ACTOR_TEAROFF_ID, *Class.ClassName);
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

		FString ComponentName = SchemaReplicatedDataName(Data.ClassPathToSchemaName, Group, ClassPath);
		Writer.Printf("component {0} {", *ComponentName);
		Writer.Indent();
		Writer.Printf("id = {0};", ComponentId);

		AddComponentId(ComponentId, ActorSchemaData.SchemaComponents, PropertyGroupToSchemaComponentType(Group));

		int FieldCounter = 0;
		for (auto& RepProp : RepData[Group])
		{
			FieldCounter++;
			WriteSchemaRepField(Writer, RepProp.Value, RepProp.Value->ReplicationData->Handle);
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
		FString ComponentName = SchemaHandoverDataName(Data.ClassPathToSchemaName, ClassPath);
		Writer.Printf("component {0} {", *ComponentName);
		Writer.Indent();
		Writer.Printf("id = {0};", ComponentId);

		AddComponentId(ComponentId, ActorSchemaData.SchemaComponents, ESchemaComponentType::SCHEMA_Handover);

		int FieldCounter = 0;
		for (auto& Prop : HandoverData)
		{
			FieldCounter++;
			WriteSchemaHandoverField(Writer, Prop.Value, FieldCounter);
		}
		Writer.Outdent().Print("}");
	}

	GenerateSubobjectSchemaForActor(IdGenerator, ClassPath, TypeInfo, SubobjectsWriter, ActorSchemaData,
									Data.ActorClassPathToSchema.Find(ClassPath));

	Data.ActorClassPathToSchema.Add(ClassPath, ActorSchemaData);

	// Cache the NCD for this Actor
	if (Class.bIsActor)
	{
		const float NCD = Class.ClassNCD;
		if (Data.NetCullDistanceToComponentId.Find(NCD) == nullptr)
		{
			if (FMath::FloorToFloat(NCD) != NCD)
			{
				UE_LOG(LogSchemaGenerator, Warning,
					   TEXT("Fractional Net Cull Distance values are not supported and may result in incorrect behaviour. "
							"Please modify class's (%s) Net Cull Distance Squared value (%f)"),
					   *Class.ClassName, NCD);
			}

			Data.NetCullDistanceToComponentId.Add(NCD, 0);
		}
	}
}

void SchemaGenerator::GenerateRPCEndpointsSchema(FString SchemaPath)
{
	FCodeWriter Writer;

	Writer.Print(R"""(
		// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
		// Note that this file has been generated automatically
		package unreal.generated;)""");
	Writer.PrintNewLine();
	Writer.Print("import \"unreal/gdk/core_types.schema\";");
	Writer.Print("import \"unreal/gdk/rpc_payload.schema\";");

	GenerateRPCEndpoint(Writer, TEXT("ClientEndpoint"), SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID,
						{ ERPCType::ServerReliable, ERPCType::ServerUnreliable }, { ERPCType::ClientReliable, ERPCType::ClientUnreliable });
	GenerateRPCEndpoint(Writer, TEXT("ServerEndpoint"), SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID,
						{ ERPCType::ClientReliable, ERPCType::ClientUnreliable }, { ERPCType::ServerReliable, ERPCType::ServerUnreliable });
	GenerateRPCEndpoint(Writer, TEXT("MulticastRPCs"), SpatialConstants::MULTICAST_RPCS_COMPONENT_ID, { ERPCType::NetMulticast }, {});

	Writer.WriteToFile(FString::Printf(TEXT("%srpc_endpoints.schema"), *SchemaPath));
}

// Add the component ID to the passed schema components array and the set of components of that type.
void SchemaGenerator::AddComponentId(const Worker_ComponentId ComponentId, ComponentIdPerType& SchemaComponents,
									 const ESchemaComponentType ComponentType)
{
	SchemaComponents[ComponentType] = ComponentId;
	Data.SchemaComponentTypeToComponents[ComponentType].Add(ComponentId);
}
